#pragma once
#include <vector>
namespace Shiro {
    struct MultiOptionData {
        MultiOptionData();
        int selection;
        int num;
        int *param;
        int *vals;
        std::vector<std::string> labels;
    };
}