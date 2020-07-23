#include "ControllerBindings.h"
Shiro::ControllerBindings::ControllerBindings() :
    name(""),
    controllerIndex(-1),
    controllerID(-1),
    hatIndex(-1) {}
void Shiro::ControllerBindings::read(PDINI::INI& ini, const std::string sectionName) {
    if (!ini.get(sectionName, "CONTROLLER_NAME", name) && !ini.get(sectionName, "CONTROLLER_INDEX", controllerIndex)) {
        // When no controller name nor index is set in the INI, just disable
        // controller input completely.
        name = "";
        controllerIndex = -1;
    }
    // A controller was selected; controller axes and hat settings have defaults,
    // but buttons don't.
    unsigned buttonGets = 0u;
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