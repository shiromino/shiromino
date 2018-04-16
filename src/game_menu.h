#ifndef _game_menu_h
#define _game_menu_h

#include "bstrlib.h"
#include "core.h"
#include <stdbool.h>

#define MENU_PRACTICE_NUMOPTS 15

#define MENU_ID_MAIN 0
#define MENU_ID_PRACTICE 1
#define MENU_ID_REPLAY 2

enum {
    MENU_LABEL = 0,

    MENU_ACTION = 1,
    MENU_MULTIOPT = 2,
    MENU_TOGGLE = 3,
    MENU_TEXTINPUT = 4,
    MENU_GAME = 5,
    MENU_GAME_MULTIOPT = 6,
    MENU_METAGAME = 7
};

struct game_args
{
    int num;
    void **ptrs;
};

struct menu_opt
{
    int type;
    int (*value_update_callback)(coreState *cs);
    int render_update;
    bstring label;
    int x;
    int y;
    int value_x;
    int value_y;

    unsigned int label_text_flags;
    unsigned int value_text_flags;
    unsigned int label_text_rgba;
    unsigned int value_text_rgba;

    void *data;
};

struct action_opt_data
{
    int (*action)(game_t *, int); // limited to functions with this shape
    int val;
};

struct multi_opt_data
{
    int selection;
    int num;

    int *param;
    int *vals;
    bstring *labels;
};

struct text_opt_data
{
    int active;

    int position;
    int selection;
    int leftmost_position;
    int visible_chars;

    bstring text;
};

struct toggle_opt_data
{
    bool *param;
    bstring labels[2];
};

struct game_opt_data
{
    int mode;
    struct game_args args;
    // void **args;
};

struct game_multiopt_data
{
    int mode;
    int num;
    int selection;
    bstring *labels;

    struct game_args *args; // array of argument lists. so each argument list is an array of (void *), which can be
                            // dereferenced and filled with..
};                          // ..whatever data types are appropriate. bit confusing, I know.

struct metagame_opt_data
{
    int mode;
    int submode;
    int num_args;
    int num_subargs;

    void **args;
    void **sub_args;
};

typedef struct
{
    struct menu_opt **menu;
    int menu_id;

    struct
    {
        int selection;
        int opt_selection;
    } main_menu_data;

    struct
    {
        struct pracdata *pracdata_mirror;
        int selection;
    } practice_menu_data;

    SDL_Texture *target_tex;
    int use_target_tex;
    int numopts;
    int selection;

    int is_paged;
    int page;
    int page_length;

    int page_text_x;
    int page_text_y;

    bstring title;
    int x;
    int y;
} menudata;

struct menu_opt *std_game_multiopt_create(coreState *cs, unsigned int mode, int num_sections, bstring label);

struct menu_opt *menu_opt_create(int type, int (*value_update_callback)(coreState *cs), bstring label);
void menu_opt_destroy(struct menu_opt *m);

int menu_text_toggle(coreState *cs);
int menu_text_insert(coreState *cs, char *);
int menu_text_backspace(coreState *cs);
int menu_text_delete(coreState *cs);
int menu_text_seek_left(coreState *cs);
int menu_text_seek_right(coreState *cs);
int menu_text_seek_home(coreState *cs);
int menu_text_seek_end(coreState *cs);
int menu_text_select_all(coreState *cs);
int menu_text_copy(coreState *cs);
int menu_text_cut(coreState *cs);

game_t *menu_create(coreState *cs);

int menu_init(game_t *g);
int menu_quit(game_t *g);
int menu_input(game_t *g);
int menu_frame(game_t *g);
int menu_clear(game_t *g);

int mload_main(game_t *g, int val);
int mload_practice(game_t *g, int val);
int mload_replay(game_t *g, int val);
int mload_options(game_t *g, int val);

int menu_action_quit(game_t *g, int val);
int menu_is_practice(game_t *g);

#endif
