#include "config.h"

#include <time.h>
#include <LittleFS.h>

#include <optional>

// This struct is only used for selecting special day bitmaps
struct DayOfYear {
    unsigned short day;
    constexpr DayOfYear(unsigned short month, unsigned short day_of_month)
        : day{ month * 32 + day_of_month} { }
    constexpr DayOfYear(const struct tm& time)
        : DayOfYear{ time.tm_mon + 1, time.tm_mday } { }
    bool operator==(const DayOfYear& other) const { return day == other.day; };
};

const char* bitmaps_weekdays_light[] {
    "pictures_day/survivor_light.bmp",
    "pictures_day/monk_light.bmp",
    "pictures_day/hunter_light.bmp",
    "pictures_day/gourmand_light.bmp",
    "pictures_day/artificer_light.bmp",
    "pictures_day/spearmaster_light.bmp",
    "pictures_day/rivulet_light.bmp",
};

const char* bitmaps_weekdays_sleep_light[] {
    "pictures_night/sleep_survivor_light.bmp",
    "pictures_night/sleep_monk_light.bmp",
    "pictures_night/sleep_hunter_light.bmp",
    "pictures_night/sleep_gourmand_light.bmp",
    "pictures_night/sleep_artificer_light.bmp",
    "pictures_night/sleep_spearmaster_light.bmp",
    "pictures_night/sleep_rivulet_light.bmp",
};

const char* bitmaps_weekdays_dark[] {
    "pictures_day/survivor_dark.bmp",
    "pictures_day/monk_dark.bmp",
    "pictures_day/hunter_dark.bmp",
    "pictures_day/gourmand_dark.bmp",
    "pictures_day/artificer_dark.bmp",
    "pictures_day/spearmaster_dark.bmp",
    "pictures_day/rivulet_dark.bmp",
};

const char* bitmaps_weekdays_sleep_dark[] {
    "pictures_night/sleep_survivor_dark.bmp",
    "pictures_night/sleep_monk_dark.bmp",
    "pictures_night/sleep_hunter_dark.bmp",
    "pictures_night/sleep_gourmanddark.bmp",
    "pictures_night/sleep_artificedark.bmp",
    "pictures_night/sleep_spearmasdark.bmp",
    "pictures_night/sleep_rivulet_dark.bmp",
};

const char* getBackgroundImageName(struct tm now) {
    const char** const bitmaps_weekday
        = display_mode == DisplayMode::Light
        ? bitmaps_weekdays_light
        : bitmaps_weekdays_dark;

    const char** const bitmaps_sleep
        = display_mode == DisplayMode::Light
        ? bitmaps_weekdays_sleep_light
        : bitmaps_weekdays_sleep_dark;

    int minutes_into_day = now.tm_hour * 60 + now.tm_min;

    bool is_sleep = minutes_into_day < config.wakeup_time || minutes_into_day >= config.sleep_time;
    if (minutes_into_day < config.wakeup_time) {
        // the slugcat from the previous day is still sleeping, adjust date to yesterday
        now.tm_mday -= 1;
        mktime(&now); // normalize the date
    }

    int weekday = (now.tm_wday + 6) % 7; // monday == 0
    if (is_sleep) {
        return bitmaps_sleep[weekday];
    }

    return bitmaps_weekday[weekday];
}

struct BitmapFile {
    File f;
    uint32_t width;
    uint32_t height;
    uint32_t data_offset;
};

std::optional<BitmapFile> getBackgroundImage(struct tm now) {
    const char* bitmap_name = getBackgroundImageName(now);
    Serial.printf_P(PSTR("Loading Background bitmap: %s\n"), bitmap_name);
    File bitmap = LittleFS.open(bitmap_name, "r");

    bitmap.seek(10);
    uint32_t data_offset = 0;
    bitmap.readBytes((char*)&data_offset, 4);
    bitmap.seek(data_offset);

    bitmap.seek(18);
    uint32_t width = 0;
    uint32_t height = 0;
    bitmap.readBytes((char*)&width, 4);
    bitmap.readBytes((char*)&height, 4);

    bitmap.seek(28);
    uint16_t bits_per_pixel = 0;
    bitmap.readBytes((char*)&bits_per_pixel, 2);

    // TODO: check width, height, bpp, size of the file, alignemt of width to 16
    // Have meaningful error messages
    return { BitmapFile{.f = std::move(bitmap), .width = width, .height = height, .data_offset = data_offset} };
}
