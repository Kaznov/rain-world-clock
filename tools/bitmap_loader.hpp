#ifndef BITMAP_OADER_HPP
#define BITMAP_LOADER_HPP

#include <expected>
#include <vector>

inline constexpr size_t bitmapColorMapSize(size_t bpp) {
    // no map if pixels are saved fully, otherwise it's bpp^^2 RGB triplets
    return bpp == 24 ? 0 : (1 << bpp) * 3;
}

inline constexpr size_t bitmapScanlineWidth(size_t width, size_t bpp) {
    // scanlines of bitpacked bitmaps have to be aligned to 4-byte boundaries
    return (width * bpp + 32 - 1) / 32 * 4;
}

inline constexpr size_t bitmapDataSize(size_t width, size_t height, size_t bpp) {
    return height * bitmapScanlineWidth(width, bpp);
}


struct BitmapData {
    std::vector<unsigned char> data;
    std::vector<unsigned char> color_map;
    size_t width;
    size_t height;
    size_t bits_per_pixel;

    BitmapData(size_t width_, size_t height_, size_t bpp)
        : data(bitmapDataSize(width_, height_, bpp))
        , color_map(bitmapColorMapSize(bpp))
        , width{width_}
        , height{height_}
        , bits_per_pixel{bpp} { }
};

std::expected<BitmapData, const char*> loadBitmap(const char* filename);
std::expected<void, const char*> saveBitmap(const char* filename, const BitmapData& bitmap);


#endif // BIRMAL_LOADER_HPP
