#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_mixer.h"
#include <assert.h>
#include <string.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include "CoreState.h"
#include "Game.h"
#include "GameType.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "Player.h"
#include "QRS0.h"
#include "Records.h"
#include "RotationTables.h"
#include "Timer.h"
#include "asset/Sfx.h"
#include "game_menu.h" // questionable dependency - TODO look into these
#include "game_qs.h"   // questionable dependency
#include "gfx_old.h"   // questionable dependency
#include "gfx_qs.h"    // very questionable dependency
#include "gfx_structures.h"
#include "input/KeyFlags.h"
#include "input/Mouse.h"
#include "random.h"
#include "replay.h"

const char *qrspiece_names[25] = {"I5", "J5", "L5",  "X",  "S5", "Z5",       "N",  "G",  "U",  "T5", "Fa", "Fb", "P",
                                  "Q", "W", "Ya", "Yb", "V", /**/ "I", "T", "J", "L", "O", "S", "Z"};

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

const std::string get_qrspiece_name(size_t n)
{
    if(n > 24)
        return {};

    return {qrspiece_names[n]};
}

void qrsdata_destroy(qrsdata *q)
{
    if(!q)
        return;

    randomizer_destroy(q->randomizer);
    delete q->p1->def;
    free(q->p1);
    delete q->p1counters;

    if(q->replay)
    {
        free(q->replay);
    }

    delete q->hold;

    delete q;
}

/*pracdata::pracdata() :
    game_type_int(0),
    game_type(Shiro::GameType::SIMULATE_QRS),
    field_w(0),
    //long_history(nullptr),
    usr_sequence(),
    usr_seq_expand(),
    usr_seq_len(0u),
    usr_seq_expand_len(0u),
    field_edit_in_progress(false),
    palette_selection(0),
    field_selection(0),
    field_selection_vertex1_x(0),
    field_selection_vertex1_y(0),
    field_selection_vertex2_x(0),
    field_selection_vertex2_y(0),
    usr_timings(nullptr),
    paused(0),
    grid_lines_shown(false),
    brackets(false),
    invisible(false),
    hist_index(0),
    lock_protect(0),
    infinite_floorkicks(false),
    piece_subset(0),
    randomizer_seed(0l)
{
    for (int& item : usr_sequence) {
        item = 0;
    }
    for (int& item : usr_seq_expand) {
        item = 0;
    }
}*/

void pracdata_destroy(pracdata *d)
{
    if(!d)
        return;

    if (d->usr_timings) {
        delete d->usr_timings;
    }
}

