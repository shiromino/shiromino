#include "RotationTables.h"
constexpr bool _ = false;
const std::array<std::array<std::array<std::array<bool, 5>, 5>, 4>, 18> Shiro::PentoRotationTables =
{
    std::array<std::array<std::array<bool, 5>, 5>, 4>
    { // I
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, 1},
            {_, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, 1},
            {_, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _}
        }
    },

    { // J
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, _},
            {_, _, _, 1, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, 1, 1, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // L
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, 1, _},
            {1, 1, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // X
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // S
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Z
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // N
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // G
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, 1, 1, 1, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, 1, 1, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // U
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {1, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, _, 1, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // T
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Fa
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Fb
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // P
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, _, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Q
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {1, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        }
    },

    { // W
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {1, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, 1, 1, _, _},
            {1, 1, _, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Ya
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Yb
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {1, 1, 1, 1, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, 1, _, _, _},
            {1, 1, 1, 1, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, 1, _, _, _},
            {_, 1, 1, _, _},
            {_, 1, _, _, _},
            {_, 1, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // V
        std::array<std::array<bool, 5>, 5>
        { // FLAT
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {1, _, _, _, _},
            {1, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, 1, 1, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            std::array<bool, 5>
            {_, _, _, _, _},
            {_, _, 1, _, _},
            {_, _, 1, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            std::array<bool, 5>
            {_, _, _, _, _},
            {1, _, _, _, _},
            {1, _, _, _, _},
            {1, 1, 1, _, _},
            {_, _, _, _, _}
        }
    }
};

const std::array<std::array<std::array<std::array<bool, 4>, 4>, 4>, 7> Shiro::TetroRotationTables =
{
    std::array<std::array<std::array<bool, 4>, 4>, 4>
    { // I4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, 1, 1},
            {_, _, _, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {_, _, 1, _},
            {_, _, 1, _},
            {_, _, 1, _},
            {_, _, 1, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, 1, 1},
            {_, _, _, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, _, 1, _},
            {_, _, 1, _},
            {_, _, 1, _},
            {_, _, 1, _}
        }
    },

    { // T4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, 1, _},
            {_, 1, _, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {_, 1, _, _},
            {1, 1, _, _},
            {_, 1, _, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, _, _},
            {1, 1, 1, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, 1, _, _},
            {_, 1, 1, _},
            {_, 1, _, _},
            {_, _, _, _}
        }
    },

    { // J4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, 1, _},
            {_, _, 1, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {_, 1, _, _},
            {_, 1, _, _},
            {1, 1, _, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {1, _, _, _},
            {1, 1, 1, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, 1, 1, _},
            {_, 1, _, _},
            {_, 1, _, _},
            {_, _, _, _}
        }
    },

    { // L4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, 1, _},
            {1, _, _, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {1, 1, _, _},
            {_, 1, _, _},
            {_, 1, _, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {_, _, 1, _},
            {1, 1, 1, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, 1, _, _},
            {_, 1, _, _},
            {_, 1, 1, _},
            {_, _, _, _}
        }
    },

    { // 1
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {_, 1, 1, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {_, 1, 1, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {_, 1, 1, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {_, 1, 1, _},
            {_, _, _, _}
        }
    },

    { // S4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {1, 1, _, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {1, _, _, _},
            {1, 1, _, _},
            {_, 1, _, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {_, 1, 1, _},
            {1, 1, _, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {1, _, _, _},
            {1, 1, _, _},
            {_, 1, _, _},
            {_, _, _, _}
        }
    },

    { // Z4
        std::array<std::array<bool, 4>, 4>
        { // FLAT
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, _, _},
            {_, 1, 1, _},
            {_, _, _, _}
        },

        { // CW
            std::array<bool, 4>
            {_, _, 1, _},
            {_, 1, 1, _},
            {_, 1, _, _},
            {_, _, _, _}
        },

        { // FLIP
            std::array<bool, 4>
            {_, _, _, _},
            {1, 1, _, _},
            {_, 1, 1, _},
            {_, _, _, _}
        },

        { // CCW
            std::array<bool, 4>
            {_, _, 1, _},
            {_, 1, 1, _},
            {_, 1, _, _},
            {_, _, _, _}
        }
    }
};