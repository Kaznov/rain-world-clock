#ifndef RWCLOCK_CLOCK_COORDINATES_HPP_
#define RWCLOCK_CLOCK_COORDINATES_HPP_

#include <array>
#include <cmath>

#include "config.hpp"
#include "display.hpp"

struct Position {
    short x;
    short y;
};

template <std::size_t N>
constexpr std::array<Position, N> get_positions_on_circle(short r, short x0 = 0, short y0 = 0) {
    static_assert(N%4 == 0);
    constexpr std::size_t qN = N / 4;
    constexpr double pi = 3.14159265358979323846;

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

    std::array<Position, N> result{};
    for (std::size_t i = 0; i < N; ++i) {
        short x = x0 + dx[i];
        short y = y0 + dy[i];
        result[i] = Position{x, y};
    }

    return result;
}

constexpr std::array<Position, 12> hour_circles_positions
    = get_positions_on_circle<12>(CLOCK_R_HOURS, CLOCK_X0, CLOCK_Y0);
constexpr std::array<Position, 60> minutes_circles_positions
    = get_positions_on_circle<60>(CLOCK_R_MINUTES, CLOCK_X0, CLOCK_Y0);

#endif  // RWCLOCK_CLOCK_COORDINATES_HPP_
