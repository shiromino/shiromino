#pragma once

#include <cstdint>

namespace Shiro {
    class Timer {
    public:
        Timer();
        Timer(double fps);
        Timer(double fps, uint64_t time);

        operator uint64_t();
        Timer& operator++();
        Timer operator++(int);
        Timer& operator--();
        Timer operator--(int);

        uint64_t hr();
        uint64_t min();
        uint64_t sec();
        uint64_t csec();
        uint64_t msec();

        double fps;
        uint64_t time;
    };
}
