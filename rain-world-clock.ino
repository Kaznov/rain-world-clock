/*
  Rain World Clock

  Copyright (c) 2023 - present, Kamil Kaznowski
*/

#include "bitmap_selector.h"
#include "clock_coordinates.h"
#include "config.h"
#include "connection.h"
#include "display.h"

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

void drawDisplay(const struct tm& now) {
  Serial.printf_P(PSTR("Drawing display for %02d:%02d\n"), now.tm_hour, now.tm_min);
  const unsigned char* const bitmap = getBackgroundImage(now);
  const Palette palette = getPalette();

  display.firstPage();
  do {
    display.fillScreen(palette.background_color);
    display.drawBitmap(PICTURE_X0, PICTURE_Y0, bitmap, PICTURE_WIDTH, PICTURE_HEIGHT, GxEPD_WHITE, GxEPD_BLACK);
    drawClock(now, palette);
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

  delay(1000);   // for serial initialization
  Serial.println();
  Serial.println(F("Startup..."));

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

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
    char old_timezone[64];
    strcpy(old_timezone, timezone);
    getLocalDataFromServer();
    if (strcmp(old_timezone, timezone) != 0) {
      configNTP();
    }
  }

  m = micros64();
  tv_sec = m / 1000000;
  tv_usec = m % 1000000;
  Serial.printf("Finished. tv_sec: %ld, tv_usec: %ld\n", tv_sec, tv_usec);

  delayUntilNextMinute();
}
