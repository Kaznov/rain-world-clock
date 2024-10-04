#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxEPD2.h>
#include <GxEPD2_BW.h>

#include "config.hpp"

using DisplayType = GxEPD2_750_T7; // GDEW075T7 800x480, EK79655 (GD7965)
static constexpr short PAGE_COUNT = 2;
static constexpr short PAGE_HEIGHT = DisplayType::HEIGHT / PAGE_COUNT;
using Display = GxEPD2_BW < DisplayType, PAGE_HEIGHT>;
using DisplayBufferType = uint8_t[DisplayType::WIDTH * PAGE_HEIGHT / 8];
constexpr int DISPLAY_BUFFER_SIZE = sizeof(DisplayBufferType);

extern Display display;

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

struct Palette {
    unsigned short front_color;
    unsigned short detail_color;
    unsigned short background_color;
};

constexpr Palette DARK_PALETTE = {
    .front_color = GxEPD_WHITE,
    .detail_color = GxEPD_WHITE,
    .background_color = GxEPD_BLACK
};

constexpr Palette LIGHT_PALETTE = {
    .front_color = GxEPD_BLACK,
    .detail_color = GxEPD_DARKGREY,
    .background_color = GxEPD_WHITE
};

#endif
