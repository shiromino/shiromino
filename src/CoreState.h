#pragma once
#include "AssetStore.h"
#include "DASDirection.h"
#include "DisplayMode.h"
#include "Game.h"
#include "GuiScreenManager.h"
#include "Gfx/Screen.h"
#include "Gfx/Animation.h"
#include "gfx_structures.h"
#include "Input/KeyFlags.h"
#include "Player.h"
#include "Settings.h"
#include "RecordList.h"
#include <filesystem>
#include "SDL.h"
#include <vector>
#include <deque>
#define RECENT_FRAMES 60

struct CoreState {
    CoreState() = delete;

    CoreState(Shiro::Settings& settings);
    ~CoreState();

    bool init();

    void run();

    Shiro::Screen screen;

    double fps;        // because tap fps = 61.68

    int text_editing;
    int (*text_toggle)(CoreState *);    // used to abstract text editing functions, can be set/called by other parts of the code
    int (*text_insert)(CoreState *, char *);
    int (*text_backspace)(CoreState *);
    int (*text_delete)(CoreState *);
    int (*text_seek_left)(CoreState *);
    int (*text_seek_right)(CoreState *);
    int (*text_seek_home)(CoreState *);
    int (*text_seek_end)(CoreState *);
    int (*text_select_all)(CoreState *);
    int (*text_copy)(CoreState *);
    int (*text_cut)(CoreState *);

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

    Shiro::Settings& settings;
    Shiro::AssetStore* assets;
    SDL_Texture *bg;
    SDL_Texture *bg_old;
    Sint16 bg_r, bg_g, bg_b;

    std::vector<gfx_button> gfx_buttons;
    std::size_t gfx_messages_max;
    std::size_t gfx_buttons_max;
    // TODO: Replace all gfx_* code with the new Gfx system.
    Shiro::Gfx gfx;

    Shiro::KeyFlags prev_keys_raw;
    Shiro::KeyFlags keys_raw;
    Shiro::KeyFlags prev_keys;
    Shiro::KeyFlags keys;
    Shiro::KeyFlags pressed;
    Shiro::DASDirection hold_dir;
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

    Shiro::DisplayMode displayMode;
    bool motionBlur;

    //long double avg_sleep_ms;
    //long double avg_sleep_ms_recent;
    long frames;

    //long double avg_sleep_ms_recent_array[RECENT_FRAMES];
    //int recent_frame_overload;

    Shiro::RecordList records;
    // struct scoredb archive;
    Shiro::Player player;
};

int is_left_input_repeat(CoreState *cs, int delay);
int is_right_input_repeat(CoreState *cs, int delay);
int is_up_input_repeat(CoreState *cs, int delay);
int is_down_input_repeat(CoreState *cs, int delay);

struct bindings *bindings_copy(struct bindings *src);

int load_files(CoreState *cs);

int process_events(CoreState* cs);
bool procgame(game_t *g, int input_enabled);

void handle_replay_input(CoreState* cs);
void update_input_repeat(CoreState *cs);
void update_pressed(CoreState *cs);

int button_emergency_inactive(CoreState *cs);
int gfx_buttons_input(CoreState *cs);

int request_fps(CoreState *cs, double fps);