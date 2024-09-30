#ifndef RWCLOCK_CONFIG_HPP_
#define RWCLOCK_CONFIG_HPP_

#include <wl_definitions.h>

constexpr const char day_api[] PROGMEM = "http://d1ox7ijvdplfv4.cloudfront.net";

constexpr int HEIGHT = 480;
constexpr int WIDTH = 800;

constexpr int MAX_PICTURE_WIDTH = 320;
constexpr int MAX_PICTURE_HEIGHT = 320;

constexpr int CLOCK_WIDTH = HEIGHT;
constexpr int CLOCK_HEIGHT = HEIGHT;
constexpr int CLOCK_X0 = WIDTH / 2;
constexpr int CLOCK_Y0 = HEIGHT / 2;

constexpr int CLOCK_R_HOURS = 210;
constexpr int CLOCK_R_MINUTES = 170;

constexpr int HOURS_CIRCLE_OUTER_R = 29;
constexpr int HOURS_CIRCLE_INNER_R = 25;
constexpr int MINUTES_CIRCLE_OUTER_R = 7;
constexpr int MINUTES_CIRCLE_INNER_R = 3;

constexpr short DC_PIN = 4;     // D2
constexpr short RST_PIN = 2;    // D4
constexpr short BUSY_PIN = 5;   // D1

enum class DisplayMode {
    Light = 0,
    Dark = 1
};

struct UserConfiguration {
    char wifi_ssid[WL_SSID_MAX_LENGTH + 1] = "";
    char wifi_password[WL_WPA_KEY_MAX_LENGTH + 1] = "";

    char location[64] = "";
    char timezone[64] = "";

    bool manual_timezone = false;
    bool skip_weather_data = false;

    int wakeup_time = 7*60;    // slugcat wakes up
    int sleep_time = 22*60;    // slugcat goes to sleep

    DisplayMode day_mode = DisplayMode::Light;
    DisplayMode night_mode = DisplayMode::Light;
};

extern UserConfiguration config;

void readConfig();

#endif  // RWCLOCK_CONFIG_HPP_
