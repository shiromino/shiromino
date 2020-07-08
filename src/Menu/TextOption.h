#pragma once
namespace Shiro {
    struct TextOptionData {
        int active;
        int position;
        int selection;
        int leftmost_position;
        int visible_chars;
        std::string text;
    };
}