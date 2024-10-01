/*
  Rain World Clock

  Copyright (c) 2023 - present, Kamil Kaznowski
*/

#include "bitmap_selector.hpp"
#include "clock_coordinates.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "date_utils.hpp"
#include "display.hpp"

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

void drawBitmapFromFile(BitmapFile& bmp, int current_page, int x_center, int y_center, Palette p) {
  unsigned char* display_buffer = getDisplayBuffer();

  const int picture_x0 = x_center - bmp.width / 2;
  const int picture_y0 = y_center - bmp.height / 2;

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

    if (p.background_color == GxEPD_WHITE) {
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

void drawDisplay(const struct tm& now) {
  Serial.printf_P(PSTR("Drawing display for %02d:%02d\n"), now.tm_hour, now.tm_min);

  std::optional<BitmapFile> bitmap = getBackgroundImage(now);
  if (!bitmap) { return; }

  const Palette palette = getCurrentDisplayMode(now) == DisplayMode::Dark
    ? DARK_PALETTE
    : LIGHT_PALETTE;

  int current_page = 0;
  display.firstPage();
  do {
    display.fillScreen(palette.background_color);
    // display.drawBitmap(PICTURE_X0, PICTURE_Y0, bitmap, PICTURE_WIDTH, PICTURE_HEIGHT, GxEPD_WHITE, GxEPD_BLACK);
    drawClock(now, palette);
    drawBitmapFromFile(*bitmap, current_page, CLOCK_X0, CLOCK_Y0, palette);
    ++current_page;
    current_page %= PAGE_COUNT;
  } while (display.nextPage());

  display.powerOff();
}

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

  // Connects to WiFi, keeps the connection on
  connectToWiFi();

  // This function sets both timezone, as well as info about the day/weather
  updateLocalDataFromServer();

  // Configs the NTP servers, keeps the time updated
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

  last = now;
  drawDisplay(now_local);

  if (now_local.tm_min == 0) {
    updateLocalDataFromServer();
  }

  delayUntilNextMinute();
}
