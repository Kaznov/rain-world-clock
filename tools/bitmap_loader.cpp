#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <expected>
#include <vector>

#include "bitmap_loader.hpp"

using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;

// The fields in the structure are not packed! Exposition only!
// struct BitmapFileHeader {
//   i16 bfType;
//   u32 bfSize;
//   i16 bfReserved1;
//   i16 bfReserved2;
//   u32 bfOffBits;
// };
static constexpr size_t FILE_HEADER_SIZE = 14;

struct BitmapCoreInfoHeader {
    u32 size;
    u16 width;
    u16 height;
    u16 planes;
    u16 bitCount;
};
static_assert(sizeof(BitmapCoreInfoHeader) == 12);

struct BitmapInfoHeader {
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitCount;
    u32 compression;
    u32 sizeImage;
    i32 xPelsPerMeter;
    i32 yPelsPerMeter;
    u32 clrUsed;
    u32 clrImportant;
};
static_assert(sizeof(BitmapInfoHeader) == 40);

struct BitmapInfoHeaderV4 {
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitCount;
    u32 compression;
    u32 sizeImage;
    i32 xPelsPerMeter;
    i32 yPelsPerMeter;
    u32 clrUsed;
    u32 clrImportant;
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
    u32 alphaMask;
    u32 CSType;
    u32 endpoints[9];
    u32 gammaRed;
    u32 gammaGreen;
    u32 gammaBlue;
};
static_assert(sizeof(BitmapInfoHeaderV4) == 108);

struct BitmapInfoHeaderV5 {
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitCount;
    u32 compression;
    u32 sizeImage;
    i32 xPelsPerMeter;
    i32 yPelsPerMeter;
    u32 clrUsed;
    u32 clrImportant;
    u32 redMask;
    u32 greenMask;
    u32 blueMask;
    u32 alphaMask;
    u32 CSType;
    u32 endpoints[9];
    u32 gammaRed;
    u32 gammaGreen;
    u32 gammaBlue;
    u32 intent;
    u32 profileData;
    u32 profileSize;
    u32 reserved;
};
static_assert(sizeof(BitmapInfoHeaderV5) == 124);

struct BitmapCoreInfo {
    u16 width;
    u16 height;
    u16 bits_per_pixel;
};

template <typename T>
T from_bytes(void* data) {
    unsigned char buffer[sizeof(T)];
    memcpy(buffer, data, sizeof(T));
    return std::bit_cast<T>(buffer);
}

template <typename Header>
std::expected<BitmapCoreInfo, const char*> readBitmapInfoHeader(FILE* f) {
    Header info_header;
    unsigned char info_header_bytes[sizeof(info_header)]{};
    if (fread(info_header_bytes + sizeof(u32), sizeof(info_header) - sizeof(u32), 1, f) != 1) {
        return std::unexpected{"Cannot read bitmap info header"};
    }
    memcpy(&info_header, info_header_bytes, sizeof(info_header));

    if ((info_header.width < 0) || (info_header.width > UINT16_MAX) ||
        (info_header.height < 0) || (info_header.height > UINT16_MAX)) {
        return std::unexpected{"Incorrect bitmap size in the header, bitmap too big?"};
    }

    if constexpr (!std::is_same_v<Header, BitmapCoreInfoHeader>) {
        if (info_header.compression != 0) {
            return std::unexpected{"This program doesn't accept compressed bitmaps"};
        }
    }

    return BitmapCoreInfo {
        .width = static_cast<u16>(info_header.width),
        .height = static_cast<u16>(info_header.height),
        .bits_per_pixel = info_header.bitCount
    };
};

struct FileGuard {
    FILE* f;
    ~FileGuard() { fclose(f); }
};

