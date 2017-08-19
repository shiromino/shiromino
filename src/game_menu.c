/*
    game_menu.c - logic functions governing the
    manipulation of and interactions with various
    menus
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "bstrlib.h"

#include "core.h"
#include "file_io.h"
#include "qrs.h"
#include "gfx.h"
#include "game_qs.h"
#include "game_menu.h"
#include "gfx_menu.h"
#include "audio.h"

struct menu_opt *std_game_multiopt_create(coreState *cs, unsigned int mode, int num_sections, bstring label)
{
    struct menu_opt *m = menu_opt_create(MENU_GAME_MULTIOPT, NULL, label);
    struct game_multiopt_data *d6 = m->data;
    int i = 0;

    d6->mode = QUINTESSE;
    d6->num = num_sections;
    d6->selection = 0;
    d6->labels = malloc(num_sections * sizeof(bstring));
    d6->labels[0] = NULL;
    for(i = 1; i < num_sections; i++) {
        d6->labels[i] = bformat("%d", 100*i);
    }

    d6->args = malloc(num_sections * sizeof(struct game_args));
    for(i = 0; i < num_sections; i++) {
        d6->args[i].num = 4;
        d6->args[i].ptrs = malloc(4 * sizeof(void *));
        d6->args[i].ptrs[0] = malloc(sizeof(coreState *));
        d6->args[i].ptrs[1] = malloc(sizeof(int));
        d6->args[i].ptrs[2] = malloc(sizeof(unsigned int));
        d6->args[i].ptrs[3] = malloc(sizeof(char *));

        *(coreState **)(d6->args[i].ptrs[0]) = cs;
        *(int *)(d6->args[i].ptrs[1]) = 100*i;
        *(unsigned int *)(d6->args[i].ptrs[2]) = mode;
        *(char **)(d6->args[i].ptrs[3]) = NULL;
    }

    return m;
}

struct menu_opt *menu_opt_create(int type, int (*value_update_callback)(coreState *cs), bstring label)
{
    struct menu_opt *m = malloc(sizeof(struct menu_opt));

    m->type = type;
    m->value_update_callback = value_update_callback;
    m->label = label;
    m->x = 0;
    m->y = 0;
    m->render_update = 1;

    m->label_text_flags = 0;
    m->value_text_flags = 0;
    m->label_text_rgba = RGBA_DEFAULT;
    m->value_text_rgba = RGBA_DEFAULT;

    struct action_opt_data *d1 = NULL;
    struct multi_opt_data *d2 = NULL;
    struct toggle_opt_data *d3 = NULL;
    struct game_opt_data *d4 = NULL;
    struct metagame_opt_data *d5 = NULL;
    struct game_multiopt_data *d6 = NULL;
    struct text_opt_data *d7 = NULL;

    switch(type) {
        case MENU_LABEL:
            m->data = NULL;
            break;

        case MENU_ACTION:
            m->data = malloc(sizeof(struct action_opt_data));
            d1 = m->data;
            d1->action = NULL;
            d1->val = 0;
            break;

        case MENU_MULTIOPT:
            m->data = malloc(sizeof(struct multi_opt_data));
            d2 = m->data;
            d2->selection = 0;
            d2->num = 0;
            d2->vals = NULL;
            d2->labels = NULL;
            break;

        case MENU_TEXTINPUT:
            m->data = malloc(sizeof(struct text_opt_data));
            d7 = m->data;
            d7->active = 0;
            d7->position = 0;
            d7->selection = 0;
            d7->leftmost_position = 0;
            d7->visible_chars = 15;
            d7->text = bfromcstr("\0");
            break;

        case MENU_TOGGLE:
            m->data = malloc(sizeof(struct toggle_opt_data));
            d3 = m->data;
            d3->param = NULL;
            d3->labels[0] = NULL;
            d3->labels[1] = NULL;
            break;

        case MENU_GAME:
            m->data = malloc(sizeof(struct game_opt_data));
            d4 = m->data;
            d4->mode = MODE_INVALID;
            d4->args.num = 0;
            d4->args.ptrs = NULL;
            break;

        case MENU_GAME_MULTIOPT:
            m->data = malloc(sizeof(struct game_multiopt_data));
            d6 = m->data;
            d6->mode = MODE_INVALID;
            d6->num = 0;
            d6->selection = 0;
            d6->labels = NULL;
            d6->args = NULL;
            break;


        case MENU_METAGAME:
            m->data = malloc(sizeof(struct metagame_opt_data));
            d5 = m->data;
            d5->mode = MODE_INVALID;
            d5->submode = MODE_INVALID;
            d5->num_args = 0;
            d5->num_subargs = 0;
            d5->args = NULL;
            d5->sub_args = NULL;
            break;

        default:
            free(m);
            return NULL;
    }

    return m;
}

void menu_opt_destroy(struct menu_opt *m)
{
    if(!m)
        return;

    struct multi_opt_data *d2 = NULL;
    struct toggle_opt_data *d3 = NULL;
    struct game_opt_data *d4 = NULL;
    struct game_multiopt_data *d6 = NULL;

    int i = 0;
    int j = 0;

    switch(m->type) {
        case MENU_LABEL:
            break;

        case MENU_ACTION:
            free(m->data);
            break;

        case MENU_MULTIOPT:
            d2 = m->data;
            free(d2->vals);

            for(i = 0; i < d2->num; i++) {
                if(d2->labels[i])
                    bdestroy(d2->labels[i]);
            }

            free(d2->labels);
            free(d2);
            break;

        case MENU_TOGGLE:
            d3 = m->data;
            bdestroy(d3->labels[0]);
            bdestroy(d3->labels[1]);
            free(m->data);
            break;

        case MENU_GAME:
            d4 = m->data;
            for(i = 0; i < d4->args.num; i++) {
                if(d4->args.ptrs[i])
                    free(d4->args.ptrs[i]);
            }

            free(d4->args.ptrs);
            free(m->data);
            break;

        case MENU_GAME_MULTIOPT:
            d6 = m->data;
            for(i = 0; i < d6->num; i++) {
                if(d6->args[i].ptrs) {
                    for(j = 0; j < d6->args[i].num; j++)
                        free(d6->args[i].ptrs[j]);

                    free(d6->args[i].ptrs);
                }

                if(d6->labels[i])
                    bdestroy(d6->labels[i]);
            }

            free(d6->args);
            free(m->data);
            break;

        case MENU_METAGAME:        // TODO
            //d5 = m->data;

            //free(m->data);
            break;

        default:
            break;
    }

    free(m);
}

int menu_text_toggle(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;

    if(cs->button_emergency_override)
        return 0;

    if(!cs->text_editing) {
        cs->text_editing = 1;
        d7->active = 1;
    } else {
        cs->text_editing = 0;
        d7->active = 0;
    }

    return 0;
}

int menu_text_insert(coreState *cs, char *str)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    bstring bstr = bfromcstr(str);

    if(bstr) {
        if(d7->selection) {
            bdestroy(t);
            d7->text = bstr;
            d7->selection = 0;
            d7->position = bstr->slen;
            d7->leftmost_position = 0;
            if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != d7->text->slen - d7->visible_chars)
                d7->leftmost_position = d7->position - d7->visible_chars + 1;
        } else {
            if(binsert(t, d7->position, bstr, 0xFF) == BSTR_OK) {
                if(t->slen > 2000) {
                    btrunc(t, 2000);
                    d7->position = 2000;
                } else
                    d7->position += bstr->slen;

                if(d7->position == t->slen && d7->leftmost_position < t->slen - d7->visible_chars) {
                    d7->leftmost_position = t->slen - d7->visible_chars;
                }

                if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != d7->text->slen - d7->visible_chars)
                    d7->leftmost_position = d7->position - d7->visible_chars + 1;
            }

            bdestroy(bstr);
        }
    }

    if(d->menu[d->selection]->value_update_callback)
        d->menu[d->selection]->value_update_callback(cs);

    return 0;
}

int menu_text_backspace(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection) {
        d7->selection = 0;
        btrunc(t, 0);
        d7->position = 0;
        d7->leftmost_position = 0;
    } else if(d7->position > 0) {
        bdelete(t, d7->position - 1, 1);
        d7->position--;
        if(d7->position < d7->leftmost_position + 1 && d7->leftmost_position)
            d7->leftmost_position--;
    }

    if(d->menu[d->selection]->value_update_callback)
        d->menu[d->selection]->value_update_callback(cs);

    return 0;
}

int menu_text_delete(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection) {
        d7->selection = 0;
        btrunc(t, 0);
        d7->position = 0;
        d7->leftmost_position = 0;
    } else if(d7->position < d7->text->slen) {
        bdelete(t, d7->position, 1);
    }

    if(d->menu[d->selection]->value_update_callback)
        d->menu[d->selection]->value_update_callback(cs);

    return 0;
}

int menu_text_seek_left(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    //bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position > 0) {
        d7->position--;
        if(d7->position < d7->leftmost_position + 1 && d7->leftmost_position)
            d7->leftmost_position--;
    }

    return 0;
}

int menu_text_seek_right(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position < t->slen) {
        d7->position++;
        if(d7->position > d7->leftmost_position + d7->visible_chars - 1 && d7->leftmost_position != d7->text->slen - d7->visible_chars)
            d7->leftmost_position++;
    }

    return 0;
}

int menu_text_seek_home(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position > 0) {
        d7->position = 0;
        d7->leftmost_position = 0;
    }

    return 0;
}

int menu_text_seek_end(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;
    bstring t = d7->text;

    if(cs->button_emergency_override)
        return 0;

    if(d7->position < t->slen) {
        d7->position = t->slen;
        d7->leftmost_position = d7->position - d7->visible_chars;
        if(d7->leftmost_position < 0)
            d7->leftmost_position = 0;
    }

    return 0;
}

int menu_text_select_all(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;

    if(cs->button_emergency_override)
        return 0;

    d7->selection = !d7->selection;

    return 0;
}

int menu_text_copy(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection) {
        SDL_SetClipboardText((char *)(d7->text->data));
        d7->selection = 0;
    }

    if(d->menu[d->selection]->value_update_callback)
        d->menu[d->selection]->value_update_callback(cs);

    return 0;
}

int menu_text_cut(coreState *cs)
{
    menudata *d = cs->menu->data;
    struct text_opt_data *d7 = d->menu[d->selection]->data;

    if(cs->button_emergency_override)
        return 0;

    if(d7->selection) {
        SDL_SetClipboardText((char *)(d7->text->data));
        d7->selection = 0;
        btrunc(d7->text, 0);
        d7->position = 0;
    }

    if(d->menu[d->selection]->value_update_callback)
        d->menu[d->selection]->value_update_callback(cs);

    return 0;
}

game_t *menu_create(coreState *cs)
{
    if(!cs)
        return NULL;

    game_t *g = malloc(sizeof(game_t));

    g->origin = cs;
    g->field = NULL;
    g->init = menu_init;
    g->quit = menu_quit;
    g->preframe = NULL;
    g->input = menu_input;
    g->frame = NULL;
    g->draw = gfx_drawmenu;

    g->data = malloc(sizeof(menudata));
    menudata *d = (menudata *)(g->data);

    d->target_tex = SDL_CreateTexture(cs->screen.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 640, 480);
    d->menu = NULL;
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
    d->title = NULL;
    d->x = 0;
    d->y = 0;

    return g;
}

int menu_init(game_t *g)
{
    if(!g)
        return -1;

    if(mload_main(g, 0))
        printf("Failed to load main menu\n");

    menudata *d = g->data;

    SDL_SetRenderTarget(g->origin->screen.renderer, d->target_tex);
    SDL_SetTextureBlendMode(d->target_tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
    SDL_RenderClear(g->origin->screen.renderer);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 255);
    SDL_SetRenderTarget(g->origin->screen.renderer, NULL);

    g->origin->bg = (asset_by_name(g->origin, "bg-temp"))->data;
    g->origin->bg_old = g->origin->bg;

    return 0;
}

int menu_quit(game_t *g)
{
    if(!g)
        return -1;

    menudata *d = (menudata *)(g->data);
    int i = 0;

    if(d->menu) {
        for(i = 0; i < d->numopts; i++) {
            if(d->menu[i])
                menu_opt_destroy(d->menu[i]);
        }

        free(d->menu);
        d->menu = NULL;
    }

    if(d->title)
        bdestroy(d->title);

    free(d);

    return 0;
}

int menu_input(game_t *g)
{
    if(!g)
        return -1;

    coreState *cs = g->origin;
    struct keyflags *k = cs->keys[0];

    menudata *d = (menudata *)(g->data);

    struct menu_opt *m = NULL;

    struct action_opt_data *d1 = NULL;
    struct multi_opt_data *d2 = NULL;
    struct toggle_opt_data *d3 = NULL;
    struct game_opt_data *d4 = NULL;
    struct metagame_opt_data *d5 = NULL;
    struct game_multiopt_data *d6 = NULL;

    struct asset *menu_choose = asset_by_name(cs, "menu_choose");

    int i = 0;
    int update = 0;

    if(cs->text_editing) {
        return 0;
    }

    if(k->escape == 1 || cs->keys[1]->escape == 1) {
        if(!(d->menu_id == MENU_ID_MAIN)) {
            mload_main(g, 0);
            return 0;
        }
    }

    if(!d->menu)
        return 0;

    if(d->menu[d->selection]->type != MENU_TEXTINPUT) {
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

    if((k->up == 1 || k->up > 18) && d->selection > 0)
    {
        update = 1;
        for(i = d->selection - 1;; i--)
        {
            if(d->is_paged) {
                if(d->selection == d->page * d->page_length) {
                    break;
                }
            }

            if(i == -1)
                i = d->numopts - 1;
            if(d->menu[i]->type != MENU_LABEL) {
                d->selection = i;
                if(k->up == 1) play_sfx(menu_choose->data, menu_choose->volume);
                if(d->menu[d->selection]->type == MENU_TEXTINPUT) {
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
                } else {
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

    if((k->down == 1 || k->down > 18) && (d->selection < d->numopts - 1))
    {
        update = 1;
        for(i = d->selection + 1;; i++)
        {
            if(d->is_paged) {
                if(d->selection == (d->page+1) * d->page_length - 1) {
                    break;
                }
            }

            if(i == d->numopts)
                i = 0;
            if(d->menu[i]->type != MENU_LABEL) {
                d->selection = i;
                if(k->down == 1) play_sfx(menu_choose->data, menu_choose->volume);
                if(d->menu[d->selection]->type == MENU_TEXTINPUT) {
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
                } else {
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

    if(d->is_paged) {
        if((k->left == 1 || k->left > 18) && d->page > 0) {
            update = 1;
            d->selection = d->selection - d->page_length;
            d->page--;
        }

        if((k->right == 1 || k->right > 18) && d->page < ((d->numopts - 1) / d->page_length)) {
            update = 1;
            d->selection = d->selection + d->page_length;
            d->page++;

            if(d->selection >= d->numopts)
                d->selection = d->numopts - 1;
        }
    }

    if(update && d->use_target_tex) {
        for(i = 0; i < d->numopts; i++) {
            d->menu[i]->render_update = 1;
        }
    }

    if(d->menu_id == MENU_ID_MAIN)
        d->main_menu_data.selection = d->selection;
    else if(d->menu_id == MENU_ID_PRACTICE)
        d->practice_menu_data.selection = d->selection;

    if(d->menu[d->selection]->type != MENU_TEXTINPUT) {
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

    m = d->menu[d->selection];

    switch(m->type)
    {
        case MENU_ACTION:
            d1 = d->menu[d->selection]->data;

            if(k->a == 1 || k->start == 1) {
                if(d1->action) {
                    if(d1->action(g, d1->val)) {
                        printf("Received quit signal, shutting down.\n");
                        return 1;
                    }

                    return 0;
                }
            }

            break;

        case MENU_MULTIOPT:
            d2 = d->menu[d->selection]->data;

            if(!d->is_paged) {
                if( (k->left == 1 || k->left > 14) && d2->selection > 0 ) {
                    d2->selection--;
                    *(d2->param) = d2->vals[d2->selection];
                    if(d->menu[d->selection]->value_update_callback)
                        d->menu[d->selection]->value_update_callback(cs);
                }

                if( (k->right == 1 || k->right > 14) && d2->selection < (d2->num - 1) ) {
                    d2->selection++;
                    *(d2->param) = d2->vals[d2->selection];
                    if(d->menu[d->selection]->value_update_callback)
                        d->menu[d->selection]->value_update_callback(cs);
                }
            }

            break;

        case MENU_TEXTINPUT:
            break;

        case MENU_TOGGLE:
            d3 = d->menu[d->selection]->data;

            if(!d->is_paged) {
                if(k->a == 1 || k->left == 1 || k->right == 1) {
                    *(d3->param) = *(d3->param) ? false : true;
                    if(d->menu[d->selection]->value_update_callback)
                        d->menu[d->selection]->value_update_callback(cs);
                }
            }

            break;

        case MENU_GAME:
            d4 = d->menu[d->selection]->data;

            if(k->a == 1 || k->start == 1)
            {
                switch(d4->mode)
                {
                    case QUINTESSE:
                        if(d4->args.ptrs) {
                            if(d4->args.ptrs[0] && d4->args.ptrs[1] && d4->args.ptrs[2] && d4->args.ptrs[3]) {
                                g->origin->p1game = qs_game_create( *((coreState **)(d4->args.ptrs[0])),
                                                                         *((int *)(d4->args.ptrs[1])),
                                                                         *((unsigned int *)(d4->args.ptrs[2])),
                                                                         *((char **)(d4->args.ptrs[3]))
                                                                       );
                                if(g->origin->p1game) {
                                    g->origin->p1game->init(g->origin->p1game);

                                    return 0;
                                }
                            }
                        } else {
                            g->origin->p1game = qs_game_create(g->origin, 0, 0, NULL);
                            if(g->origin->p1game) {
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

        case MENU_GAME_MULTIOPT:
            d6 = d->menu[d->selection]->data;

            if(!d->is_paged) {
                if( (k->left == 1 || k->left > 14) && d6->selection > 0 ) {
                    d6->selection--;
                }

                if( (k->right == 1 || k->right > 14) && d6->selection < (d6->num - 1) ) {
                    d6->selection++;
                }

                if(d->menu_id == MENU_ID_MAIN)
                    d->main_menu_data.opt_selection = d6->selection;
            }

            if(k->a == 1 || k->start == 1)
            {
                switch(d6->mode)
                {
                    case QUINTESSE:
                        if(d6->args[d6->selection].ptrs) {
                            if(d6->args[d6->selection].ptrs[0] && d6->args[d6->selection].ptrs[1] && d6->args[d6->selection].ptrs[2] && d6->args[d6->selection].ptrs[3]) {
                                g->origin->p1game = qs_game_create( *((coreState **)(d6->args[d6->selection].ptrs[0])),
                                                                         *((int *)(d6->args[d6->selection].ptrs[1])),
                                                                         *((unsigned int *)(d6->args[d6->selection].ptrs[2])),
                                                                         *((char **)(d6->args[d6->selection].ptrs[3]))
                                                                       );
                                if(g->origin->p1game) {
                                    g->origin->p1game->init(g->origin->p1game);

                                    return 0;
                                }
                            }
                        } else {
                            g->origin->p1game = qs_game_create(g->origin, 0, 0, NULL);
                            if(g->origin->p1game) {
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

        case MENU_METAGAME:
            d5 = d->menu[d->selection]->data;

            if(k->a == 1 || k->start == 1)
            {
                switch(d5->mode)
                {
                    /*case META_TAS:
                        break;

                    case META_REPLAY:
                        break;*/

                    default:
                        break;
                }
            }

            break;

        default:
            break;
    }

    return 0;
}

