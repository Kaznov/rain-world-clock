#ifndef CONNECTION_H
#define CONNECTION_H

#include "config.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <time.h>

ESP8266WiFiMulti WiFiMulti;

void configNTP() {
    Serial.println(F("[NTP] Updating NTP config"));

    settimeofday_cb([]() {
        Serial.println(F("[NTP] Time updated"));
    });

    // These addresses are not copied, they need to be in static memory!
    configTime(timezone, "pool.ntp.org", "time.nist.gov", "time.google.com");
}

void createQuery() {
    day_query[0] = '\0';
    strcat_P(day_query, day_api);
    strcat_P(day_query, PSTR("/?q="));
    strcat(day_query, location);
}

void getLocalDataFromServer() {
    if (day_query[0] == '\0') {
        createQuery();
    }

    HTTPClient http;
    WiFiClient wifiClient;
    http.useHTTP10(true);
    Serial.println(F("Connecting with day data server..."));
    Serial.printf_P(PSTR("[HTTP] query: %s\n"), day_query);
    Serial.println(F("[HTTP] begin..."));
    if (!http.begin(wifiClient, day_query)) {
        Serial.println(F("[HTTP] Unable to connect"));
        return;
    }

    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf_P(PSTR("[HTTP] GET... failed, error: %d %s\n"),
                      httpCode,
                      http.errorToString(httpCode).c_str());
        return;
    }
    // HTTP header has been send and Server response header has been handled
    Serial.printf_P(PSTR("[HTTP] GET successful, code: %d\n"), httpCode);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();
    
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    const char* tz = doc["tz"];
    if (tz != nullptr) {
        strcpy(timezone, tz);
    }

    Serial.printf_P(PSTR("Timezone: %s\n"), timezone);

    auto ampmFormatToMinutes = [](const char* time_str) -> short {
        if (strlen(time_str) != 8) return 0;
        int hours = (time_str[0] - '0') * 10 + (time_str[1] - '0');
        int minutes = (time_str[3] - '0') * 10 + (time_str[4] - '0');
        if (time_str[6] == 'P') hours += 12;
        return (short)(hours * 60 + minutes);
    };

    day_data = DayData {
        .timestamp = time(nullptr),
        .temp_now =         doc[F("current_temp_c")],
        .weather_now =      doc[F("current_condition_code")],
        .temp_today =       doc[F("today_max_temp_c")],
        .weather_today =    doc[F("today_condition_code")],
        .temp_tonight =     doc[F("today_min_temp_c")] /*TODO, not true*/,
        .weather_tonight =  0 /*TODO*/,
        .temp_tomorrow =    doc[F("tomorrow_max_temp_c")],
        .weather_tomorrow = doc[F("tomorrow_condition_code")],
        .sunrise =  ampmFormatToMinutes(doc[F("sunrise")]),
        .sunset =   ampmFormatToMinutes(doc[F("sunset")])
    };

    Serial.printf_P(PSTR("Temp now:            %dC\n"), day_data.temp_now);
    Serial.printf_P(PSTR("Weather ID now:      %d\n"), day_data.weather_now);
    Serial.printf_P(PSTR("Temp today:          %dC\n"), day_data.temp_today);
    Serial.printf_P(PSTR("Weather ID today:    %d\n"), day_data.weather_today);
    Serial.printf_P(PSTR("Temp tonight:        %dC\n"), day_data.temp_tonight);
    Serial.printf_P(PSTR("Weather ID tonight:  %d\n"), day_data.weather_tonight);
    Serial.printf_P(PSTR("Temp tomorrow:       %dC\n"), day_data.temp_tomorrow);
    Serial.printf_P(PSTR("Weather ID tomorrow: %d\n"), day_data.weather_tomorrow);

    Serial.printf_P(PSTR("Sunrise: %02d:%02d\n"), day_data.sunrise / 60, day_data.sunrise % 60);
    Serial.printf_P(PSTR("Sunset:  %02d:%02d\n"), day_data.sunset / 60, day_data.sunset % 60);

    Serial.println(F("Day data from server collected"));
}

#endif
