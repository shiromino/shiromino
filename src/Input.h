#pragma once
#include "types.h"
#define BUTTON_PRESSED_THIS_FRAME 2
#define JOYSTICK_DEAD_ZONE 8000
typedef enum {
    DAS_NONE,
    DAS_LEFT,
    DAS_RIGHT,
    DAS_UP,
    DAS_DOWN
} das_direction;
struct keyflags {
    Shiro::u8 left;
    Shiro::u8 right;
    Shiro::u8 up;
    Shiro::u8 down;
    Shiro::u8 a;
    Shiro::u8 b;
    Shiro::u8 c;
    Shiro::u8 d;
    Shiro::u8 start;
    Shiro::u8 escape;
};