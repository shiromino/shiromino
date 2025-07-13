#pragma once
#include "SDL_render.h"
#include <string>
#include <vector>
#include "Game.h"
#include "gfx_structures.h"
#include "menu/Option.h"

#define MENU_ID_MAIN 0
#define MENU_ID_PRACTICE 1
#define MENU_ID_REPLAY 2

struct CoreState;

struct menudata
{
    menudata() :
        menu_id(0),
        main_menu_data({ 0, 0 }),
        practice_menu_data({ nullptr, 0 }),
        target_tex(nullptr),
        use_target_tex(0),
        target_tex_update(false),
        numopts(0),
        selection(0),
        is_paged(0),
        page(0),
        page_length(0),
        page_text_x(0),
        page_text_y(0),
        x(0),
        y(0) {}

    std::vector<Shiro::MenuOption> menu;
    std::vector<gfx_button> menuButtons;
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
    bool target_tex_update;
    int numopts;
    int selection;

    int is_paged;
    int page;
    int page_length;

    int page_text_x;
    int page_text_y;

    std::string title;
    int x;
    int y;
};

Shiro::MenuOption std_game_multiopt_create(CoreState *cs, unsigned int mode, int num_sections, std::string label);

bool menu_is_using_target_tex(game_t *g);
int menu_update_target_tex_size(game_t *g, int w, int h);

void menu_update_replay_pagination(menudata *d, int from_selection);
int menu_delete_selected_replay(CoreState *cs, void *data);

int menu_text_toggle(CoreState *cs);
int menu_text_insert(CoreState *cs, char *);
int menu_text_backspace(CoreState *cs);
int menu_text_delete(CoreState *cs);
int menu_text_seek_left(CoreState *cs);
int menu_text_seek_right(CoreState *cs);
int menu_text_seek_home(CoreState *cs);
int menu_text_seek_end(CoreState *cs);
int menu_text_select_all(CoreState *cs);
int menu_text_copy(CoreState *cs);
int menu_text_cut(CoreState *cs);

game_t *menu_create(CoreState *cs);

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
int menu_is_main(game_t *g);