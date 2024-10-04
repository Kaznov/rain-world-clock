/*
  Rain World Clock

  Copyright (c) 2023 - present, Kamil Kaznowski
*/

#include "bitmap_selector.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "date_utils.hpp"
#include "display.hpp"
#include "drawing.hpp"

#include <ctime>
#include <LittleFS.h>

void delayUntilNextMinute() {
  struct timeval tv{};
  _gettimeofday_r(nullptr, &tv, nullptr);

  Serial.printf("Delay. tv_sec: %lld, tv_usec: %lld\n", tv.tv_sec, tv.tv_usec);

  auto s = 60 - (int)(tv.tv_sec % 60) - 1;
  const auto ms = 1000 - (int)(tv.tv_usec / 1000);

  delay(1000 * s + ms);
}

void setup() {
  // serial is initialized by display init, do NOT init it here explicitly
  display.init(115200, true, 2, false);
  display.setRotation(0);
  delay(1000);   // for serial initialization

  Serial.println();
  Serial.println(F("Rain World Clock startup..."));

  if (!LittleFS.begin()) {
    return;
  }

  // Reads WiFi settings, information about location/timezone if provided
  readConfig();

  // Reads for what days/nights should there be a special background
  readSpecialBitmapsConfig();

  // Connects to WiFi, keeps the connection on
  connectToWiFi();

  // This function sets both timezone, as well as info about the day/weather
  updateLocalDataFromServer();

  // Configs the NTP servers, keeps the time updated
  configNTP();

  // TODO: wait for NTP config by checking time() with timeout, error on failure

  // Manual time set, for debugging
  // struct timeval tv;
  // tv.tv_sec = 1707930000; // Feb 14th 2024, 5:00PM GMT+0
  // tv.tv_usec = 0;
  // settimeofday(&tv, nullptr);

  delay(1000);  // Let the NTP process finish
}

void loop() {
  static time_t last = 0;
  time_t now = time(nullptr);
  struct tm now_local {};
  localtime_r(&now, &now_local);

  if (now_local.tm_min % 30 == 0) {
    updateLocalDataFromServer();
  }

  if (now_local.tm_min % 15 == 0 || abs(now - last) > 120 /*s*/) {
    // Full update on every quarter or time update from NTP
    display.setFullWindow();
  } else {
    display.setPartialWindow(0, 0, display.width(), display.height());
  }

  last = now;
  drawDisplay(now_local);

  delayUntilNextMinute();
}
