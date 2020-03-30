#include "SDL.h"
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <utility>

#include "core.h"
#include "Grid.hpp"
#include "PieceDef.hpp"
#include "qrs.h"
#include "random.h"
#include "replay.h"
#include "Timer.hpp"

#include "game_menu.h" // questionable dependency - TODO look into these
#include "game_qs.h"   // questionable dependency
#include "gfx.h"       // questionable dependency
#include "gfx_qs.h"    // very questionable dependency
#include "gfx_structures.h"

#include "rotation_tables.h"

using namespace Shiro;
using namespace std;

const char *qrspiece_names[25] = {"I", "J", "L",  "X",  "S", "Z",       "N",  "G",  "U",  "T", "Fa", "Fb", "P",
                                  "Q", "W", "Ya", "Yb", "V", /**/ "I4", "T4", "J4", "L4", "O", "S4", "Z4"};

/* */

QRS_Timings::QRS_Timings() : QRS_Timings(0u, 4, 30, 14, 30, 30, 40) {}

QRS_Timings::QRS_Timings(unsigned level, int grav, int lock, int das, int are, int lineare, int lineclear) :
    level(level),
    grav(grav),
    lock(lock),
    das(das),
    are(are),
    lineare(lineare),
    lineclear(lineclear) {}

QRS_Counters::QRS_Counters() :
    init(0),
    lock(0),
    are(0),
    lineare(0),
    lineclear(0),
    floorkicks(0u),
    hold_flash(0) {}

const string get_qrspiece_name(size_t n)
{
    if(n < 0 || n > 24)
        return {};

    return {qrspiece_names[n]};
}

void qrsdata_destroy(qrsdata *q)
{
    if(!q)
        return;

    int i;

    // if(q->p1->def) piecedef_destroy(q->p1->def);
    free(q->p1);
    delete q->p1counters;

    if(q->replay)
    {
        free(q->replay);
    }

    delete q->hold;
    delete q;
}

void pracdata_destroy(pracdata *d)
{
    if(!d)
        return;

    int i = 0;

    if (d->usr_timings) {
        delete d->usr_timings;
    }
}

pracdata *pracdata_cpy(pracdata *d)
{
    if(!d)
        return NULL;

    pracdata *cpy = (pracdata *)malloc(sizeof(pracdata));
    int i = 0;

    for(i = 0; i < d->usr_seq_len; i++)
    {
        cpy->usr_sequence[i] = d->usr_sequence[i];
    }

    for(i = 0; i < d->usr_seq_expand_len; i++)
    {
        cpy->usr_seq_expand[i] = d->usr_seq_expand[i];
    }

    cpy->usr_seq_len = d->usr_seq_len;
    cpy->usr_seq_expand_len = d->usr_seq_expand_len;

    cpy->usr_field_undo = d->usr_field_undo;
    cpy->usr_field_redo = d->usr_field_redo;

    cpy->field_edit_in_progress = 0;

    cpy->usr_field = d->usr_field;

    cpy->palette_selection = QRS_PIECE_GARBAGE;
    cpy->field_selection = d->field_selection;
    cpy->field_selection_vertex1_x = d->field_selection_vertex1_x;
    cpy->field_selection_vertex1_y = d->field_selection_vertex1_y;
    cpy->field_selection_vertex2_x = d->field_selection_vertex2_x;
    cpy->field_selection_vertex2_y = d->field_selection_vertex2_y;

    cpy->usr_timings = new QRS_Timings(*d->usr_timings);
    cpy->usr_timings->level = 0;
    /*
    cpy->usr_timings->grav = d->usr_timings->grav;
    cpy->usr_timings->lock = d->usr_timings->lock;
    cpy->usr_timings->are = d->usr_timings->are;
    cpy->usr_timings->lineare = d->usr_timings->lineare;
    cpy->usr_timings->das = d->usr_timings->das;
    cpy->usr_timings->lineclear = d->usr_timings->lineclear;
    */

    cpy->hist_index = 0;
    cpy->paused = QRS_FIELD_EDIT;
    cpy->grid_lines_shown = d->grid_lines_shown;
    cpy->brackets = d->brackets;
    cpy->invisible = d->invisible;
    cpy->lock_protect = d->lock_protect;
    cpy->infinite_floorkicks = d->infinite_floorkicks;
    cpy->piece_subset = d->piece_subset;
    cpy->randomizer_seed = d->randomizer_seed;

    return cpy;
}

vector<PieceDef> qrspool_create()
{
    vector<PieceDef> pool(25);
    int i = 0;
    int j = 0;
    int n = 5;

    for(i = 0; i < 25; i++)
    {
        if(i >= 18)
        {
            n = 4;
        }

        pool[i].qrsID = i;
        pool[i].flags = PDNONE;
        pool[i].anchorX = ANCHORX_QRS;
        pool[i].anchorY = ANCHORY_QRS;

        for(j = 0; j < 4; j++)
        {
            if (n == 5) {
                pool[i].rotationTable[j] = qrspent_yx_rotation_tables[i][j];
            }
            else if (n == 4) {
                pool[i].rotationTable[j] = qrstet_yx_rotation_tables[i - 18][j];
            }
        }

        if(!(i == QRS_I || i == QRS_N || i == QRS_G || i == QRS_J || i == QRS_L ||
             i == QRS_T || i == QRS_Ya || i == QRS_Yb || i == QRS_I4 || i == QRS_T4))
        {
            pool[i].flags = static_cast<PieceDefFlag>(pool[i].flags ^ PDNOFKICK);
        }

        if(i == QRS_T)
        {
            pool[i].flags = static_cast<PieceDefFlag>(pool[i].flags | PDFLATFLOORKICKS | PDONECELLFLOORKICKS | PDPREFERWKICK | PDAIRBORNEFKICKS);
        }

        if(i == QRS_T4)
        {
            pool[i].flags = static_cast<PieceDefFlag>(pool[i].flags | PDFLIPFLOORKICKS | PDONECELLFLOORKICKS | PDPREFERWKICK | PDAIRBORNEFKICKS);
        }
    }

    return pool;
}

