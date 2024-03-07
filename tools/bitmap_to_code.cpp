#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "bitmap_loader.hpp"

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./bitmap_to_code path out_file" << std::endl;
        return EXIT_FAILURE;
    }

    namespace fs = std::filesystem;

    std::ofstream out{argv[2]};
    out << std::hex;

    for (const fs::directory_entry& dir_entry: fs::directory_iterator{argv[1]}) {
        const auto path = dir_entry.path();
        if (path.extension() != ".bmp") {
            continue;
        }
        const auto name = path.stem();
        
        const auto load_result = loadBitmap(path.c_str());

        if (!load_result) {
            std::cout << "Cannot load file: " << path << ", "
                      << load_result.error() << std::endl;
            return EXIT_FAILURE;
        }

        const auto bmp = load_result.value();
        const auto scanline_width = bitmapScanlineWidth(bmp.width, bmp.bits_per_pixel);
        if (bmp.height * scanline_width != bmp.data.size()) {
            std::cout << "Wrong bitmap width: " << bmp.width << std::endl;
            return EXIT_FAILURE;
        }

        if (bmp.bits_per_pixel != 1) {
            std::cout << "Wrong bits per pixel: " << bmp.bits_per_pixel << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<unsigned char> buffer(bmp.width * bmp.height * bmp.bits_per_pixel / 8);
        
        // copy the lines in reverse order to the buffer
        for (std::size_t row = 0; row < bmp.height; ++row) {
            std::size_t offset = row * scanline_width;
            std::size_t dest_offset = (bmp.height - 1 - row) * scanline_width;
            std::copy_n(bmp.data.begin() + offset, scanline_width,
                        buffer.begin() + dest_offset);
        }

        out << "const unsigned char " << name.c_str() << "[] PROGMEM {";
        for (std::size_t idx = 0; unsigned char b : buffer) {
            if (idx % 16 == 0) {
                out << "\n    ";
            }

            out << "0x" << std::setw(2) << std::setfill('0') << std::right << unsigned{b} << ", ";
            ++idx;
        }

        out << "\n};\n\n";
    }
}

