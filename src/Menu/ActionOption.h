#pragma once
#include "Game.h"
namespace Shiro {
    struct ActionOptionData {
        int (*action)(game_t *, int);
        int val;
    };
}