/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "SDL.h"

namespace Shiro {
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
        void updatePosition(const int windowW, const int windowH);

        int x;
        int y;
        int logicalX;
        int logicalY;

        Button leftButton;
        Button rightButton;

        bool hideOnStartup;
        bool shown;
        Uint32 hideTicks;
    };
}