#ifndef RWCLOCK_CONNECTION_HPP_
#define RWCLOCK_CONNECTION_HPP_

#include <optional>

struct MeteoData {
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
};

extern MeteoData meteo_data;

void connectToWiFi();
void configNTP();
void updateLocalDataFromServer();

#endif  // RWCLOCK_CONNECTION_HPP_
