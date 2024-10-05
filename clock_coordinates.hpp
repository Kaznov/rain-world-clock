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

template <int N>
constexpr std::array<Position, N> get_positions_on_circle(short r) {
    static_assert(N%4 == 0);
    constexpr int qN = N / 4;
    constexpr double pi = 3.14159265358979323846;

    short dx[N]{};
    short dy[N]{};

    for (int i = 0; i < qN; ++i) {
        short x = (short)std::round(std::cos((pi * 2) * i / N - (pi / 2)) * r);
        short y = (short)std::round(std::sin((pi * 2) * i / N - (pi / 2)) * r);

        dx[i] = x;
        dy[i] = y;
    }

    // symmetry around (0, 0)
    for (int i = qN; i < N; ++i) {
        dx[i] = -dy[i - qN];
        dy[i] = dx[i - qN];
    }

    std::array<Position, N> result{};
    for (int i = 0; i < N; ++i) {
        result[i] = Position{dx[i], dy[i]};
    }

    return result;
}

constexpr std::array<Position, 12> hour_circles_positions
    = get_positions_on_circle<12>(CLOCK_R_HOURS);
constexpr std::array<Position, 60> minutes_circles_positions
    = get_positions_on_circle<60>(CLOCK_R_MINUTES);

#endif  // RWCLOCK_CLOCK_COORDINATES_HPP_
