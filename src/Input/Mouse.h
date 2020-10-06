/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once

namespace Shiro {
    struct Mouse {
        Mouse();

        enum class Button {
            notPressed,
            pressed,
            pressedThisFrame
        };

        // TODO: Consider changing how windowW/windowH are passed in.
        void update(const int windowW, const int windowH);

        int x;
        int y;
        int logicalX;
        int logicalY;

        Button leftButton;
        Button rightButton;
    };
}