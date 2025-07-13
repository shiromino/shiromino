/*
    game_menu.c - logic functions governing the
    manipulation of and interactions with various
    menus
*/

// TODO: Change malloc/free of structs containing std::string to new/delete.

#include "game_menu.h"
#include "SDL_blendmode.h"
#include "SDL_clipboard.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_pixels.h"
#include <assert.h>
#include <string.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <new>
#include <sstream>
#include "CoreState.h"
#include "GameType.h"
#include "QRS0.h"
#include "Records.h"
#include "RefreshRates.h"
#include "Settings.h"
#include "asset/Image.h"
#include "asset/Music.h"
#include "asset/Sfx.h"
#include "game_qs.h"
#include "gfx_menu.h"
#include "gfx_old.h"
#include "input/KeyFlags.h"
#include "input/Mouse.h"
#include "menu/ActionOption.h"
#include "menu/ElementType.h"
#include "menu/GameArguments.h"
#include "menu/GameMultiOption.h"
#include "menu/GameOption.h"
#include "menu/MultiOption.h"
#include "menu/Option.h"
#include "menu/TextOption.h"
#include "menu/ToggleOption.h"
#include "replay.h"
#include "video/Background.h"
#include "video/Screen.h"

Shiro::MenuOption std_game_multiopt_create(CoreState *cs, unsigned int mode, int num_sections, std::string label)
{
    Shiro::MenuOption m = Shiro::create_menu_option(Shiro::ElementType::MENU_GAME_MULTIOPT, NULL, label);
    Shiro::GameMultiOptionData *d6 = (Shiro::GameMultiOptionData *)m.data;

    d6->mode = QUINTESSE;
    d6->num = num_sections;
    d6->selection = 0;
    d6->labels.push_back("");
    for (int i = 1; i < num_sections; i++) {
        std::stringstream ss;
        ss << 100 * i;
        d6->labels.push_back(ss.str());
    }

    d6->args = (Shiro::GameArguments *)malloc(num_sections * sizeof(Shiro::GameArguments));
    if(!d6->args)
    {
        throw std::bad_alloc();
    }
    assert(d6->args != nullptr);
    for (int i = 0; i < num_sections; i++) {
        d6->args[i].num = 4;
        d6->args[i].ptrs = (void **)malloc(4 * sizeof(void *));
        if(!d6->args[i].ptrs)
        {
            throw std::bad_alloc();
        }
        if (
            !(d6->args[i].ptrs[0] = malloc(sizeof(CoreState*))) ||
            !(d6->args[i].ptrs[1] = malloc(sizeof(int))) ||
            !(d6->args[i].ptrs[2] = malloc(sizeof(unsigned int))) ||
            !(d6->args[i].ptrs[3] = malloc(sizeof(char*))))
        {
            throw std::bad_alloc();
        }

        *(CoreState **)(d6->args[i].ptrs[0]) = cs;
        *(int *)(d6->args[i].ptrs[1]) = 100 * i;
        *(unsigned int *)(d6->args[i].ptrs[2]) = mode;
        *(char **)(d6->args[i].ptrs[3]) = NULL;
    }

    return m;
}

bool menu_is_using_target_tex(game_t *g)
{
    if(!g || !g->data)
    {
        return false;
    }

    menudata *d = (menudata *)g->data;
    return d->use_target_tex != 0;
}

int menu_update_target_tex_size(game_t *g, int w, int h)
{
    if(!g || !g->data || w < 0 || h < 0)
    {
        return -1;
    }

    menudata *d = (menudata *)g->data;
    bool usingTarget = false;

    if(d->target_tex != nullptr)
    {
        if(SDL_GetRenderTarget(g->origin->screen.renderer) == d->target_tex)
        {
            usingTarget = true;
            SDL_SetRenderTarget(g->origin->screen.renderer, NULL);
        }

        SDL_DestroyTexture(d->target_tex);
    }

    d->target_tex = SDL_CreateTexture(g->origin->screen.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    if(d->target_tex == nullptr)
    {
        return -1;
    }

    if(usingTarget)
    {
        SDL_SetRenderTarget(g->origin->screen.renderer, d->target_tex);
    }

    for(int i = 0; i < d->numopts; i++)
    {
        d->menu[i].render_update = 1;
    }

    SDL_SetTextureBlendMode(d->target_tex, SDL_BLENDMODE_BLEND);

    return 0;
}

void menu_update_replay_pagination(menudata *d, int from_selection)
{
    if(d == nullptr)
    {
        return;
    }

    if(!d->is_paged)
    {
        return;
    }

    for(size_t i = from_selection; i < d->menu.size(); i++)
    {
        d->menu[i].y = 60 + (i % 20) * 20;
        d->menu[i].label_text_rgba = (i % 2) ? 0xA0A0FFFF : RGBA_DEFAULT_MACRO;
    }

    for(int i = 0; i < d->numopts; i++)
    {
        d->menu[i].render_update = true;
    }

    d->target_tex_update = true;
}

int menu_delete_selected_replay(CoreState *cs, void *)
{
    menudata *d = (menudata *)cs->menu->data;

    if(d->selection < 0 || d->selection >= static_cast<int>(d->menu.size()))
    {
        return 0;
    }

    Shiro::MenuOption m = d->menu[d->selection];
    if(m.type == Shiro::ElementType::MENU_GAME)
    {
        Shiro::GameOptionData *d4 = (Shiro::GameOptionData *)m.data;
        int replay_id = *(int *)(d4->args.ptrs[3]);

        scoredb_delete_replay(&cs->records, replay_id);

        d->menu.erase(d->menu.begin() + d->selection);

        if(d->selection >= static_cast<int>(d->menu.size()))
        {
            d->selection = static_cast<int>(d->menu.size()) - 1;
        }

        d->numopts--;

        menu_update_replay_pagination(d, d->selection);
    }

    return 0;
}

int menu_text_toggle(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

    if(cs->button_emergency_override)
        return 0;

    if(!cs->text_editing)
    {
        cs->text_editing = 1;
        d7->active = 1;
    }
    else
    {
        cs->text_editing = 0;
        d7->active = 0;
    }

    return 0;
}

int menu_text_insert(CoreState *cs, char *str)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    std::string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(str)
    {
        if(d7->selection)
        {
            d7->text = str;
            d7->selection = 0;
            d7->position = static_cast<int>(strlen(str));
            d7->leftmost_position = 0;
            if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != int(d7->text.size()) - d7->visible_chars)
                d7->leftmost_position = d7->position - d7->visible_chars + 1;
        }
        else
        {
            t.insert(d7->position, str);
            if (t.size() > 2000)
            {
                t.resize(2000);
                d7->position = 2000;
            }
            else
                d7->position += static_cast<int>(strlen(str));

            if(d7->position == int(t.size()) && d7->leftmost_position < int(t.size()) - d7->visible_chars)
            {
                d7->leftmost_position = static_cast<int>(t.size() - d7->visible_chars);
            }

            if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != int(d7->text.size()) - d7->visible_chars)
                d7->leftmost_position = d7->position - d7->visible_chars + 1;
        }
    }

    if(d->menu[d->selection].value_update_callback)
        d->menu[d->selection].value_update_callback(cs);

    return 0;
}

int menu_text_backspace(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    std::string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection)
    {
        d7->selection = 0;
        t.resize(0);
        d7->position = 0;
        d7->leftmost_position = 0;
    }
    else if(d7->position > 0)
    {
        t.replace(static_cast<std::size_t>(d7->position) - 1, 1, "");
        d7->position--;
        if(d7->position < d7->leftmost_position + 1 && d7->leftmost_position)
            d7->leftmost_position--;
    }

    if(d->menu[d->selection].value_update_callback)
        d->menu[d->selection].value_update_callback(cs);

    return 0;
}

