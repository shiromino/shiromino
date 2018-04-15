#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "core.h"
#include "random.h"
#include "file_io.h"
#include "qrs.h"
#include "grid.h"
#include "timer.h"
#include "piecedef.h"
#include "replay.h"

#include "game_menu.h" // questionable dependency - TODO look into these
#include "game_qs.h" // questionable dependency
#include "gfx.h" // questionable dependency
#include "gfx_qs.h" // very questionable dependency
#include "gfx_structures.h"

#include "rotation_tables.h"

const char *qrspiece_names[25] =
{
    "I", "J", "L", "X", "S",
    "Z", "N", "G", "U", "T",
    "Fa", "Fb", "P", "Q", "W",
    "Ya", "Yb", "V", /**/ "I4", "T4",
    "J4", "L4", "O", "S4", "Z4"
};

const char *get_qrspiece_name(int n)
{
    if(n < 0 || n > 24)
        return NULL;

    return qrspiece_names[n];
}

/* */

void qrsdata_destroy(qrsdata *q)
{
   if(!q)
      return;

    int i;

    for(i = 0; i < 25; i++)
        piecedef_destroy(q->piecepool[i]);
    nz_timer_destroy(q->timer);
    //if(q->p1->def) piecedef_destroy(q->p1->def);
    free(q->p1);
    free(q->p1counters);
    free(q->piecepool);

    if(q->replay) {
        free(q->replay);
    }

   free(q);
}

void pracdata_destroy(struct pracdata *d)
{
    if(!d)
        return;

    int i = 0;

    if(d->usr_field_undo) {
        for(i = 0; i < d->usr_field_undo_len; i++) {
            if(d->usr_field_undo[i])
                grid_destroy(d->usr_field_undo[i]);
        }

        free(d->usr_field_undo);
    }

    if(d->usr_field_redo) {
        for(i = 0; i < d->usr_field_redo_len; i++) {
            if(d->usr_field_redo[i])
                grid_destroy(d->usr_field_redo[i]);
        }

        free(d->usr_field_redo);
    }

    if(d->usr_field)
        grid_destroy(d->usr_field);

    if(d->usr_timings)
        free(d->usr_timings);
}

struct pracdata *pracdata_cpy(struct pracdata *d)
{
    if(!d)
        return NULL;

    struct pracdata *cpy = malloc(sizeof(struct pracdata));
    int i = 0;

    for(i = 0; i < d->usr_seq_len; i++) {
        cpy->usr_sequence[i] = d->usr_sequence[i];
    }

    for(i = 0; i < d->usr_seq_expand_len; i++) {
        cpy->usr_seq_expand[i] = d->usr_seq_expand[i];
    }

    cpy->usr_seq_len = d->usr_seq_len;
    cpy->usr_seq_expand_len = d->usr_seq_expand_len;

    cpy->usr_field_undo = malloc(d->usr_field_undo_len * sizeof(grid_t *));
    cpy->usr_field_redo = malloc(d->usr_field_redo_len * sizeof(grid_t *));

    for(i = 0; i < d->usr_field_undo_len; i++) {
        cpy->usr_field_undo[i] = gridcpy(d->usr_field_undo[i], NULL);
    }

    for(i = 0; i < d->usr_field_redo_len; i++) {
        cpy->usr_field_redo[i] = gridcpy(d->usr_field_redo[i], NULL);
    }

    cpy->usr_field_undo_len = d->usr_field_undo_len;
    cpy->usr_field_redo_len = d->usr_field_redo_len;

    cpy->field_edit_in_progress = 0;

    cpy->usr_field = gridcpy(d->usr_field, NULL);

    cpy->palette_selection = QRS_PIECE_GARBAGE;
    cpy->field_selection = d->field_selection;
    cpy->field_selection_vertex1_x = d->field_selection_vertex1_x;
    cpy->field_selection_vertex1_y = d->field_selection_vertex1_y;
    cpy->field_selection_vertex2_x = d->field_selection_vertex2_x;
    cpy->field_selection_vertex2_y = d->field_selection_vertex2_y;

