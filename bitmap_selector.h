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

const char* bitmaps_special_light[] {
    "pictures_special_day/anniversary_light.bmp",
    "pictures_special_day/bday_light.bmp",
    "pictures_special_day/christmas_light.bmp",
    "pictures_special_day/halloween_light.bmp",
    "pictures_special_day/inv_light.bmp",
    "pictures_special_day/valentine1_light.bmp",
    "pictures_special_day/valentine2_light.bmp",
    "pictures_special_day/valentine3_light.bmp",
    "pictures_special_day/sleep_together_light.bmp"
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

const char* bitmaps_special_dark[] {
    "pictures_special_day/anniversary_dark.bmp",
    "pictures_special_day/bday_dark.bmp",
    "pictures_special_day/christmas_dark.bmp",
    "pictures_special_day/halloween_dark.bmp",
    "pictures_special_day/inv_dark.bmp",
    "pictures_special_day/valentine1_dark.bmp",
    "pictures_special_day/valentine2_dark.bmp",
    "pictures_special_day/valentine3_dark.bmp",
    "pictures_special_day/sleep_togetherdarkt.bmp"
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

    const char** const bitmaps_special
        = display_mode == DisplayMode::Light
        ? bitmaps_special_light
        : bitmaps_special_dark;

    int minutes_into_day = now.tm_hour * 60 + now.tm_min;

    bool is_sleep = minutes_into_day < DAY_TIME || minutes_into_day >= NIGHT_TIME;
    if (minutes_into_day < DAY_TIME) {
        // the slugcat from the previous day is still sleeping, adjust date to yesterday
        now.tm_mday -= 1;
        mktime(&now); // normalize the date
    }

    int weekday = (now.tm_wday + 6) % 7; // monday == 0
    if (is_sleep) {
        return bitmaps_sleep[weekday];
    }

    DayOfYear day_of_year = DayOfYear(now);

    static constexpr DayOfYear special_days[] {
        DayOfYear(3, 28),   // rain world anniversary
        DayOfYear(bday_month, bday_day_of_month), // birthday
        DayOfYear(12, 25),  // christmas
        DayOfYear(10, 31),  // halloween
        DayOfYear(4, 1),    // prima aprilis, inv
        DayOfYear(3, 15),   // sleep day
        DayOfYear(2, 14)    // valentine
    };

    for (std::size_t i = 0; i < array_size(special_days); ++i) {
        if (day_of_year == special_days[i]) {
            return bitmaps_special[i];
        }
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