Grid *qrsfield_create()
{
    return new Grid(QRS_FIELD_W, QRS_FIELD_H);
}

int qrsfield_set_w(Grid *field, int w)
{
    if(w % 2 || w < 4 || w > 12 || !field)
        return 1;

    int i = 0;
    int j = 0;

    for(i = 0; i < field->getWidth(); i++)
    {
        for(j = 0; j < field->getHeight(); j++)
        {
            if (field->getCell(i, j) == QRS_FIELD_W_LIMITER) {
                field->setCell(i, j, 0);
            }
        }
    }

    for(i = 0; i < (QRS_FIELD_W - w) / 2; i++)
    {
        for(j = 0; j < QRS_FIELD_H; j++)
        {
            field->setCell(i, j, QRS_FIELD_W_LIMITER);
            field->setCell(QRS_FIELD_W - i - 1, j, QRS_FIELD_W_LIMITER);
        }
    }

    return 0;
}

int qrsfield_clear(Grid *field) { return 0; }

int ufu_not_exists(coreState *cs)
{
    if(!cs->p1game)
        return 1;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    if(!q)
        return 1;

    if((q->pracdata->usr_field_undo.size() || q->pracdata->usr_field_redo.size()) && q->pracdata->paused == QRS_FIELD_EDIT)
        return 0;
    else
        return 1;

    return 1;
}

int usr_field_bkp(coreState *cs, pracdata *d) {
    if (!d) {
        return 1;
    }

    if (!d->usr_field_undo.size()) {
        gfx_createbutton(
            cs, "CLEAR UNDO", QRS_FIELD_X + (16 * 16) - 6, QRS_FIELD_Y + 23 * 16 + 8 - 6, 0, push_undo_clear_confirm, ufu_not_exists, NULL, 0xC0C0FFFF);
    }
    d->usr_field_undo.push_back(d->usr_field);

    d->usr_field_redo.clear();

    return 0;
}

int usr_field_undo(coreState *cs, pracdata *d)
{
    if (!d) {
        return 1;
    }

    if (!d->usr_field_undo.size()) {
        return 0;
    }

    d->usr_field_redo.push_back(d->usr_field);

    d->usr_field = d->usr_field_undo.back();
    d->usr_field_undo.pop_back();

    return 0;
}

int usr_field_redo(coreState *cs, pracdata *d)
{
    if(!d)
    {
        return 1;
    }

    if (!d->usr_field_redo.size()) {
        return 0;
    }

    d->usr_field_undo.push_back(d->usr_field);

    d->usr_field = d->usr_field_redo.back();
    d->usr_field_redo.pop_back();

    return 0;
}

int push_undo_clear_confirm(coreState *cs, void *data)
{
    struct text_formatting *fmt = text_fmt_create(DRAWTEXT_CENTERED, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);

    cs->button_emergency_override = 1;

    gfx_pushmessage(
        cs, "CONFIRM DELETE\nUNDO HISTORY?", 640 / 2 - 7 * 16, 480 / 2 - 16, MESSAGE_EMERGENCY, monofont_square, fmt, -1, button_emergency_inactive);

    gfx_createbutton(
        cs, "YES", 640 / 2 - 6 * 16 - 6, 480 / 2 + 3 * 16 - 6, BUTTON_EMERGENCY, undo_clear_confirm_yes, button_emergency_inactive, NULL, 0xB0FFB0FF);
    gfx_createbutton(
        cs, "NO", 640 / 2 + 4 * 16 - 6, 480 / 2 + 3 * 16 - 6, BUTTON_EMERGENCY, undo_clear_confirm_no, button_emergency_inactive, NULL, 0xFFA0A0FF);

    return 0;
}

int undo_clear_confirm_yes(coreState *cs, void *data)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;
    usr_field_undo_clear(cs, data);
    if(q->pracdata->field_edit_in_progress)
        q->pracdata->field_edit_in_progress = 0;

    cs->button_emergency_override = 0;
    cs->mouse_left_down = 0;
    return 0;
}

int undo_clear_confirm_no(coreState *cs, void *data)
{
    cs->button_emergency_override = 0;
    cs->mouse_left_down = 0;
    return 0;
}

int usr_field_undo_clear(coreState *cs, void *data)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;

    q->pracdata->usr_field_undo.clear();
    q->pracdata->usr_field_redo.clear();

    return 0;
}

