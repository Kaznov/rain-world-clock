#include "connection.hpp"
#include "bufref_json.hpp"
#include "config.hpp"
#include "display.hpp"
#include "meteo.hpp"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <ctime>

static ESP8266WiFiMulti WiFiMulti;
static char day_query[80] = "";

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(config.wifi_ssid, config.wifi_password);

  Serial.println(F("[WiFi] Connecting..."));

  // wait for WiFi connection, 10s
  wl_status_t wifi_connect_result = WiFiMulti.run(10000);

  if (wifi_connect_result != WL_CONNECTED) {
    Serial.printf_P(PSTR("[WiFi] Unable to connect, error: %d\n\n"), wifi_connect_result);
  } else {
    Serial.println(F("[WiFi] Connected"));
  }
}

void configNTP() {
    Serial.println(F("[NTP] Updating NTP config"));

    settimeofday_cb([]() {
        Serial.println(F("[NTP] Time updated"));
    });

    // These addresses are not copied, they need to be in static memory!
    configTime(config.timezone, "pool.ntp.org", "time.nist.gov", "time.google.com");
}

static void createQuery() {
    day_query[0] = '\0';
    strcat_P(day_query, day_api);
    strcat_P(day_query, PSTR("/?q="));

    //strlcat(day_query, config.location, sizeof(day_query));
    // Unfortunately, URI needs some characters escaped.
    // We only escape the few most common here

    int i = strlen(day_query);
    int size = sizeof(day_query);
    for (char* next = config.location; *next; ++next) {
        switch (*next)
        {
        case ' ':
        case '#':
        case '$':
        case '%':
        case '&':
        case '+':
        case ',':
        case '/':
        case ':':
        case ';':
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '`':
        case '{':
        case '|':
        case '}':
        case '~':
            if (i + 3 >= size) { return; }
            sprintf(day_query + i, "%%%X", *next);
            i += 3;
            break;

        default:
            if (i + 1 >= size) { return; }
            day_query[i++] = *next;
        }
    }
}

void updateLocalDataFromServer() {
    if (config.location[0] == '\0') {
        return;
    }

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

    BufRefJsonDocument doc((char*)getDisplayBuffer(), DISPLAY_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    const char* tz = doc["tz"];
    if (tz != nullptr              // there is timezone data
        && !config.manual_timezone // automatic timezone mode
        && strcmp(tz, config.timezone) != 0) { // there was a change (DST?)
        strlcpy(config.timezone, tz, sizeof(config.timezone));
        configNTP();

        Serial.printf_P(PSTR("Timezone changed to: %s\n"), config.timezone);
    }

    auto ampmFormatToMinutes = [](const char* time_str) -> short {
        if (strlen(time_str) != 8) return 0;
        int hours = (time_str[0] - '0') * 10 + (time_str[1] - '0');
        int minutes = (time_str[3] - '0') * 10 + (time_str[4] - '0');
        if (time_str[6] == 'P') hours += 12;
        return (short)(hours * 60 + minutes);
    };

    meteo_data = {
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

    const MeteoData& meteo = meteo_data;

    Serial.printf_P(PSTR("Temp now:            %dC\n"), meteo.temp_now);
    Serial.printf_P(PSTR("Weather ID now:      %d\n"), meteo.weather_now);
    Serial.printf_P(PSTR("Temp today:          %dC\n"), meteo.temp_today);
    Serial.printf_P(PSTR("Weather ID today:    %d\n"), meteo.weather_today);
    Serial.printf_P(PSTR("Temp tonight:        %dC\n"), meteo.temp_tonight);
    Serial.printf_P(PSTR("Weather ID tonight:  %d\n"), meteo.weather_tonight);
    Serial.printf_P(PSTR("Temp tomorrow:       %dC\n"), meteo.temp_tomorrow);
    Serial.printf_P(PSTR("Weather ID tomorrow: %d\n"), meteo.weather_tomorrow);

    Serial.printf_P(PSTR("Sunrise: %02d:%02d\n"), meteo.sunrise / 60, meteo.sunrise % 60);
    Serial.printf_P(PSTR("Sunset:  %02d:%02d\n"), meteo.sunset / 60, meteo.sunset % 60);

    Serial.println(F("Day data from server collected"));
}
