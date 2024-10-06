#ifndef RWCLOCK_BITMAP_SELECTOR_HPP_
#define RWCLOCK_BITMAP_SELECTOR_HPP_

#include "config.hpp"
#include "display.hpp"

#include <ctime>
#include <optional>
#include <LittleFS.h>

struct BitmapFile {
    File f;
    uint32_t width;
    uint32_t height;
    uint32_t data_offset;
};

std::optional<BitmapFile> loadBitmap(const char* path);
std::optional<BitmapFile> getBackgroundImage(struct tm now);
void readSpecialBitmapsConfig();

#endif  // RWCLOCK_BITMAP_SELECTOR_HPP_
