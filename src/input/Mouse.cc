#include "input/Mouse.h"
#include "SDL_timer.h"
#include "video/Screen.h"

namespace Shiro {
    Mouse::Mouse() :
        x(0),
        y(0),
        logicalX(0),
        logicalY(0),
        leftButton(Button::notPressed),
        rightButton(Button::notPressed),
        hideOnStartup(true),
        shown(false),
        hideTicks(SDL_GetTicks()) {}

    bool Mouse::operator==(const Mouse& cmp) const {
        return
            x == cmp.x &&
            y == cmp.y &&
            logicalX == cmp.logicalX &&
            logicalY == cmp.logicalY &&
            leftButton == cmp.leftButton &&
            rightButton == cmp.rightButton &&
            hideOnStartup == cmp.hideOnStartup &&
            shown == cmp.shown &&
            hideTicks == cmp.hideTicks;
    }

    bool Mouse::operator!=(const Mouse& cmp) const {
        return !(*this == cmp);
    }

    void Mouse::updateButtonState() {
        if (leftButton == Button::pressedThisFrame) {
            leftButton = Button::pressed;
        }
        if (rightButton == Button::pressedThisFrame) {
            rightButton = Button::pressed;
        }
    }

    void Mouse::updateLogicalPosition(const Screen& screen) {
        if( x < screen.innerRenderAreaX ||
            x > ( screen.innerRenderAreaX + static_cast<int>(float(screen.logicalW) * screen.render_scale) ) ||
            y < screen.innerRenderAreaY ||
            y > ( screen.innerRenderAreaY + static_cast<int>(float(screen.logicalH) * screen.render_scale) ) )
        {
            logicalX = logicalY = -1;
            return;
        }

        logicalX = static_cast<int>(float(x - screen.innerRenderAreaX) / screen.render_scale);
        logicalY = static_cast<int>(float(y - screen.innerRenderAreaY) / screen.render_scale);

        /*
        float scale;
        // Normal 4:3 aspect ratio, no conversion necessary
        if (windowW == (windowH * 4) / 3) {
            scale = windowW / 640.0f;
            logicalX = static_cast<int>(x / scale);
            logicalY = static_cast<int>(y / scale);
        }
        // Squished vertically (results in horizontal bars on the top and bottom of the window)
        else if (windowW < (windowH * 4) / 3) {
            scale = windowW / 640.0f;
            int yOffset = (windowH - ((windowW * 3) / 4)) / 2;
            if (y < yOffset || y >= windowH - yOffset) {
                logicalY = -1;
            }
            else {
                logicalY = static_cast<int>((y - yOffset) / scale);
            }

            logicalX = static_cast<int>(x / scale);
        }
        // Squished horizontally (results in vertical bars on the left and right of the window)
        else {
            scale = windowH / 480.0f;
            int xOffset = (windowW - ((windowH * 4) / 3)) / 2;
            if (x < xOffset || x >= windowW - xOffset) {
                logicalX = -1;
            }
            else {
                logicalX = static_cast<int>((x - xOffset) / scale);
            }

            logicalY = static_cast<int>(y / scale);
        }
        */
    }
}