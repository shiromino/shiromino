#pragma once

#define SIMULATE_QRS   0
#define SIMULATE_G1    0x0010
#define SIMULATE_G2    0x0020
#define SIMULATE_G3    0x0040
#define BIG_MODE       0x8000

#define MENU_FPS       60.00
#define PENTOMINO_FPS  60.00
#define G1_FPS         59.84
#define G2_FPS         (57272700.0 / 8.0 / 443.0 / 262.0)
#define G3_FPS         59.94

#define RECENT_FRAMES 60
#define FRAMEDELAY_ERR 0

#define BUTTON_PRESSED_THIS_FRAME 2
#define JOYSTICK_DEAD_ZONE 8000

#include <string>
#include <vector>
#include <memory>

#include "SDL.h"
#include "SDL_mixer.h"
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
    gfx_image ASSET_IMG_NONE = {NULL};

#define IMG(name, filename) gfx_image name;
#include "images.h"
#undef IMG

#define FONT(name, sheetName, outlineSheetName, charW, charH) BitFont name;
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

struct coreState
{
    coreState() {}

    double fps;        // because tap fps = 61.68

    int text_editing;
    int (*text_toggle)(coreState *);    // used to abstract text editing functions, can be set/called by other parts of the code
    int (*text_insert)(coreState *, char *);
    int (*text_backspace)(coreState *);
    int (*text_delete)(coreState *);
    int (*text_seek_left)(coreState *);
    int (*text_seek_right)(coreState *);
    int (*text_seek_home)(coreState *);
    int (*text_seek_end)(coreState *);
    int (*text_select_all)(coreState *);
    int (*text_copy)(coreState *);
    int (*text_cut)(coreState *);

    int left_arrow_das;
    int right_arrow_das;
    int backspace_das;
    int delete_das;
    int select_all;
    int undo;
    int redo;

    int zero_pressed;
    int one_pressed;
    int two_pressed;
    int three_pressed;
    int four_pressed;
    int five_pressed;
    int six_pressed;
    int seven_pressed;
    int nine_pressed;

    struct {
        const char *name;
        unsigned int w;
        unsigned int h;
        SDL_Window *window;
        SDL_Renderer *renderer;
        //SDL_Texture *target_tex;
    } screen;

    char *iniFilename;
    char *calling_path;

    Shiro::Settings* settings;
    struct assetdb *assets;
    SDL_Texture *bg;
    SDL_Texture *bg_old;
    //gfx_animation *g2_bgs[10];

    //gfx_animation *anim_bg;
    //gfx_animation *anim_bg_old;

    std::vector<gfx_message> gfx_messages;
    std::vector<gfx_animation> gfx_animations;
    std::vector<gfx_button> gfx_buttons;
    int gfx_messages_max;
    int gfx_animations_max;
    int gfx_buttons_max;

    struct keyflags prev_keys_raw;
    struct keyflags keys_raw;
    struct keyflags prev_keys;
    struct keyflags keys;
    struct keyflags pressed;
    das_direction hold_dir;
    int hold_time;

    SDL_Joystick *joystick;
    int mouse_x;
    int mouse_y;
    int logical_mouse_x;
    int logical_mouse_y;
    int mouse_left_down;
    int mouse_right_down;

    int menu_input_override;
    int button_emergency_override;

    game_t *p1game;
    game_t *menu;
    struct pracdata *pracdata_mirror;
    GuiScreenManager *screenManager;

    gameDisplayMode displayMode;
    bool motionBlur;

    //long double avg_sleep_ms;
    //long double avg_sleep_ms_recent;
    long frames;

    //long double avg_sleep_ms_recent_array[RECENT_FRAMES];
    //int recent_frame_overload;

    struct scoredb scores;
    // struct scoredb archive;
    struct player player;
};

struct game
{
    int (*init)(game_t *);
    int (*quit)(game_t *);
    int (*preframe)(game_t *);
    int (*input)(game_t *);
    int (*frame)(game_t *);
    int (*draw)(game_t *);

    unsigned long frame_counter;

    coreState *origin;
    Shiro::Grid *field;
    void *data;
};

class Game
{
public:
    Game(coreState& cs) : cs(cs), frameCounter(0) {}
    virtual ~Game() {}

    virtual int init() { return 0; }
    virtual int quit() { return 0; }
    virtual int input() { return 0; }
    virtual int frame() = 0;
    virtual int draw() { return 0; }

protected:
    coreState& cs;
    unsigned long frameCounter;
};

extern struct bindings defaultkeybinds[2];

extern BindableVariables bindables;

int is_left_input_repeat(coreState *cs, int delay);
int is_right_input_repeat(coreState *cs, int delay);
int is_up_input_repeat(coreState *cs, int delay);
int is_down_input_repeat(coreState *cs, int delay);

struct bindings *bindings_copy(struct bindings *src);

void coreState_initialize(coreState *cs);
void coreState_destroy(coreState *cs);

gfx_animation *load_anim_bg(coreState *cs, const char *directory, int frame_multiplier);
int load_files(coreState *cs);

int init(coreState *cs, Shiro::Settings* s);
void quit(coreState *cs);

int run(coreState *cs);
int procevents(coreState *cs, GuiWindow& wind);
int procgame(game_t *g, int input_enabled);

void handle_replay_input(coreState* cs);
void update_input_repeat(coreState *cs);
void update_pressed(coreState *cs);

int button_emergency_inactive(coreState *cs);
int gfx_buttons_input(coreState *cs);

int request_fps(coreState *cs, double fps);
