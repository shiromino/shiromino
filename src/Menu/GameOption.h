#pragma once
#include "GameArguments.h"
namespace Shiro {
    struct GameOptionData {
        GameOptionData();
        int mode;
        GameArguments args;
    };
}