    cpy->usr_timings = malloc(sizeof(qrs_timings));
    cpy->usr_timings->level = 0;
    cpy->usr_timings->grav = d->usr_timings->grav;
    cpy->usr_timings->lock = d->usr_timings->lock;
    cpy->usr_timings->are = d->usr_timings->are;
    cpy->usr_timings->lineare = d->usr_timings->lineare;
    cpy->usr_timings->das = d->usr_timings->das;
    cpy->usr_timings->lineclear = d->usr_timings->lineclear;

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

piecedef **qrspool_create()
{
   piecedef **pool = malloc(25 * sizeof(piecedef *));
   int i = 0;
    int j = 0;
    int n = 5;

    int *arr = NULL;

   for(i = 0; i < 25; i++) {
        if(i >= 18) n = 4;

      pool[i] = malloc(sizeof(piecedef));
        pool[i]->qrs_id = i;
        pool[i]->flags = 0;
        pool[i]->anchorx = ANCHORX_QRS;
        pool[i]->anchory = ANCHORY_QRS;

        for(j = 0; j < 4; j++) {
            if(n == 5)
                arr = (int *)(qrspent_yx_rotation_tables[i][j]);
            else
                arr = (int *)(qrstet_yx_rotation_tables[i - 18][j]);

            pool[i]->rotation_tables[j] = grid_from_1d_int_array(arr, n, n);
        }

      if(i == QRS_I || i == QRS_N || i == QRS_G || i == QRS_J || i == QRS_L || i == QRS_T || i == QRS_Ya || i == QRS_Yb)
            pool[i]->flags ^= PDNOFKICK;
        if(i == QRS_T)
            pool[i]->flags |= PDFLATFLOORKICKS;

        if(i == QRS_I4)
         pool[i]->flags ^= PDNOWKICK;

      if(i == QRS_I4 || i == QRS_T4) {
         if(i == QRS_T4)
                pool[i]->flags |= PDFLATFLOORKICKS | PDONECELLFLOORKICKS | PDPREFERWKICK | PDAIRBORNEFKICKS;
      } else pool[i]->flags ^= PDNOFKICK;
   }

   return pool;
}

piecedef *qrspiece_cpy(piecedef **piecepool, int index)
{
    if(index < 0 || index > 24)
        return NULL;

    return piecedef_cpy(piecepool[index]);
}

grid_t *qrsfield_create()
{
   grid_t *g = grid_create(12, 22);

   return g;
}

int qrsfield_set_w(grid_t *field, int w)
{
    if(w % 2 || w < 4 || w > 12 || !field)
        return 1;

    int i = 0;
    int j = 0;

    for(i = 0; i < field->w; i++) {
        for(j = 0; j < field->h; j++) {
            if(gridgetcell(field, i, j) == QRS_FIELD_W_LIMITER)
                gridsetcell(field, i, j, 0);
        }
    }

    for(i = 0; i < (QRS_FIELD_W - w)/2; i++) {
        for(j = 0; j < 22; j++) {
            gridsetcell(field, i, j, QRS_FIELD_W_LIMITER);
            gridsetcell(field, QRS_FIELD_W - i - 1, j, QRS_FIELD_W_LIMITER);
        }
    }

    return 0;
}

int qrsfield_clear(grid_t *field)
{
    return 0;
}

int ufu_not_exists(coreState *cs)
{
    if(!cs->p1game)
        return 1;

    qrsdata *q = cs->p1game->data;
    if(!q)
        return 1;

    if((q->pracdata->usr_field_undo_len || q->pracdata->usr_field_redo_len) && q->pracdata->paused == QRS_FIELD_EDIT)
        return 0;
    else
        return 1;

    return 1;
}

int usr_field_bkp(coreState *cs, struct pracdata *d)
{
    if(!d)
        return 1;

    int i = 0;

    if(!d->usr_field_undo) {
        d->usr_field_undo = malloc(sizeof(grid_t *));
        d->usr_field_undo[0] = gridcpy(d->usr_field, NULL);
        d->usr_field_undo_len = 1;
        gfx_createbutton(cs, "CLEAR UNDO", QRS_FIELD_X + (16*16) - 6, QRS_FIELD_Y + 23*16 + 8 - 6,
                         0, push_undo_clear_confirm, ufu_not_exists, NULL, 0xC0C0FFFF);
    } else {
        d->usr_field_undo_len++;
        d->usr_field_undo = realloc(d->usr_field_undo, d->usr_field_undo_len * sizeof(grid_t *));
        d->usr_field_undo[d->usr_field_undo_len - 1] = gridcpy(d->usr_field, NULL);
    }

    if(d->usr_field_redo) {
        for(i = 0; i < d->usr_field_redo_len; i++) {
            grid_destroy(d->usr_field_redo[i]);
        }

        free(d->usr_field_redo);
        d->usr_field_redo = NULL;
        d->usr_field_redo_len = 0;
    }

    return 0;
}

int usr_field_undo(coreState *cs, struct pracdata *d)
{
    if(!d)
        return 1;

    if(!d->usr_field_undo)
        return 0;

    if(!d->usr_field_redo) {
        d->usr_field_redo = malloc(sizeof(grid_t *));
        d->usr_field_redo[0] = gridcpy(d->usr_field, NULL);
        d->usr_field_redo_len = 1;
    } else {
        d->usr_field_redo_len++;
        d->usr_field_redo = realloc(d->usr_field_redo, d->usr_field_redo_len * sizeof(grid_t *));
        d->usr_field_redo[d->usr_field_redo_len - 1] = gridcpy(d->usr_field, NULL);
    }

    d->usr_field_undo_len--;
    d->usr_field = d->usr_field_undo[d->usr_field_undo_len];

    if(!d->usr_field_undo_len) {
        free(d->usr_field_undo);
        d->usr_field_undo = NULL;
    } else {
        d->usr_field_undo = realloc(d->usr_field_undo, d->usr_field_undo_len * sizeof(grid_t *));
    }

    return 0;
}

int usr_field_redo(coreState *cs, struct pracdata *d)
{
    if(!d)
        return 1;

    if(!d->usr_field_redo)
        return 0;

    if(!d->usr_field_undo) {
        d->usr_field_undo = malloc(sizeof(grid_t *));
        d->usr_field_undo[0] = gridcpy(d->usr_field, NULL);
        d->usr_field_undo_len = 1;
    } else {
        d->usr_field_undo_len++;
        d->usr_field_undo = realloc(d->usr_field_undo, d->usr_field_undo_len * sizeof(grid_t *));
        d->usr_field_undo[d->usr_field_undo_len - 1] = gridcpy(d->usr_field, NULL);
    }

    d->usr_field_redo_len--;
    d->usr_field = d->usr_field_redo[d->usr_field_redo_len];

    if(!d->usr_field_redo_len) {
        free(d->usr_field_redo);
        d->usr_field_redo = NULL;
    } else {
        d->usr_field_redo = realloc(d->usr_field_redo, d->usr_field_redo_len * sizeof(grid_t *));
    }

    return 0;
}

int push_undo_clear_confirm(coreState *cs, void *data)
{
    struct text_formatting *fmt = text_fmt_create(DRAWTEXT_CENTERED, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);

    cs->button_emergency_override = 1;

    gfx_pushmessage(cs, "CONFIRM DELETE\nUNDO HISTORY?", 640/2 - 7*16, 480/2 - 16,
                    MESSAGE_EMERGENCY, monofont_square, fmt, -1, button_emergency_inactive);

    gfx_createbutton(cs, "YES", 640/2 - 6*16 - 6, 480/2 + 3*16 - 6,
                     BUTTON_EMERGENCY, undo_clear_confirm_yes, button_emergency_inactive, NULL, 0xB0FFB0FF);
    gfx_createbutton(cs, "NO", 640/2 + 4*16 - 6, 480/2 + 3*16 - 6,
                     BUTTON_EMERGENCY, undo_clear_confirm_no, button_emergency_inactive, NULL, 0xFFA0A0FF);

    return 0;
}

int undo_clear_confirm_yes(coreState *cs, void *data)
{
    qrsdata *q = cs->p1game->data;
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
    qrsdata *q = cs->p1game->data;
    int i = 0;

    if(q->pracdata->usr_field_undo) {
        for(i = 0; i < q->pracdata->usr_field_undo_len; i++) {
            grid_destroy(q->pracdata->usr_field_undo[i]);
        }

        free(q->pracdata->usr_field_undo);
        q->pracdata->usr_field_undo = NULL;
        q->pracdata->usr_field_undo_len = 0;
    }

    if(q->pracdata->usr_field_redo) {
        for(i = 0; i < q->pracdata->usr_field_redo_len; i++) {
            grid_destroy(q->pracdata->usr_field_redo[i]);
        }

        free(q->pracdata->usr_field_redo);
        q->pracdata->usr_field_redo = NULL;
        q->pracdata->usr_field_redo_len = 0;
    }

    return 0;
}

int qrs_input(game_t *g)
{
    coreState *cs = g->origin;
    struct keyflags *k = NULL;

    qrsdata *q = g->data;
    struct pracdata *d = q->pracdata;
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
    if(cs->settings) {
        scale = cs->settings->video_scale;
    }

    init = q->p1counters->init;

    if(d) {
        if(d->paused == QRS_FIELD_EDIT) {
            cell_x = (cs->mouse_x - q->field_x*scale) / (16*scale) - 1;
            cell_y = (cs->mouse_y - q->field_y*scale) / (16*scale) - 2;
            palette_cell_x = (cs->mouse_x - FIELD_EDITOR_PALETTE_X*scale) / (16*scale);
            palette_cell_y = (cs->mouse_y - FIELD_EDITOR_PALETTE_Y*scale) / (16*scale);

            if(cs->select_all && !cs->text_editing) {
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

            if(SDL_GetModState() & KMOD_SHIFT && cs->mouse_left_down) {
                if(cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME) {
                    d->field_selection = 1;
                    d->field_selection_vertex1_x = cell_x;
                    d->field_selection_vertex1_y = cell_y;
                }

                d->field_selection_vertex2_x = cell_x;
                d->field_selection_vertex2_y = cell_y;
            } else {
                if(cs->mouse_left_down) {
                    if(palette_cell_x == 0) {
                        switch(palette_cell_y) {
                            case 0:
                                d->palette_selection = QRS_X+1;
                                break;
                            case 1:
                                d->palette_selection = QRS_N+1;
                                break;
                            case 2:
                                d->palette_selection = QRS_G+1;
                                break;
                            case 3:
                                d->palette_selection = QRS_U+1;
                                break;
                            case 4:
                                d->palette_selection = QRS_T+1;
                                break;
                            case 5:
                                d->palette_selection = QRS_Fa+1;
                                break;
                            case 6:
                                break;
                            case 7:
                                d->palette_selection = QRS_I4+1;
                                break;
                            case 8:
                                d->palette_selection = QRS_T4+1;
                                break;
                            case 9:
                                d->palette_selection = QRS_J4+1;
                                break;
                            case 10:
                                d->palette_selection = QRS_L4+1;
                                break;
                            case 11:
                                d->palette_selection = QRS_O+1;
                                break;
                            case 12:
                                d->palette_selection = QRS_S4+1;
                                break;
                            case 13:
                                d->palette_selection = QRS_Z4+1;
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
                    } else if(d->field_selection) {
                        if(cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME) {
                            d->field_selection = 0;
                            cs->mouse_left_down = 0;
                        }
                    } else if(cs->mouse_left_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20) {
                        if(gridgetcell(d->usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER) {
                            if(d->palette_selection != QRS_PIECE_GEM) {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(d->usr_field, cell_x, cell_y + 2, d->palette_selection);
                            } else if(gridgetcell(d->usr_field, cell_x, cell_y + 2) > 0) {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(d->usr_field, cell_x, cell_y + 2,
                                            gridgetcell(d->usr_field, cell_x, cell_y + 2) | QRS_PIECE_GEM);
                            }
                        }
                    }
                } else if(cs->mouse_right_down) {
                    if(d->field_selection) {
                        if(cs->mouse_right_down == BUTTON_PRESSED_THIS_FRAME) {
                            d->field_selection = 0;
                            cs->mouse_right_down = 0;
                        }
                    } else if(cs->mouse_right_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20) {
                        if(gridgetcell(d->usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER) {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            gridsetcell(d->usr_field, cell_x, cell_y + 2, 0);
                        }
                    }
                }

                if(cs->delete_das == 2 || cs->backspace_das == 2) {
                    if(d->field_selection && !cs->text_editing) {
                        if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x) {
                            lesser_x = d->field_selection_vertex1_x;
                            greater_x = d->field_selection_vertex2_x;
                        } else {
                            lesser_x = d->field_selection_vertex2_x;
                            greater_x = d->field_selection_vertex1_x;
                        }

                        if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y) {
                            lesser_y = d->field_selection_vertex1_y;
                            greater_y = d->field_selection_vertex2_y;
                        } else {
                            lesser_y = d->field_selection_vertex2_y;
                            greater_y = d->field_selection_vertex1_y;
                        }

                        for(i = lesser_x; i <= greater_x; i++) {
                            for(j = lesser_y; j <= greater_y; j++) {
                                if(i >= 0 && i < 12 && j >= 0 && j < 20) {
                                    if(gridgetcell(d->usr_field, i, j + 2) != QRS_FIELD_W_LIMITER) {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        gridsetcell(d->usr_field, i, j + 2, 0);
                                    }
                                }
                            }
                        }

                        d->field_selection = 0;
                    }
                }
            }

            c = 0;
            if(cs->zero_pressed) {
                c = d->palette_selection;
                if(d->field_selection)
                    cs->zero_pressed = 0;
            }
            if(cs->one_pressed) {
                c = 19;
                if(d->field_selection)
                    cs->one_pressed = 0;
            }
            if(cs->two_pressed) {
                c = 20;
                if(d->field_selection)
                    cs->two_pressed = 0;
            }
            if(cs->three_pressed) {
                c = 21;
                if(d->field_selection)
                    cs->three_pressed = 0;
            }
            if(cs->four_pressed) {
                c = 22;
                if(d->field_selection)
                    cs->four_pressed = 0;
            }
            if(cs->five_pressed) {
                c = 23;
                if(d->field_selection)
                    cs->five_pressed = 0;
            }
            if(cs->six_pressed) {
                c = 24;
                if(d->field_selection)
                    cs->six_pressed = 0;
            }
            if(cs->seven_pressed) {
                c = 25;
                if(d->field_selection)
                    cs->seven_pressed = 0;
            }
            if(cs->nine_pressed) {
                c = QRS_PIECE_BRACKETS;
                if(d->field_selection)
                    cs->nine_pressed = 0;
            }

            if(c && d->field_selection) {
                if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x) {
                    lesser_x = d->field_selection_vertex1_x;
                    greater_x = d->field_selection_vertex2_x;
                } else {
                    lesser_x = d->field_selection_vertex2_x;
                    greater_x = d->field_selection_vertex1_x;
                }

                if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y) {
                    lesser_y = d->field_selection_vertex1_y;
                    greater_y = d->field_selection_vertex2_y;
                } else {
                    lesser_y = d->field_selection_vertex2_y;
                    greater_y = d->field_selection_vertex1_y;
                }

                for(i = lesser_x; i <= greater_x; i++) {
                    for(j = lesser_y; j <= greater_y; j++) {
                        if(i >= 0 && i < 12 && j >= 0 && j < 20) {
                            if(gridgetcell(d->usr_field, i, j + 2) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM) {
                                if(SDL_GetModState() & KMOD_SHIFT) {
                                    if(IS_STACK(gridgetcell(d->usr_field, i, j + 2))) {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        gridsetcell(d->usr_field, i, j + 2, c);
                                    }
                                } else {
                                    if(!d->field_edit_in_progress)
                                        usr_field_bkp(cs, d);
                                    d->field_edit_in_progress = 1;
                                    edit_action_occurred = 1;
                                    gridsetcell(d->usr_field, i, j + 2, c);
                                }
                            } else if(gridgetcell(d->usr_field, i, j + 2) > 0 && c == QRS_PIECE_GEM) {
                                if(SDL_GetModState() & KMOD_SHIFT) {
                                    if(IS_STACK(gridgetcell(d->usr_field, i, j + 2))) {
                                        if(!d->field_edit_in_progress)
                                            usr_field_bkp(cs, d);
                                        d->field_edit_in_progress = 1;
                                        edit_action_occurred = 1;
                                        gridsetcell(d->usr_field, i, j + 2, gridgetcell(d->usr_field, i, j + 2) | c);
                                    }
                                } else {
                                    if(!d->field_edit_in_progress)
                                        usr_field_bkp(cs, d);
                                    d->field_edit_in_progress = 1;
                                    edit_action_occurred = 1;
                                    gridsetcell(d->usr_field, i, j + 2, gridgetcell(d->usr_field, i, j + 2) | c);
                                }
                            }
                        }
                    }
                }

                d->field_selection = 0;
            } else if(c) {
                if(cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20) {
                    if(gridgetcell(d->usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM) {
                        if(SDL_GetModState() & KMOD_SHIFT) {
                            if(IS_STACK(gridgetcell(d->usr_field, cell_x, cell_y + 2))) {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(d->usr_field, cell_x, cell_y + 2, c);
                            }
                        } else {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            gridsetcell(d->usr_field, cell_x, cell_y + 2, c);
                        }
                    } else if(gridgetcell(d->usr_field, cell_x, cell_y + 2) > 0 && c == QRS_PIECE_GEM) {
                        if(SDL_GetModState() & KMOD_SHIFT) {
                            if(IS_STACK(gridgetcell(d->usr_field, cell_x, cell_y + 2))) {
                                if(!d->field_edit_in_progress)
                                    usr_field_bkp(cs, d);
                                d->field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(d->usr_field, cell_x, cell_y + 2, gridgetcell(d->usr_field, cell_x, cell_y + 2) | c);
                            }
                        } else {
                            if(!d->field_edit_in_progress)
                                usr_field_bkp(cs, d);
                            d->field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            gridsetcell(d->usr_field, cell_x, cell_y + 2, gridgetcell(d->usr_field, cell_x, cell_y + 2) | c);
                        }
                    }
                }
            }

            if(!edit_action_occurred)
                d->field_edit_in_progress = 0;
        }
    }

    if(cs->menu && cs->menu_input_override)
    {
        return 0;
    }

    // hacky way to go back to the practice menu if a game is running from that menu
   if(cs->prev_keys.escape || cs->keys.escape) {
        if(menu_is_practice(cs->menu)) {
            cs->menu_input_override = 1;
            if(d) {
                d->paused = QRS_FIELD_EDIT;
                qs_update_pracdata(cs);
                if(!ufu_not_exists(cs)) {
                    // if the clear undo button exists? doesn't exist? i feel like i should know what this means but i do not :|
                    // TODO use something more sane to detect for this sort of thing
                    gfx_createbutton(cs, "CLEAR UNDO", QRS_FIELD_X + (16*16) - 6, QRS_FIELD_Y + 23*16 + 8 - 6,
                                     0, push_undo_clear_confirm, ufu_not_exists, NULL, 0xC0C0FFFF);
                }
            }

            cs->prev_keys.escape = 0;
            cs->keys.escape = 0;

            return 0;
        } else
            return 1;
    }

   if(init < 120)
      return 0;

    k = &cs->keys;

    if(p->state & (PSFALL | PSLOCK) && !(p->state & PSPRELOCKED))
    {
        if(cs->pressed.a || cs->pressed.c) {
            if(qrs_rotate(g, p, CCW) == 0) {
                if(q->max_floorkicks != 0 && q->p1counters->floorkicks >= q->max_floorkicks) {
                    if(q->lock_on_rotate == 1) {
                        q->lock_on_rotate = 2;
                    } else
                        q->lock_on_rotate = 1;
                }
            }
        }

        if(cs->pressed.b == 1) {
            if(qrs_rotate(g, p, CW) == 0) {
                if(q->max_floorkicks != 0 && q->p1counters->floorkicks >= q->max_floorkicks) {
                    if(q->lock_on_rotate == 1) {
                        q->lock_on_rotate = 2;
                    } else
                        q->lock_on_rotate = 1;
                }
            }
        }

        //if(k->d == 1) qrs_rotate(g, p, FLIP);
    }

    if(p->state & (PSFALL | PSLOCK) && !(p->state & PSPRELOCKED))
    {
        q->active_piece_time++;
        if(cs->pressed.left || (is_left_input_repeat(cs, 1 + p->speeds->das)) ) {
            qrs_move(g, p, MOVE_LEFT);
            /* moved_left = 1; */
        }

        if(cs->pressed.right || is_right_input_repeat(cs, 1 + p->speeds->das)) {
            qrs_move(g, p, MOVE_RIGHT);
            /* moved_right = 1; */
        }

        /* if(moved_left && moved_right) { */
        /*     if(k->left < k->right) */
        /*         qrs_move(g, p, MOVE_LEFT); */
        /*     else if(k->right < k->left) */
        /*         qrs_move(g, p, MOVE_RIGHT); */
        /* } */

        if(!qrs_isonground(g, p)) {
            p->state &= ~PSLOCK;
            p->state |= PSFALL;
        }

        if(k->down)
        {
            const bool lock_protect_enabled = q->is_practice ? q->pracdata->lock_protect : q->lock_protect;
            const bool should_lock_protect = lock_protect_enabled && q->lock_held;

            q->soft_drop_counter++;
            if(p->state & PSFALL) {
                qrs_fall(g, p, 256);
                if(qrs_isonground(g, p) && !should_lock_protect) {
                    qrs_fall(g, p, 256);
                    q->lock_held = 1;
                    p->state &= ~PSLOCK;
                    p->state &= ~PSFALL;
                    p->state |= PSLOCKPRESSED;
                }
            } else if(p->state & PSLOCK && !should_lock_protect) {
                q->lock_held = 1;
                p->state &= ~PSLOCK;
                p->state |= PSLOCKPRESSED;
            }
        }

        if(k->up == 1) {
            if(!(q->game_type == SIMULATE_G1)) {
                int num_rows = qrs_fall(g, p, 20 * 256);
                if(num_rows > q->sonic_drop_height)
                    q->sonic_drop_height = num_rows;
            }
        }

        if(k->d == 1 && q->hold_enabled) {
            qrs_hold(g, p);
        }
    }

    if(!k->down) {
        if(q->lock_held)
            q->lock_held = 0;
    }

   return 0;
}

int qrs_start_record(game_t *g)
{
    qrsdata *q = g->data;

    g2_seed_bkp();

    q->replay = malloc(sizeof(struct replay));

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
    qrsdata *q = g->data;

    q->replay->time = q->timer->time;
    q->replay->ending_level = q->level;
    q->replay->grade = q->grade;

    scoredb_add(&g->origin->scores, &g->origin->player, q->replay);

    // TODO: Extract this into some (sum) method.
    int tetrisSum = 0;
    for (size_t i = 0; i < MAX_SECTIONS; i++)
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
    qrsdata *q = g->data;

    q->replay = malloc(sizeof(struct replay));
    scoredb_get_full_replay(&g->origin->scores, q->replay, replay_id);

    return 0;
}

int qrs_start_playback(game_t *g)
{
    qrsdata *q = g->data;

    q->playback = 1;
    q->playback_index = 0;

    return 0;
}

int qrs_end_playback(game_t *g)
{
    qrsdata *q = g->data;

    q->playback = 0;

    return 0;
}

int qrs_move(game_t *g, qrs_player *p, int offset)
{
   if(!g || !p)
      return -1;

   int bkp_x = p->x;
   p->x += offset;

   if(qrs_chkcollision(g, p)) {
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
   //int c = p->def->color;

   p->orient = (p->orient + direction) & 3;

   if(qrs_chkcollision(g, p))
   {
        if(p->def->flags & PDPREFERWKICK) {
            if(qrs_wallkick(g, p))
          {
             if(qrs_floorkick(g, p))
             {
                p->orient = bkp_o;
                return 1;
             }
          }
        } else {
            if(qrs_floorkick(g, p))
          {
             if(qrs_wallkick(g, p))
             {
                p->orient = bkp_o;
                return 1;
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
        qrs_fall(g, p, 20*256);
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

    if(k->d && q->special_irs && !q->hold_enabled)
    {
        direction = FLIP;
    }

   p->orient = direction;
   if(qrs_chkcollision(g, p))
      p->orient = 0;
    else if(direction)
    {
        sfx_play(&g->origin->assets->prerotate);
    }

   return 0;
}

int qrs_wallkick(game_t *g, qrs_player *p)
{
   if(!g || !p)
      return -1;

   piece_id c = p->def->qrs_id;
    int o = p->orient;
   int x = gridpostox(p->def->rotation_tables[0], qrs_chkcollision(g, p) - 1);
    //printf("Trying to kick with collision at x = %d\n", x);

   if(p->def->flags & PDNOWKICK)
      return 1;

    switch(c) {
        case QRS_I:
        case QRS_I4:
            if(o == CW || o == CCW)
                return 1;
            break;
        case QRS_J:
            if( (o == CW || o == CCW) && x == 2 )
                return 1;
            break;
        case QRS_L:
        case QRS_L4:
        case QRS_J4:
        case QRS_T4:
            if( (o == CW || o == CCW) && x == 1 )
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
          if(p->def->rotation_tables[0]->w == 4 && c != QRS_I4)
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
    qrsdata *q = g->data;
    piecedef *temp = NULL;

    if(!q->hold_enabled)
        return 1;

    if(p->state & PSUSEDHOLD)
        return 1;

    if(!q->hold) {
        q->hold = piecedef_cpy(p->def);
        if(qs_initnext(g, p, INITNEXT_DURING_ACTIVE_PLAY) == 1) {  // if there is no next piece to swap in
            piecedef_destroy(q->hold);
            q->hold = NULL;

            return 1;
        }

        p->state |= PSUSEDHOLD;
    } else {
        temp = q->hold;
        q->hold = p->def;
        p->def = temp;

        if(p->def->qrs_id >= 18) // tetrominoes spawn where they do in TGM
           p->y = ROWTOY(SPAWNY_QRS + 2);
        else
            p->y = ROWTOY(SPAWNY_QRS);

        p->x = SPAWNX_QRS;
       p->orient = FLAT;
       p->state = PSFALL|PSSPAWN|PSUSEDHOLD;

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

    qrsdata *q = g->data;
    int bkp_y = p->y;

    if(!q->max_floorkicks)
    {
        return 1;
    }

    if(p->orient == CW || p->orient == CCW)
    {
        if(p->def->flags & PDFLATFLOORKICKS)
            return 1;
    }

    if(p->orient == FLAT || p->orient == FLIP)
    {
        if(!(p->def->flags & PDFLATFLOORKICKS))
            return 1;
    }

   p->y -= 256;

   if(qrs_chkcollision(g, p)) {
      if(p->def->flags & PDONECELLFLOORKICKS) {
            p->y = bkp_y;
            return 1;
        }

      p->y -= 256;
      if(qrs_chkcollision(g, p)) {
         p->y = bkp_y;
         return 1;
      }
   }

    q->p1counters->floorkicks++;
    //printf("Number of floorkicks so far: %d\n", q->p1counters->floorkicks);

   return 0;
}

int qrs_fall(game_t *g, qrs_player *p, int grav)
{
   if(!g || !p)
      return -1;

   if(!grav)
      grav = p->speeds->grav;

   int bkp_y = p->y;

   while(p->y < (bkp_y + grav)) {
      p->y += 256;
      if(qrs_chkcollision(g, p)) {
         p->y -= (256 + (p->y & 255));

            if(p->state & PSFALL && grav != 28*256) {
                sfx_play(&g->origin->assets->land);
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

   qrsdata *q = g->data;
   grid_t *d = p->def->rotation_tables[p->orient];
   grid_t *f = g->field;

   int i = 0;
   int ax = ANCHORX_QRS;
   int ay = ANCHORY_QRS;
   piece_id c = p->def->qrs_id;
   int s = d->w * d->h;

   for(i = 0; i < s; i++) {
      int from_x = gridpostox(d, i);
      int from_y = gridpostoy(d, i);
      int to_x = (p->x - ax) + from_x;
      int to_y = (YTOROW(p->y) - ay) + from_y;

      if(gridgetcell(d, from_x, from_y)) {
            int value = c + 1;
            if(p->def->flags & PDBRACKETS)
                value |= QRS_PIECE_BRACKETS;

            if(q->state_flags & GAMESTATE_FADING) {
                SET_PIECE_FADE_COUNTER(value, q->piece_fade_rate);
            }

            gridsetcell(f, to_x, to_y, value);
        }
   }

   p->state &= ~(PSLOCK | PSFALL);
   //p->state |= PSPRELOCKFLASH1;
   sfx_play(&g->origin->assets->lock);

   return 0;
}

int qrs_chkcollision(game_t *g, qrs_player *p)
{
   if(!g || !p)
      return -1;

   grid_t *d = p->def->rotation_tables[p->orient];
   grid_t *f = g->field;
   int d_x = 0;
   int d_y = 0;
   int d_val = 0;
   int f_x = 0;
   int f_y = 0;
   int f_val = 0;
   int i = 0;
   int s = d->w * d->h;

   for(i = 0; i < s; i++) {
      d_x = gridpostox(d, i);
      d_y = gridpostoy(d, i);
      f_x = p->x - p->def->anchorx + d_x;
      f_y = YTOROW(p->y) - p->def->anchory + d_y;

      d_val = gridgetcell(d, d_x, d_y);
      f_val = gridgetcell(f, f_x, f_y);

      // printf("Checking piecedef val %d against %d at position %d (field y = %d)\n", d_val, f_val, i, f_y);

      if(d_val && f_val) {
            // gridgetcell returns 8128 on out of bounds, so it will default to collision = true

            // the +1 slightly confuses things, but is required for cases where the collision is at position = 0
            // this way we don't return 0 (== no collision) when there in fact was a collision
            // TODO put in a macro for QRS_COLLISION_FALSE, set it to some non-zero value
         return i + 1;
      }
   }

   return 0;
}

int qrs_isonground(game_t *g, qrs_player *p)
{
   if(!g || !p)
      return -1;

   p->y += 256;

   if(qrs_chkcollision(g, p)) {
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

    qrsdata *q = g->data;

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

       for(j = (QRS_FIELD_W - q->field_w)/2; j < (QRS_FIELD_W/2 + q->field_w/2); j++)
       {
           if(gridgetcell(g->field, j, i))
               k++;
           if(gridgetcell(g->field, j, i) & QRS_PIECE_GEM)
               gem = true;
           if(gridgetcell(g->field, j, i) == QRS_PIECE_GARBAGE)
               garbage++;
       }

       if(k == q->field_w && garbage != q->field_w)
       {
           n++;
           gfx_qs_lineclear(g, i);
           for(j = (QRS_FIELD_W - q->field_w)/2; j < (QRS_FIELD_W/2 + q->field_w/2); j++)
               gridsetcell(g->field, j, i, -2);

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

    qrsdata *q = g->data;
    grid_t *field = g->field;

   int i = 0;
   int j = 0;
   int n = 0;

   for(i = QRS_FIELD_H - 1; i > 0; i--)
   {
      while(gridgetcell(field, 6, i - n) == -2)
         n++;

      if(i - n >= 0)
         gridrowcpy(field, NULL, i - n, i);
      else {
         for(j = (QRS_FIELD_W - q->field_w)/2; j < (QRS_FIELD_W/2 + q->field_w/2); j++) {
            gridsetcell(field, j, i, 0);
         }
      }
   }

   return 0;
}

int qrs_spawn_garbage(game_t *g, unsigned int flags)
{
    qrsdata *q = g->data;
    int i = 0;

    if(q->garbage) {
        // TODO
    } else if(flags & GARBAGE_COPY_BOTTOM_ROW) {
        for(i = 0; i < 21; i++) {
            gridrowcpy(g->field, NULL, i+1, i);
        }

        gridrowcpy(g->field, NULL, 20, 21);
        for(i = 0; i < QRS_FIELD_W; i++) {
            if(gridgetcell(g->field, i, 20) == QRS_FIELD_W_LIMITER)
                continue;
            if(gridgetcell(g->field, i, 20))
                gridsetcell(g->field, i, 21, QRS_PIECE_GARBAGE);
        }
    } else {
        // random garbage based on several factors (TODO)
    }

    return 0;
}