pracdata *pracdata_cpy(pracdata *d)
{
    if(!d)
        return NULL;

    pracdata *cpy = (pracdata *)malloc(sizeof(pracdata));
    assert(cpy != nullptr);

    for(std::size_t i = 0; i < d->usr_seq_len; i++)
    {
        cpy->usr_sequence[i] = d->usr_sequence[i];
    }

    for(std::size_t i = 0; i < d->usr_seq_expand_len; i++)
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

std::vector<Shiro::PieceDefinition> qrspool_create()
{
    std::vector<Shiro::PieceDefinition> pool(25);
    int n = 5;

    for(std::size_t i = 0u; i < 25u; i++)
    {
        if(i >= 18u)
        {
            n = 4;
        }

        pool[i].qrsID = static_cast<std::uint8_t>(i);
        pool[i].flags = Shiro::PDNONE;
        pool[i].anchorX = ANCHORX_QRS;
        pool[i].anchorY = ANCHORY_QRS;

        for(std::size_t j = 0u; j < 4u; j++)
        {
            if (n == 5) {
                pool[i].rotationTable[j] = Shiro::PentoRotationTables[i][j];
            }
            else if (n == 4) {
                pool[i].rotationTable[j] = Shiro::TetroRotationTables[i - 18][j];
            }
        }

        if(!(i == QRS_I || i == QRS_N || i == QRS_G || i == QRS_J || i == QRS_L ||
             i == QRS_T || i == QRS_Ya || i == QRS_Yb || i == QRS_I4 || i == QRS_T4))
        {
            pool[i].flags = static_cast<Shiro::PieceDefinitionFlag>(pool[i].flags ^ Shiro::PDNOFKICK);
        }

        if((i == QRS_Ya) || (i == QRS_Yb) || (i == QRS_L) || (i == QRS_J) || (i == QRS_N) || (i == QRS_G))
        {
            pool[i].flags = static_cast<Shiro::PieceDefinitionFlag>(
                pool[i].flags | Shiro::PDONECELLFLOORKICKS);
        }

        if(i == QRS_T)
        {
            pool[i].flags = static_cast<Shiro::PieceDefinitionFlag>(
                pool[i].flags |
                Shiro::PDFLATFLOORKICKS |
                Shiro::PDONECELLFLOORKICKS |
                //Shiro::PDPREFERWKICK |
                Shiro::PDAIRBORNEFKICKS
            );
        }

        if(i == QRS_T4)
        {
            pool[i].flags = static_cast<Shiro::PieceDefinitionFlag>(
                pool[i].flags |
                Shiro::PDFLIPFLOORKICKS |
                Shiro::PDONECELLFLOORKICKS |
                Shiro::PDPREFERWKICK |
                Shiro::PDAIRBORNEFKICKS
            );
        }

        if(i >= 18u)
        {
            pool[i].flags = static_cast<Shiro::PieceDefinitionFlag>(pool[i].flags | Shiro::PDNOCEILKICKS);
        }
    }

    return pool;
}

Shiro::Grid *qrsfield_create()
{
    return new Shiro::Grid(QRS_FIELD_W, QRS_FIELD_H);
}

int qrsfield_set_w(Shiro::Grid *field, int w)
{
    if(w % 2 || w < 4 || w > 12 || !field)
        return 1;

    for(std::size_t i = 0; i < field->getWidth(); i++)
    {
        for(std::size_t j = 0; j < field->getHeight(); j++)
        {
            if (field->getCell(i, j) == QRS_FIELD_W_LIMITER) {
                field->setCell(i, j, 0);
            }
        }
    }

    for(int i = 0; i < (QRS_FIELD_W - w) / 2; i++)
    {
        for(int j = 0; j < QRS_FIELD_H; j++)
        {
            field->setCell(i, j, QRS_FIELD_W_LIMITER);
            field->setCell(QRS_FIELD_W - i - 1, j, QRS_FIELD_W_LIMITER);
        }
    }

    return 0;
}

int qrsfield_clear() { return 0; }

int undo_clear_button_should_activate(CoreState *cs)
{
    if(!cs->p1game)
        return 0;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    if(!q)
        return 0;

    if(!q->pracdata)
        return 0;

    if((q->pracdata->usr_field_undo.size() || q->pracdata->usr_field_redo.size()) && q->pracdata->paused == QRS_FIELD_EDIT)
        return 1;
    else
        return 0;

    return 0;
}

int undo_clear_button_should_deactivate(CoreState *cs)
{
    if(undo_clear_button_should_activate(cs) == 0)
        return 1;

    if(!cs->p1game)
        return 1;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    if(!q)
        return 1;

    if(!q->pracdata)
        return 1;

    if(q->pracdata->usr_field_locked)
    {
        return 1;
    }

    return 0;
}

int usr_field_undo_history_exists(CoreState *cs)
{
    if(!cs->p1game)
        return 0;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    if(!q)
        return 0;

    if(!q->pracdata)
        return 0;

    if(q->pracdata->usr_field_undo.size() > 0 && q->pracdata->paused == QRS_FIELD_EDIT)
        return 1;
    else
        return 0;

    return 0;
}

int usr_field_undo_history_not_exists(CoreState *cs)
{
    return usr_field_undo_history_exists(cs) == 0;
}

int usr_field_redo_history_exists(CoreState *cs)
{
    if(!cs->p1game)
        return 0;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    if(!q)
        return 0;

    if(!q->pracdata)
        return 0;

    if(q->pracdata->usr_field_redo.size() > 0 && q->pracdata->paused == QRS_FIELD_EDIT)
        return 1;
    else
        return 0;

    return 0;
}

int usr_field_redo_history_not_exists(CoreState *cs)
{
    return usr_field_redo_history_exists(cs) == 0;
}

int lock_usr_field(CoreState *cs, void *)
{
    if(!cs || !cs->p1game)
        return -1;

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(q->pracdata)
    {
        q->pracdata->usr_field_locked = true;
    }

    return 0;
}

int unlock_usr_field(CoreState *cs, void *)
{
    if(!cs || !cs->p1game)
        return -1;

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(q->pracdata)
    {
        q->pracdata->usr_field_locked = false;
    }

    return 0;
}

int usr_field_is_locked(CoreState *cs)
{
    if(!cs || !cs->p1game)
        return -1;

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(q->pracdata)
    {
        if(q->pracdata->usr_field_locked)
        {
            return 1;
        }
    }

    return 0;
}

int usr_field_is_unlocked(CoreState *cs)
{
    if(!cs || !cs->p1game)
        return -1;

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(q->pracdata)
    {
        if(!q->pracdata->usr_field_locked)
        {
            return 1;
        }
    }

    return 0;
}

int usr_field_bkp(CoreState *, pracdata *d) {
    if (!d) {
        return 1;
    }

    if (!d->usr_field_undo.size()) {
        /*gfx_createbutton(
            cs, "CLEAR UNDO", QRS_FIELD_X + (16 * 16) - 6, QRS_FIELD_Y + 23 * 16 + 8 - 6, 0, push_undo_clear_confirm, undo_clear_button_should_activate, NULL, 0xC0C0FFFF);*/
    }
    d->usr_field_undo.push_back(d->usr_field);

    d->usr_field_redo.clear();

    return 0;
}

int usr_field_undo(CoreState *, pracdata *d)
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

int usr_field_redo(CoreState *, pracdata *d)
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

int push_undo_clear_confirm(CoreState *cs, void *)
{
    cs->button_emergency_override = true;

    // this crashes the game because of not being able to load the font

    /*Shiro::MessageEntity::push(cs->gfx,
        Shiro::FontAsset::get(cs->assetMgr, "squarefont"),
        "CONFIRM DELETE\nUNDO HISTORY?",
        640 / 2 - 7 * 16,
        480 / 2 - 16,
        0,
        0,
        1.0f,
        RGBA_DEFAULT,
        SIZE_MAX,
        Shiro::GfxLayer::emergencyMessages,
        [cs] { return cs->button_emergency_inactive(); }
    );*/

    gfx_createbutton(
        cs, "YES", 640 / 2 - 6 * 16 - 6, 480 / 2 + 3 * 16 - 6, BUTTON_EMERGENCY, undo_clear_confirm_yes, [](CoreState* cs) { return (int)cs->button_emergency_inactive(); }, NULL, 0xB0FFB0FF);
    gfx_createbutton(
        cs, "NO", 640 / 2 + 4 * 16 - 6, 480 / 2 + 3 * 16 - 6, BUTTON_EMERGENCY, undo_clear_confirm_no, [](CoreState* cs) { return (int)cs->button_emergency_inactive(); }, NULL, 0xFFA0A0FF);

    return 0;
}

int undo_clear_confirm_yes(CoreState *cs, void *data)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;
    usr_field_undo_clear(cs, data);
    if(q->pracdata->field_edit_in_progress)
        q->pracdata->field_edit_in_progress = 0;

    cs->button_emergency_override = false;
    cs->mouse.leftButton = Shiro::Mouse::Button::notPressed;
    return 0;
}

