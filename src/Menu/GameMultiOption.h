#pragma once
#include "GameArguments.h"
namespace Shiro {
    struct GameMultiOptionData {
        int mode;
        int num;
        int selection;
        std::vector<std::string> labels;
        // Array of argument lists. So, each argument list is an array of (void *), which can be dereferenced and filled with whatever data types are appropriate.
        GameArguments *args;
    };
}