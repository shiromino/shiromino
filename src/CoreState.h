#include "core.h"
#include <SDL.h>
#include <vector>
struct CoreState
{
    CoreState() {}

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

    struct {
        const char *name;
        unsigned int w;
        unsigned int h;
        SDL_Window *window;
        SDL_Renderer *renderer;
#ifdef OPENGL_INTERPOLATION
        SDL_Texture *target_tex;
        GLuint interpolate_shading_prog;
#endif
    } screen;

    char *iniFilename;
    char *calling_path;

    Shiro::Settings* settings;
    struct assetdb *assets;
    SDL_Texture *bg;
    SDL_Texture *bg_old;
    Sint16 bg_r, bg_g, bg_b;
    //gfx_animation *g2_bgs[10];

    //gfx_animation *anim_bg;
    //gfx_animation *anim_bg_old;

    std::vector<gfx_message> gfx_messages;
    std::vector<gfx_animation> gfx_animations;
    std::vector<gfx_button> gfx_buttons;
    std::size_t gfx_messages_max;
    std::size_t gfx_animations_max;
    std::size_t gfx_buttons_max;

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