int menu_text_delete(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    std::string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection)
    {
        d7->selection = 0;
        t.resize(0);
        d7->position = 0;
        d7->leftmost_position = 0;
    }
    else if(d7->position < int(d7->text.size()))
    {
        t.replace(d7->position, 1, "");
    }

    if(d->menu[d->selection].value_update_callback)
        d->menu[d->selection].value_update_callback(cs);

    return 0;
}

int menu_text_seek_left(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    // string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position > 0)
    {
        d7->position--;
        if(d7->position < d7->leftmost_position + 1 && d7->leftmost_position)
            d7->leftmost_position--;
    }

    return 0;
}

int menu_text_seek_right(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    std::string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position < int(t.size()))
    {
        d7->position++;
        if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != int(d7->text.size()) - d7->visible_chars)
            d7->leftmost_position++;
    }

    return 0;
}

int menu_text_seek_home(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

    if(cs->button_emergency_override)
        return 0;

    d7->position = 0;
    d7->leftmost_position = 0;

    return 0;
}

int menu_text_seek_end(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;
    std::string& t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    d7->position = static_cast<int>(t.size());
    d7->leftmost_position = d7->position - d7->visible_chars;
    if(d7->leftmost_position < 0)
        d7->leftmost_position = 0;

    return 0;
}

int menu_text_select_all(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

    if(cs->button_emergency_override)
        return 0;

    d7->selection = !d7->selection;

    return 0;
}

int menu_text_copy(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection)
    {
        SDL_SetClipboardText(d7->text.c_str());
        d7->selection = 0;
    }

    if(d->menu[d->selection].value_update_callback)
        d->menu[d->selection].value_update_callback(cs);

    return 0;
}

int menu_text_cut(CoreState *cs)
{
    menudata *d = (menudata *)cs->menu->data;
    Shiro::TextOptionData *d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection)
    {
        SDL_SetClipboardText(d7->text.c_str());
        d7->selection = 0;
        d7->text.resize(0);
        d7->position = 0;
    }

    if(d->menu[d->selection].value_update_callback)
        d->menu[d->selection].value_update_callback(cs);

    return 0;
}

