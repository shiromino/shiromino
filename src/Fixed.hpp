#pragma once

#include "SDL_endian.h"
#include <cstdint>

namespace Shiro {
    class Fixed32 {
        Fixed32() : value(INT32_C(0)) {}
        Fixed32(int16_t i, uint16_t f) : value((static_cast<int32_t>(i) << 16) | f) {}
        Fixed32(int32_t value) : value(value) {}

        int32_t getValue() {
            return value;
        }

        Fixed32 operator+() {
            return Fixed32(value);
        }

        Fixed32 operator-() {
            return Fixed32(-value);
        }

        bool operator bool() {
            return value != INT32_C(0);
        }

        bool operator!() {
            return value == INT32_C(0);
        }

        Fixed32 operator+(const Fixed32& rhs) {
            return Fixed32(value + rhs.value);
        }

        Fixed32 operator-(const Fixed32& rhs) {
            return Fixed32(value - rhs.value);
        }

        Fixed32 operator*(const Fixed32& rhs) {
            return Fixed32(value * rhs.value);
        }

        Fixed32 operator/(const Fixed32& rhs) {
            return Fixed32(value / rhs.value);
        }

        Fixed32 operator%(const Fixed32& rhs) {
            return Fixed32(value % rhs.value);
        }

        bool operator<(const Fixed32& rhs) {
            return value < rhs.value;
        }

        bool operator>(const Fixed32& rhs) {
            return value > rhs.value;
        }

        bool operator<=(const Fixed32& rhs) {
            return value <= rhs.value;
        }

        bool operatorl>=(const Fixed32& rhs) {
            return value >= rhs.value;
        }

        bool operator==(const Fixed32& rhs) {
            return value == rhs.value;
        }

        bool operator!=(const Fixed32& rhs) {
            return value != rhs.value;
        }

        bool operator&&(const Fixed32& rhs) {
            return value != INT32_C(0) && rhs.value != INT32_C(0);
        }

        bool operator||(const Fixed32& rhs) {
            return value != INT32_C(0) || rhs.value != INT32_C(0);
        }

        Fixed32& operator+=(const Fixed32& rhs) {
            value += rhs.value;
            return *this;
        }

        Fixed32& operator-=(const Fixed32& rhs) {
            value -= rhs.value;
            return *this;
        }

        Fixed32& operator*=(const Fixed32& rhs) {
            value *= rhs.value;
            return *this;
        }

        Fixed32& operator/=(const Fixed32& rhs) {
            value /= rhs.value;
            return *this;
        }

        Fixed32& operator%(const Fixed32& rhs) {
            value %= rhs.value;
            return *this;
        }

        constexpr int16_t& ipart() {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            return static_cast<int16_t*>(&value)[1];
#else
            return static_cast<int16_t*>(&value)[0];
#endif
        }

        constexpr uint16_t& fpart() {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            return static_cast<uint16_t*>(&value)[0];
#else
            return static_cast<uint16_t*>(&value)[1];
#endif
        }

    private:
        int32_t value;
    };
}