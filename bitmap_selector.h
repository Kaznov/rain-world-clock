#include "config.h"
#include "slugcats_bitmaps.h"

#include <time.h>

// This struct is only used for selecting special day bitmaps 
struct DayOfYear {
    unsigned short day;
    constexpr DayOfYear(unsigned short month, unsigned short day_of_month)
        : day{ month * 32 + day_of_month} { }
    constexpr DayOfYear(const struct tm& time)
        : DayOfYear{ time.tm_mon + 1, time.tm_mday } { }
    bool operator==(const DayOfYear&) const = default;
};

const unsigned char* bitmaps_weekdays_light[] {
    survivor_light,
    monk_light,
    hunter_light,
    gourmand_light,
    artificer_light,
    spearmaster_light,
    rivulet_light,
};

const unsigned char* bitmaps_weekdays_sleep_light[] {
    sleep_survivor_light,
    sleep_monk_light,
    sleep_hunter_light,
    sleep_gourmand_light,
    sleep_artificer_light,
    sleep_spearmaster_light,
    sleep_rivulet_light,
};

const unsigned char* bitmaps_special_light[] {
    anniversary_light,
    bday_light,
    christmas_light,
    halloween_light,
    inv_light,
    valentine1_light,
    valentine2_light,
    valentine3_light,
    sleep_together_light
};

const unsigned char* bitmaps_weekdays_dark[] {
    survivor_dark,
    monk_dark,
    hunter_dark,
    gourmand_dark,
    artificer_dark,
    spearmaster_dark,
    rivulet_dark,
};

const unsigned char* bitmaps_weekdays_sleep_dark[] {
    sleep_survivor_dark,
    sleep_monk_dark,
    sleep_hunter_dark,
    sleep_gourmand_dark,
    sleep_artificer_dark,
    sleep_spearmaster_dark,
    sleep_rivulet_dark,
};

const unsigned char* bitmaps_special_dark[] {
    anniversary_dark,
    bday_dark,
    christmas_dark,
    halloween_dark,
    inv_dark,
    valentine1_dark,
    valentine2_dark,
    valentine3_dark,
    sleep_together_dark
};

const unsigned char* getBackgroundImage(struct tm now) {
    const unsigned char** const bitmaps_weekday
        = display_mode == DisplayMode::Light
        ? bitmaps_weekdays_light
        : bitmaps_weekdays_dark;

    const unsigned char** const bitmaps_sleep
        = display_mode == DisplayMode::Light
        ? bitmaps_weekdays_sleep_light
        : bitmaps_weekdays_sleep_dark;

    const unsigned char** const bitmaps_special
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
