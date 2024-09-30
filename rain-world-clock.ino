/*
  Rain World Clock

  Copyright (c) 2023 - present, Kamil Kaznowski
*/

#include "bitmap_selector.h"
#include "clock_coordinates.h"
#include "config.h"
#include "connection.h"
#include "display.h"

#include "LittleFS.h"

#include <time.h>

void drawClock(const struct tm& now, const Palette& palette) {
  const std::size_t now_h = (std::size_t)now.tm_hour % 12;
  const std::size_t now_m = (std::size_t)now.tm_min;

  for (std::size_t h = now_h; h < 12; ++h) {
    const auto [hx, hy] = hour_circles_positions[h];
    fillCircle(hx, hy, HOURS_CIRCLE_OUTER_R, palette.detail_color);
  }

  {
    const auto [now_hx, now_hy] = hour_circles_positions[now_h];
    // fill inner proportionally to how much of this hour has already passed
    const short now_hr = HOURS_CIRCLE_INNER_R * now_m / 60;  // < linear in r, not area
    fillCircle(now_hx, now_hy, now_hr, palette.background_color);
  }

  for (std::size_t m = now_m; m < 60; ++m) {
    const auto [mx, my] = minutes_circles_positions[m];
    fillCircle(mx, my, MINUTES_CIRCLE_OUTER_R, palette.detail_color);
  }

  {
    const auto [now_mx, now_my] = minutes_circles_positions[now_m];
    fillCircle(now_mx, now_my, MINUTES_CIRCLE_INNER_R, palette.background_color);
  }

  const short ring_inner_r = CLOCK_R_MINUTES - MINUTES_CIRCLE_OUTER_R - 1;
  const short ring_outer_r = CLOCK_R_MINUTES + MINUTES_CIRCLE_OUTER_R + 1;

  display.drawCircle(CLOCK_X0, CLOCK_Y0, ring_inner_r, palette.front_color);
  display.drawCircle(CLOCK_X0, CLOCK_Y0, ring_outer_r, palette.front_color);
}

void drawBitmapFromFile(BitmapFile& bmp, int current_page, int x_center, int y_center) {
  unsigned char* display_buffer = getDisplayBuffer();

  const int picture_x0 = x_center - bmp.width / 2;
  const int picture_y0 = y_center - bmp.height / 2;

  Serial.printf("%d, %d,  %d, %d,\n\n", bmp.width, bmp.height, picture_x0, picture_y0);

  const int begin_draw_row = current_page * PAGE_HEIGHT;
  const int end_draw_row   = (current_page + 1) * PAGE_HEIGHT;

  const int begin_bitmap_row = picture_y0;
  const int end_bitmap_row   = picture_y0 + bmp.height;

  const int begin_copy_row = std::max(begin_draw_row, begin_bitmap_row);
  const int end_copy_row   = std::min(end_draw_row, end_bitmap_row); 

  const int begin_copy_row_in_bmp = bmp.height - (end_copy_row - picture_y0);

  bmp.f.seek(begin_copy_row_in_bmp * bmp.width / 8 + bmp.data_offset);

  for (int h = end_copy_row - 1; h >= begin_copy_row; --h) {
    unsigned char row[MAX_PICTURE_WIDTH / 8];
    bmp.f.readBytes((char*)row, bmp.width / 8);
    unsigned char* row_buffer = display_buffer + WIDTH / 8 * (h % PAGE_HEIGHT) + picture_x0 / 8;

    if (1) {
      for (int i = 0; i < bmp.width / 8; ++i) {
        row_buffer[i] &= row[i];
      }
    } else {
      for (int i = 0; i < bmp.width / 8; ++i) {
        row_buffer[i] |= row[i];
      }
    }
  }
}

const char* readJsonElementText(const JsonDocument& doc, const char* key) {
  const char* value = doc[key].as<const char*>();
  if (value == nullptr) {
    return {};
  }

  Serial.printf_P(PSTR("Read JSON value: %s: %s\n"), key, value);
  return value;
}