game_t *menu_create(CoreState *cs)
{
    if(!cs)
        return NULL;

    game_t *g = (game_t *)malloc(sizeof(game_t));
    if(!g)
    {
        throw std::bad_alloc();
    }

    g->origin = cs;
    g->field = NULL;
    g->init = menu_init;
    g->quit = menu_quit;
    g->preframe = NULL;
    g->input = menu_input;
    g->frame = NULL;
    g->draw = gfx_drawmenu;

    g->data = new menudata;
    menudata *d = (menudata *)(g->data);

    d->target_tex = SDL_CreateTexture(cs->screen.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
    SDL_SetTextureBlendMode(d->target_tex, SDL_BLENDMODE_BLEND);
    d->menu_id = -1;
    d->main_menu_data.selection = 0;
    d->main_menu_data.opt_selection = 0;
    d->practice_menu_data.pracdata_mirror = NULL;
    d->practice_menu_data.selection = 0;
    d->selection = 0;
    d->numopts = 0;
    d->is_paged = 0;
    d->page = 0;
    d->page_length = 0;
    d->page_text_x = 0;
    d->page_text_y = 0;
    d->title = "";
    d->x = 0;
    d->y = 0;

    return g;
}

int menu_init(game_t *g)
{
    if (!g) {
        return -1;
    }

    if (mload_main(g, 0)) {
        std::cerr << "Failed to load main menu" << std::endl;
    }

    menudata *d = (menudata *)g->data;

    SDL_SetRenderTarget(g->origin->screen.renderer, d->target_tex);
    SDL_SetTextureBlendMode(d->target_tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
    SDL_RenderClear(g->origin->screen.renderer);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 255);
    SDL_SetRenderTarget(g->origin->screen.renderer, NULL);

    g->origin->bg.transition(Shiro::ImageAsset::get(g->origin->assetMgr, "bg_temp"));

    return 0;
}

int menu_quit(game_t *g)
{
    if(!g)
        return -1;

    menudata *d = (menudata *)(g->data);
    int i = 0;

    if (d->menu.size()) {
        for (i = 0; i < d->numopts; i++) {
            Shiro::destroy_menu_option(d->menu[i]);
        }
    }

    delete d;

    return 0;
}

int menu_input(game_t *g)
{
    if(!g)
        return -1;

    CoreState *cs = g->origin;

    menudata *d = (menudata *)(g->data);

    Shiro::MenuOption *m = NULL;

    Shiro::ActionOptionData *d1 = NULL;
    Shiro::MultiOptionData *d2 = NULL;
    Shiro::ToggleOptionData *d3 = NULL;
    Shiro::GameOptionData *d4 = NULL;
    // Shiro::MetaGameOptionData *d5 = NULL;
    Shiro::GameMultiOptionData *d6 = NULL;
    Shiro::TextOptionData *d7 = NULL;

    int i = 0;
    bool update = d->target_tex_update;

    const int DAS = 18;

    if(cs->text_editing)
    {
        return 0;
    }

    if(cs->pressed.escape == 1)
    {
        if(!(d->menu_id == MENU_ID_MAIN))
        {
            mload_main(g, 0);
            return 0;
        }
    }

    if(!d->menu.size())
        return 0;

    if(d->menu[d->selection].type != Shiro::ElementType::MENU_TEXTINPUT)
    {
        cs->text_toggle = NULL;
        cs->text_insert = NULL;
        cs->text_backspace = NULL;
        cs->text_delete = NULL;
        cs->text_seek_left = NULL;
        cs->text_seek_right = NULL;
        cs->text_seek_home = NULL;
        cs->text_seek_end = NULL;
        cs->text_select_all = NULL;
        cs->text_copy = NULL;
        cs->text_cut = NULL;
    }

    if((cs->pressed.up || cs->is_up_input_repeat(DAS)) && d->selection > 0 && !(SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)))
    {
        update = true;
        for(i = d->selection - 1;; i--)
        {
            if(d->is_paged)
            {
                if(d->selection == d->page * d->page_length)
                {
                    break;
                }
            }

            if(i == -1)
                i = d->numopts - 1;
            if(d->menu[i].type != Shiro::ElementType::MENU_LABEL)
            {
                d->selection = i;
                if(cs->pressed.up == 1) {
                    Shiro::SfxAsset::get(cs->assetMgr, "menu_choose").play(cs->settings);
                }
                if(d->menu[d->selection].type == Shiro::ElementType::MENU_TEXTINPUT)
                {
                    cs->text_toggle = menu_text_toggle;
                    cs->text_insert = menu_text_insert;
                    cs->text_backspace = menu_text_backspace;
                    cs->text_delete = menu_text_delete;
                    cs->text_seek_left = menu_text_seek_left;
                    cs->text_seek_right = menu_text_seek_right;
                    cs->text_seek_home = menu_text_seek_home;
                    cs->text_seek_end = menu_text_seek_end;
                    cs->text_select_all = menu_text_select_all;
                    cs->text_copy = menu_text_copy;
                    cs->text_cut = menu_text_cut;
                }
                else
                {
                    cs->text_toggle = NULL;
                    cs->text_insert = NULL;
                    cs->text_backspace = NULL;
                    cs->text_delete = NULL;
                    cs->text_seek_left = NULL;
                    cs->text_seek_right = NULL;
                    cs->text_seek_home = NULL;
                    cs->text_seek_end = NULL;
                    cs->text_select_all = NULL;
                    cs->text_copy = NULL;
                    cs->text_cut = NULL;
                }

                break;
            }
        }
    }

    if((cs->pressed.down || cs->is_down_input_repeat(DAS)) && (d->selection < d->numopts - 1) && !(SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)))
    {
        update = true;
        for(i = d->selection + 1;; i++)
        {
            if(d->is_paged)
            {
                if(d->selection == (d->page + 1) * d->page_length - 1)
                {
                    break;
                }
            }

            if(i == d->numopts)
            {
                do
                {
                    i--;
                } while(d->menu[i].type == Shiro::ElementType::MENU_LABEL);

                break;
            }

            if(d->menu[i].type != Shiro::ElementType::MENU_LABEL)
            {
                d->selection = i;
                if(cs->pressed.down == 1) {
                    Shiro::SfxAsset::get(cs->assetMgr, "menu_choose").play(cs->settings);
                }
                if(d->menu[d->selection].type == Shiro::ElementType::MENU_TEXTINPUT)
                {
                    cs->text_toggle = menu_text_toggle;
                    cs->text_insert = menu_text_insert;
                    cs->text_backspace = menu_text_backspace;
                    cs->text_delete = menu_text_delete;
                    cs->text_seek_left = menu_text_seek_left;
                    cs->text_seek_right = menu_text_seek_right;
                    cs->text_seek_home = menu_text_seek_home;
                    cs->text_seek_end = menu_text_seek_end;
                    cs->text_select_all = menu_text_select_all;
                    cs->text_copy = menu_text_copy;
                    cs->text_cut = menu_text_cut;
                }
                else
                {
                    cs->text_toggle = NULL;
                    cs->text_insert = NULL;
                    cs->text_backspace = NULL;
                    cs->text_delete = NULL;
                    cs->text_seek_left = NULL;
                    cs->text_seek_right = NULL;
                    cs->text_seek_home = NULL;
                    cs->text_seek_end = NULL;
                    cs->text_select_all = NULL;
                    cs->text_copy = NULL;
                    cs->text_cut = NULL;
                }

                break;
            }
        }
    }

    if(d->is_paged)
    {
        if((cs->pressed.left || cs->is_left_input_repeat(DAS)) && d->page > 0 && !(SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)))
        {
            update = true;
            d->selection = d->selection - d->page_length;
            d->page--;
        }

        if((cs->pressed.right || cs->is_right_input_repeat(DAS)) && d->page < ((d->numopts - 1) / d->page_length) && !(SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)))
        {
            update = true;
            d->selection = d->selection + d->page_length;
            d->page++;

            if(d->selection >= d->numopts)
                d->selection = d->numopts - 1;
        }
    }

    if(d->menu_id == MENU_ID_MAIN)
        d->main_menu_data.selection = d->selection;
    else if(d->menu_id == MENU_ID_PRACTICE)
        d->practice_menu_data.selection = d->selection;

    if(d->menu[d->selection].type != Shiro::ElementType::MENU_TEXTINPUT)
    {
        cs->text_toggle = NULL;
        cs->text_insert = NULL;
        cs->text_backspace = NULL;
        cs->text_delete = NULL;
        cs->text_seek_left = NULL;
        cs->text_seek_right = NULL;
        cs->text_seek_home = NULL;
        cs->text_seek_end = NULL;
        cs->text_select_all = NULL;
        cs->text_copy = NULL;
        cs->text_cut = NULL;
    }

    m = &d->menu[d->selection];

    if(!(SDL_GetModState() & (KMOD_CTRL | KMOD_SHIFT)))
    {
        switch(m->type)
        {
            case Shiro::ElementType::MENU_ACTION:
                d1 = (Shiro::ActionOptionData *)d->menu[d->selection].data;

                if(cs->pressed.a == 1 || cs->pressed.start == 1)
                {
                    if(d1->action)
                    {
                        int quitStatus = d1->action(g, d1->val);
                        if(quitStatus == 1)
                        {
                            std::cerr << "Received quit signal, shutting down." << std::endl;
                            return 1;
                        }
                        else if (quitStatus == 2) {
                            return 2;
                        }

                        return 0;
                    }
                }

                break;

            case Shiro::ElementType::MENU_MULTIOPT:
                d2 = (Shiro::MultiOptionData *)d->menu[d->selection].data;

                if(!d->is_paged)
                {

                    if((cs->pressed.left || cs->is_left_input_repeat(DAS)) && d2->selection > 0)
                    {
                        d2->selection--;
                        *(d2->param) = d2->vals[d2->selection];
                        if(d->menu[d->selection].value_update_callback)
                            d->menu[d->selection].value_update_callback(cs);
                    }

                    if((cs->pressed.right || cs->is_right_input_repeat(DAS)) && d2->selection < (d2->num - 1))
                    {
                        d2->selection++;
                        *(d2->param) = d2->vals[d2->selection];
                        if(d->menu[d->selection].value_update_callback)
                            d->menu[d->selection].value_update_callback(cs);
                    }
                }

                break;

            case Shiro::ElementType::MENU_TEXTINPUT:
                d7 = (Shiro::TextOptionData *)d->menu[d->selection].data;

                if(d7->leftmost_position < 0)
                    d7->leftmost_position = 0;
                if(d7->position < 0)
                    d7->position = 0;
                if(d7->position > int(d7->text.size()))
                    d7->position = int(d7->text.size());

                break;

            case Shiro::ElementType::MENU_TOGGLE:
                d3 = (Shiro::ToggleOptionData *)d->menu[d->selection].data;

                if(!d->is_paged)
                {
                    if(cs->pressed.a || cs->pressed.left || cs->pressed.right)
                    {
                        *(d3->param) = *(d3->param) ? false : true;
                        if(d->menu[d->selection].value_update_callback)
                            d->menu[d->selection].value_update_callback(cs);
                    }
                }

                break;

            case Shiro::ElementType::MENU_GAME:
                d4 = (Shiro::GameOptionData *)d->menu[d->selection].data;

                if(cs->pressed.a == 1 || cs->pressed.start == 1)
                {
                    switch(d4->mode)
                    {
                        case QUINTESSE:
                            if(d4->args.ptrs)
                            {
                                if(d4->args.ptrs[0] && d4->args.ptrs[1] && d4->args.ptrs[2] && d4->args.ptrs[3])
                                {
                                    g->origin->p1game = qs_game_create(*((CoreState **)(d4->args.ptrs[0])),
                                                                       *((int *)(d4->args.ptrs[1])),
                                                                       *((unsigned int *)(d4->args.ptrs[2])),
                                                                       *((int *)(d4->args.ptrs[3])));
                                    if(g->origin->p1game)
                                    {
                                        g->origin->p1game->init(g->origin->p1game);

                                        return 0;
                                    }
                                }
                            }
                            else
                            {
                                g->origin->p1game = qs_game_create(g->origin, 0, 0, NO_REPLAY);
                                if(g->origin->p1game)
                                {
                                    g->origin->p1game->init(g->origin->p1game);

                                    return 0;
                                }
                            }

                            break;

                        default:
                            break;
                    }
                }

                break;

            case Shiro::ElementType::MENU_GAME_MULTIOPT:
                d6 = (Shiro::GameMultiOptionData *)d->menu[d->selection].data;

                if(!d->is_paged)
                {
                    if((cs->pressed.left == 1 || cs->is_left_input_repeat(DAS)) && d6->selection > 0)
                    {
                        d6->selection--;
                    }

                    if((cs->pressed.right == 1 || cs->is_right_input_repeat(DAS)) && d6->selection < (d6->num - 1))
                    {
                        d6->selection++;
                    }

                    if(d->menu_id == MENU_ID_MAIN)
                        d->main_menu_data.opt_selection = d6->selection;
                }

                if(cs->pressed.a == 1 || cs->pressed.start == 1)
                {
                    switch(d6->mode)
                    {
                        case QUINTESSE:
                            if(d6->args[d6->selection].ptrs)
                            {
                                if(d6->args[d6->selection].ptrs[0] && d6->args[d6->selection].ptrs[1] && d6->args[d6->selection].ptrs[2] &&
                                   d6->args[d6->selection].ptrs[3])
                                {
                                    g->origin->p1game = qs_game_create(*((CoreState **)(d6->args[d6->selection].ptrs[0])),
                                                                       *((int *)(d6->args[d6->selection].ptrs[1])),
                                                                       *((unsigned int *)(d6->args[d6->selection].ptrs[2])),
                                                                       NO_REPLAY);
                                    if(g->origin->p1game)
                                    {
                                        g->origin->p1game->init(g->origin->p1game);

                                        return 0;
                                    }
                                }
                            }
                            else
                            {
                                g->origin->p1game = qs_game_create(g->origin, 0, 0, NO_REPLAY);
                                if(g->origin->p1game)
                                {
                                    g->origin->p1game->init(g->origin->p1game);

                                    return 0;
                                }
                            }

                            break;

                        default:
                            break;
                    }
                }

                break;

            default:
                break;
        }
    }

    // process mouse for menudata->menuButtons here

    if(d->menuButtons.size() > 0)
    {
        for(auto it = d->menuButtons.begin(); it != d->menuButtons.end(); it++) {
            gfx_button& b = *it;

            if(!b.active)
            {
                if(b.activate_check)
                {
                    if(b.activate_check(cs))
                    {
                        b.active = true;
                    }
                }

                continue;
            }

            if(b.deactivate_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
            {
                if(b.deactivate_check(cs))
                {
                    b.active = false;
                    if(b.visible)
                    {
                        update = true;
                    }

                    continue;
                }
            }

            if(cs->button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
            {
                b.highlighted = 0;
                if(b.deactivate_check)
                {
                    if(b.deactivate_check(cs))
                    {
                        b.active = false;
                        if(b.visible)
                        {
                            update = true;
                        }
                    }
                }

                continue;
            }

            if(static_cast<std::size_t>(cs->mouse.logicalX) < static_cast<std::size_t>(b.x + b.w) && static_cast<std::size_t>(cs->mouse.logicalX) >= static_cast<std::size_t>(b.x) &&
               static_cast<std::size_t>(cs->mouse.logicalY) < static_cast<std::size_t>(b.y + b.h) && static_cast<std::size_t>(cs->mouse.logicalY) >= static_cast<std::size_t>(b.y))
                b.highlighted = 1;
            else
                b.highlighted = 0;

            if(b.highlighted && cs->mouse.leftButton == Shiro::Mouse::Button::pressedThisFrame)
            {
                if(b.type == BUTTON_TYPE_ACTION && b.action)
                {
                    b.action(cs, b.data);
                }
                else if(b.type == BUTTON_TYPE_TOGGLE && b.boolPtr)
                {
                    (*b.boolPtr) = !(*b.boolPtr);
                    b.toggleValue = !b.toggleValue;

                    if(b.toggleValue == false)
                        b.text = b.toggleOffText;
                    else
                        b.text = b.toggleOnText;

                    b.w = 2 * 6 + 15 * (b.text.size());
                }

                b.clicked = 4;
            }
            if (b.clicked) {
                b.clicked--;
            }
        }

        for (auto it = d->menuButtons.begin(); it != d->menuButtons.end(); it++) {
            gfx_button& b = *it;

            if(cs->button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
            {
                if(b.deactivate_check)
                {
                    if(b.deactivate_check(cs))
                    {
                        b.active = false;
                    }
                }

                continue;
            }

            if(b.deactivate_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
            {
                if(b.deactivate_check(cs))
                {
                    b.active = false;
                }
            }
        }
    }

    if(update && d->use_target_tex)
    {
        for(i = 0; i < d->numopts; i++)
        {
            d->menu[i].render_update = true;
        }
    }

    d->target_tex_update = false;

    return 0;
}

int menu_frame(game_t *) // nothing right now
{
    return 0;
}

int menu_clear(game_t *g)
{
    if(!g)
        return -1;

    menudata *d = (menudata *)(g->data);
    SDL_SetRenderTarget(g->origin->screen.renderer, d->target_tex);
    SDL_SetTextureBlendMode(d->target_tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
    SDL_RenderClear(g->origin->screen.renderer);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 255);
    SDL_SetRenderTarget(g->origin->screen.renderer, NULL);

    int i = 0;

    if(!d->menu.size())
        return 0;

    for(i = 0; i < d->numopts; i++)
    {
        Shiro::destroy_menu_option(d->menu[i]);
    }

    d->menu.clear();
    d->menuButtons.clear();
    d->use_target_tex = 0;

    d->title = "";

    d->x = 0;
    d->y = 0;
    d->selection = 0;
    d->numopts = 0;
    d->is_paged = 0;

    return 0;
}

int mload_main(game_t *g, int)
{
    if(!g)
        return -1;

    CoreState *cs = g->origin;
    menudata *d = (menudata *)(g->data);
    Shiro::MenuOption *m = NULL;
    Shiro::ActionOptionData *d1 = NULL;
    Shiro::MultiOptionData *d2 = NULL;
    Shiro::GameMultiOptionData *d6 = NULL;
    int i = 0;

    cs->request_fps(Shiro::RefreshRates::menu);

    if(d->menu_id == MENU_ID_MAIN)
    {
        return 0;
    }

    menu_clear(g); // data->menu guaranteed to be NULL upon return
    if(cs->p1game)
    {
        cs->p1game->quit(cs->p1game);
        free(cs->p1game);
        cs->p1game = NULL;
    }

    cs->bg.transition(Shiro::ImageAsset::get(cs->assetMgr, "bg_temp"));

    //d->menu.resize(16, menu_opt());
    d->menu_id = MENU_ID_MAIN;
    d->use_target_tex = 0;
    d->selection = d->main_menu_data.selection;
    d->title = "MAIN MENU";
    d->x = 4 * 16;
    d->y = 3 * 16;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_PENTOMINO, 12, "PENTOMINO C"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;

    d6->num++;
    d6->labels.push_back("ACID RAIN");
    assert(d6->args != nullptr);
    void* argsTemp = realloc(d6->args, d6->num * sizeof(Shiro::GameArguments));
    if(!argsTemp)
    {
        throw std::bad_alloc();
    }
    d6->args = (Shiro::GameArguments*)argsTemp;

    d6->args[d6->num - 1].num = 4;
    d6->args[d6->num - 1].ptrs = (void **)malloc(4 * sizeof(void *));
    assert(d6->args[d6->num - 1].ptrs != nullptr);
    d6->args[d6->num - 1].ptrs[0] = malloc(sizeof(CoreState *));
    assert(d6->args[d6->num - 1].ptrs[0] != nullptr);
    d6->args[d6->num - 1].ptrs[1] = malloc(sizeof(int));
    assert(d6->args[d6->num - 1].ptrs[1] != nullptr);
    d6->args[d6->num - 1].ptrs[2] = malloc(sizeof(unsigned int));
    assert(d6->args[d6->num - 1].ptrs[2] != nullptr);
    d6->args[d6->num - 1].ptrs[3] = malloc(sizeof(char *));
    assert(d6->args[d6->num - 1].ptrs[3] != nullptr);
    *(CoreState **)(d6->args[d6->num - 1].ptrs[0]) = cs;
    *(int *)(d6->args[d6->num - 1].ptrs[1]) = 1500;
    *(unsigned int *)(d6->args[d6->num - 1].ptrs[2]) = MODE_PENTOMINO;
    *(char **)(d6->args[d6->num - 1].ptrs[3]) = NULL;

    d6->num++;
    d6->labels.push_back("ULTIMATE ACID RAIN");
    void* argsTemp2 = realloc(d6->args, d6->num * sizeof(Shiro::GameArguments));
    if(!argsTemp2)
    {
        throw std::bad_alloc();
    }
    d6->args = (Shiro::GameArguments*)argsTemp2;

    d6->args[d6->num - 1].num = 4;
    if (
        !(d6->args[d6->num - 1].ptrs = (void**)malloc(4 * sizeof(void*))) ||
        !(d6->args[d6->num - 1].ptrs[0] = malloc(sizeof(CoreState*))) ||
        !(d6->args[d6->num - 1].ptrs[1] = malloc(sizeof(int))) ||
        !(d6->args[d6->num - 1].ptrs[2] = malloc(sizeof(unsigned int))) ||
        !(d6->args[d6->num - 1].ptrs[3] = malloc(sizeof(char*))))
    {
        throw std::bad_alloc();
    }
    *(CoreState **)(d6->args[d6->num - 1].ptrs[0]) = cs;
    *(int *)(d6->args[d6->num - 1].ptrs[1]) = 1700;
    *(unsigned int *)(d6->args[d6->num - 1].ptrs[2]) = MODE_PENTOMINO;
    *(char **)(d6->args[d6->num - 1].ptrs[3]) = NULL;

    if(d->main_menu_data.selection == 0)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 7 * 16;
    m->value_x = m->x + 12 * 16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G1_MASTER, 10, "G1 MASTER"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 1)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 8 * 16;
    m->value_x = m->x + 10 * 16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G1_20G, 10, "G1 20G"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 2)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 9 * 16;
    m->value_x = m->x + 10 * 16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G2_MASTER, 10, "G2 MASTER"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 3)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 10 * 16;
    m->value_x = m->x + 10 * 16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFFFF40FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G2_DEATH, 10, "G2 DEATH"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 4)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 11 * 16;
    m->value_x = m->x + 10 * 16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFF4040FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G3_TERROR, 13, "G3 TERROR"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 5)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 12 * 16;
    m->value_x = m->x + 10 * 16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFF4040FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(std_game_multiopt_create(g->origin, MODE_G2_MASTER | static_cast<int>(Shiro::GameType::BIG_MODE), 10, "BIG MASTER"));
    m = &d->menu.back();
    d6 = (Shiro::GameMultiOptionData *)m->data;
    if(d->main_menu_data.selection == 6)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4 * 16;
    m->y = 13 * 16;
    m->value_x = m->x + 11 * 16;
    m->value_y = m->y;
    m->label_text_rgba = 0x40FF40FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "REPLAY"));
    m = &d->menu.back();
    d1 = (Shiro::ActionOptionData *)m->data;
    d1->action = mload_replay;
    d1->val = 0;
    m->x = 4 * 16;
    m->y = 15 * 16;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "MULTI-EDITOR"));
    m = &d->menu.back();
    d1 = (Shiro::ActionOptionData *)m->data;
    d1->action = mload_practice;
    d1->val = 0;
    m->x = 4 * 16;
    m->y = 17 * 16;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_LABEL, NULL, "SETTINGS"));
    m = &d->menu.back();
    m->x = 4 * 16;
    m->y = 20 * 16;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "MASTER VOLUME"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 101;
    d2->param = &g->origin->settings.masterVolume;
    d2->vals = (int *)malloc(101 * sizeof(int));
    if(!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.clear();
    for(i = 0; i < 101; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels.push_back(ss.str());
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings.masterVolume;
    m->x = 4 * 16;
    m->y = 22 * 16;
    m->value_x = 21 * 16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT | DRAWTEXT_VALUE_BAR;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "SFX VOLUME"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 101;
    d2->param = &g->origin->settings.sfxVolume;
    d2->vals = (int *)malloc(101 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.clear();
    for(i = 0; i < 101; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels.push_back(ss.str());
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings.sfxVolume;
    m->x = 4 * 16;
    m->y = 23 * 16;
    m->value_x = 21 * 16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT | DRAWTEXT_VALUE_BAR;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "MUSIC VOLUME"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 101;
    d2->param = &g->origin->settings.musicVolume;
    d2->vals = (int *)malloc(101 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.clear();
    for(i = 0; i < 101; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels.push_back(ss.str());
        assert(d2->vals != nullptr);
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings.musicVolume;
    m->x = 4 * 16;
    m->y = 24 * 16;
    m->value_x = 21 * 16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT | DRAWTEXT_VALUE_BAR;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "QUIT"));
    m = &d->menu.back();
    d1 = (Shiro::ActionOptionData *)m->data;
    d1->action = menu_action_quit;
    d1->val = 0;
    m->x = 4 * 16;
    m->y = 27 * 16;

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_LABEL, NULL, "Pentomino C rev 1.5"));
    m = &d->menu.back();
    m->x = 638 - (19 * 15);
    m->y = 2;
    m->label_text_rgba = 0x808080A0;

    d->numopts = d->menu.size();

    return 0;
}

int mload_practice(game_t *g, int)
{
    CoreState *cs = g->origin;
    menudata *d = (menudata *)(g->data);
    Shiro::MenuOption *m = NULL;
    Shiro::ActionOptionData *d1 = NULL;
    Shiro::MultiOptionData *d2 = NULL;
    Shiro::TextOptionData *d7 = NULL;
    Shiro::ToggleOptionData *d8 = NULL;
    int i = 0;
    int pracdata_mirror_existed = 0;

    int grav_ = 0;
    int lock_ = 0;
    int are_ = 0;
    int lineare_ = 0;
    int lineclear_ = 0;
    int das_ = 0;
    int garbage_delay_ = 0;
    int width_ = 0;

    auto game_type_ = Shiro::GameType::SIMULATE_QRS;
    int lock_protect_ = 0;

    // standard x values for the menu option labels and values
    int optsX = 17 * 16;
    int optsValueX = optsX + 27 * 8;

    // y value for the first menu option, then incremented by multiples of 16
    int optsY = 5 * 16;

    // TODO: piece sequence restore from pracdata struct (need to save char* that the user enters)

    menu_clear(g); // data->menu guaranteed to be NULL upon return

    cs->menu_input_override = 1;

    cs->p1game = qs_game_create(cs, 0, QRS_PRACTICE | TETROMINO_ONLY, NO_REPLAY);
    cs->p1game->init(cs->p1game);

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(!d->practice_menu_data.pracdata_mirror)
        d->practice_menu_data.pracdata_mirror = q->pracdata;
    else
        pracdata_mirror_existed = 1;

    //cs->bg.transition();
    cs->bg.transition(Shiro::ImageAsset::get(cs->assetMgr, "multi_editor_bg"));

    Shiro::MusicAsset::get(cs->assetMgr, "multi_editor_bgm").play(cs->settings);

    d->menu_id = MENU_ID_PRACTICE;
    d->selection = 0;
    d->title = ""; // "PRACTICE";
    d->x = 20 * 16;
    d->y = 2 * 16;

    //
    /* */
    //

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "RETURN"));
    m = &d->menu.back();
    d1 = (Shiro::ActionOptionData *)m->data;
    d1->action = mload_main;
    d1->val = 0;
    m->x = optsX + 10 * 8;
    m->y = optsY - 48;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;

    //
    /* */
    //

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "PLAY"));
    m = &d->menu.back();
    d1 = (Shiro::ActionOptionData *)m->data;
    d1->action = qs_game_pracinit;
    d1->val = 0;
    m->x = optsX + 11 * 8;
    m->y = optsY - 32;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;

    //
    /* */
    //

    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, qs_update_pracdata, "GOAL LEVEL"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 22; // OFF, 100, 200, 300, 400, ..., 900, 999, 1000, ..., 1900, 2000 -> 22
    d2->param = &q->pracdata->goal_level;
    d2->vals = (int *)malloc(d2->num * sizeof(int));
    d2->labels.resize(d2->num);

    d2->labels[0] = "OFF";
    d2->vals[0] = 0;

    for(i = 1; i < d2->num; i++)
    {
        if(i == 10)
        {
            d2->labels[i] = "999";
            d2->vals[i] = 999;

            continue;
        }

        int n_ = i;

        if(i > 10)
            n_--;

        std::stringstream ss;
        ss << 100 * n_;
        d2->labels[i] = ss.str();
        d2->vals[i] = 100 * n_;
    }

    if(pracdata_mirror_existed)
    {
        int goal_level_ = q->pracdata->goal_level;

        if(goal_level_ == 0)
        {
            d2->selection = 0;
        }
        else
        {
            d2->selection = goal_level_ / 100;

            if(d2->selection >= d2->num || d2->selection < 0)
            {
                d2->selection = 0;
            }
        }
    }
    else
    {
        d2->selection = 0;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFFFFAFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, qs_update_pracdata, "GOAL TIME"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 26; // OFF, 20.00, 30.00, 54.00, 1:00, 1:01.68, 1:30, 2:00, 3:00, ..., 19:00, 20:00 -> 26
    d2->param = &q->pracdata->goal_time;
    d2->vals = (int *)malloc(d2->num * sizeof(int));
    d2->labels.resize(d2->num);

    d2->labels[0] = "OFF";
    d2->vals[0] = 0;

    for(i = 1; i < d2->num; i++)
    {
        if(i == 1)
        {
            d2->labels[i] = "20.00";
            d2->vals[i] = 20 * 60;
        }
        else if(i == 2)
        {
            d2->labels[i] = "30.00";
            d2->vals[i] = 30 * 60;
        }
        else if(i == 3)
        {
            d2->labels[i] = "54.00";
            d2->vals[i] = 54 * 60;
        }
        else if(i == 4)
        {
            d2->labels[i] = "1:00";
            d2->vals[i] = (1 * 60 * 60);
        }
        else if(i == 5)
        {
            d2->labels[i] = "1:01.68";
            d2->vals[i] = (1 * 60 * 60) + (1 * 60) + 41;
        }
        else if(i == 6)
        {
            d2->labels[i] = "1:30";
            d2->vals[i] = (1 * 60 * 60) + (30 * 60);
        }
        else
        {
            std::stringstream ss;
            ss << (i - 5) << ":00";
            d2->labels[i] = ss.str();
            d2->vals[i] = ((i - 5) * 60 * 60);
        }
    }

    if(pracdata_mirror_existed)
    {
        int goal_time_ = q->pracdata->goal_time;

        if(goal_time_ == 0)
        {
            d2->selection = 0;
        }
        else
        {
            if(goal_time_ == 20 * 60)
            {
                d2->selection = 1;
            }
            else if(goal_time_ == 30 * 60)
            {
                d2->selection = 2;
            }
            else if(goal_time_ == 54 * 60)
            {
                d2->selection = 3;
            }
            else if(goal_time_ == (1 * 60 * 60))
            {
                d2->selection = 4;
            }
            else if(goal_time_ == (1 * 60 * 60) + (1 * 60) + 41)
            {
                d2->selection = 5;
            }
            else if(goal_time_ == (1 * 60 * 60) + (30 * 60))
            {
                d2->selection = 6;
            }
            else
            {
                int n_ = goal_time_ / (60 * 60);
                d2->selection = n_ + 5;

                if(d2->selection >= d2->num || d2->selection < 0)
                {
                    d2->selection = 0;
                }
            }
        }
    }
    else
    {
        d2->selection = 0;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xCFCFFFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 24;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "GRAVITY"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 135;
    d2->param = &q->pracdata->usr_timings->grav;
    d2->vals = (int *)malloc(135 * sizeof(int));
    d2->labels.resize(135);
    for(i = 0; i < 128; i++)
    {
        std::stringstream ss;
        ss << 2 * i;
        d2->labels[i] = ss.str();
        d2->vals[i] = 2 * i;
    }

    d2->labels[128] = "1G";
    d2->vals[128] = 256;
    d2->labels[129] = "1.5G";
    d2->vals[129] = 256 + 128;
    d2->labels[130] = "2G";
    d2->vals[130] = 2 * 256;
    d2->labels[131] = "3G";
    d2->vals[131] = 3 * 256;
    d2->labels[132] = "4G";
    d2->vals[132] = 4 * 256;
    d2->labels[133] = "5G";
    d2->vals[133] = 5 * 256;
    d2->labels[134] = "20G";
    d2->vals[134] = 20 * 256;

    if(pracdata_mirror_existed)
    {
        grav_ = q->pracdata->usr_timings->grav;
        if(grav_ <= 256)
        {
            d2->selection = (grav_ / 2) < 0 ? 0 : (grav_ / 2);
        }
        else if(grav_ == 256 + 128)
        {
            d2->selection = 129;
        }
        else if(grav_ == 2 * 256)
        {
            d2->selection = 130;
        }
        else if(grav_ == 3 * 256)
        {
            d2->selection = 131;
        }
        else if(grav_ == 4 * 256)
        {
            d2->selection = 132;
        }
        else if(grav_ == 5 * 256)
        {
            d2->selection = 133;
        }
        else
        {
            d2->selection = 134;
        }
    }
    else
    {
        d2->selection = 134;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x70FF70FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "LOCK"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 101;
    d2->param = &q->pracdata->usr_timings->lock;
    d2->vals = (int *)malloc(101 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(101);
    d2->labels[0] = "OFF";
    d2->vals[0] = -1;
    for(i = 1; i < 101; i++)
    {
        std::stringstream ss;
        ss << i - 1;
        d2->labels[i] = ss.str();
        d2->vals[i] = i - 1;
    }

    if(pracdata_mirror_existed)
    {
        lock_ = q->pracdata->usr_timings->lock;
        d2->selection = lock_ + 1;
    }
    else
    {
        d2->selection = 31;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFF5050FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "ARE"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->are;
    d2->vals = (int *)malloc(100 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(100);
    for(i = 0; i < 100; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels[i] = ss.str();
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed)
    {
        are_ = q->pracdata->usr_timings->are;
        d2->selection = are_;
    }
    else
    {
        d2->selection = 12;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFFA030FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "LINE ARE"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->lineare;
    d2->vals = (int *)malloc(100 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(100);
    for(i = 0; i < 100; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels[i] = ss.str();
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed)
    {
        lineare_ = q->pracdata->usr_timings->lineare;
        d2->selection = lineare_;
    }
    else
    {
        d2->selection = 6;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFFFF20FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "LINE CLEAR"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->lineclear;
    d2->vals = (int *)malloc(100 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(100);
    for(i = 0; i < 100; i++)
    {
        std::stringstream ss;
        ss << i;
        d2->labels[i] = ss.str();
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed)
    {
        lineclear_ = q->pracdata->usr_timings->lineclear;
        d2->selection = lineclear_;
    }
    else
    {
        d2->selection = 6;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x8080FFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "DAS"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 99;
    d2->param = &q->pracdata->usr_timings->das;
    d2->vals = (int *)malloc(99 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(99);
    for(i = 0; i < 99; i++)
    {
        std::stringstream ss;
        ss << i + 1;
        d2->labels[i] = ss.str();
        d2->vals[i] = i + 1;
    }

    if(pracdata_mirror_existed)
    {
        das_ = q->pracdata->usr_timings->das;
        d2->selection = das_ - 1;
    }
    else
    {
        d2->selection = 7;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFF00FFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "GARBAGE DELAY"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 101;
    d2->param = &q->pracdata->garbage_delay;
    d2->vals = (int *)malloc(101 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(101);

    d2->vals[0] = -1;
    d2->labels[0] = "OFF";

    for(i = 1; i < 101; i++)
    {
        std::stringstream ss;
        ss << i - 1;
        d2->labels[i] = ss.str();
        d2->vals[i] = i - 1;
    }

    if(pracdata_mirror_existed)
    {
        garbage_delay_ = q->pracdata->garbage_delay;
        d2->selection = garbage_delay_ + 1;
    }
    else
    {
        d2->selection = 0;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFF30A0FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    optsY += 24;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, qs_update_pracdata, "WIDTH"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 5;
    d2->param = &q->pracdata->field_w;
    d2->vals = (int *)malloc(5 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.resize(5);
    for(i = 0; i < 5; i++)
    {
        std::stringstream ss;
        ss << 2 * (i + 2);
        d2->labels[i] = ss.str();
        d2->vals[i] = 2 * (i + 2);
    }

    if(pracdata_mirror_existed)
    {
        width_ = q->field_w; // TODO: have q->field_w and q->game_type save in the pracdata struct so they can be restored here
        d2->selection = (width_ / 2) - 2;
    }
    else
    {
        d2->selection = 3;
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, qs_update_pracdata, "GAME TYPE"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 4;

    // TODO: This will be rewritten in another way.
    //d2->param = &q->pracdata->game_type;
    d2->param = &q->pracdata->game_type_int;

    d2->vals = (int *)malloc(4 * sizeof(int));
    if (!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.clear();
    d2->labels.resize(4);
    d2->labels.resize(4);
    d2->labels[0] = "QRS";
    d2->labels[1] = "G1";
    d2->labels[2] = "G2";
    d2->labels[3] = "G3";
    /*
    d2->vals[0] = static_cast<int>(Shiro::GameType::SIMULATE_QRS);
    d2->vals[1] = static_cast<int>(Shiro::GameType::SIMULATE_G1);
    d2->vals[2] = static_cast<int>(Shiro::GameType::SIMULATE_G2);
    d2->vals[3] = static_cast<int>(Shiro::GameType::SIMULATE_G3);
    */

    d2->vals[0] = 0;
    d2->vals[1] = 1;
    d2->vals[2] = 2;
    d2->vals[3] = 3;

    if(pracdata_mirror_existed)
    {
        game_type_ = q->game_type;
        if(game_type_ == Shiro::GameType::SIMULATE_QRS)
        {
            d2->selection = 0;
        }
        else if(game_type_ == Shiro::GameType::SIMULATE_G1)
        {
            d2->selection = 1;
        }
        else if(game_type_ == Shiro::GameType::SIMULATE_G2)
        {
            d2->selection = 2;
        }
        else
        {
            d2->selection = 3;
        }
    }
    else
    {
        d2->selection = 2;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TOGGLE, NULL, "INVISIBLE"));
    m = &d->menu.back();
    d8 = (Shiro::ToggleOptionData *)m->data;
    d8->param = &q->pracdata->invisible;
    d8->labels[0] = "OFF";
    d8->labels[1] = "ON";
    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TOGGLE, qs_update_pracdata, "BRACKETS"));
    m = &d->menu.back();
    d8 = (Shiro::ToggleOptionData *)m->data;
    d8->param = &q->pracdata->brackets;
    d8->labels[0] = "OFF";
    d8->labels[1] = "ON";
    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TOGGLE, NULL, "INFINITE FLOORKICKS"));
    m = &d->menu.back();
    d8 = (Shiro::ToggleOptionData *)m->data;
    d8->param = &q->pracdata->infinite_floorkicks;
    d8->labels[0] = "OFF";
    d8->labels[1] = "ON";
    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_MULTIOPT, NULL, "LOCK PROTECTION"));
    m = &d->menu.back();
    d2 = (Shiro::MultiOptionData *)m->data;
    d2->num = 3;
    d2->param = &q->pracdata->lock_protect;
    d2->vals = (int *)malloc(3 * sizeof(int));
    if(!d2->vals)
    {
        throw std::bad_alloc();
    }
    d2->labels.clear();
    d2->labels.resize(3);
    d2->labels[0] = "AUTO";
    d2->labels[1] = "OFF";
    d2->labels[2] = "ON";
    d2->vals[0] = -1;
    d2->vals[1] = 0;
    d2->vals[2] = 1;

    if(pracdata_mirror_existed)
    {
        lock_protect_ = q->pracdata->lock_protect;
        d2->selection = lock_protect_ + 1;
    }
    else
    {
        d2->selection = 0;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 16;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TOGGLE, NULL, "FUMENIZE"));
    m = &d->menu.back();
    d8 = (Shiro::ToggleOptionData *)m->data;
    d8->param = &q->pracdata->usr_field_fumen;
    d8->labels[0] = "OFF";
    d8->labels[1] = "ON";
    m->x = optsX;
    m->y = optsY;
    m->value_x = optsValueX;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->label_text_rgba = 0xFF1010FF;
    m->value_text_rgba = m->label_text_rgba;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT | DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    optsY += 38;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TEXTINPUT, qs_update_pracdata, "PIECE SEQUENCE"));
    m = &d->menu.back();
    d7 = (Shiro::TextOptionData *)m->data;
    d7->visible_chars = 24;
    m->x = optsX;
    m->y = optsY;
    m->value_x = m->x;
    m->value_y = m->y + 18;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT; //|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x00EF00FF;

    optsY += 42;
    d->menu.push_back(Shiro::create_menu_option(Shiro::ElementType::MENU_TEXTINPUT, qs_update_pracdata, "RANDOMIZER SEED"));
    m = &d->menu.back();
    d7 = (Shiro::TextOptionData *)m->data;
    d7->visible_chars = 20;
    m->x = optsX;
    m->y = optsY;
    m->value_x = m->x;
    m->value_y = m->y + 18;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT; //|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x2020FFFF;

    gfx_button undoClearButton;
    undoClearButton.type = BUTTON_TYPE_ACTION;
    undoClearButton.action = undo_clear_confirm_yes;
    undoClearButton.text = "CLEAR";
    undoClearButton.active = false;
    undoClearButton.activate_check = undo_clear_button_should_activate;
    undoClearButton.deactivate_check = undo_clear_button_should_deactivate;
    undoClearButton.x = 81; // old: QRS_FIELD_X + (16 * 16) - 6;
    undoClearButton.y = 443; // old: QRS_FIELD_Y + 23 * 16 + 8 - 6;
    undoClearButton.w = 2 * 6 + 15 * (undoClearButton.text.size());
    undoClearButton.h = 28;
    undoClearButton.text_rgba_mod = 0xC0C0FFFF;

    gfx_button doUndoButton;
    doUndoButton.type = BUTTON_TYPE_ACTION;
    doUndoButton.action = usr_field_undo_button_action;
    doUndoButton.text = "";
    doUndoButton.active = false;
    //doUndoButton.visible = false;
    doUndoButton.activate_check = usr_field_undo_history_exists;
    doUndoButton.deactivate_check = usr_field_undo_history_not_exists;
    doUndoButton.x = QRS_FIELD_X - 4;
    doUndoButton.y = QRS_FIELD_Y + 23 * 16;
    doUndoButton.w = 25;
    doUndoButton.h = 27;

    gfx_button doRedoButton;
    doRedoButton.type = BUTTON_TYPE_ACTION;
    doRedoButton.action = usr_field_redo_button_action;
    doRedoButton.text = "";
    doRedoButton.active = false;
    //doRedoButton.visible = false;
    doRedoButton.activate_check = usr_field_redo_history_exists;
    doRedoButton.deactivate_check = usr_field_redo_history_not_exists;
    doRedoButton.x = QRS_FIELD_X + 13 * 16 + 2 + 7;
    doRedoButton.y = QRS_FIELD_Y + 23 * 16;
    doRedoButton.w = 25;
    doRedoButton.h = 27;

    gfx_button lockFieldButton;
    lockFieldButton.type = BUTTON_TYPE_ACTION;
    lockFieldButton.action = lock_usr_field;
    lockFieldButton.text = "";
    lockFieldButton.active = false;
    lockFieldButton.visible = false;
    lockFieldButton.activate_check = usr_field_is_unlocked;
    lockFieldButton.deactivate_check = usr_field_is_locked;
    lockFieldButton.x = QRS_FIELD_X + 14 * 16 + 2;
    lockFieldButton.y = QRS_FIELD_Y + 21 * 16;
    lockFieldButton.w = 20;
    lockFieldButton.h = 20;

    gfx_button unlockFieldButton;
    unlockFieldButton.type = BUTTON_TYPE_ACTION;
    unlockFieldButton.action = unlock_usr_field;
    unlockFieldButton.text = "";
    unlockFieldButton.active = false;
    unlockFieldButton.visible = false;
    unlockFieldButton.activate_check = usr_field_is_locked;
    unlockFieldButton.deactivate_check = usr_field_is_unlocked;
    unlockFieldButton.x = QRS_FIELD_X + 14 * 16 + 2;
    unlockFieldButton.y = QRS_FIELD_Y + 21 * 16;
    unlockFieldButton.w = 20;
    unlockFieldButton.h = 20;

    d->menuButtons.push_back(undoClearButton);
    d->menuButtons.push_back(doUndoButton);
    d->menuButtons.push_back(doRedoButton);
    d->menuButtons.push_back(lockFieldButton);
    d->menuButtons.push_back(unlockFieldButton);

    d->numopts = d->menu.size();

    qs_update_pracdata(cs);

    return 0;
}

int mload_options(game_t *g, int)
{
    if(!g)
        return -1;

    /*
        menudata *d = (menudata *)(g->data);
        menu_opt *m = NULL;
        int i = 0;

        menu_clear(g);

        d->menu = malloc(8 * sizeof(menu_opt *));
        d->selection = 0;
        d->numopts = 8;
        d->title = "CONTROLS";
        d->x = 4;
        d->y = 3;
    */

    return 0;
}

#define BUF_SIZE 64

int mload_replay(game_t *g, int)
{
    menudata *d = (menudata *)(g->data);
    Shiro::MenuOption *m = NULL;
    Shiro::ActionOptionData *d1 = NULL;
    Shiro::GameOptionData *d4 = NULL;

    struct replay *r = NULL;
    int replayCount = 0;
    struct replay *replaylist = scoredb_get_replay_list(&g->origin->records, &g->origin->player, &replayCount);

    menu_clear(g); // data->menu guaranteed to be NULL upon return

    d->menu_id = MENU_ID_REPLAY;
    d->use_target_tex = 1;
    d->selection = 0;
    d->numopts = 0;
    d->title = "REPLAY";
    d->x = 20;
    d->y = 16;

    d->is_paged = 1;
    d->page = 0;
    d->page_length = 20;
    d->page_text_x = 640 - 16;
    d->page_text_y = 16;

    if(replaylist)
    {
        d->numopts = replayCount + 1;
        d->menu.resize(d->numopts);
        d->menu[0] = Shiro::create_menu_option(Shiro::ElementType::MENU_ACTION, NULL, "RETURN");
        m = &d->menu[0];
        d1 = (Shiro::ActionOptionData *)m->data;
        d1->action = mload_main;
        d1->val = 0;
        m->x = 20;
        m->y = 60;
        m->label_text_flags = DRAWTEXT_THIN_FONT;

        for(int i = 1; i < replayCount + 1; i++)
        {
            d->menu[i] = Shiro::create_menu_option(Shiro::ElementType::MENU_GAME, NULL, "");
            r = &replaylist[i - 1];

            d->menu[i].label = get_replay_descriptor(r);
            m = &d->menu[i];
            d4 = (Shiro::GameOptionData *)m->data;
            d4->mode = QUINTESSE;
            d4->args.num = 4;
            if (
                !(d4->args.ptrs = (void **)malloc(4 * sizeof(void *))) ||
                !(d4->args.ptrs[0] = malloc(sizeof(CoreState *))) ||
                !(d4->args.ptrs[1] = malloc(sizeof(int))) ||
                !(d4->args.ptrs[2] = malloc(sizeof(unsigned int))) ||
                !(d4->args.ptrs[3] = malloc(sizeof(int))))
            {
                throw std::bad_alloc();
            }
            *(CoreState **)(d4->args.ptrs[0]) = g->origin;
            *(int *)(d4->args.ptrs[1]) = 0;
            *(unsigned int *)(d4->args.ptrs[2]) = r->mode;
            *(int *)(d4->args.ptrs[3]) = r->index;
            m->x = 20 - 13;
            m->y = 60 + (i % 20) * 20;
            m->label_text_flags = DRAWTEXT_THIN_FONT;
            m->label_text_rgba = (i % 2) ? 0xA0A0FFFF : RGBA_DEFAULT_MACRO;
        }
    }

    auto activateLambda = [](CoreState *) { int c = ((SDL_GetModState() & KMOD_SHIFT) != 0); return c; };
    auto deactivateLambda = [](CoreState *) { int c = ((SDL_GetModState() & KMOD_SHIFT) == 0); return c; };

    gfx_button deleteReplayButton;
    deleteReplayButton.type = BUTTON_TYPE_ACTION;
    deleteReplayButton.action = menu_delete_selected_replay;
    deleteReplayButton.text = "DELETE";
    deleteReplayButton.active = false;
    deleteReplayButton.activate_check = activateLambda;
    deleteReplayButton.deactivate_check = deactivateLambda;
    deleteReplayButton.w = 2 * 6 + 15 * (deleteReplayButton.text.size());
    deleteReplayButton.h = 28;
    deleteReplayButton.x = 320 - (deleteReplayButton.w / 2);
    deleteReplayButton.y = 9;
    deleteReplayButton.text_rgba_mod = 0xFF0000FF;

    d->menuButtons.push_back(deleteReplayButton);

    delete[] replaylist;

    return 0;
}

int menu_action_quit(game_t *, int) { return 1; }

int menu_is_practice(game_t *g)
{
    if(!g)
        return 0;

    menudata *d = (menudata *)g->data;
    if(d->menu_id == MENU_ID_PRACTICE)
        return 1;

    else
        return 0;
}

int menu_is_main(game_t *g)
{
    if(!g)
    {
        return 0;
    }

    menudata *d = (menudata *)g->data;
    if(d->menu_id == MENU_ID_MAIN)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}