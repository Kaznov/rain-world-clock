#ifndef CONFIG_H
#define CONFIG_H

#include <time.h>

const char day_api[] PROGMEM = "http://d1ox7ijvdplfv4.cloudfront.net";
char day_query[64] = "";

constexpr short HEIGHT = 480;
constexpr short WIDTH = 800;

constexpr short PICTURE_WIDTH = 288;  // size of the prepared bitmaps
constexpr short PICTURE_HEIGHT = 288; // size of the prepared bitmaps

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

constexpr short PICTURE_X0 = CLOCK_X0 - PICTURE_WIDTH / 2;
constexpr short PICTURE_Y0 = CLOCK_Y0 - PICTURE_HEIGHT / 2;

constexpr short DC_PIN = 4;     // D2
constexpr short RST_PIN = 2;    // D4
constexpr short BUSY_PIN = 5;   // D1

char ssid[32] = "Your WiFi SSID";
char password[64] = "Your WiFi password";

char location[64] = "Neugraben";
char timezone[64] = "";

constexpr unsigned char bday_month = 1;        // [1, 12]
constexpr unsigned char bday_day_of_month = 1; // [1, 31]

constexpr short DAY_TIME = 7*60;    // slugcat wakes up
constexpr short NIGHT_TIME = 23*60; // slugcat goes to sleep

enum class DisplayMode {
    Light = 0,
    Dark = 1
} display_mode;

struct DayData {
    time_t timestamp;
    short temp_now;
    short weather_now;
    short temp_today;
    short weather_today;
    short temp_tonight;
    short weather_tonight;
    short temp_tomorrow;
    short weather_tomorrow;
    short sunrise;
    short sunset;
} day_data;

template <class T, std::size_t N>
constexpr std::size_t array_size(const T (&array)[N]) noexcept
{
    return N;
}

#endif
