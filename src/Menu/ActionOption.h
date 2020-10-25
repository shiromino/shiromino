#pragma once
#include "Game.h"
namespace Shiro {
    struct ActionOptionData {
        ActionOptionData();
        int (*action)(game_t *, int);
        int val;
    };
}