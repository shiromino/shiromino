#pragma once
namespace Shiro {
    enum class GameType {
        SIMULATE_QRS = 0x0000,
        SIMULATE_G1 = 0x0010,
        SIMULATE_G2 = 0x0020,
        SIMULATE_G3 = 0x0040,
        BIG_MODE = 0x8000
    };
}