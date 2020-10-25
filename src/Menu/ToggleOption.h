#pragma once
namespace Shiro {
    struct ToggleOptionData {
        ToggleOptionData();
        bool *param;
        std::string labels[2];
    };
}