int menu_frame(game_t *g)        // nothing right now
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

    if(!d->menu)
        return 0;

    for(i = 0; i < d->numopts; i++) {
        if(d->menu[i])
            menu_opt_destroy(d->menu[i]);
    }

    d->menu = NULL;
    d->use_target_tex = 0;

    if(d->title)
        bdestroy(d->title);

    d->x = 0;
    d->y = 0;
    d->selection = 0;
    d->numopts = 0;
    d->is_paged = 0;

    return 0;
}

int mload_main(game_t *g, int val)
{
    if(!g)
        return -1;

    coreState *cs = g->origin;
    menudata *d = (menudata *)(g->data);
    struct menu_opt *m = NULL;
    struct action_opt_data *d1 = NULL;
    struct multi_opt_data *d2 = NULL;
    struct game_opt_data *d4 = NULL;
    struct game_multiopt_data *d6 = NULL;
    int i = 0;

    request_fps(cs, 60);

    if(d->menu_id == MENU_ID_MAIN) {
        return 0;
    }

    menu_clear(g);        // data->menu guaranteed to be NULL upon return
    if(cs->p1game) {
        cs->p1game->quit(cs->p1game);
        free(cs->p1game);
        cs->p1game = NULL;
    }

    cs->bg = (asset_by_name(cs, "bg-temp"))->data;
    cs->bg_old = cs->bg;

    d->menu = malloc(15 * sizeof(struct menu_opt *));
    d->menu_id = MENU_ID_MAIN;
    d->use_target_tex = 0;
    d->selection = d->main_menu_data.selection;
    d->numopts = 15;
    d->title = bfromcstr("MAIN MENU");
    d->x = 4*16;
    d->y = 3*16;

    d->menu[0] = menu_opt_create(MENU_GAME_MULTIOPT, NULL, bfromcstr("PENTOMINO"));
    m = d->menu[0];
    d6 = m->data;
    d6->mode = QUINTESSE;
    d6->num = 4;    // normal, no lock delay, pentomino only, pentomino only & no lock delay
    if(d->main_menu_data.selection == 0)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    d6->labels = malloc(4 * sizeof(bstring));
    d6->labels[0] = NULL;
    d6->labels[1] = bfromcstr("NO LOCK");
    d6->labels[2] = bfromcstr("PENTO ONLY");
    d6->labels[3] = bfromcstr("NO LOCK, PENTO ONLY");
    d6->args = malloc(4 * sizeof(struct game_args));
    for(i = 0; i < 4; i++) {
        d6->args[i].num = 4;
        d6->args[i].ptrs = malloc(4 * sizeof(void *));
        d6->args[i].ptrs[0] = malloc(sizeof(coreState *));
        d6->args[i].ptrs[1] = malloc(sizeof(int));
        d6->args[i].ptrs[2] = malloc(sizeof(unsigned int));
        d6->args[i].ptrs[3] = malloc(sizeof(char *));

        *(coreState **)(d6->args[i].ptrs[0]) = g->origin;
        *(int *)(d6->args[i].ptrs[1]) = 0;
        *(char **)(d6->args[i].ptrs[3]) = NULL;
    }
    *(unsigned int *)(d6->args[0].ptrs[2]) = 0;    // default: 0
    *(unsigned int *)(d6->args[1].ptrs[2]) = NO_LOCK_DELAY;
    *(unsigned int *)(d6->args[2].ptrs[2]) = PENTOMINO_ONLY;
    *(unsigned int *)(d6->args[3].ptrs[2]) = NO_LOCK_DELAY | PENTOMINO_ONLY;
    m->x = 4*16;
    m->y = 6*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[1] = menu_opt_create(MENU_GAME_MULTIOPT, NULL, bfromcstr("PENTOMINO 20G"));
    m = d->menu[1];
    d6 = m->data;
    d6->mode = QUINTESSE;
    d6->num = 4;    // normal, no lock delay, pentomino only, pentomino only & no lock delay
    if(d->main_menu_data.selection == 1)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    d6->labels = malloc(4 * sizeof(bstring));
    d6->labels[0] = NULL;
    d6->labels[1] = bfromcstr("NO LOCK");
    d6->labels[2] = bfromcstr("PENTO ONLY");
    d6->labels[3] = bfromcstr("NO LOCK, PENTO ONLY");
    d6->args = malloc(4 * sizeof(struct game_args));
    for(i = 0; i < 4; i++) {
        d6->args[i].num = 4;
        d6->args[i].ptrs = malloc(4 * sizeof(void *));
        d6->args[i].ptrs[0] = malloc(sizeof(coreState *));
        d6->args[i].ptrs[1] = malloc(sizeof(int));
        d6->args[i].ptrs[2] = malloc(sizeof(unsigned int));
        d6->args[i].ptrs[3] = malloc(sizeof(char *));

        *(coreState **)(d6->args[i].ptrs[0]) = g->origin;
        *(int *)(d6->args[i].ptrs[1]) = 700;
        *(char **)(d6->args[i].ptrs[3]) = NULL;
    }
    *(unsigned int *)(d6->args[0].ptrs[2]) = 0;
    *(unsigned int *)(d6->args[1].ptrs[2]) = NO_LOCK_DELAY;
    *(unsigned int *)(d6->args[2].ptrs[2]) = PENTOMINO_ONLY;
    *(unsigned int *)(d6->args[3].ptrs[2]) = NO_LOCK_DELAY | PENTOMINO_ONLY;
    m->x = 4*16;
    m->y = 7*16;
    m->value_x = m->x + 14*16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[2] = menu_opt_create(MENU_GAME, NULL, bfromcstr("NIGHTMARE MODE"));
    m = d->menu[2];
    d4 = m->data;
    d4->mode = QUINTESSE;
    d4->args.num = 4;
    d4->args.ptrs = malloc(4 * sizeof(void *));
    d4->args.ptrs[0] = malloc(sizeof(coreState *));
    d4->args.ptrs[1] = malloc(sizeof(int));
    d4->args.ptrs[2] = malloc(sizeof(unsigned int));
    d4->args.ptrs[3] = malloc(sizeof(char *));
    *(coreState **)(d4->args.ptrs[0]) = g->origin;
    *(int *)(d4->args.ptrs[1]) = 0;
    *(unsigned int *)(d4->args.ptrs[2]) = NIGHTMARE_MODE;
    *(char **)(d4->args.ptrs[3]) = NULL;
    m->x = 4*16;
    m->y = 8*16;
    m->label_text_rgba = 0xFFA0A0FF;

    d->menu[3] = std_game_multiopt_create(g->origin, MODE_G1_MASTER, 10, bfromcstr("G1 MASTER"));
    m = d->menu[3];
    d6 = m->data;
    if(d->main_menu_data.selection == 3)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4*16;
    m->y = 9*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[4] = std_game_multiopt_create(g->origin, MODE_G1_20G, 10, bfromcstr("G1 20G"));
    m = d->menu[4];
    d6 = m->data;
    if(d->main_menu_data.selection == 4)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4*16;
    m->y = 10*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[5] = std_game_multiopt_create(g->origin, MODE_G2_MASTER, 10, bfromcstr("G2 MASTER"));
    m = d->menu[5];
    d6 = m->data;
    if(d->main_menu_data.selection == 5)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4*16;
    m->y = 11*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFFFF40FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[6] = std_game_multiopt_create(g->origin, MODE_G2_DEATH, 10, bfromcstr("G2 DEATH"));
    m = d->menu[6];
    d6 = m->data;
    if(d->main_menu_data.selection == 6)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4*16;
    m->y = 12*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFF4040FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[7] = std_game_multiopt_create(g->origin, MODE_G3_TERROR, 13, bfromcstr("G3 TERROR"));
    m = d->menu[7];
    d6 = m->data;
    if(d->main_menu_data.selection == 7)
        d6->selection = d->main_menu_data.opt_selection;
    else
        d6->selection = 0;
    m->x = 4*16;
    m->y = 13*16;
    m->value_x = m->x + 10*16;
    m->value_y = m->y;
    m->label_text_rgba = 0xFF4040FF;
    m->value_text_rgba = 0xA0A0FFFF;

    d->menu[8] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("PRACTICE"));
    m = d->menu[8];
    d1 = m->data;
    d1->action = mload_practice;
    d1->val = 0;
    m->x = 4*16;
    m->y = 15*16;

    d->menu[9] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("REPLAY"));
    m = d->menu[9];
    d1 = m->data;
    d1->action = mload_replay;
    d1->val = 0;
    m->x = 4*16;
    m->y = 16*16;

    d->menu[10] = menu_opt_create(MENU_LABEL, NULL, bfromcstr("SETTINGS"));
    m = d->menu[10];
    m->x = 4*16;
    m->y = 19*16;

    d->menu[11] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("MASTER VOLUME"));
    m = d->menu[11];
    d2 = m->data;
    d2->num = 101;
    d2->param = &g->origin->settings->master_volume;
    d2->vals = malloc(101 * sizeof(int));
    d2->labels = malloc(101 * sizeof(bstring));
    for(i = 0; i < 101; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings->master_volume;
    m->x = 4*16;
    m->y = 21*16;
    m->value_x = 21*16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT|DRAWTEXT_VALUE_BAR;

    d->menu[12] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("SFX VOLUME"));
    m = d->menu[12];
    d2 = m->data;
    d2->num = 101;
    d2->param = &g->origin->settings->sfx_volume;
    d2->vals = malloc(101 * sizeof(int));
    d2->labels = malloc(101 * sizeof(bstring));
    for(i = 0; i < 101; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings->sfx_volume;
    m->x = 4*16;
    m->y = 22*16;
    m->value_x = 21*16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT|DRAWTEXT_VALUE_BAR;

    d->menu[13] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("MUSIC VOLUME"));
    m = d->menu[13];
    d2 = m->data;
    d2->num = 101;
    d2->param = &g->origin->settings->mus_volume;
    d2->vals = malloc(101 * sizeof(int));
    d2->labels = malloc(101 * sizeof(bstring));
    for(i = 0; i < 101; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }
    d2->selection = g->origin->settings->mus_volume;
    m->x = 4*16;
    m->y = 23*16;
    m->value_x = 21*16;
    m->value_y = m->y;
    m->value_text_flags = DRAWTEXT_ALIGN_RIGHT|DRAWTEXT_VALUE_BAR;

    d->menu[14] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("QUIT"));
    m = d->menu[14];
    d1 = m->data;
    d1->action = menu_action_quit;
    d1->val = 0;
    m->x = 4*16;
    m->y = 26*16;

    return 0;
}

