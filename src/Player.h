#pragma once
namespace Shiro {
    struct Player {
        int playerId;
        const unsigned char *name;
        int tetroCount;
        int pentoCount;
        int tetrisCount;
    };
}