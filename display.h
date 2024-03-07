#include "GxEPD2.h"
#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxEPD2_BW.h>
#include "config.h"

using DisplayType = GxEPD2_750_T7; // GDEW075T7 800x480, EK79655 (GD7965)
using Display = GxEPD2_BW < DisplayType, DisplayType::HEIGHT / 2 >;
Display display(DisplayType(/*CS=D8*/ SS, DC_PIN, RST_PIN, BUSY_PIN));
using DisplayBufferType = uint8_t[DisplayType::WIDTH * DisplayType::HEIGHT / 8 / 2];

static_assert(DisplayType::WIDTH == WIDTH);
static_assert(DisplayType::HEIGHT == HEIGHT);

// private members robbing by Johannes "litb" Schaub
// http://bloglitb.blogspot.com/2010/07/access-to-private-members-thats-easy.html

// It is a *very* dirty trick to get to the private buffer of display,
// both for manual management and usage as a different purpose buffer
template<typename Tag>
struct result {
    /* export it ... */
    typedef typename Tag::type type;
    static type ptr;
};

template<typename Tag>
typename result<Tag>::type result<Tag>::ptr;

template<typename Tag, typename Tag::type p>
struct rob : result<Tag> {
    /* fill it ... */
    struct filler {
        filler() { result<Tag>::ptr = p; }
    };
    static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

struct BufferTypeWrapper { typedef DisplayBufferType Display::*type; };
template class rob<BufferTypeWrapper, &Display::_buffer>;

inline unsigned char* getDisplayBuffer() {
    return display.*result<BufferTypeWrapper>::ptr;
}

void fillCircle(short x0, short y0, short r, unsigned short color) {
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

struct Palette {
    unsigned short front_color;
    unsigned short detail_color;
    unsigned short background_color;
};

Palette getPalette() {
    if (display_mode == DisplayMode::Dark) {
        return {
            .front_color = GxEPD_WHITE,
            .detail_color = GxEPD_WHITE, // grey causes visual issues with display!
            .background_color = GxEPD_BLACK
        };
    }
    else {
        return {
            .front_color = GxEPD_BLACK, 
            .detail_color = GxEPD_DARKGREY,
            .background_color = GxEPD_WHITE
        };
    }
}


#endif