int mload_practice(game_t *g, int val)
{
    coreState *cs = g->origin;
    menudata *d = (menudata *)(g->data);
    struct menu_opt *m = NULL;
    struct action_opt_data *d1 = NULL;
    struct multi_opt_data *d2 = NULL;
    struct text_opt_data *d7 = NULL;
    struct toggle_opt_data *d8 = NULL;
    int i = 0;
    int pracdata_mirror_existed = 0;

    int grav_ = 0;
    int lock_ = 0;
    int are_ = 0;
    int lineare_ = 0;
    int lineclear_ = 0;
    int das_ = 0;
    int width_ = 0;

    int game_type_ = 0;
    int lock_protect_ = 0;

    //TODO: piece sequence restore from pracdata struct (need to save char* that the user enters)

    menu_clear(g);        // data->menu guaranteed to be NULL upon return

    cs->menu_input_override = 1;

    cs->p1game = qs_game_create(cs, 0, QRS_PRACTICE|TETROMINO_ONLY, NULL);
    cs->p1game->init(cs->p1game);

    qrsdata *q = cs->p1game->data;

    if(!d->practice_menu_data.pracdata_mirror)
        d->practice_menu_data.pracdata_mirror = q->pracdata;
    else
        pracdata_mirror_existed = 1;

    cs->bg = NULL;
    cs->bg_old = NULL;

    d->menu = malloc(MENU_PRACTICE_NUMOPTS * sizeof(struct menu_opt *));
    d->menu_id = MENU_ID_PRACTICE;
    d->selection = 0;
    d->numopts = MENU_PRACTICE_NUMOPTS;
    d->title = NULL;//bfromcstr("PRACTICE");
    d->x = 20*16;
    d->y = 2*16;

    //
    /* */
    //

    d->menu[0] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("RETURN"));
    m = d->menu[0];
    d1 = m->data;
    d1->action = mload_main;
    d1->val = 0;
    m->x = 16*16;
    m->y = 6*16 + 6;

    //
    /* */
    //

    d->menu[1] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("PLAY"));
    m = d->menu[1];
    d1 = m->data;
    d1->action = qs_game_pracinit;
    d1->val = 0;
    m->x = 16*16;
    m->y = 7*16 + 6;

    //
    /* */
    //

    d->menu[2] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("GRAVITY"));
    m = d->menu[2];
    d2 = m->data;
    d2->num = 135;
    d2->param = &q->pracdata->usr_timings->grav;
    d2->vals = malloc(135 * sizeof(int));
    d2->labels = malloc(135 * sizeof(bstring));
    for(i = 0; i < 128; i++) {
        d2->labels[i] = bformat("%d", 2*i);
        d2->vals[i] = 2*i;
    }

    d2->labels[128] = bfromcstr("1G");
    d2->vals[128] = 256;
    d2->labels[129] = bfromcstr("1.5G");
    d2->vals[129] = 256 + 128;
    d2->labels[130] = bfromcstr("2G");
    d2->vals[130] = 2*256;
    d2->labels[131] = bfromcstr("3G");
    d2->vals[131] = 3*256;
    d2->labels[132] = bfromcstr("4G");
    d2->vals[132] = 4*256;
    d2->labels[133] = bfromcstr("5G");
    d2->vals[133] = 5*256;
    d2->labels[134] = bfromcstr("20G");
    d2->vals[134] = 20*256;

    if(pracdata_mirror_existed) {
        grav_ = q->pracdata->usr_timings->grav;
        if(grav_ <= 256) {
            d2->selection = (grav_ / 2) < 0 ? 0 : (grav_ / 2);
        } else if(grav_ == 256 + 128) {
            d2->selection = 129;
        } else if(grav_ == 2*256) {
            d2->selection = 130;
        } else if(grav_ == 3*256) {
            d2->selection = 131;
        } else if(grav_ == 4*256) {
            d2->selection = 132;
        } else if(grav_ == 5*256) {
            d2->selection = 133;
        } else {
            d2->selection = 134;
        }
    } else {
        d2->selection = 134;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 2*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_FIXEDSYS_FONT;
    m->value_text_flags = DRAWTEXT_FIXEDSYS_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x70FF70FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[3] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("LOCK"));
    m = d->menu[3];
    d2 = m->data;
    d2->num = 101;
    d2->param = &q->pracdata->usr_timings->lock;
    d2->vals = malloc(101 * sizeof(int));
    d2->labels = malloc(101 * sizeof(bstring));
    d2->labels[0] = bfromcstr("OFF");
    d2->vals[0] = -1;
    for(i = 1; i < 101; i++) {
        d2->labels[i] = bformat("%d", i-1);
        d2->vals[i] = i-1;
    }

    if(pracdata_mirror_existed) {
        lock_ = q->pracdata->usr_timings->lock;
        d2->selection = lock_ + 1;
    } else {
        d2->selection = 31;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 3*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFF5050FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[4] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("ARE"));
    m = d->menu[4];
    d2 = m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->are;
    d2->vals = malloc(100 * sizeof(int));
    d2->labels = malloc(100 * sizeof(bstring));
    for(i = 0; i < 100; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed) {
        are_ = q->pracdata->usr_timings->are;
        d2->selection = are_;
    } else {
        d2->selection = 12;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 4*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFFA030FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[5] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("LINE ARE"));
    m = d->menu[5];
    d2 = m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->lineare;
    d2->vals = malloc(100 * sizeof(int));
    d2->labels = malloc(100 * sizeof(bstring));
    for(i = 0; i < 100; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed) {
        lineare_ = q->pracdata->usr_timings->lineare;
        d2->selection = lineare_;
    } else {
        d2->selection = 6;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 5*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFFFF20FF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[6] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("LINE CLEAR"));
    m = d->menu[6];
    d2 = m->data;
    d2->num = 100;
    d2->param = &q->pracdata->usr_timings->lineclear;
    d2->vals = malloc(100 * sizeof(int));
    d2->labels = malloc(100 * sizeof(bstring));
    for(i = 0; i < 100; i++) {
        d2->labels[i] = bformat("%d", i);
        d2->vals[i] = i;
    }

    if(pracdata_mirror_existed) {
        lineclear_ = q->pracdata->usr_timings->lineclear;
        d2->selection = lineclear_;
    } else {
        d2->selection = 6;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 6*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0x8080FFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[7] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("DAS"));
    m = d->menu[7];
    d2 = m->data;
    d2->num = 99;
    d2->param = &q->pracdata->usr_timings->das;
    d2->vals = malloc(99 * sizeof(int));
    d2->labels = malloc(99 * sizeof(bstring));
    for(i = 0; i < 99; i++) {
        d2->labels[i] = bformat("%d", i+1);
        d2->vals[i] = i+1;
    }

    if(pracdata_mirror_existed) {
        das_ = q->pracdata->usr_timings->das;
        d2->selection = das_ - 1;
    } else {
        d2->selection = 7;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 16*16;
    m->y = 8*16 + 7*18;
    m->value_x = m->x + 13*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->label_text_rgba = 0xFF00FFFF;
    m->value_text_rgba = m->label_text_rgba;

    //
    /* */
    //

    d->menu[8] = menu_opt_create(MENU_MULTIOPT, qs_update_pracdata, bfromcstr("WIDTH"));
    m = d->menu[8];
    d2 = m->data;
    d2->num = 5;
    d2->param = &q->pracdata->field_w;
    d2->vals = malloc(5 * sizeof(int));
    d2->labels = malloc(5 * sizeof(bstring));
    for(i = 0; i < 5; i++) {
        d2->labels[i] = bformat("%d", 2*(i+2));
        d2->vals[i] = 2*(i+2);
    }

    if(pracdata_mirror_existed) {
        width_ = q->field_w;    // TODO: have q->field_w and q->game_type save in the pracdata struct so they can be restored here
        d2->selection = (width_ / 2) - 2;
    } else {
        d2->selection = 3;
    }

    m->x = 438;
    m->y = 8*16 + 2*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[9] = menu_opt_create(MENU_MULTIOPT, qs_update_pracdata, bfromcstr("GAME TYPE"));
    m = d->menu[9];
    d2 = m->data;
    d2->num = 4;
    d2->param = &q->pracdata->game_type;
    d2->vals = malloc(4 * sizeof(int));
    d2->labels = malloc(4 * sizeof(bstring));
    d2->labels[0] = bfromcstr("QRS");
    d2->labels[1] = bfromcstr("G1");
    d2->labels[2] = bfromcstr("G2");
    d2->labels[3] = bfromcstr("G3");
    d2->vals[0] = 0;
    d2->vals[1] = SIMULATE_G1;
    d2->vals[2] = SIMULATE_G2;
    d2->vals[3] = SIMULATE_G3;

    if(pracdata_mirror_existed) {
        game_type_ = q->game_type;
        if(game_type_ == 0) {
            d2->selection = 0;
        } else if(game_type_ == SIMULATE_G1) {
            d2->selection = 1;
        } else if(game_type_ == SIMULATE_G2) {
            d2->selection = 2;
        } else {
            d2->selection = 3;
        }
    } else {
        d2->selection = 2;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 438;
    m->y = 8*16 + 3*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[10] = menu_opt_create(MENU_TOGGLE, qs_update_pracdata, bfromcstr("INVISIBLE"));
    m = d->menu[10];
    d8 = m->data;
    d8->param = &q->pracdata->invisible;
    d8->labels[0] = bfromcstr("OFF");
    d8->labels[1] = bfromcstr("ON");
    m->x = 438;
    m->y = 8*16 + 4*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[11] = menu_opt_create(MENU_TOGGLE, qs_update_pracdata, bfromcstr("BRACKETS"));
    m = d->menu[11];
    d8 = m->data;
    d8->param = &q->pracdata->brackets;
    d8->labels[0] = bfromcstr("OFF");
    d8->labels[1] = bfromcstr("ON");
    m->x = 438;
    m->y = 8*16 + 5*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[12] = menu_opt_create(MENU_TOGGLE, NULL, bfromcstr("INFINITE\nFLOORKICKS"));
    m = d->menu[12];
    d8 = m->data;
    d8->param = &q->pracdata->infinite_floorkicks;
    d8->labels[0] = bfromcstr("OFF");
    d8->labels[1] = bfromcstr("ON");
    m->x = 438;
    m->y = 8*16 + 6*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y + 18;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->label_text_rgba = 0xA0A0FFFF;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[13] = menu_opt_create(MENU_MULTIOPT, NULL, bfromcstr("LOCK PROTECT"));
    m = d->menu[13];
    d2 = m->data;
    d2->num = 3;
    d2->param = &q->pracdata->lock_protect;
    d2->vals = malloc(3 * sizeof(int));
    d2->labels = malloc(3 * sizeof(bstring));
    d2->labels[0] = bfromcstr("DEFAULT");
    d2->labels[1] = bfromcstr("OFF");
    d2->labels[2] = bfromcstr("ON");
    d2->vals[0] = -1;
    d2->vals[1] = 0;
    d2->vals[2] = 1;

    if(pracdata_mirror_existed) {
        lock_protect_ = q->pracdata->lock_protect;
        d2->selection = lock_protect_ + 1;
    } else {
        d2->selection = 0;
        (*d2->param) = d2->vals[d2->selection];
    }

    m->x = 438;
    m->y = 8*16 + 8*18;
    m->value_x = m->x + 15*13;
    m->value_y = m->y + 18;
    m->label_text_flags = DRAWTEXT_THIN_FONT;
    m->label_text_rgba = 0xC0C020FF;
    m->value_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;

    //
    /* */
    //

    d->menu[MENU_PRACTICE_NUMOPTS - 1] = menu_opt_create(MENU_TEXTINPUT, qs_update_pracdata, bfromcstr("PIECE SEQUENCE"));
    m = d->menu[MENU_PRACTICE_NUMOPTS - 1];
    d7 = m->data;
    d7->visible_chars = 16;
    m->x = 16*16 + 28*13;
    m->y = 8*16 + 14*18;
    m->value_x = m->x - 16*13;
    m->value_y = m->y + 18;
    m->label_text_flags = DRAWTEXT_THIN_FONT|DRAWTEXT_ALIGN_RIGHT;
    m->value_text_flags = DRAWTEXT_THIN_FONT;//|DRAWTEXT_ALIGN_RIGHT;

    qs_update_pracdata(cs);

    return 0;
}

int mload_options(game_t *g, int val)
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
    d->title = bfromcstr("CONTROLS");
    d->x = 4;
    d->y = 3;
*/

    return 0;
}