int undo_clear_confirm_no(CoreState *cs, void *)
{
    cs->button_emergency_override = false;
    cs->mouse.leftButton = Shiro::Mouse::Button::notPressed;
    return 0;
}

int usr_field_undo_clear(CoreState *cs, void *)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;

    q->pracdata->usr_field_undo.clear();
    q->pracdata->usr_field_redo.clear();

    return 0;
}

int usr_field_undo_button_action(CoreState *cs, void *)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;

    return usr_field_undo(cs, q->pracdata);
}

int usr_field_redo_button_action(CoreState *cs, void *)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;

    return usr_field_redo(cs, q->pracdata);
}

int qrs_input(game_t *g)
{
    CoreState *cs = g->origin;
    Shiro::KeyFlags *k = NULL;

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

    init = q->p1counters->init;

    if(d)
    {
        if(d->paused == QRS_FIELD_EDIT)
        {
            if(cs->undo && !d->field_edit_in_progress)
            {
                if(SDL_GetModState() & KMOD_SHIFT)
                {
                    while(d->usr_field_undo.size() > 0)
                    {
                        usr_field_undo(cs, d);
                    }
                }
                else
                {
                    usr_field_undo(cs, d);
                }
            }

            if(cs->redo && !d->field_edit_in_progress)
            {
                if(SDL_GetModState() & KMOD_SHIFT)
                {
                    while(d->usr_field_redo.size() > 0)
                    {
                        usr_field_redo(cs, d);
                    }
                }
                else
                {
                    usr_field_redo(cs, d);
                }
            }

            if(d->usr_field_locked)
            {
                d->field_selection = 0;
            }

            if(!d->usr_field_locked)
            {
                cell_x = ((cs->mouse.logicalX - q->field_x) / 16) - 1;
                cell_y = ((cs->mouse.logicalY - q->field_y) / 16) - 2;
                palette_cell_x = (cs->mouse.logicalX - FIELD_EDITOR_PALETTE_X) / 16;
                palette_cell_y = (cs->mouse.logicalY - FIELD_EDITOR_PALETTE_Y) / 16;

                // quick workaround
                if(cs->mouse.logicalX < FIELD_EDITOR_PALETTE_X)
                    palette_cell_x = -1;
                if(cs->mouse.logicalY < FIELD_EDITOR_PALETTE_Y)
                    palette_cell_y = -1;

                if(cs->select_all && !cs->text_editing)
                {
                    d->field_selection = 1;
                    d->field_selection_vertex1_x = 0;
                    d->field_selection_vertex1_y = 0;
                    d->field_selection_vertex2_x = 11;
                    d->field_selection_vertex2_y = 19;
                }

                if(SDL_GetModState() & KMOD_SHIFT && cs->mouse.leftButton != Shiro::Mouse::Button::notPressed)
                {
                    if(cs->mouse.leftButton == Shiro::Mouse::Button::pressedThisFrame)
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
                    if(cs->mouse.leftButton != Shiro::Mouse::Button::notPressed)
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
                            if(cs->mouse.leftButton == Shiro::Mouse::Button::pressedThisFrame)
                            {
                                d->field_selection = 0;
                                //cs->mouse.leftButton = Shiro::Mouse::Button::notPressed;
                            }
                        }
                        else if(cs->mouse.leftButton != Shiro::Mouse::Button::notPressed && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
                        {
                            if(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4) != QRS_FIELD_W_LIMITER)
                            {
                                if(d->palette_selection != QRS_PIECE_GEM)
                                {
                                    if(!d->field_edit_in_progress)
                                        usr_field_bkp(cs, d);
                                    d->field_edit_in_progress = 1;
                                    edit_action_occurred = 1;
                                    d->usr_field.cell(cell_x, cell_y + 4) = d->palette_selection;
                                }
                                else if(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4) > 0)
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
                    else if(cs->mouse.rightButton != Shiro::Mouse::Button::notPressed)
                    {
                        if(d->field_selection)
                        {
                            if(cs->mouse.rightButton == Shiro::Mouse::Button::pressedThisFrame)
                            {
                                d->field_selection = 0;
                                cs->mouse.rightButton = Shiro::Mouse::Button::notPressed;
                            }
                        }
                        else if(cs->mouse.rightButton != Shiro::Mouse::Button::notPressed && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
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
                                        if(d->usr_field.getCell(i, static_cast<std::size_t>(j) + 4) != QRS_FIELD_W_LIMITER)
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
                if(cs->pressedDigits[0])
                {
                    c = d->palette_selection;
                    if(d->field_selection)
                        cs->pressedDigits[0] = false;
                }
                if(cs->pressedDigits[1])
                {
                    c = 19;
                    if(d->field_selection)
                        cs->pressedDigits[1] = false;
                }
                if(cs->pressedDigits[2])
                {
                    c = 20;
                    if(d->field_selection)
                        cs->pressedDigits[2] = false;
                }
                if(cs->pressedDigits[3])
                {
                    c = 21;
                    if(d->field_selection)
                        cs->pressedDigits[3] = false;
                }
                if(cs->pressedDigits[4])
                {
                    c = 22;
                    if(d->field_selection)
                        cs->pressedDigits[4] = false;
                }
                if(cs->pressedDigits[5])
                {
                    c = 23;
                    if(d->field_selection)
                        cs->pressedDigits[5] = false;
                }
                if(cs->pressedDigits[6])
                {
                    c = 24;
                    if(d->field_selection)
                        cs->pressedDigits[6] = false;
                }
                if(cs->pressedDigits[7])
                {
                    c = 25;
                    if(d->field_selection)
                        cs->pressedDigits[7] = false;
                }
                if(cs->pressedDigits[9])
                {
                    c = QRS_PIECE_BRACKETS;
                    if(d->field_selection)
                        cs->pressedDigits[9] = false;
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
                                if(d->usr_field.getCell(i, static_cast<std::size_t>(j) + 4) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
                                {
                                    if(SDL_GetModState() & KMOD_SHIFT)
                                    {
                                        if(IS_STACK(d->usr_field.getCell(i, static_cast<std::size_t>(j) + 4)))
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
                                else if(d->usr_field.getCell(i, static_cast<std::size_t>(j) + 4) > 0 && c == QRS_PIECE_GEM)
                                {
                                    if(SDL_GetModState() & KMOD_SHIFT)
                                    {
                                        if(IS_STACK(d->usr_field.getCell(i, static_cast<std::size_t>(j) + 4)))
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
                        if(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
                        {
                            if(SDL_GetModState() & KMOD_SHIFT)
                            {
                                if(IS_STACK(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4)))
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
                        else if(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4) > 0 && c == QRS_PIECE_GEM)
                        {
                            if(SDL_GetModState() & KMOD_SHIFT)
                            {
                                if(IS_STACK(d->usr_field.getCell(cell_x, static_cast<std::size_t>(cell_y) + 4)))
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
    if(cs->pressed.escape || ((cs->pressed.a == 1 || cs->pressed.b == 1 || cs->pressed.c == 1) && (q->state_flags & GAMESTATE_GAMEOVER)))
    {
        if(menu_is_practice(cs->menu))
        {
            cs->menu_input_override = true;
            if(d)
            {
                d->paused = QRS_FIELD_EDIT;
                if(!d->usr_field_locked && !d->usr_field_fumen)
                {
                    usr_field_bkp(cs, d);
                    d->usr_field = (*g->field);
                }

                qs_update_pracdata(cs);
                if(!undo_clear_button_should_activate(cs))
                {
                    // if the clear undo button exists? doesn't exist? i feel like i should know what this means but i do not :|
                    // TODO use something more sane to detect for this sort of thing
                    /*gfx_createbutton(cs,
                                     "CLEAR UNDO",
                                     QRS_FIELD_X + (16 * 16) - 6,
                                     QRS_FIELD_Y + 23 * 16 + 8 - 6,
                                     0,
                                     push_undo_clear_confirm,
                                     undo_clear_button_should_activate,
                                     NULL,
                                     0xC0C0FFFF);*/
                }
            }

            q->state_flags = 0;
            q->topped_out = false;

            cs->pressed.escape = 0;

            return 0;
        }
        else
        {
            if((!(q->state_flags & (GAMESTATE_CREDITS | GAMESTATE_FADE_TO_CREDITS)) && (!q->topped_out)) || q->topped_out || q->playback || (q->state_flags & GAMESTATE_GAMEOVER))
            {
                Mix_HaltMusic();
                return 1;
            }
        }
    }

    if(init < 120)
        return 0;

    k = &cs->keys;

    if(p->state & (PSFALL | PSLOCK) && !(p->state & PSPRELOCKED))
    {
        if(cs->pressed.a || cs->pressed.c)
        {
            if(qrs_rotate(g, p, Shiro::CCW) == 0)
            {
                q->rotations++;
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
            if(qrs_rotate(g, p, Shiro::CW) == 0)
            {
                q->rotations++;
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
        if(cs->pressed.left || (cs->is_left_input_repeat(1 + p->speeds->das)))
        {
            qrs_move(g, p, MOVE_LEFT);
        }

        if(cs->pressed.right || cs->is_right_input_repeat(1 + p->speeds->das))
        {
            qrs_move(g, p, MOVE_RIGHT);
        }

        if(!qrs_isonground(g, p))
        {
            p->state &= ~PSLOCK;
            p->state |= PSFALL;
        }

        if(k->up == 1)
        {
            if(!(q->game_type == Shiro::GameType::SIMULATE_G1))
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
    assert(q->replay != nullptr);

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

int qrs_end_record(game_t* g)
{
    qrsdata *q = (qrsdata *)g->data;

    q->replay->time = q->timer;
    q->replay->ending_level = q->level;
    q->replay->grade = q->grade;

    scoredb_add_replay(&g->origin->records, &g->origin->player, q->replay);

    // TODO: Extract this into some (sum) method.
    int tetrisSum = 0;
    for(size_t i = 0; i < MAX_SECTIONS; i++)
    {
        tetrisSum += q->section_tetrises[i];
    }

    g->origin->player.tetrisCount += tetrisSum;

    scoredb_update_player(&g->origin->records, &g->origin->player);

    g2_seed_restore();
    q->recording = 0;
    return 0;
}

int qrs_load_replay(game_t *g, int replay_id)
{
    qrsdata *q = (qrsdata *)g->data;

    q->replay = (struct replay *)malloc(sizeof(struct replay));
    scoredb_get_full_replay(&g->origin->records, q->replay, replay_id);

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
        if((p->def->flags & Shiro::PDPREFERWKICK) || (p->def->qrsID == QRS_T && p->orient == Shiro::FLIP))
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
    Shiro::KeyFlags *k = &g->origin->keys;
    qrs_player *p = q->p1;

    if(k->d && q->hold_enabled)
    {
        qrs_hold(g, p);
    }

    qrs_irs(g);

    if(k->up && (q->game_type != Shiro::GameType::SIMULATE_G1))
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
    Shiro::KeyFlags *k = &g->origin->keys;
    qrs_player *p = q->p1;

    int direction = 0;

    if(k->a || k->c)
        direction = Shiro::CCW;

    if(k->b)
        direction = Shiro::CW;

    if((k->d || (k->a && k->c)) && q->special_irs && !q->hold_enabled)
    {
        direction = Shiro::FLIP;
    }

    p->orient = direction;
    if(qrs_chkcollision(*g, *p))
        p->orient = 0;
    else if(direction)
    {
        Shiro::SfxAsset::get(g->origin->assetMgr, "prerotate").play(g->origin->settings);
        q->rotations++;
    }

    return 0;
}

int qrs_wallkick(game_t *g, qrs_player *p)
{
    if(!g || !p)
        return -1;

    piece_id c = p->def->qrsID;
    int o = p->orient;
    std::pair<int, int> pos;
    qrs_chkcollision(*g, *p, pos);
    int x = pos.first;
//     std::cerr << "Trying to kick with collision at x = " << x << std::endl;

    if(p->def->flags & Shiro::PDNOWKICK)
        return 1;

    switch(c)
    {
        case QRS_I:
        case QRS_I4:
            if(o == Shiro::CW || o == Shiro::CCW)
                return 1;
            break;
        case QRS_J:
            if((o == Shiro::CW || o == Shiro::CCW) && x == 2)
                return 1;
            break;
        case QRS_L:
        case QRS_L4:
        case QRS_J4:
        case QRS_T4:
            if((o == Shiro::CW || o == Shiro::CCW) && x == 1)
                return 1;
            break;
        case QRS_N:
        case QRS_G:
        case QRS_Ya:
        case QRS_Yb:
            if(o == Shiro::CW && x == 2)
                return 1;
            else if(o == Shiro::CCW && x == 1)
                return 1;
            break;
        default:
            break;
    }

    bool failedBothKicks = false;

    if(c == QRS_T && o == Shiro::CW)
    {
        // special case: T pentomino rotated to the clockwise position:
        // try to kick left before right (prevents unintuitive "Felicity's conspiracy" situation)
        if(qrs_move(g, p, MOVE_LEFT))
        {
            if(qrs_move(g, p, MOVE_RIGHT))
            {
                failedBothKicks = true;
            }
        }
    }
    else
    {
        if(qrs_move(g, p, MOVE_RIGHT))
        {
            if(qrs_move(g, p, MOVE_LEFT))
            {
                failedBothKicks = true;
            }
        }
    }

    if(failedBothKicks)
    {
        if(c == QRS_P && p->orient == Shiro::FLIP)
        {
            return qrs_move(g, p, -2);
        }
        if(c == QRS_Q && p->orient == Shiro::FLIP)
        {
            return qrs_move(g, p, 2);
        }

        if(p->def->rotationTable[0].getWidth() == 4 && c != QRS_I4)
            return 1;
        if(c != QRS_I4 && c != QRS_I && c != QRS_J && c != QRS_L && c != QRS_Ya && c != QRS_Yb)
            return 1;
        if((c == QRS_J || c == QRS_L || c == QRS_Ya || c == QRS_Yb) && (p->orient == Shiro::CW || p->orient == Shiro::CCW))
            return 1;

        if(qrs_move(g, p, 2))
        {
            if(qrs_move(g, p, -2))
                return 1;
        }
    }

    return 0;
}

int qrs_hold(game_t *g, qrs_player *p)
{
    qrsdata* q = (qrsdata *)g->data;
    Shiro::PieceDefinition* temp = NULL;

    if(!q->hold_enabled)
        return 1;

    if(p->state & PSUSEDHOLD)
        return 1;

    if(!q->hold)
    {
        q->hold = new Shiro::PieceDefinition(*p->def);
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
        q->hold = new Shiro::PieceDefinition(*p->def);
        p->def = temp;

        if(p->def->qrsID >= 18) // tetrominoes spawn where they do in TGM
            p->y = ROWTOY(SPAWNY_QRS + 2);
        else
            p->y = ROWTOY(SPAWNY_QRS);

        p->x = SPAWNX_QRS;
        p->orient = Shiro::FLAT;
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
    if(p->def->flags & Shiro::PDNOFKICK)
        return 1;
    if(!(p->def->flags & Shiro::PDAIRBORNEFKICKS) && !qrs_isonground(g, p))
        return 1;

    qrsdata *q = (qrsdata *)g->data;
    piece_id c = p->def->qrsID;
    int o = p->orient;
    int bkp_y = p->y;

    std::pair<int, int> pos;
    qrs_chkcollision(*g, *p, pos);
    int x = pos.first;
    int y = pos.second;

    if(!q->max_floorkicks)
    {
        return 1;
    }

    switch(c)
    {
        case QRS_N:
            if(o == Shiro::CW && y == 2)
                return 1;
            else if(o == Shiro::CCW && y == 1)
                return 1;
            break;
        case QRS_G:
            if(o == Shiro::CW && y == 1)
                return 1;
            else if(o == Shiro::CCW && y == 2)
                return 1;
            break;
        case QRS_T:
            if(o == Shiro::FLAT && y == 1)
                return 1;
            break;
        case QRS_Ya:
            if(o == Shiro::CW && x == 1)
                return 1;
            break;
        case QRS_Yb:
            if(o == Shiro::CCW && x == 2)
                return 1;
            break;
        case QRS_J:
            if(o == Shiro::CCW && x == 3)
                return 1;
            break;
        case QRS_L:
            if(o == Shiro::CW && x == 0)
                return 1;
            break;
        default:
            break;
    }

    // T tetromino is the only piece to use this flag
    if(p->def->flags & Shiro::PDFLIPFLOORKICKS)
    {
        if(p->orient != Shiro::FLIP)
        {
            return 1;
        }
    }
    // T5 pentomino is the only piece to use this flag
    else if(p->def->flags & Shiro::PDFLATFLOORKICKS)
    {
        if(p->orient == Shiro::CW || p->orient == Shiro::CCW)
        {
            return 1;
        }
    }
    else
    {
        if(p->orient == Shiro::FLAT || p->orient == Shiro::FLIP)
        {
            return 1;
        }
    }

    p->y -= 256;

    if(qrs_chkcollision(*g, *p))
    {
        if(p->def->flags & Shiro::PDONECELLFLOORKICKS)
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
//     std::cerr << "Number of floorkicks so far: " << q->p1counters->floorkicks << std::endl;

    return 0;
}

int qrs_ceilingkick(game_t *g, qrs_player *p)
{
    int bkp_y = p->y;

    if(p->def->flags & Shiro::PDNOCEILKICKS)
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
                Shiro::SfxAsset::get(g->origin->assetMgr, "land").play(g->origin->settings);
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
    Shiro::Grid *d = &p->def->rotationTable[p->orient];
    Shiro::Grid *f = g->field;

    constexpr std::size_t ax = ANCHORX_QRS;
    constexpr std::size_t ay = ANCHORY_QRS;
    piece_id c = p->def->qrsID;

    for (std::size_t from_y = 0, to_y = static_cast<std::size_t>(YTOROW(p->y)) - ay; from_y < d->getHeight(); from_y++, to_y++) {
        for (std::size_t from_x = 0, to_x = static_cast<std::size_t>(p->x) - ax; from_x < d->getWidth(); from_x++, to_x++) {
            if (d->getCell(from_x, from_y)) {
                int value = c + 1;
                if (p->def->flags & Shiro::PDBRACKETS) {
                    value |= QRS_PIECE_BRACKETS;
                }
                if (q->state_flags & GAMESTATE_FADING) {
                    SET_PIECE_FADE_COUNTER(value, q->piece_fade_rate);
                }

                f->cell((int)to_x, (int)to_y) = value;
            }
        }
    }

    if(q->pracdata != nullptr)
    {
        if(!q->pracdata->usr_field_locked && q->pracdata->usr_field_fumen)
        {
            usr_field_bkp(g->origin, q->pracdata);
            q->pracdata->usr_field = (*g->field);
        }
    }

    p->state &= ~(PSLOCK | PSFALL);
    // p->state |= PSPRELOCKFLASH1;
    Shiro::SfxAsset::get(g->origin->assetMgr, "lock").play(g->origin->settings);

    return 0;
}

bool qrs_chkcollision(game_t& g, qrs_player& p) {
    std::pair<int, int> pos;
    return qrs_chkcollision(g, p, pos);
}

bool qrs_chkcollision(game_t& g, qrs_player& p, std::pair<int, int>& pos) {
    Shiro::Grid *d = &p.def->rotationTable[p.orient];
    Shiro::Grid *f = g.field;
    int d_x = 0;
    int d_y = 0;
    //int d_val = 0;
    int f_x = 0;
    int f_y = 0;
    //int f_val = 0;

    for (d_y = 0, f_y = YTOROW(p.y) - p.def->anchorY; d_y < int(d->getHeight()); d_y++, f_y++) {
        for (d_x = 0, f_x = p.x - p.def->anchorY; d_x < int(d->getWidth()); d_x++, f_x++) {
            if (d->getCell(d_x, d_y) && f->getCell(f_x, f_y)) {
                pos = std::pair(d_x, d_y);
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
    Shiro::Grid *field = g->field;

    int i = 0;
    int j = 0;
    int n = 0;

    for(i = QRS_FIELD_H - 1; i > 0; i--)
    {
        while(field->getCell(4, static_cast<std::size_t>(i) - n) == -2)
            n++;

        if(i - n >= 0)
        {
            field->copyRow(static_cast<std::size_t>(i) - n, i);
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
    std::size_t i = 0u;

    if(flags & GARBAGE_COPY_BOTTOM_ROW)
    {
        for(i = 0u; i < QRS_FIELD_H - 1; i++)
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
                g->field->cell(static_cast<int>(i), QRS_FIELD_H - 1) = QRS_PIECE_GARBAGE;
            }
        }
    }
    else
    {
        // random garbage based on several factors (TODO)
    }

    return 0;
}

void qrs_embiggen(Shiro::PieceDefinition& p)
{
    int xs[5] = {-1, -1, -1, -1, -1};
    int ys[5] = {-1, -1, -1, -1, -1};
    std::size_t k = 0;

    for(std::size_t r = 0; r < p.rotationTable.size(); r++)
    {
        p.rotationTable[0].resize(p.rotationTable[r].getWidth() + 1, p.rotationTable[r].getHeight() + 1);
        for(std::size_t i = 0; i < p.rotationTable[r].getWidth(); i++)
        {
            for(std::size_t j = 0; j < p.rotationTable[r].getHeight(); j++)
            {
                if(p.rotationTable[r].getCell(i, j))
                {
                    xs[k] = static_cast<int>(i);
                    ys[k] = static_cast<int>(j);
                    k++;
                }
            }
        }

        std::size_t index = static_cast<std::size_t>(rand()) % k;
        unsigned direction = static_cast<unsigned>(rand()) % 4u;
        unsigned tries = 0;

switchStatement:
        if(tries == 4)
        {
            k = 0;
            continue;
        }

        switch(direction)
        {
            case 0:
                if(ys[index] == 0 || p.rotationTable[r].getCell(xs[index], static_cast<std::size_t>(ys[index]) - 1))
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
                if(xs[index] == (int(p.rotationTable[r].getWidth()) - 1) || p.rotationTable[r].getCell(static_cast<std::size_t>(xs[index]) + 1, ys[index]))
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
                if(ys[index] == int(p.rotationTable[r].getHeight()) - 1 || p.rotationTable[r].getCell(xs[index], static_cast<std::size_t>(ys[index]) + 1))
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
                if(xs[index] == 0 || p.rotationTable[r].getCell(static_cast<std::size_t>(xs[index]) - 1, ys[index]))
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