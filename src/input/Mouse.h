#pragma once
#include "types.h"

namespace Shiro {
    struct Screen;
    
    struct Mouse {
        Mouse();

        bool operator==(const Mouse& cmp) const;
        bool operator!=(const Mouse& cmp) const;

        enum class Button {
            notPressed,
            pressed,
            pressedThisFrame
        };

        // TODO: Consider changing how windowW/windowH are passed in.
        void updateButtonState();
        void updateLogicalPosition(const Screen& screen);

        int x;
        int y;
        int logicalX;
        int logicalY;

        Button leftButton;
        Button rightButton;

        bool hideOnStartup;
        bool shown;
        u32 hideTicks;
    };
}