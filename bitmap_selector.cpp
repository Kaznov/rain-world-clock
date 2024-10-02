#include "bitmap_selector.hpp"
#include "date_utils.hpp"

#include <ArduinoJson.h>

static const char* const bitmaps_weekdays_day_light[] {
    "survivor_light.bmp",
    "monk_light.bmp",
    "hunter_light.bmp",
    "gourmand_light.bmp",
    "artificer_light.bmp",
    "spearmaster_light.bmp",
    "rivulet_light.bmp",
};

static const char* const bitmaps_weekdays_night_light[] {
    "sleep_survivor_light.bmp",
    "sleep_monk_light.bmp",
    "sleep_hunter_light.bmp",
    "sleep_gourmand_light.bmp",
    "sleep_artificer_light.bmp",
    "sleep_spearmaster_light.bmp",
    "sleep_rivulet_light.bmp",
};

static const char* const bitmaps_weekdays_day_dark[] {
    "survivor_dark.bmp",
    "monk_dark.bmp",
    "hunter_dark.bmp",
    "gourmand_dark.bmp",
    "artificer_dark.bmp",
    "spearmaster_dark.bmp",
    "rivulet_dark.bmp",
};

static const char* const bitmaps_weekdays_night_dark[] {
    "sleep_survivor_dark.bmp",
    "sleep_monk_dark.bmp",
    "sleep_hunter_dark.bmp",
    "sleep_gourmand_dark.bmp",
    "sleep_artificer_dark.bmp",
    "sleep_spearmaster_dark.bmp",
    "sleep_rivulet_dark.bmp",
};

static const char* const regular_pictures_directory = "pictures_regular/";
static const char* const special_pictures_directory = "pictures_special/";

static char memory_pool[1024];
static int memory_pool_position = 0;

struct SpecialEvent {
    DayOfYear doy;
    const char* bitmap_light = nullptr;
    const char* bitmap_dark = nullptr;
};

static std::vector<SpecialEvent> special_days;
static std::vector<SpecialEvent> special_nights;

static char path_buffer[64];

static const char* getSpecialBackgroundImageName(DayOfYear doy, bool is_night) {
    auto is_today = [&](const SpecialEvent& e){ return e.doy == doy; };

    const auto& special_events = is_night ? special_nights : special_days;
    DisplayMode mode = is_night ? config.night_mode : config.day_mode;

    auto it = std::find_if(special_events.begin(), special_events.end(),
                            is_today);
    if (it != special_events.end()) {
        return mode == DisplayMode::Light
            ? it->bitmap_light
            : it->bitmap_dark;
    }

    return nullptr;
}

static const char* getBackgroundImagePath(struct tm now) {
    int minutes_into_day = now.tm_hour * 60 + now.tm_min;

    bool is_night = minutes_into_day < config.wakeup_time || minutes_into_day >= config.sleep_time;
    if (minutes_into_day < config.wakeup_time) {
        // the slugcat from the previous day is still sleeping, adjust date to yesterday
        now.tm_mday -= 1;
        mktime(&now); // normalize the date
    }

    int weekday = (now.tm_wday + 6) % 7; // monday == 0
    DayOfYear doy = DayOfYear{now};
    const char* bitmap_name;
    const char* directory;

    const char* special_bitmap;
    if (special_bitmap = getSpecialBackgroundImageName(doy, is_night)) {
        directory = special_pictures_directory;
        bitmap_name = special_bitmap;
    } else {
        directory = regular_pictures_directory;
        bitmap_name = is_night
            ? (config.night_mode == DisplayMode::Light
                ? bitmaps_weekdays_night_light
                : bitmaps_weekdays_night_dark)[weekday]
            : (config.day_mode == DisplayMode::Light
                ? bitmaps_weekdays_day_light
                : bitmaps_weekdays_day_dark)[weekday];
    }

    strlcpy(path_buffer, directory, sizeof(path_buffer));
    strlcat(path_buffer, bitmap_name, sizeof(path_buffer));
    return path_buffer;
}

std::optional<BitmapFile> getBackgroundImage(struct tm now) {
    const char* bitmap_name = getBackgroundImagePath(now);
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

static const char* loadNameToPool(const char* name) {
    if (name == nullptr) {
        return nullptr;
    }

    // TODO How to handle / report these errors?
    int len = strlen(name);
    if (memory_pool_position + len + 1 >= sizeof(memory_pool)) {
        return nullptr;
    }

    if (len >= LFS_NAME_MAX) {
        return nullptr;
    }

    memcpy(memory_pool + memory_pool_position, name, len + 1);

    const char* result = memory_pool + memory_pool_position;
    memory_pool_position += len + 1;
    return result;
}

static void readSpecialBitmapsFile(const char* file, std::vector<SpecialEvent>& out) {
    File special_events_file = LittleFS.open(file, "r");

    if (!special_events_file) {
        Serial.printf_P(PSTR("Can't open special bitmaps file\n"));
        return;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, special_events_file);
    special_events_file.close();

    if (error) {
        Serial.printf_P(PSTR("Can't load special bitmaps JSON\n"));
        return;
    }

    const JsonObjectConst event_map = doc.as<JsonObjectConst>();
    out.reserve(event_map.size());

    for (const auto& event : event_map) {
        int day = 0, month = 0;
        sscanf(event.key().c_str(), "%2d/%2d", &day, &month);
        if (day < 1 || day > 31 || month < 1 || month > 12) {
            // Either parsing failed or a non-logical value
            Serial.printf_P(PSTR("Can't parse event date: %s\n"), event.key().c_str());
            continue;
        }

        out.push_back(SpecialEvent{
            .doy = DayOfYear(month, day),
            .bitmap_light = loadNameToPool(event.value()["light"].as<const char*>()),
            .bitmap_dark = loadNameToPool(event.value()["dark"].as<const char*>())
        });

        Serial.printf_P(PSTR("Loaded special event: %d/%d: (%s, %s)\n"),
            day, month,
            out.back().bitmap_light ?: "",
            out.back().bitmap_dark ?: "");
    }
}

void readSpecialBitmapsConfig() {
    readSpecialBitmapsFile("special_days.json", special_days);
    readSpecialBitmapsFile("special_nights.json", special_nights);
}
