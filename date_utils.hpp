#ifndef RWCLOCK_DAY_UTILS_HPP_
#define RWCLOCK_DAY_UTILS_HPP_

struct DayOfYear {
    unsigned short day;
    constexpr DayOfYear(unsigned short month, unsigned short day_of_month)
        : day{ month * 32 + day_of_month} { }
    constexpr DayOfYear(const struct tm& time)
        : DayOfYear{ time.tm_mon + 1, time.tm_mday } { }
    bool operator==(const DayOfYear& other) const { return day == other.day; };
};

#endif  // RWCLOCK_DAY_UTILS_HPP_
