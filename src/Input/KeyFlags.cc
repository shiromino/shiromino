#include "Input/KeyFlags.h"

namespace Shiro {
    bool KeyFlags::operator==(const KeyFlags& cmp) const {
        return
            left == cmp.left &&
            right == cmp.right &&
            up == cmp.up &&
            down == cmp.down &&
            a == cmp.a &&
            b == cmp.b &&
            c == cmp.c &&
            d == cmp.d &&
            start == cmp.start &&
            escape == cmp.escape;
    }

    bool KeyFlags::operator!=(const KeyFlags& cmp) const {
        return !(*this == cmp);
    }
}