int qrs_input(game_t *g)
{
    coreState *cs = g->origin;
    struct keyflags *k = NULL;

    qrsdata *q = (qrsdata *)g->data;
    pracdata *d = q->pracdata;
    qrs_player *p = q->p1;

    int i = 0;
    int j = 0;
    int c = 0;
    int init = 120;

    int cell_x = 0;
    int cell_y = 0;

    int palette_cell_x = 0;
    int palette_cell_y = 0;

    int lesser_x = 0;
    int greater_x = 0;
    int lesser_y = 0;
    int greater_y = 0;

    int edit_action_occurred = 0;

    int moved_left = 0;
    int moved_right = 0;

    int scale = 1;
    if(cs->settings)
    {
        scale = cs->settings->videoScale;
    }

    init = q->p1counters->init;

    if(d)
    {
        if(d->paused == QRS_FIELD_EDIT)
        {
            cell_x = (cs->mouse_x - q->field_x * scale) / (16 * scale) - 1;
            cell_y = (cs->mouse_y - q->field_y * scale) / (16 * scale) - 2;
            palette_cell_x = (cs->mouse_x - FIELD_EDITOR_PALETTE_X * scale) / (16 * scale);
            palette_cell_y = (cs->mouse_y - FIELD_EDITOR_PALETTE_Y * scale) / (16 * scale);

            if(cs->select_all && !cs->text_editing)
            {
                d->field_selection = 1;
                d->field_selection_vertex1_x = 0;
                d->field_selection_vertex1_y = 0;
                d->field_selection_vertex2_x = 11;
                d->field_selection_vertex2_y = 19;
            }

            if(cs->undo && !d->field_edit_in_progress)
                usr_field_undo(cs, d);

            if(cs->redo && !d->field_edit_in_progress)
                usr_field_redo(cs, d);

            if(SDL_GetModState() & KMOD_SHIFT && cs->mouse_left_down)
            {
                if(cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
                {
                    d->field_selection = 1;
                    d->field_selection_vertex1_x = cell_x;
                    d->field_selection_vertex1_y = cell_y;
                }

                d->field_selection_vertex2_x = cell_x;
                d->field_selection_vertex2_y = cell_y;
            }
            else
            {
                if(cs->mouse_left_down)
                {
                    if(palette_cell_x == 0)
                    {
                        switch(palette_cell_y)
                        {
                            case 0:
                                d->palette_selection = QRS_X + 1;
                                break;
                            case 1:
                                d->palette_selection = QRS_N + 1;
                                break;
                            case 2:
                                d->palette_selection = QRS_G + 1;
                                break;
                            case 3:
                                d->palette_selection = QRS_U + 1;
                                break;
                            case 4:
                                d->palette_selection = QRS_T + 1;
                                break;
                            case 5:
                                d->palette_selection = QRS_Fa + 1;
                                break;
                            case 6:
                                break;
                            case 7:
                                d->palette_selection = QRS_I4 + 1;
                                break;
                            case 8:
                                d->palette_selection = QRS_T4 + 1;
                                break;
                            case 9:
                                d->palette_selection = QRS_J4 + 1;
                                break;
                            case 10:
                                d->palette_selection = QRS_L4 + 1;
                                break;
                            case 11:
                                d->palette_selection = QRS_O + 1;
                                break;
                            case 12:
                                d->palette_selection = QRS_S4 + 1;
                                break;
                            case 13:
                                d->palette_selection = QRS_Z4 + 1;
                                break;
                            case 14:
                                d->palette_selection = QRS_PIECE_GARBAGE;
                                break;
                            case 15:
                                d->palette_selection = QRS_PIECE_BRACKETS;
                                break;
                            case 16:
                                d->palette_selection = QRS_PIECE_GEM;
                                break;
                            default:
                                break;
                        }
                    }
                    else if(d->field_selection)
                    {
                        if(cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
                        {
                            d->field_selection = 0;
                            cs->mouse_left_down = 0;
                        }
                    }
                    else if(cs->mouse_left_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
                    {
                        if(d->usr_field.getCell(cell_x, cell_y + 4) != QRS_FIELD_W_LIMITER)
                        {
                            if(d->palette_selection != QRS_PIECE_GEM)
                            {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                d->usr_field.cell(cell_x, cell_y + 4) = d->palette_selection;
                            }
                            else if(d->usr_field.getCell(cell_x, cell_y + 4) > 0)
                            {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                d->usr_field.cell(cell_x, cell_y + 4) |= QRS_PIECE_GEM;
                            }
                        }
                    }
                }
                else if(cs->mouse_right_down)
                {
                    if(d->field_selection)
                    {
                        if(cs->mouse_right_down == BUTTON_PRESSED_THIS_FRAME)
                        {
                            d->field_selection = 0;
                            cs->mouse_right_down = 0;
                        }
                    }
                    else if(cs->mouse_right_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
                    {
                        if(d->usr_field.cell(cell_x, cell_y + 4) != QRS_FIELD_W_LIMITER)
                        {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            d->usr_field.cell(cell_x, cell_y + 4) = 0;
                        }
                    }
                }

                if(cs->delete_das == 2 || cs->backspace_das == 2)
                {
                    if(d->field_selection && !cs->text_editing)
                    {
                        if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x)
                        {
                            lesser_x = d->field_selection_vertex1_x;
                            greater_x = d->field_selection_vertex2_x;
                        }
                        else
                        {
                            lesser_x = d->field_selection_vertex2_x;
                            greater_x = d->field_selection_vertex1_x;
                        }

                        if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y)
                        {
                            lesser_y = d->field_selection_vertex1_y;
                            greater_y = d->field_selection_vertex2_y;
                        }
                        else
                        {
                            lesser_y = d->field_selection_vertex2_y;
                            greater_y = d->field_selection_vertex1_y;
                        }

                        for(i = lesser_x; i <= greater_x; i++)
                        {
                            for(j = lesser_y; j <= greater_y; j++)
                            {
                                if(i >= 0 && i < 12 && j >= 0 && j < 20)
                                {
                                    if(d->usr_field.getCell(i, j + 4) != QRS_FIELD_W_LIMITER)
                                    {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        d->usr_field.cell(i, j + 4) = 0;
                                    }
                                }
                            }
                        }

                        d->field_selection = 0;
                    }
                }
            }

            c = 0;
            if(cs->zero_pressed)
            {
                c = d->palette_selection;
                if(d->field_selection)
                    cs->zero_pressed = 0;
            }
            if(cs->one_pressed)
            {
                c = 19;
                if(d->field_selection)
                    cs->one_pressed = 0;
            }
            if(cs->two_pressed)
            {
                c = 20;
                if(d->field_selection)
                    cs->two_pressed = 0;
            }
            if(cs->three_pressed)
            {
                c = 21;
                if(d->field_selection)
                    cs->three_pressed = 0;
            }
            if(cs->four_pressed)
            {
                c = 22;
                if(d->field_selection)
                    cs->four_pressed = 0;
            }
            if(cs->five_pressed)
            {
                c = 23;
                if(d->field_selection)
                    cs->five_pressed = 0;
            }
            if(cs->six_pressed)
            {
                c = 24;
                if(d->field_selection)
                    cs->six_pressed = 0;
            }
            if(cs->seven_pressed)
            {
                c = 25;
                if(d->field_selection)
                    cs->seven_pressed = 0;
            }
            if(cs->nine_pressed)
            {
                c = QRS_PIECE_BRACKETS;
                if(d->field_selection)
                    cs->nine_pressed = 0;
            }

            if(c && d->field_selection)
            {
                if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x)
                {
                    lesser_x = d->field_selection_vertex1_x;
                    greater_x = d->field_selection_vertex2_x;
                }
                else
                {
                    lesser_x = d->field_selection_vertex2_x;
                    greater_x = d->field_selection_vertex1_x;
                }

                if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y)
                {
                    lesser_y = d->field_selection_vertex1_y;
                    greater_y = d->field_selection_vertex2_y;
                }
                else
                {
                    lesser_y = d->field_selection_vertex2_y;
                    greater_y = d->field_selection_vertex1_y;
                }

                for(i = lesser_x; i <= greater_x; i++)
                {
                    for(j = lesser_y; j <= greater_y; j++)
                    {
                        if(i >= 0 && i < 12 && j >= 0 && j < 20)
                        {
                            if(d->usr_field.getCell(i, j + 4) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
                            {
                                if(SDL_GetModState() & KMOD_SHIFT)
                                {
                                    if(IS_STACK(d->usr_field.getCell(i, j + 4)))
                                    {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        d->usr_field.cell(i, j + 4) = c;
                                    }
                                }
                                else
                                {
                                    if(!d->field_edit_in_progress)
                                        usr_field_bkp(cs, d);
                                    d->field_edit_in_progress = 1;
                                    edit_action_occurred = 1;
                                    d->usr_field.cell(i, j + 4) = c;
                                }
                            }
                            else if(d->usr_field.getCell(i, j + 4) > 0 && c == QRS_PIECE_GEM)
                            {
                                if(SDL_GetModState() & KMOD_SHIFT)
                                {
                                    if(IS_STACK(d->usr_field.getCell(i, j + 4)))
                                    {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        d->usr_field.cell(i, j + 4) |= c;
                                    }
                                }
                                else
                                {
                                    if(!d->field_edit_in_progress)
                                        usr_field_bkp(cs, d);
                                    d->field_edit_in_progress = 1;
                                    edit_action_occurred = 1;
                                    d->usr_field.cell(i, j + 4) |= c;
                                }
                            }
                        }
                    }
                }

                d->field_selection = 0;
            }
            else if(c)
            {
                if(cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
                {
                    if(d->usr_field.getCell(cell_x, cell_y + 4) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
                    {
                        if(SDL_GetModState() & KMOD_SHIFT)
                        {
                            if(IS_STACK(d->usr_field.getCell(cell_x, cell_y + 4)))
                            {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                d->usr_field.cell(cell_x, cell_y + 4) = c;
                            }
                        }
                        else
                        {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            d->usr_field.cell(cell_x, cell_y + 4) = c;
                        }
                    }
                    else if(d->usr_field.getCell(cell_x, cell_y + 4) > 0 && c == QRS_PIECE_GEM)
                    {
                        if(SDL_GetModState() & KMOD_SHIFT)
                        {
                            if(IS_STACK(d->usr_field.getCell(cell_x, cell_y + 4)))
                            {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                d->usr_field.cell(cell_x, cell_y + 4) |= c;
                            }
                        }
                        else
                        {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            d->usr_field.cell(cell_x, cell_y + 4) |= c;
                        }
                    }
                }
            }

            if(!edit_action_occurred)
            {
                d->field_edit_in_progress = 0;
            }
        }
    }

    if(cs->menu && cs->menu_input_override)
    {
        return 0;
    }

    // hacky way to go back to the practice menu if a game is running from that menu
    if(cs->pressed.escape)
    {
        if(menu_is_practice(cs->menu))
        {
            cs->menu_input_override = 1;
            if(d)
            {
                d->paused = QRS_FIELD_EDIT;
                qs_update_pracdata(cs);
                if(!ufu_not_exists(cs))
                {
                    // if the clear undo button exists? doesn't exist? i feel like i should know what this means but i do not :|
                    // TODO use something more sane to detect for this sort of thing
                    gfx_createbutton(cs,
                                     "CLEAR UNDO",
                                     QRS_FIELD_X + (16 * 16) - 6,
                                     QRS_FIELD_Y + 23 * 16 + 8 - 6,
                                     0,
                                     push_undo_clear_confirm,
                                     ufu_not_exists,
                                     NULL,
                                     0xC0C0FFFF);
                }
            }

            cs->pressed.escape = 0;

            return 0;
        }
        else
            return 1;
    }

    if(init < 120)
        return 0;

    k = &cs->keys;

    if(p->state & (PSFALL | PSLOCK) && !(p->state & PSPRELOCKED))
    {
        if(cs->pressed.a || cs->pressed.c)
        {
            if(qrs_rotate(g, p, CCW) == 0)
            {
                if(q->max_floorkicks != 0 && q->p1counters->floorkicks >= q->max_floorkicks)
                {
                    if(q->lock_on_rotate == 1)
                    {
                        q->lock_on_rotate = 2;
                    }
                    else
                        q->lock_on_rotate = 1;
                }
            }
        }

        if(cs->pressed.b)
        {
            if(qrs_rotate(g, p, CW) == 0)
            {
                if(q->max_floorkicks != 0 && q->p1counters->floorkicks >= q->max_floorkicks)
                {
                    if(q->lock_on_rotate == 1)
                    {
                        q->lock_on_rotate = 2;
                    }
                    else
                        q->lock_on_rotate = 1;
                }
            }
        }

        // if(k->d == 1) qrs_rotate(g, p, FLIP);
    }

    if(p->state & (PSFALL | PSLOCK) && !(p->state & PSPRELOCKED))
    {
        q->active_piece_time++;
        if(cs->pressed.left || (is_left_input_repeat(cs, 1 + p->speeds->das)))
        {
            qrs_move(g, p, MOVE_LEFT);
            /* moved_left = 1; */
        }

        if(cs->pressed.right || is_right_input_repeat(cs, 1 + p->speeds->das))
        {
            qrs_move(g, p, MOVE_RIGHT);
            /* moved_right = 1; */
        }

        /* if(moved_left && moved_right) { */
        /*     if(k->left < k->right) */
        /*         qrs_move(g, p, MOVE_LEFT); */
        /*     else if(k->right < k->left) */
        /*         qrs_move(g, p, MOVE_RIGHT); */
        /* } */

        if(!qrs_isonground(g, p))
        {
            p->state &= ~PSLOCK;
            p->state |= PSFALL;
        }

        if(k->down)
        {
            const bool lock_protect_enabled = q->is_practice ? q->pracdata->lock_protect : q->lock_protect;
            const bool should_lock_protect = lock_protect_enabled && q->lock_held;

            q->soft_drop_counter++;
            if(p->state & PSFALL)
            {
                qrs_fall(g, p, 256);
                if(qrs_isonground(g, p) && !should_lock_protect)
                {
                    qrs_fall(g, p, 256);
                    q->lock_held = 1;
                    p->state &= ~PSLOCK;
                    p->state &= ~PSFALL;
                    p->state |= PSLOCKPRESSED;
                }
            }
            else if(p->state & PSLOCK && !should_lock_protect)
            {
                q->lock_held = 1;
                p->state &= ~PSLOCK;
                p->state |= PSLOCKPRESSED;
            }
        }

        if(k->up == 1)
        {
            if(!(q->game_type == SIMULATE_G1))
            {
                int num_rows = qrs_fall(g, p, 20 * 256);
                if(num_rows > q->sonic_drop_height)
                    q->sonic_drop_height = num_rows;
            }
        }

        if(k->d == 1 && q->hold_enabled)
        {
            qrs_hold(g, p);
        }
    }

    if(!k->down)
    {
        if(q->lock_held)
            q->lock_held = 0;
    }

    return 0;
}

int qrs_start_record(game_t *g)
{
    qrsdata *q = (qrsdata *)g->data;

    g2_seed_bkp();

    q->replay = (struct replay *)malloc(sizeof(struct replay));

    memset(q->replay->pinputs, 0, sizeof(struct packed_input) * MAX_KEYFLAGS);

    q->replay->len = 0;
    q->replay->mlen = 36000;
    q->replay->mode = q->mode_type;
    q->replay->mode_flags = q->mode_flags;
    q->replay->seed = q->randomizer_seed;
    q->replay->grade = NO_GRADE;
    q->replay->time = 0;
    q->replay->starting_level = q->level;
    q->replay->ending_level = 0;

    q->replay->date = time(0);

    q->recording = 1;
    return 0;
}

int qrs_end_record(game_t *g)
{
    qrsdata *q = (qrsdata *)g->data;

    q->replay->time = q->timer;
    q->replay->ending_level = q->level;
    q->replay->grade = q->grade;

    scoredb_add(&g->origin->scores, &g->origin->player, q->replay);

    // TODO: Extract this into some (sum) method.
    int tetrisSum = 0;
    for(size_t i = 0; i < MAX_SECTIONS; i++)
    {
        tetrisSum += q->section_tetrises[i];
    }

    g->origin->player.tetrisCount += tetrisSum;

    scoredb_update_player(&g->origin->scores, &g->origin->player);

    g2_seed_restore();
    q->recording = 0;
    return 0;
}

int qrs_load_replay(game_t *g, int replay_id)
{
    qrsdata *q = (qrsdata *)g->data;

    q->replay = (struct replay *)malloc(sizeof(struct replay));
    scoredb_get_full_replay(&g->origin->scores, q->replay, replay_id);

    return 0;
}

int qrs_start_playback(game_t *g)
{
    qrsdata *q = (qrsdata *)g->data;

    q->playback = 1;
    q->playback_index = 0;

    return 0;
}

int qrs_end_playback(game_t *g)
{
    qrsdata *q = (qrsdata *)g->data;

    q->playback = 0;

    return 0;
}

int qrs_move(game_t *g, qrs_player *p, int offset)
{
    if(!g || !p)
        return -1;

    int bkp_x = p->x;
    p->x += offset;

    if(qrs_chkcollision(*g, *p))
    {
        p->x = bkp_x;
        return 1;
    }

    return 0;
}

int qrs_rotate(game_t *g, qrs_player *p, int direction)
{
    if(!g || !p)
        return -1;
    if(!(p->state & (PSFALL | PSLOCK)))
        return 1;

    int bkp_o = p->orient;
    // int c = p->def->color;

    p->orient = (p->orient + direction) & 3;

    if(qrs_chkcollision(*g, *p))
    {
        if(p->def->flags & PDPREFERWKICK)
        {
            if(qrs_wallkick(g, p))
            {
                if(qrs_floorkick(g, p))
                {
                    if(qrs_ceilingkick(g, p))
                    {
                        p->orient = bkp_o;
                        return 1;
                    }
                }
            }
        }
        else
        {
            if(qrs_floorkick(g, p))
            {
                if(qrs_wallkick(g, p))
                {
                    if(qrs_ceilingkick(g, p))
                    {
                        p->orient = bkp_o;
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

int qrs_proc_initials(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)(g->data);
    struct keyflags *k = &g->origin->keys;
    qrs_player *p = q->p1;

    if(k->d && q->hold_enabled)
    {
        qrs_hold(g, p);
    }

    qrs_irs(g);

    if(k->up && (q->game_type != SIMULATE_G1))
    {
        qrs_fall(g, p, 20 * 256);
    }

    return 0;
}

int qrs_irs(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)(g->data);
    struct keyflags *k = &g->origin->keys;
    qrs_player *p = q->p1;

    int direction = 0;

    if(k->a || k->c)
        direction = CCW;

    if(k->b)
        direction = CW;

    if((k->d || (k->a && k->c)) && q->special_irs && !q->hold_enabled)
    {
        direction = FLIP;
    }

    p->orient = direction;
    if(qrs_chkcollision(*g, *p))
        p->orient = 0;
    else if(direction)
    {
        g->origin->assets->prerotate->play(*g->origin->settings);
    }

    return 0;
}

int qrs_wallkick(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;

    piece_id c = p->def->qrsID;
    int o = p->orient;
    pair<int, int> pos;
    qrs_chkcollision(*g, *p, pos);
    int x = pos.first;
    // printf("Trying to kick with collision at x = %d\n", x);

    if(p->def->flags & PDNOWKICK)
        return 1;

    switch(c)
    {
        case QRS_I:
        case QRS_I4:
            if(o == CW || o == CCW)
                return 1;
            break;
        case QRS_J:
            if((o == CW || o == CCW) && x == 2)
                return 1;
            break;
        case QRS_L:
        case QRS_L4:
        case QRS_J4:
        case QRS_T4:
            if((o == CW || o == CCW) && x == 1)
                return 1;
            break;
        case QRS_N:
        case QRS_G:
        case QRS_Ya:
        case QRS_Yb:
            if(o == CW && x == 2)
                return 1;
            else if(o == CCW && x == 1)
                return 1;
            break;
        default:
            break;
    }

    if(qrs_move(g, p, MOVE_RIGHT))
    {
        if(qrs_move(g, p, MOVE_LEFT))
        {
            if(p->def->rotationTable[0].getWidth() == 4 && c != QRS_I4)
                return 1;
            if(c != QRS_I4 && c != QRS_I && c != QRS_J && c != QRS_L && c != QRS_Ya && c != QRS_Yb)
                return 1;
            if((c == QRS_J || c == QRS_L || c == QRS_Ya || c == QRS_Yb) && (p->orient == CW || p->orient == CCW))
                return 1;

            if(qrs_move(g, p, 2))
            {
                if(qrs_move(g, p, -2))
                    return 1;
            }
        }
    }

    return 0;
}

int qrs_hold(game_t *g, qrs_player *p)
{
    qrsdata* q = (qrsdata *)g->data;
    PieceDef* temp = NULL;

    if(!q->hold_enabled)
        return 1;

    if(p->state & PSUSEDHOLD)
        return 1;

    if(!q->hold)
    {
        q->hold = new PieceDef(*p->def);
        if(qs_initnext(g, p, INITNEXT_DURING_ACTIVE_PLAY) == 1)
        { // if there is no next piece to swap in
            delete q->hold;
            q->hold = NULL;

            return 1;
        }

        p->state |= PSUSEDHOLD;
    }
    else
    {
        temp = q->hold;
        q->hold = new PieceDef(*p->def);
        p->def = temp;

        if(p->def->qrsID >= 18) // tetrominoes spawn where they do in TGM
            p->y = ROWTOY(SPAWNY_QRS + 2);
        else
            p->y = ROWTOY(SPAWNY_QRS);

        p->x = SPAWNX_QRS;
        p->orient = FLAT;
        p->state = PSFALL | PSSPAWN | PSUSEDHOLD;

        q->p1counters->lock = 0;
        q->p1counters->hold_flash = 1;
    }

    return 0;
}

int qrs_floorkick(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;
    if(p->def->flags & PDNOFKICK)
        return 1;
    if(!(p->def->flags & PDAIRBORNEFKICKS) && !qrs_isonground(g, p))
        return 1;

    qrsdata *q = (qrsdata *)g->data;
    int bkp_y = p->y;

    if(!q->max_floorkicks)
    {
        return 1;
    }

    if(p->def->flags & PDFLIPFLOORKICKS)
    {
        if(p->orient != FLIP)
        {
            return 1;
        }
    }
    else if(p->def->flags & PDFLATFLOORKICKS)
    {
        if(p->orient == CW || p->orient == CCW)
        {
            return 1;
        }
    }
    else
    {
        if(p->orient == FLAT || p->orient == FLIP)
        {
            return 1;
        }
    }

    p->y -= 256;

    if(qrs_chkcollision(*g, *p))
    {
        if(p->def->flags & PDONECELLFLOORKICKS)
        {
            p->y = bkp_y;
            return 1;
        }

        p->y -= 256;
        if(qrs_chkcollision(*g, *p))
        {
            p->y = bkp_y;
            return 1;
        }
    }

    q->p1counters->floorkicks++;
    // printf("Number of floorkicks so far: %d\n", q->p1counters->floorkicks);

    return 0;
}

int qrs_ceilingkick(game_t *g, qrs_player *p)
{
    qrsdata *q = (qrsdata *)g->data;
    int bkp_y = p->y;

    if(q->mode_type != MODE_PENTOMINO)
    {
        return 1;
    }

    p->y += 256;
    if(qrs_chkcollision(*g, *p))
    {
        p->y = bkp_y;
        return 1;
    }

    return 0;
}

int qrs_fall(game_t *g, qrs_player *p, int grav)
{
    if(!g || !p)
        return -1;

    if(!grav)
        grav = p->speeds->grav;

    int bkp_y = p->y;

    while(p->y < (bkp_y + grav))
    {
        p->y += 256;
        if(qrs_chkcollision(*g, *p))
        {
            p->y -= (256 + (p->y & 255));

            if(p->state & PSFALL && grav != 28 * 256)
            {
                g->origin->assets->land->play(*g->origin->settings);
            }
            p->state &= ~PSFALL;
            p->state |= PSLOCK;
            return -1;
        }
    }

    if(p->y - bkp_y > grav)
        p->y = bkp_y + grav;

    return (YTOROW(p->y) - YTOROW(bkp_y));
}

int qrs_lock(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;

    qrsdata *q = (qrsdata *)g->data;
    Grid *d = &p->def->rotationTable[p->orient];
    Grid *f = g->field;

    int i = 0;
    int ax = ANCHORX_QRS;
    int ay = ANCHORY_QRS;
    piece_id c = p->def->qrsID;

    for (size_t from_y = 0, to_y = YTOROW(p->y) - ay; from_y < d->getHeight(); from_y++, to_y++) {
        for (size_t from_x = 0, to_x = p->x - ax; from_x < d->getWidth(); from_x++, to_x++) {
            if (d->getCell(from_x, from_y)) {
                int value = c + 1;
                if (p->def->flags & PDBRACKETS) {
                    value |= QRS_PIECE_BRACKETS;
                }
                if (q->state_flags & GAMESTATE_FADING) {
                    SET_PIECE_FADE_COUNTER(value, q->piece_fade_rate);
                }

                f->cell((int)to_x, (int)to_y) = value;
            }
        }
    }

    p->state &= ~(PSLOCK | PSFALL);
    // p->state |= PSPRELOCKFLASH1;
    g->origin->assets->lock->play(*g->origin->settings);

    return 0;
}

bool qrs_chkcollision(game_t& g, qrs_player& p) {
    pair<int, int> pos;
    return qrs_chkcollision(g, p, pos);
}

bool qrs_chkcollision(game_t& g, qrs_player& p, pair<int, int>& pos) {
    Grid *d = &p.def->rotationTable[p.orient];
    Grid *f = g.field;
    int d_x = 0;
    int d_y = 0;
    //int d_val = 0;
    int f_x = 0;
    int f_y = 0;
    //int f_val = 0;

    for (d_y = 0, f_y = YTOROW(p.y) - p.def->anchorY; d_y < d->getHeight(); d_y++, f_y++) {
        for (d_x = 0, f_x = p.x - p.def->anchorY; d_x < d->getWidth(); d_x++, f_x++) {
            if (d->getCell(d_x, d_y) && f->getCell(f_x, f_y)) {
                pos = pair(d_x, d_y);
                return true;
            }
        }
    }

    return false;
}

int qrs_isonground(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;

    p->y += 256;

    if(qrs_chkcollision(*g, *p))
    {
        p->y -= 256;
        return 1;
    }

    p->y -= 256;

    return 0;
}

int qrs_lineclear(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;

    qrsdata *q = (qrsdata *)g->data;

    int i = 0;
    int j = 0;
    int k = 0;
    int n = 0;
    int garbage = 0;
    bool gem = false;

    int row = YTOROW(p->y);

    for(i = row - 1; (i < row + 4) && (i < QRS_FIELD_H); i++)
    {
        k = 0;
        garbage = 0;

        int startX = 0;
        j = 0;

        while(g->field->getCell(j, i) == QRS_WALL)
        {
            startX++;
            j++;
        }

        for(; j < startX + q->field_w; j++)
        {
            int cell = g->field->getCell(j, i);

            if(cell && cell != QRS_WALL)
            {
                k++;
            }

            if(cell & QRS_PIECE_GEM)
            {
                gem = true;
            }

            if(cell == QRS_PIECE_GARBAGE)
            {
                garbage++;
            }
        }

        if(k == q->field_w && garbage != q->field_w)
        {
            n++;
            if(!(q->state_flags & GAMESTATE_BIGMODE))
            {
                gfx_qs_lineclear(g, i);
            }

            for(j = startX; j < startX + q->field_w; j++)
            {
                g->field->cell(j, i) = -2;
            }

            if(gem)
            {
                // sfx_play(&g->origin->assets->gem) the gem clear sound effect whenever we get one
            }
        }
    }

    return n;
}

int qrs_dropfield(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)g->data;
    Grid *field = g->field;

    int i = 0;
    int j = 0;
    int n = 0;

    for(i = QRS_FIELD_H - 1; i > 0; i--)
    {
        while(field->getCell(4, i - n) == -2)
            n++;

        if(i - n >= 0)
        {
            field->copyRow(i - n, i);
        }
        else
        {
            if(q->state_flags & GAMESTATE_BIGMODE)
            {
                for(j = 0; j < q->field_w; j++)
                {
                    field->cell(j, i) = 0;
                }

                field->cell(j, i) = QRS_WALL;
            }
            else
            {
                for(j = (QRS_FIELD_W - q->field_w) / 2; j < (QRS_FIELD_W / 2 + q->field_w / 2); j++)
                {
                    field->cell(j, i) = 0;
                }
            }
        }
    }

    return 0;
}

int qrs_spawn_garbage(game_t *g, unsigned int flags)
{
    qrsdata *q = (qrsdata *)g->data;
    int i = 0;

    if(!q->garbage.getWidth() || !q->garbage.getHeight())
    {
        // TODO
    }
    else if(flags & GARBAGE_COPY_BOTTOM_ROW)
    {
        for(i = 0; i < QRS_FIELD_H - 1; i++)
        {
            g->field->copyRow(i + 1, i);
        }

        g->field->copyRow(QRS_FIELD_H - 2, QRS_FIELD_H - 1);
        for(i = 0; i < QRS_FIELD_W; i++)
        {
            if(g->field->getCell(i, QRS_FIELD_H - 2) == QRS_FIELD_W_LIMITER)
            {
                continue;
            }

            if(g->field->getCell(i, QRS_FIELD_H - 2))
            {
                g->field->cell(i, QRS_FIELD_H - 1) = QRS_PIECE_GARBAGE;
            }
        }
    }
    else
    {
        // random garbage based on several factors (TODO)
    }

    return 0;
}

void qrs_embiggen(PieceDef& p)
{
    int xs[5] = {-1, -1, -1, -1, -1};
    int ys[5] = {-1, -1, -1, -1, -1};
    int k = 0;

    for(int r = 0; r < p.rotationTable.size(); r++)
    {
        p.rotationTable[0].resize(p.rotationTable[r].getWidth() + 1, p.rotationTable[r].getHeight() + 1);
        for(int i = 0; i < p.rotationTable[r].getWidth(); i++)
        {
            for(int j = 0; j < p.rotationTable[r].getHeight(); j++)
            {
                if(p.rotationTable[r].getCell(i, j))
                {
                    xs[k] = i;
                    ys[k] = j;
                    k++;
                }
            }
        }

        int index = rand() % k;
        int direction = rand() % 4;
        int tries = 0;

switchStatement:
        if(tries == 4)
        {
            k = 0;
            continue;
        }

        switch(direction)
        {
            case 0:
                if(ys[index] == 0 || p.rotationTable[r].getCell(xs[index], ys[index] - 1))
                {
                    direction++;
                    tries++;
                    goto switchStatement;
                }
                else
                {
                    p.rotationTable[r].cell(xs[index], ys[index] - 1) = 1;
                }

                break;

            case 1:
                if(xs[index] == (p.rotationTable[r].getWidth() - 1) || p.rotationTable[r].getCell(xs[index] + 1, ys[index]))
                {
                    direction++;
                    tries++;
                    goto switchStatement;
                }
                else
                {
                    p.rotationTable[r].cell(xs[index] + 1, ys[index]) = 1;
                }

                break;

            case 2:
                if(ys[index] == p.rotationTable[r].getHeight() - 1 || p.rotationTable[r].getCell(xs[index], ys[index] + 1))
                {
                    direction++;
                    tries++;
                    goto switchStatement;
                }
                else
                {
                    p.rotationTable[r].cell(xs[index], ys[index] + 1) = 1;
                }

                break;

            case 3:
                if(xs[index] == 0 || p.rotationTable[r].getCell(xs[index] - 1, ys[index]))
                {
                    direction = 0;
                    tries++;
                    goto switchStatement;
                }
                else
                {
                    p.rotationTable[r].cell(xs[index] - 1, ys[index]) = 1;
                }

                break;

            default:
                break;
        }

        k = 0;
    }
}
