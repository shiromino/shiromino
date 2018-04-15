#ifndef __PLAYER_H_
#define __PLAYER_H_

struct player {
    int playerId;

    const unsigned char *name;

    int tetroCount;
    int pentoCount;

    int tetrisCount;
};

#endif // __PLAYER_H_
