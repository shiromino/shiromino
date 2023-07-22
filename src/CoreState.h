#pragma once
#include "asset/Asset.h"
#include "AssetStore.h"
#include "DASDirection.h"
#include "DisplayMode.h"
#include "Game.h"
#include "ARS.h"
#include "gui/ScreenManager.h"
#include "video/Screen.h"
#include "video/Background.h"
#include "video/Gfx.h"
#include "gfx_structures.h"
#include "input/KeyFlags.h"
#include "input/Mouse.h"
#include "Player.h"
#include "Settings.h"
#include "Records.h"
#include "SDL.h"
#include <vector>
#define RECENT_FRAMES 60

struct CoreState {
    CoreState() = delete;

    CoreState(Shiro::Settings& settings);
    ~CoreState();

    bool init();

    void run();

    bool is_left_input_repeat(unsigned delay);
    bool is_right_input_repeat(unsigned delay);
    bool is_up_input_repeat(unsigned delay);
    bool is_down_input_repeat(unsigned delay);

    void load_files(); // TODO: Make this return bool to indicate whether loading succeeded.

    bool process_events();

    void handle_replay_input();
    void update_input_repeat();
    void update_pressed();

    bool button_emergency_inactive();
    void gfx_buttons_input();

    int request_fps(double fps);

    Shiro::Screen screen;

    double fps;        // because tap fps = 61.68

    // TODO: Create a text editing class, moving members from `text_editing` up to and including `redo`.
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

    unsigned left_arrow_das;
    unsigned right_arrow_das;
    unsigned backspace_das;
    unsigned delete_das;
    bool select_all;
    bool undo;
    bool redo;

    bool pressedDigits[10];

    Shiro::Settings& settings;
    // TODO: Refactor so all assets are loaded by the asset manager.
    Shiro::AssetStore* assets;
    Shiro::AssetManager assetMgr;

    Shiro::Background bg;
    std::vector<gfx_button> gfx_buttons;
    // TODO: Replace all gfx_* code with the new Gfx system.
    Shiro::Renderer gfxRenderer;

    Shiro::KeyFlags prev_keys_raw;
    Shiro::KeyFlags keys_raw;
    Shiro::KeyFlags prev_keys;
    Shiro::KeyFlags keys;
    Shiro::KeyFlags pressed;
    Shiro::DASDirection hold_dir;
    unsigned hold_time;

    SDL_Joystick *joystick;
    Shiro::Mouse mouse;

    bool menu_input_override;
    bool button_emergency_override;

    game_t *p1game;
    game_t *menu;
    struct pracdata *pracdata_mirror;
    ScreenManager *screenManager;

    ARS ars;
    Game *SPMgame;

    Shiro::DisplayMode displayMode;
    bool motionBlur;

    //long double avg_sleep_ms;
    //long double avg_sleep_ms_recent;
    unsigned long frames;

    //long double avg_sleep_ms_recent_array[RECENT_FRAMES];
    //int recent_frame_overload;

    Shiro::Records::List records;
    // struct scoredb archive;
    Shiro::Player player;
};