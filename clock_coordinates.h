#ifndef CLOCK_COORDINATES_H
#define CLOCK_COORDINATES_H

#include <array>
#include <cmath>
#include <numbers>

#include "display.h"

struct Position {
    short x;
    short y;
};

template <std::size_t N>
consteval std::array<Position, N> get_positions_on_circle(short r, short x0 = 0, short y0 = 0) {
    static_assert(N%4 == 0);
    constexpr std::size_t qN = N / 4;
    constexpr double pi = std::numbers::pi;

    short dx[N]{};
    short dy[N]{};

    for (std::size_t i = 0; i < qN; ++i) {
        short x = (short)std::round(std::cos((pi * 2) * i / N - (pi / 2)) * r);
        short y = (short)std::round(std::sin((pi * 2) * i / N - (pi / 2)) * r);

        dx[i] = x;
        dy[i] = y;
    }

    // symmetry around (0, 0)
    for (std::size_t i = qN; i < N; ++i) {
        dx[i] = -dy[i - qN];
        dy[i] = dx[i - qN];
    }

    std::array<Position, N> result;
    result.fill(Position{x0, y0});

    for (std::size_t i = 0; i < N; ++i) {
        result[i].x += dx[i];
        result[i].y += dy[i];
    }

    // std::rotate(result.begin(), result.begin() + 1, result.end());
    return result;
}

constexpr std::array<Position, 12> hour_circles_positions
    = get_positions_on_circle<12>(CLOCK_R_HOURS, CLOCK_X0, CLOCK_Y0);
constexpr std::array<Position, 60> minutes_circles_positions
    = get_positions_on_circle<60>(CLOCK_R_MINUTES, CLOCK_X0, CLOCK_Y0);

#endif
