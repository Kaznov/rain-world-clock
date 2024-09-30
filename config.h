#ifndef CONFIG_H
#define CONFIG_H

#include <wl_definitions.h>

const char day_api[] PROGMEM = "http://d1ox7ijvdplfv4.cloudfront.net";
char day_query[80] = "";

constexpr short HEIGHT = 480;
constexpr short WIDTH = 800;

constexpr int MAX_PICTURE_WIDTH = 320;
constexpr int MAX_PICTURE_HEIGHT = 320;

constexpr short CLOCK_WIDTH = HEIGHT;
constexpr short CLOCK_HEIGHT = HEIGHT;
constexpr short CLOCK_X0 = WIDTH / 2;
constexpr short CLOCK_Y0 = HEIGHT / 2;

constexpr short CLOCK_R_HOURS = 210;
constexpr short CLOCK_R_MINUTES = 170;

constexpr short HOURS_CIRCLE_OUTER_R = 29;
constexpr short HOURS_CIRCLE_INNER_R = 25;
constexpr short MINUTES_CIRCLE_OUTER_R = 7;
constexpr short MINUTES_CIRCLE_INNER_R = 3;

constexpr short DC_PIN = 4;     // D2
constexpr short RST_PIN = 2;    // D4
constexpr short BUSY_PIN = 5;   // D1

enum class DisplayMode {
    Light = 0,
    Dark = 1
} display_mode;

struct UserConfiguration {
    char wifi_ssid[WL_SSID_MAX_LENGTH + 1];
    char wifi_password[WL_WPA_KEY_MAX_LENGTH + 1];

    char location[32];
    char timezone[32];

    int wakeup_time = 7*60;    // slugcat wakes up
    int sleep_time = 22*60;    // slugcat goes to sleep

    DisplayMode day_mode = DisplayMode::Light;
    DisplayMode night_mode = DisplayMode::Light;
} config;

struct DayData {
    time_t timestamp;
    bool is_day;
    short temp_now;
    short weather_now;
    short temp_today;
    short weather_today;
    short wind_today;
    short temp_tonight;
    short weather_tonight;
    short wind_tonight;
    short temp_tomorrow;
    short weather_tomorrow;
    short wind_tomorrow;
    short sunrise;
    short sunset;
    short moonset;
    short moonrise;
    short moon_illumination;
} day_data;

#endif