int mload_replay(game_t *g, int val)
{
    menudata *d = (menudata *)(g->data);
    struct menu_opt *m = NULL;
    struct action_opt_data *d1 = NULL;
    struct game_opt_data *d4 = NULL;

    int i = 0;
    int j = 0;
    int k = 0;
    int num = 0;

    struct bstrList *filenames = NULL;
    struct bstrList *filenames_sorted = NULL;

    struct replay *r = NULL;
    struct replay **replaylist = NULL;

    bstring label = NULL;
    nz_timer *t = nz_timer_create(60);

    struct tm* ts;
    char strbuf[80];

    menu_clear(g);        // data->menu guaranteed to be NULL upon return

    d->menu = NULL;
    d->menu_id = MENU_ID_REPLAY;
    d->use_target_tex = 1;
    d->selection = 0;
    d->numopts = 0;
    d->title = bfromcstr("REPLAY");
    d->x = 20;
    d->y = 16;

    d->is_paged = 1;
    d->page = 0;
    d->page_length = 20;
    d->page_text_x = 640-16;
    d->page_text_y = 16;

    filenames = get_replay_list();
    if(filenames) {
        filenames_sorted = malloc(sizeof(struct bstrList));
        filenames_sorted->qty = filenames->qty;
        filenames_sorted->entry = malloc(2*filenames->qty * sizeof(bstring));

        for(i = 0; i < filenames->qty; i++) {
            r = read_replay_file((char *)(filenames->entry[i]->data), 0);
            if(!r) {
                continue;
            }

            if(!num) {
                replaylist = malloc(2*filenames->qty * sizeof(struct replay *));
                num++;
                replaylist[0] = r;
                filenames_sorted->entry[0] = bstrcpy(filenames->entry[i]);
                continue;
            }

            if(num == 1) {
                if(r == compare_replays(r, replaylist[0])) {
                    replaylist[1] = replaylist[0];
                    filenames_sorted->entry[1] = filenames_sorted->entry[0];
                    replaylist[0] = r;
                    filenames_sorted->entry[0] = bstrcpy(filenames->entry[i]);
                } else {
                    replaylist[1] = r;
                    filenames_sorted->entry[1] = bstrcpy(filenames->entry[i]);
                }

                num++;
                continue;
            }

            for(j = 0; j < num; j++) {
                if(r == compare_replays(r, replaylist[j])) {
                    for(k = num+1; k > j; k--) {
                        replaylist[k] = replaylist[k-1];
                        filenames_sorted->entry[k] = filenames_sorted->entry[k-1];
                    }

                    replaylist[j] = r;
                    filenames_sorted->entry[j] = bstrcpy(filenames->entry[i]);
                    break;
                }

                replaylist[num] = r;
                filenames_sorted->entry[num] = bstrcpy(filenames->entry[i]);
            }

            num++;
        }

        replaylist = realloc(replaylist, num * sizeof(struct replay *));
        filenames_sorted->entry = realloc(filenames_sorted->entry, num * sizeof(bstring));
        filenames_sorted->qty = num;

        bstrListDestroy(filenames);

        d->menu = malloc((num+1) * sizeof(struct menu_opt *));
        d->numopts = num+1;
        d->menu[0] = menu_opt_create(MENU_ACTION, NULL, bfromcstr("RETURN"));
        m = d->menu[0];
        d1 = m->data;
        d1->action = mload_main;
        d1->val = 0;
        m->x = 20;
        m->y = 60;
        m->label_text_flags = DRAWTEXT_THIN_FONT;

        for(i = 1; i < num+1; i++) {
            d->menu[i] = menu_opt_create(MENU_GAME, NULL, NULL);
            r = replaylist[i-1];
            t->time = r->time;
            ts = localtime(&r->date);
            strftime(strbuf, sizeof(strbuf), "%Y.%m.%d", &ts);

            switch(replaylist[i-1]->mode) {
                case MODE_UNSPECIFIED:
                    label = bformat("%s  PENTOMINO  %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                case MODE_G2_DEATH:
                    label = bformat("%s  G2 DEATH   %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                case MODE_G3_TERROR:
                    label = bformat("%s  G3 TERROR  %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                case MODE_G1_20G:
                    label = bformat("%s  G1 20G     %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                case MODE_G1_MASTER:
                    label = bformat("%s  G1 MASTER  %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                case MODE_G2_MASTER:
                    label = bformat("%s  G2 MASTER  %4d-%-4d  %02d:%02d:%02d   %s", get_grade_name(r->grade), r->starting_level, r->ending_level, timegetmin(t), timegetsec(t) % 60, timegetmsec(t)/10, strbuf);
                    break;
                default:
                    break;
            }

            d->menu[i]->label = label;
            m = d->menu[i];
            d4 = m->data;
            d4->mode = QUINTESSE;
            d4->args.num = 4;
            d4->args.ptrs = malloc(4 * sizeof(void *));
            d4->args.ptrs[0] = malloc(sizeof(coreState *));
            d4->args.ptrs[1] = malloc(sizeof(int));
            d4->args.ptrs[2] = malloc(sizeof(unsigned int));
            d4->args.ptrs[3] = malloc(sizeof(char *));
            *(coreState **)(d4->args.ptrs[0]) = g->origin;
            *(int *)(d4->args.ptrs[1]) = 0;
            *(unsigned int *)(d4->args.ptrs[2]) = r->mode;
            *(char **)(d4->args.ptrs[3]) = malloc((filenames_sorted->entry[i-1]->slen+1) * sizeof(char));
            strcpy(*(char **)(d4->args.ptrs[3]), (char *)(filenames_sorted->entry[i-1]->data));
            m->x = 20 - 13;
            m->y = 60 + (i % 20) * 20;
            m->label_text_flags = DRAWTEXT_THIN_FONT;
            m->label_text_rgba = (i % 2) ? 0xA0A0FFFF : RGBA_DEFAULT;
        }

        if(filenames_sorted->entry) {
            for(i = 0; i < num; i++) {
                if(filenames_sorted->entry[i])
                    bdestroy(filenames_sorted->entry[i]);
            }

            free(filenames_sorted->entry);
        }

        free(filenames_sorted);
    }

    nz_timer_destroy(t);

    if(replaylist) {
        for(i = 0; i < num; i++) {
            free(replaylist[i]);
        }

        free(replaylist);
    }

    return 0;
}

int menu_action_quit(game_t *g, int val)
{
    return 1;
}

int menu_is_practice(game_t *g)
{
    if(!g)
        return 0;

    menudata *d = g->data;
    if(d->menu_id == MENU_ID_PRACTICE)
        return 1;

    else return 0;
}
