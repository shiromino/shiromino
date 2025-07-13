#include "ControllerBindings.h"
#include "PDINI.h"
#include <algorithm>

Shiro::ControllerBindings::ControllerBindings() :
    name(""),
    controllerIndex(-1),
    controllerID(-1),
    hatIndex(-1) {}
void Shiro::ControllerBindings::read(PDINI::INI& ini) {
    const auto sectionName = "PLAYER_1_CONTROLLER_BINDINGS";
    if (!ini.get(sectionName, "CONTROLLER_NAME", name) && !ini.get(sectionName, "CONTROLLER_INDEX", controllerIndex)) {
        // When no controller name nor index is set in the INI, just disable
        // controller input completely.
        name = "";
        controllerIndex = -1;
        return;
    }
    // A controller was selected; controller axes and hat settings have defaults,
    // but buttons don't.
    [[maybe_unused]] unsigned buttonGets = 0u;
    buttonGets += ini.get(sectionName, "BUTTON_LEFT", buttons.left) && buttons.left >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_RIGHT", buttons.right) && buttons.right >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_UP", buttons.up) && buttons.up >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_DOWN", buttons.down) && buttons.down >= 0;
    buttonGets += ini.get(sectionName, "BUTTON_START", buttons.start) ? buttons.start >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_A", buttons.a) ? buttons.a >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_B", buttons.b) ? buttons.b >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_C", buttons.c) ? buttons.c >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_D", buttons.d) ? buttons.d >= 0 : 0u;
    buttonGets += ini.get(sectionName, "BUTTON_ESCAPE", buttons.escape) ? buttons.escape >= 0 : 0u;
    unsigned axisGets = 0u;
    axisGets += ini.get(sectionName, "AXIS_X", axes.x) && axes.x >= 0;
    axisGets += ini.get(sectionName, "AXIS_Y", axes.y) && axes.y >= 0;
    auto appliedDeadZone = deadZone;
    if (ini.get(sectionName, "DEAD_ZONE", appliedDeadZone)) {
        this->deadZone = std::clamp(appliedDeadZone, 0.0f, 1.0f);
    }
    else {
        this->deadZone = 0.5f;
    }
    if (axisGets == 0u) {
        axes.x = 0;
        axes.right = 1;
        axes.y = 1;
        axes.down = 1;
    }
    else {
        if (axes.x >= 0) {
            std::string axisDirection = "";
            if (ini.get(sectionName, "AXIS_RIGHT", axisDirection)) {
                if (axisDirection == "+") {
                    axes.right = 1;
                }
                else if (axisDirection == "-") {
                    axes.right = -1;
                }
            }
            if (axes.right == 0) {
                axes.right = 1;
            }
        }
        else {
            axes.x = 0;
            axes.right = 1;
        }
        if (axes.y >= 0) {
            std::string axisDirection = "";
            if (ini.get(sectionName, "AXIS_DOWN", axisDirection)) {
                if (axisDirection == "+") {
                    axes.down = 1;
                }
                else if (axisDirection == "-") {
                    axes.down = -1;
                }
            }
            if (axes.down == 0) {
                axes.down = 1;
            }
        }
        else {
            axes.y = 1;
            axes.down = 1;
        }
        // Use default axis numbers if the user accidentally makes them
        // identical.
        if (axes.x == axes.y) {
            axes.x = 0;
            axes.y = 1;
        }
    }
    if (!ini.get(sectionName, "HAT_INDEX", hatIndex) || hatIndex < 0) {
        hatIndex = 0;
    }
}

void Shiro::ControllerBindings::write(PDINI::INI& ini) const {
    if (controllerIndex == -1) {
        return;
    }

    const auto sectionName = "PLAYER_1_CONTROLLER_BINDINGS";
    ini.set(sectionName, "CONTROLLER_NAME", name);
    ini.set(sectionName, "CONTROLLER_INDEX", controllerIndex);

    if (buttons.left >= 0) ini.set(sectionName, "BUTTON_LEFT", buttons.left);
    if (buttons.right >= 0) ini.set(sectionName, "BUTTON_RIGHT", buttons.right);
    if (buttons.up >= 0) ini.set(sectionName, "BUTTON_UP", buttons.up);
    if (buttons.down >= 0) ini.set(sectionName, "BUTTON_DOWN", buttons.down);
    if (buttons.start >= 0) ini.set(sectionName, "BUTTON_START", buttons.start);
    if (buttons.a >= 0) ini.set(sectionName, "BUTTON_A", buttons.a);
    if (buttons.b >= 0) ini.set(sectionName, "BUTTON_B", buttons.b);
    if (buttons.c >= 0) ini.set(sectionName, "BUTTON_C", buttons.c);
    if (buttons.d >= 0) ini.set(sectionName, "BUTTON_D", buttons.d);
    if (buttons.escape >= 0) ini.set(sectionName, "BUTTON_ESCAPE", buttons.escape);

    ini.set(sectionName, "AXIS_X", axes.x);
    ini.set(sectionName, "AXIS_Y", axes.y);
    ini.set(sectionName, "DEAD_ZONE", deadZone);
    ini.set(sectionName, "AXIS_RIGHT", axes.right > 0 ? "+" : "-");
    ini.set(sectionName, "AXIS_DOWN", axes.down > 0 ? "+" : "-");

    ini.set(sectionName, "HAT_INDEX", hatIndex);
}

Shiro::ControllerBindings::Buttons::Buttons() :
    left(-1),
    right(-1),
    up(-1),
    down(-1),
    start(-1),
    a(-1),
    b(-1),
    c(-1),
    d(-1),
    escape(-1) {}

Shiro::ControllerBindings::Axes::Axes() :
    x(-1),
    right(0),
    y(-1),
    down(0) {}