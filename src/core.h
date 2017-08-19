#ifndef _core_h
#define _core_h

#define SIMULATE_QRS   0
#define SIMULATE_G1    0x0010
#define SIMULATE_G2    0x0020
#define SIMULATE_G3    0x0040

#define FPS            60.0
#define G2_FPS        61.68

#define ASSET_IMG    0
#define ASSET_WAV    1
#define ASSET_MUS    2

#define RECENT_FRAMES 60
#define FRAMEDELAY_ERR 0

#define BUTTON_PRESSED_THIS_FRAME 2
#define JOYSTICK_DEAD_ZONE 8000

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef struct coreState_ coreState;

#include "bstrlib.h"
#include "grid.h"
#include "gfx_structures.h"

enum {
    MODE_INVALID,
    QUINTESSE
};

struct bindings {
    SDL_Keycode left;
    SDL_Keycode right;
    SDL_Keycode up;
    SDL_Keycode down;
    SDL_Keycode start;
    SDL_Keycode a;
    SDL_Keycode b;
    SDL_Keycode c;
    SDL_Keycode d;
    SDL_Keycode escape;
};

struct keyflags {
    Uint8 left;
    Uint8 right;
    Uint8 up;
    Uint8 down;
    Uint8 start;
    Uint8 a;
    Uint8 b;
    Uint8 c;
    Uint8 d;
    Uint8 escape;
};

struct asset {
    int type;
    int volume;
    char *name;
    void *data;
};

struct assetdb {
    struct asset **entry;
    int num;
};
/*
struct text_box {
    struct bstrList *lines;
};*/

struct settings {
    struct bindings *keybinds;

    int video_scale;
    int fullscreen;

    int master_volume;
    int sfx_volume;
    int mus_volume;

    char *home_path;
};

struct replay {
    struct keyflags *inputs;
    unsigned int len;
    unsigned int mlen;

    int mode;
    unsigned int mode_flags;
    long seed;
    int grade;
    long time;
    int starting_level;
    int ending_level;

    time_t date;
};

typedef struct game game_t;

#include "qrs.h"

struct coreState_ {
    int running;
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
        char *name;
        unsigned int w;
        unsigned int h;
        SDL_Window *window;
        SDL_Renderer *renderer;
        //SDL_Texture *target_tex;
    } screen;

    char *cfg_filename;
    char *calling_path;

    struct settings *settings;
    struct assetdb *assets;
    SDL_Texture *bg;
    SDL_Texture *bg_old;
    //gfx_animation *g2_bgs[10];

    //gfx_animation *anim_bg;
    //gfx_animation *anim_bg_old;

    gfx_message **gfx_messages;
    gfx_animation **gfx_animations;
    gfx_button **gfx_buttons;
    int gfx_messages_max;
    int gfx_animations_max;
    int gfx_buttons_max;

    struct keyflags *keys[2];
    SDL_Joystick *joystick;
    int mouse_x;
    int mouse_y;
    int mouse_left_down;
    int mouse_right_down;

    int master_volume;
    int sfx_volume;
    int mus_volume;

    int menu_input_override;
    int button_emergency_override;

    game_t *p1game;
    game_t *menu;
    struct pracdata *pracdata_mirror;

    long double avg_sleep_ms;
    long double avg_sleep_ms_recent;
    long frames;

    long double avg_sleep_ms_recent_array[RECENT_FRAMES];
    int recent_frame_overload;

    int obnoxious_text;        // experimental
};

struct game {
    int (*init)(game_t *);
    int (*quit)(game_t *);
    int (*preframe)(game_t *);
    int (*input)(game_t *);
    int (*frame)(game_t *);
    int (*draw)(game_t *);

    unsigned long frame_counter;

    coreState *origin;
    grid_t *field;
    void *data;
};

extern struct bindings defaultkeybinds[2];
extern struct settings defaultsettings;

void keyflags_init(struct keyflags *k);
void keyflags_update(coreState *cs);
struct bindings *bindings_copy(struct bindings *src);

int load_asset(coreState *cs, int type, char *name);
int unload_asset(coreState *cs, int index);
struct asset *asset(coreState *cs, int index);
struct asset *asset_by_name(coreState *cs, char *name);

coreState *coreState_create();
void coreState_destroy(coreState *cs);

gfx_animation *load_anim_bg(coreState *cs, const char *directory, int frame_multiplier);
int load_files(coreState *cs);

int init(coreState *cs, struct settings *s);
void quit(coreState *cs);

int run(coreState *cs);
int procevents(coreState *cs);
int procgame(game_t *g, int input_enabled);

int button_emergency_inactive(coreState *cs);
int gfx_buttons_input(coreState *cs);

int request_fps(coreState *cs, double fps);
long framedelay(Uint64 ns_elap, double fps);

long waste_time();
int toggle_obnoxious_text(coreState *cs, void *data);

struct replay *compare_replays(struct replay *r1, struct replay *r2);

#endif