int readJsonElementTime(const JsonDocument& doc, const char* key) {
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

bool readJsonElementBoolean(const JsonDocument& doc, const char* key) {
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

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, config_file);
  config_file.close();

  if (error) {
    Serial.printf_P(PSTR("Can't load config\n"));
    return;
  }

  const char* wifi_ssid = readJsonElementText(doc, "wifi_ssid");
  strlcpy(config.wifi_ssid, wifi_ssid, sizeof(config.wifi_ssid));

  const char* wifi_password = readJsonElementText(doc, "wifi_password");
  strlcpy(config.wifi_password, wifi_password, sizeof(config.wifi_password));

  const char* location = readJsonElementText(doc, "location");
  strlcpy(config.location, location, sizeof(config.location));

  config.wakeup_time = readJsonElementTime(doc, "wakeup_time");
  config.sleep_time = readJsonElementTime(doc, "sleep_time");

  config.day_mode = readJsonElementBoolean(doc, "day_dark_mode")
    ? DisplayMode::Dark
    : DisplayMode::Light;

  config.night_mode = readJsonElementBoolean(doc, "night_dark_mode")
    ? DisplayMode::Dark
    : DisplayMode::Light;
}

void drawDisplay(const struct tm& now) {
  Serial.printf_P(PSTR("Drawing display for %02d:%02d\n"), now.tm_hour, now.tm_min);
  std::optional<BitmapFile> bitmap = getBackgroundImage(now);
  if (!bitmap) { return; }

  const Palette palette = getPalette();

  int current_page = 0;
  display.firstPage();
  do {
    display.fillScreen(palette.background_color);
    // display.drawBitmap(PICTURE_X0, PICTURE_Y0, bitmap, PICTURE_WIDTH, PICTURE_HEIGHT, GxEPD_WHITE, GxEPD_BLACK);
    drawClock(now, palette);
    drawBitmapFromFile(*bitmap, current_page, CLOCK_X0, CLOCK_Y0);
    ++current_page;
    current_page %= PAGE_COUNT;
  } while (display.nextPage());

  display.powerOff();
}

void delayUntilNextMinute() {
  // static constexpr unsigned ms_in_minute = 60'000;
  // const uint64_t millis = micros64() / 1000;
  // const unsigned ms_left = ms_in_minute - (unsigned)(millis % ms_in_minute);
  // delay(ms_left);

  uint64_t m = micros64();

  uint64_t tv_sec = m / 1000000;
  uint64_t tv_usec = m % 1000000;

  Serial.printf("Delay. tv_sec: %ld, tv_usec: %ld\n", tv_sec, tv_usec);

  auto s = 60 - (int)(tv_sec % 60) - 1;
  const auto ms = 1000 - (int)(tv_usec / 1000);

  delay(1000 * s + ms);
}

void setup() {
  // serial is initialized by display init, do NOT init it here explicitly
  display.init(115200, true, 2, false);
  display.setRotation(0);

  LittleFS.begin();

  delay(1000);   // for serial initialization
  Serial.println();
  Serial.println(F("Startup..."));

  readConfig();

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

  // This function sets both timezone, as well as info about the day/weather
  getLocalDataFromServer();

  // Configs the NTP servers
  configNTP();

  // Manual time set, for debugging
  // struct timeval tv;
  // tv.tv_sec = 1707906403;
  // tv.tv_usec = 0;
  // settimeofday(&tv, nullptr);

  delay(1000);  // Let the NTP process finish
}

void loop() {
  static time_t last = 0;
  time_t now = time(nullptr);
  struct tm now_local {};
  localtime_r(&now, &now_local);

  if (now_local.tm_min % 15 == 0 || abs(now - last) > 120 /*s*/) {
    // Full update on every quarter or time update from NTP
    display.setFullWindow();
  } else {
    display.setPartialWindow(0, 0, display.width(), display.height());
  }

  uint64_t m = micros64();
  uint64_t tv_sec = m / 1000000;
  uint64_t tv_usec = m % 1000000;

  Serial.printf("Printing. tv_sec: %ld, tv_usec: %ld\n", tv_sec, tv_usec);

  last = now;
  drawDisplay(now_local);

  if (now_local.tm_min == 0) {
    // Update weather info and timezone every hour
    char old_timezone[sizeof(config.timezone)];
    strcpy(old_timezone, config.timezone);
    getLocalDataFromServer();
    if (strcmp(old_timezone, config.timezone) != 0) {
      configNTP();
    }
  }

  m = micros64();
  tv_sec = m / 1000000;
  tv_usec = m % 1000000;
  Serial.printf("Finished. tv_sec: %ld, tv_usec: %ld\n", tv_sec, tv_usec);

  delayUntilNextMinute();
}
