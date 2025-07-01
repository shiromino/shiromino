#pragma once

namespace Shiro { class Grid; }
struct CoreState;

typedef struct game game_t;
struct game {
    int (*init)(game_t *);
    int (*quit)(game_t *);
    int (*preframe)(game_t *);
    int (*input)(game_t *);
    int (*frame)(game_t *);
    int (*draw)(game_t *);

    bool update(bool inputEnabled);

    unsigned long frame_counter;

    CoreState *origin;
    Shiro::Grid *field;
    void *data;
};
class Game {
public:
    Game(CoreState& cs) : cs(cs), frameCounter(0) {}
    virtual ~Game() {}

    virtual int init() { return 0; }
    virtual int quit() { return 0; }
    virtual int input() { return 0; }
    virtual int frame() = 0;
    virtual int draw() { return 0; }

protected:
    CoreState& cs;
    unsigned long frameCounter;
};