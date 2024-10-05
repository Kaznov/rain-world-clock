
#include "bitmap_selector.hpp"
#include "clock_coordinates.hpp"
#include "display.hpp"
#include "drawing.hpp"

#include <optional>

static void fillCircle(short x0, short y0, short r, unsigned short color) {
    if (color == GxEPD_WHITE || color == GxEPD_BLACK) {
        display.fillCircle(x0, y0, r, GxEPD_WHITE);
    }
    else if (color == GxEPD_DARKGREY) {
        // shade, filling half of pixels black, half white
        for (short y = y0 - r; y < y0 + r; ++y) {
            for (short x = x0 - r; x < x0 + r; ++x) {
                short dx = x - x0;
                short dy = y - y0;
                if (dx * dx + dy * dy >= r * r) continue;
                display.drawPixel(x, y, ((x+y) & 1) ? GxEPD_WHITE : GxEPD_BLACK);
            }
        }
    }
}

static void drawClock(const struct tm& now, const Palette& palette, short x0, short y0) {
  const int now_h = now.tm_hour % 12;
  const int now_m = now.tm_min;

  for (int h = now_h; h < 12; ++h) {
    const auto [hx, hy] = hour_circles_positions[h];
    fillCircle(hx + x0, hy + y0, HOURS_CIRCLE_OUTER_R, palette.detail_color);
  }

  {
    const auto [now_hx, now_hy] = hour_circles_positions[now_h];
    // fill inner proportionally to how much of this hour has already passed
    const short now_hr = HOURS_CIRCLE_INNER_R * now_m / 60;  // < linear in r, not area
    fillCircle(now_hx + x0, now_hy + y0, now_hr, palette.background_color);
  }

  for (int m = now_m; m < 60; ++m) {
    const auto [mx, my] = minutes_circles_positions[m];
    fillCircle(mx + x0, my + y0, MINUTES_CIRCLE_OUTER_R, palette.detail_color);
  }

  {
    const auto [now_mx, now_my] = minutes_circles_positions[now_m];
    fillCircle(now_mx + x0, now_my + y0, MINUTES_CIRCLE_INNER_R, palette.background_color);
  }

  const short ring_inner_r = CLOCK_R_MINUTES - MINUTES_CIRCLE_OUTER_R - 1;
  const short ring_outer_r = CLOCK_R_MINUTES + MINUTES_CIRCLE_OUTER_R + 1;

  display.drawCircle(x0, y0, ring_inner_r, palette.front_color);
  display.drawCircle(x0, y0, ring_outer_r, palette.front_color);
}

static void drawBitmapFromFile(BitmapFile& bmp, int current_page, int x_center, int y_center, Palette p) {
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
    drawClock(now, palette, clock_x0(), clock_y0());
    drawBitmapFromFile(*bitmap, current_page, clock_x0(), clock_y0(), palette);
    ++current_page;
    current_page %= PAGE_COUNT;
  } while (display.nextPage());

  display.powerOff();
}
