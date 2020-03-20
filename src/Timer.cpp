#include <cstdint>

#include "Timer.hpp"

using namespace Shiro;

Timer::Timer() : fps(60.0), time(0) {}
Timer::Timer(double fps) : fps(fps), time(0) {}
Timer::Timer(double fps, uint64_t time) : fps(fps), time(time) {}

Timer::operator uint64_t() { return time; }

Timer& Timer::operator++() {
    if (time < UINT64_MAX) {
        time++;
    }
    return *this;
}

Timer Timer::operator++(int) {
    Timer result(*this);
    ++(*this);
    return result;
}

Timer& Timer::operator--() {
    if (time > 0) {
        time--;
    }
    return *this;
}

Timer Timer::operator--(int) {
    Timer result(*this);
    --(*this);
    return result;
}

uint64_t Timer::hr() { return (time / fps) / 3600.0; }
uint64_t Timer::min() { return (time / fps) / 60.0; }
uint64_t Timer::sec() { return time / fps; }
uint64_t Timer::csec() { return (time / fps) * 100.0; }
uint64_t Timer::msec() { return (time / fps) * 1000.0; }
