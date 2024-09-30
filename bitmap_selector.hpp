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

const char* getBackgroundImageName(struct tm now);
std::optional<BitmapFile> getBackgroundImage(struct tm now);

#endif  // RWCLOCK_BITMAP_SELECTOR_HPP_
