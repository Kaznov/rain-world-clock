#include "config.hpp"

#include "LittleFS.h"
#include "ArduinoJson.h"

UserConfiguration config;

static constexpr int WPA_KEY_MIN_LENGTH = 8;

static const char* readJsonElementText(const JsonDocument& doc, const char* key) {
  const char* value = doc[key].as<const char*>();
  if (value == nullptr) {
    return {};
  }

  Serial.printf_P(PSTR("Read JSON value: %s: %s\n"), key, value);
  return value;
}

static int readJsonElementTime(const JsonDocument& doc, const char* key) {
  const char* time_value = doc[key].as<const char*>();
  if (time_value == nullptr) {
    return -1;
  }

  Serial.printf_P(PSTR("Read JSON value: %s: %s\n"), key, time_value);

  int hour = -1, minute = -1;
  sscanf(time_value, "%2d:%2d", &hour, &minute);

  Serial.printf_P(PSTR("Hour, minutes: %d: %d\n"), hour, minute);

  if (hour < 0 || minute < 0 || hour >= 24 || minute >= 60) {
    return -1;
  }

  return hour * 60 + minute;
}

static bool readJsonElementBoolean(const JsonDocument& doc, const char* key) {
  bool is_boolean = doc[key].is<bool>();

  if (is_boolean == false) {
    return false;
  }

  bool value = doc[key].as<bool>();
  Serial.printf_P(PSTR("Read JSON value: %s: %s\n"), key, (value ? "true" : "false"));

  return value;
}

void readConfig() {
  File config_file = LittleFS.open("config.json", "r");

  if (!config_file) {
    Serial.printf_P(PSTR("Can't open config file\n"));
    return;
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, config_file);
  config_file.close();

  if (error) {
    Serial.printf_P(PSTR("Can't load config\n"));
    return;
  }

  const char* wifi_ssid = readJsonElementText(doc, "wifi_ssid");
  if (!wifi_ssid) {
    return;
  }
  strlcpy(config.wifi_ssid, wifi_ssid, sizeof(config.wifi_ssid));

  const char* wifi_password = readJsonElementText(doc, "wifi_password");
  if (!wifi_password) {
    return;
  }
  if (strlen(wifi_password) < WPA_KEY_MIN_LENGTH) {
    return;
  }
  strlcpy(config.wifi_password, wifi_password, sizeof(config.wifi_password));

  const char* location = readJsonElementText(doc, "location");
  if (location) {
    strlcpy(config.location, location, sizeof(config.location));
  }

  const char* timezone = readJsonElementText(doc, "timezone");
  if (timezone) {
    strlcpy(config.timezone, timezone, sizeof(config.timezone));
    config.manual_timezone = true;
  }

  int wakeup_time = readJsonElementTime(doc, "wakeup_time");
  if (wakeup_time != -1) {
    config.wakeup_time = wakeup_time;
  }

  int sleep_time = readJsonElementTime(doc, "sleep_time");
  if (sleep_time != -1) {
    config.sleep_time = sleep_time;
  }

  // These two return false on missing values, no need for extra checks

  config.day_mode = readJsonElementBoolean(doc, "day_dark_mode")
    ? DisplayMode::Dark
    : DisplayMode::Light;

  config.night_mode = readJsonElementBoolean(doc, "night_dark_mode")
    ? DisplayMode::Dark
    : DisplayMode::Light;
}

DisplayMode getCurrentDisplayMode(const tm & now)
{
    int minutes_into_day = now.tm_hour * 60 + now.tm_min;
    bool is_sleep = minutes_into_day < config.wakeup_time || minutes_into_day >= config.sleep_time;
    return is_sleep ? config.night_mode : config.day_mode;
}
