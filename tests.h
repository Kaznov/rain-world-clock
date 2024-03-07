#ifndef TESTS_H
#define TESTS_H

#include "config.h"
#include "display.h"

const unsigned char* bitmaps_light[] {
    survivor_light,
    monk_light,
    hunter_light,
    gourmand_light,
    artificer_light,
    rivulet_light,
    spearmaster_light,
    sleep_survivor_light,
    sleep_monk_light,
    sleep_hunter_light,
    sleep_gourmand_light,
    sleep_artificer_light,
    sleep_rivulet_light,
    sleep_spearmaster_light,
    anniversary_light,
    bday_light,
    christmas_light,
    halloween_light,
    inv_light,
    valentine1_light,
    valentine2_light,
    valentine3_light,
    sleep_together_light,
};

const unsigned char* bitmaps_dark[] {
    survivor_dark,
    monk_dark,
    hunter_dark,
    gourmand_dark,
    artificer_dark,
    rivulet_dark,
    spearmaster_dark,
    sleep_survivor_dark,
    sleep_monk_dark,
    sleep_hunter_dark,
    sleep_gourmand_dark,
    sleep_artificer_dark,
    sleep_rivulet_dark,
    sleep_spearmaster_dark,
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

void test_all_bitmaps() {
    displayMode = DisplayMode::Dark;
    for (auto bitmap : bitmaps_dark) {
        drawDisplay(bitmap);
        delay(5000);
    }

    displayMode = DisplayMode::Light;
    for (auto bitmap : bitmaps_light) {
        drawDisplay(bitmap);
        delay(5000);
    }
}

#endif
