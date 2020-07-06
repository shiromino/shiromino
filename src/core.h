#pragma once

#define RECENT_FRAMES 60
#define FRAMEDELAY_ERR 0

#include <string>
#include <vector>
#include <memory>

#include "Config.hpp"
#include "SDL.h"
#include "SDL_mixer.h"
#ifdef OPENGL_INTERPOLATION
#include "glad.h"
#endif
#include "SGUIL/SGUIL.hpp"
#include "GuiScreenManager.hpp"
#include "Settings.hpp"

#include "Grid.hpp"
#include "gfx_structures.h"
#include "Audio.hpp"

#include "scores.h"

struct player
{
    int playerId;

    const unsigned char *name;

    int tetroCount;
    int pentoCount;

    int tetrisCount;
};

enum {
    MODE_INVALID,
    QUINTESSE
};

enum gameDisplayMode
{
    game_display_default,
    game_display_detailed,
    game_display_centered,
    game_display_bare
};

typedef enum {
    DAS_NONE,
    DAS_LEFT,
    DAS_RIGHT,
    DAS_UP,
    DAS_DOWN
} das_direction;

struct keyflags
{
    Uint8 left;
    Uint8 right;
    Uint8 up;
    Uint8 down;
    Uint8 a;
    Uint8 b;
    Uint8 c;
    Uint8 d;
    Uint8 start;
    Uint8 escape;
};

// TODO: Refactor lists of assets with a number appended into arrays.
struct assetdb
{
    gfx_image ASSET_IMG_NONE = { NULL };

#define IMG(name, filename) gfx_image name
#include "images.h"
#undef IMG

#define FONT(name, sheetName, outlineSheetName, charW, charH) BitFont name
#include "fonts.h"
#undef FONT

#define MUSIC(name, i) Shiro::Music* name[i];
#define DEF_ARRAY
#include "music.h"
#undef DEF_ARRAY
#undef MUSIC

#define SFX(name) Shiro::Sfx* name;
#define DEF_ARRAY
#define SFX_ARRAY(name, i) Shiro::Sfx* name[i];
#include "sfx.h"
#undef SFX
#undef DEF_ARRAY
#undef SFX_ARRAY
};

typedef struct game game_t;

#include "qrs.h"

struct game
{
    int (*init)(game_t *);
    int (*quit)(game_t *);
    int (*preframe)(game_t *);
    int (*input)(game_t *);
    int (*frame)(game_t *);
    int (*draw)(game_t *);

    unsigned long frame_counter;

    CoreState *origin;
    Shiro::Grid *field;
    void *data;
};

class Game
{
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

extern struct bindings defaultkeybinds[2];

extern BindableVariables bindables;

int is_left_input_repeat(CoreState *cs, int delay);
int is_right_input_repeat(CoreState *cs, int delay);
int is_up_input_repeat(CoreState *cs, int delay);
int is_down_input_repeat(CoreState *cs, int delay);

struct bindings *bindings_copy(struct bindings *src);

void CoreState_initialize(CoreState *cs);
void CoreState_destroy(CoreState *cs);

gfx_animation *load_anim_bg(CoreState *cs, const char *directory, int frame_multiplier);
int load_files(CoreState *cs);

int init(CoreState *cs, Shiro::Settings* s);
void quit(CoreState *cs);

int run(CoreState *cs);
int procevents(CoreState *cs, GuiWindow& wind);
int procgame(game_t *g, int input_enabled);

void handle_replay_input(CoreState* cs);
void update_input_repeat(CoreState *cs);
void update_pressed(CoreState *cs);

int button_emergency_inactive(CoreState *cs);
int gfx_buttons_input(CoreState *cs);

int request_fps(CoreState *cs, double fps);