std::expected<BitmapData, const char*> loadBitmap(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f == nullptr) {
        return std::unexpected{"Cannot open the file for reading"};
    }

    // closes file on function exit
    FileGuard guard{f};

    unsigned char header_bytes[FILE_HEADER_SIZE + sizeof(u32)];
    if (fread(header_bytes, sizeof(header_bytes), 1, f) != 1) {
        return std::unexpected{"The file is too short for a bitmap file"};
    }
    if (header_bytes[0] != 'B' || header_bytes[1] != 'M') {
        return std::unexpected{"The file is not a bitmap file"};
    }

    // offset of pixel data
    const u32 data_offset = from_bytes<u32>(header_bytes + 10);
    // size of the info header, identifying its type
    const u32 header_size = from_bytes<u32>(header_bytes + 14);

    std::expected<BitmapCoreInfo, const char*> read_info_header_result;

    if (header_size == sizeof(BitmapCoreInfoHeader)) {
        read_info_header_result = readBitmapInfoHeader<BitmapCoreInfoHeader>(f);
    }
    else if (header_size == sizeof(BitmapInfoHeader)) {
        read_info_header_result = readBitmapInfoHeader<BitmapInfoHeader>(f);
    }
    else if (header_size == sizeof(BitmapInfoHeaderV4)) {
        read_info_header_result = readBitmapInfoHeader<BitmapInfoHeaderV4>(f);
    }
    else if (header_size == sizeof(BitmapInfoHeaderV5)) {
        read_info_header_result = readBitmapInfoHeader<BitmapInfoHeaderV5>(f);
    } else {
        return std::unexpected{"Unknown info header type"};
    }

    if (!read_info_header_result.has_value()) {
        return std::unexpected{read_info_header_result.error()};
    }

    const auto info = read_info_header_result.value();

    if (info.bits_per_pixel != 1 &&
        info.bits_per_pixel != 4 &&
        info.bits_per_pixel != 8 &&
        info.bits_per_pixel != 24) {
        return std::unexpected{"The number of bits per pixel is not supported"};
    }

    BitmapData result(info.width, info.height, info.bits_per_pixel);

    // If the representation is not the standard triplet, we need to read the color map
    const size_t color_map_size = result.color_map.size();
    if (fread(result.color_map.data(), 1, color_map_size, f) != color_map_size) {
        return std::unexpected{"Cannot read the color map"};
    }

    const size_t data_size = result.data.size();
    fseek(f, data_offset, SEEK_SET);
    if (fread(result.data.data(), 1, data_size, f) != data_size) {
        return std::unexpected{"Cannot read pixel data"};
    }

    return result;
}

std::expected<void, const char*> saveBitmap(const char* filename, const BitmapData& bitmap) {
    const size_t file_size_z = FILE_HEADER_SIZE
                                + sizeof(BitmapCoreInfoHeader)
                                + bitmap.color_map.size()
                                + bitmap.data.size();
    if (file_size_z > UINT32_MAX) {
        return std::unexpected{"Bitmap is too big"};
    }
    const u32 file_size = static_cast<u32>(file_size_z);

    if (bitmap.height > UINT16_MAX || bitmap.width > UINT16_MAX) {
        return std::unexpected{"Bitmap dimentions are too big"};
    }
    const u16 height = static_cast<u16>(bitmap.height);
    const u16 width = static_cast<u16>(bitmap.width);

    FILE* f = fopen(filename, "wb");
    if (f == nullptr) {
        return std::unexpected{"Cannot open the file for writing"};
    }

    // closes file on function exit
    FileGuard guard{f};

    // Write file header (see struct BitmapFileHeader on top of the file )
    fwrite("BM", 2, 1, f);
    fwrite(&file_size, sizeof(u32), 1, f);
    const u32 reserved_zero = 0;
    fwrite(&reserved_zero, sizeof(u32), 1, f);
    const u32 data_offset = static_cast<u32>(FILE_HEADER_SIZE
                                                + sizeof(BitmapCoreInfoHeader)
                                                + bitmap.color_map.size());
    fwrite(&data_offset, sizeof(u32), 1, f);

    const BitmapCoreInfoHeader info_header {
        .size = sizeof(BitmapCoreInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bitCount = static_cast<u16>(bitmap.bits_per_pixel)
    };

    fwrite(&info_header, sizeof(info_header), 1, f);
    fwrite(bitmap.color_map.data(), bitmap.color_map.size(), 1, f);
    fwrite(bitmap.data.data(), bitmap.data.size(), 1, f);

    if (ferror(f)) {
        return std::unexpected{"Writing to bitmap file failed"};
    }

    return {};
}
