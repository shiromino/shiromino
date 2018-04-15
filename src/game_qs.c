#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "zed_dbg.h"

#include "core.h"
#include "random.h"
#include "game_qs.h"
#include "gfx_qs.h"
#include "game_menu.h"
#include "gfx.h"
#include "qrs.h"
#include "timer.h"

#include "replay.h"

const char *grade_names[37] =
{
    "  9", "  8", "  7", "  6", "  5", "  4", "  3", "  2", "  1",
    " S1", " S2", " S3", " S4", " S5", " S6", " S7", " S8", " S9", "S10", "S11", "S12", "S13",
    " m1", " m2", " m3", " m4", " m5", " m6", " m7", " m8", " m9",
    "  M", " MK", " MV", " MO", " MM", " GM"
};

const char *internal_grade_names[31] =
{
    " 9 ", " 8 ", " 7 ", " 6 ", " 5 ", " 4 ", " 4+", " 3 ", " 3+",
    " 2-", " 2 ", " 2+", " 1-", " 1 ", " 1+", "S1-", "S1 ", "S1+",
    "S2 ", "S3 ", "S4-", "S4 ", "S5 ", "S5+", "S6 ", "S6+", "S7 ", "S7+", "S8 ", "S8+", "S9 "
};

static int g1_grade_score_reqs[17] =
{
    400,
    800,
    1400,
    2000,
    3500,
    5500,
    8000,
    12000,

    16000, // S1
    22000,
    30000,
    40000,
    52000,
    66000,
    82000,
    100000,
    120000 // S9
};

static qrs_timings qs_curve[QS_CURVE_MAX] =
{
    {0, 4, 40, 12, 25, 25, 12},
    {100, 16, 40, 12, 25, 25, 12},
    {120, 32, 40, 12, 25, 25, 12},
    {140, 48, 40, 12, 25, 25, 12},
    {160, 64, 40, 12, 25, 25, 12},
    {180, 96, 40, 12, 25, 25, 12},
    {200, 128, 40, 12, 25, 25, 12},
    {250, 192, 40, 12, 25, 25, 12},
    {300, 256, 40, 12, 25, 25, 12},
    {400, 512, 40, 12, 25, 25, 12},
    {450, 768, 40, 12, 25, 25, 12},
    {480, 5 * 256, 40, 12, 25, 25, 12},

    {500, 20 * 256, 30, 10, 20, 16, 12},
    {600, 20 * 256, 30, 10, 18, 16, 8},
    {700, 20 * 256, 26, 8, 16, 16, 6},
    {800, 20 * 256, 26, 8, 12, 12, 6},
    {900, 20 * 256, 20, 8, 12, 12, 6},
    {1000, 20 * 256, 19, 7, 10, 10, 4},
    {1100, 20 * 256, 19, 7, 8, 8, 4}
};

static qrs_timings g1_master_curve[G1_MASTER_CURVE_MAX] =
{
    {0, 4, 30, 14, 30, 30, 41},
    {30, 6, 30, 14, 30, 30, 41},
    {35, 8, 30, 14, 30, 30, 41},
    {40, 10, 30, 14, 30, 30, 41},
    {50, 12, 30, 14, 30, 30, 41},
    {60, 16, 30, 14, 30, 30, 41},
    {70, 32, 30, 14, 30, 30, 41},
    {80, 48, 30, 14, 30, 30, 41},
    {90, 64, 30, 14, 30, 30, 41},
    {100, 80, 30, 14, 30, 30, 41},
    {120, 96, 30, 14, 30, 30, 41},
    {140, 112, 30, 14, 30, 30, 41},
    {160, 128, 30, 14, 30, 30, 41},
    {170, 144, 30, 14, 30, 30, 41},
    {200, 4, 30, 14, 30, 30, 41},
    {220, 32, 30, 14, 30, 30, 41},
    {230, 64, 30, 14, 30, 30, 41},
    {233, 96, 30, 14, 30, 30, 41},
    {236, 128, 30, 14, 30, 30, 41},
    {239, 160, 30, 14, 30, 30, 41},
    {243, 192, 30, 14, 30, 30, 41},
    {247, 224, 30, 14, 30, 30, 41},
    {251, 256, 30, 14, 30, 30, 41},
    {300, 512, 30, 14, 30, 30, 41},
    {330, 768, 30, 14, 30, 30, 41},
    {360, 1024, 30, 14, 30, 30, 41},
    {400, 1280, 30, 14, 30, 30, 41},
    {420, 1024, 30, 14, 30, 30, 41},
    {450, 768, 30, 14, 30, 30, 41},
    {500, 5120, 30, 14, 30, 30, 41}
};

static qrs_timings g2_master_curve[G2_MASTER_CURVE_MAX] =
{
    {0, 4, 30, 14, 25, 25, 40},
    {30, 6, 30, 14, 25, 25, 40},
    {35, 8, 30, 14, 25, 25, 40},
    {40, 10, 30, 14, 25, 25, 40},
    {50, 12, 30, 14, 25, 25, 40},
    {60, 16, 30, 14, 25, 25, 40},
    {70, 32, 30, 14, 25, 25, 40},
    {80, 48, 30, 14, 25, 25, 40},
    {90, 64, 30, 14, 25, 25, 40},
    {100, 80, 30, 14, 25, 25, 40},
    {120, 96, 30, 14, 25, 25, 40},
    {140, 112, 30, 14, 25, 25, 40},
    {160, 128, 30, 14, 25, 25, 40},
    {170, 144, 30, 14, 25, 25, 40},
    {200, 4, 30, 14, 25, 25, 40},
    {220, 32, 30, 14, 25, 25, 40},
    {230, 64, 30, 14, 25, 25, 40},
    {233, 96, 30, 14, 25, 25, 40},
    {236, 128, 30, 14, 25, 25, 40},
    {239, 160, 30, 14, 25, 25, 40},
    {243, 192, 30, 14, 25, 25, 40},
    {247, 224, 30, 14, 25, 25, 40},
    {251, 256, 30, 14, 25, 25, 40},
    {300, 512, 30, 14, 25, 25, 40},
    {330, 768, 30, 14, 25, 25, 40},
    {360, 1024, 30, 14, 25, 25, 40},
    {400, 1280, 30, 14, 25, 25, 40},
    {420, 1024, 30, 14, 25, 25, 40},
    {450, 768, 30, 14, 25, 25, 40},
    {500, 5120, 30, 8, 25, 25, 25},
    {601, 5120, 30, 8, 25, 16, 16},
    {701, 5120, 30, 8, 16, 12, 12},
    {801, 5120, 30, 8, 12, 6, 6},
    {900, 5120, 30, 6, 12, 6, 6},
    {901, 5120, 17, 6, 12, 6, 6}
};

static qrs_timings g2_death_curve[G2_DEATH_CURVE_MAX] =
{
    {0, 5120, 30, 10, 16, 12, 12},
    {101, 5120, 26, 10, 12, 6, 6},
    {200, 5120, 26, 9, 12, 6, 6},
    {201, 5120, 22, 9, 12, 6, 6},
    {300, 5120, 22, 8, 12, 6, 6},
    {301, 5120, 18, 8, 6, 6, 6},
    {400, 5120, 18, 6, 6, 6, 6},
    {401, 5120, 15, 6, 5, 5, 5},
    {500, 5120, 15, 6, 4, 4, 4}
};

static qrs_timings g3_terror_curve[G3_TERROR_CURVE_MAX] =
{
    {0, 5120, 18, 8, 10, 6, 6},
    {100, 5120, 18, 6, 10, 5, 5},
    {200, 5120, 17, 6, 10, 4, 4},
    {300, 5120, 15, 6, 4, 4, 4},
    {500, 5120, 13, 4, 4, 3, 3},
    {600, 5120, 12, 4, 4, 3, 3},
    {1100, 5120, 10, 4, 4, 3, 3},
    {1200, 5120, 8, 4, 4, 3, 3},
    {1300, 5120, 15, 4, 4, 4, 6}
};

// TODO
int g2_advance_garbage[12][24] =
{
    {QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
     QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
     QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
     QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 1 y
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,   // column 2 y
      1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1,   // column 3
      1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 4
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 5
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 6
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 7
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 8
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 9
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,   // column 10
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

     {QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
      QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
      QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL,
      QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL, QRS_WALL},
};

int grade_points_table[32][4] =
{
    {10, 20, 40, 50},
    {10, 20, 30, 40},
    {10, 20, 30, 40},
    {10, 15, 30, 40},
    {10, 15, 20, 40},
    {5, 15, 20, 30},
    {5, 10, 20, 30},
    {5, 10, 15, 30},
    {5, 10, 15, 30},
    {5, 10, 15, 30},

    // internal grades 10-31 (" 2 " to "S9 ") are all the same table
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30},
    {2, 12, 13, 30}
};

int grade_point_decays[32] =
{
    125, 80, 80, 50, 45, 45, 45, 40, 40, 40, 40, 40, 30, 30, 30,
    20, 20, 20, 20, 20, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 10, 10
};

float g2_grade_point_combo_table[10][4] =
{
    {1.0, 1.0, 1.0, 1.0},
    {1.2, 1.4, 1.5, 1.0},
    {1.2, 1.5, 1.8, 1.0},
    {1.4, 1.6, 2.0, 1.0},
    {1.4, 1.7, 2.2, 1.0},
    {1.4, 1.8, 2.3, 1.0},
    {1.4, 1.9, 2.4, 1.0},
    {1.5, 2.0, 2.5, 1.0},
    {1.5, 2.1, 2.6, 1.0},
    {2.0, 2.5, 3.0, 1.0}
};

const char *get_grade_name(int grade)
{
    grade &= 0xff;
    if (grade > 36)
        return " --";

    else return grade_names[grade];
}

const char *get_internal_grade_name(int index)
{
    if (index < 0 || index > 31)
        return "N/A";

    else return internal_grade_names[index];
}

int internal_to_displayed_grade(int internal_grade)
{
    if (internal_grade < INTERNAL_GRADE_9)
        return -1;

    else switch (internal_grade)
    {
    case INTERNAL_GRADE_9:
    case INTERNAL_GRADE_8:
    case INTERNAL_GRADE_7:
    case INTERNAL_GRADE_6:
    case INTERNAL_GRADE_5:
    case INTERNAL_GRADE_4:
        return internal_grade;

    case INTERNAL_GRADE_4_PLUS:
        return GRADE_4;

    case INTERNAL_GRADE_3:
    case INTERNAL_GRADE_3_PLUS:
        return GRADE_3;

    case INTERNAL_GRADE_2_MINUS:
    case INTERNAL_GRADE_2:
    case INTERNAL_GRADE_2_PLUS:
        return GRADE_2;

    case INTERNAL_GRADE_1_MINUS:
    case INTERNAL_GRADE_1:
    case INTERNAL_GRADE_1_PLUS:
        return GRADE_1;

    case INTERNAL_GRADE_S1_MINUS:
    case INTERNAL_GRADE_S1:
    case INTERNAL_GRADE_S1_PLUS:
        return GRADE_S1;

    case INTERNAL_GRADE_S2:
        return GRADE_S2;

    case INTERNAL_GRADE_S3:
        return GRADE_S3;

    case INTERNAL_GRADE_S4_MINUS:
    case INTERNAL_GRADE_S4:
    case INTERNAL_GRADE_S4_PLUS:
        return GRADE_S4;

    case INTERNAL_GRADE_S5:
    case INTERNAL_GRADE_S5_PLUS:
        return GRADE_S5;

    case INTERNAL_GRADE_S6:
    case INTERNAL_GRADE_S6_PLUS:
        return GRADE_S6;

    case INTERNAL_GRADE_S7:
    case INTERNAL_GRADE_S7_PLUS:
        return GRADE_S7;

    case INTERNAL_GRADE_S8:
    case INTERNAL_GRADE_S8_PLUS:
        return GRADE_S8;

    case INTERNAL_GRADE_S9:
    default:
        return GRADE_S9;
    }

    return -1;
}

struct levelmusic
{
    int fromlevel;
    int musicindex; // -1 for mute
};

static const struct levelmusic pentomino_music[] = {
    { 0, 0 },
    { 490, -1 },
    { 500, 1 },
    { 690, -1 },
    { 700, 2 },
    { 980, -1 },
    { 1000, 3 },
    { 9999, -1 }
};

static const struct levelmusic g2_master_music[] = {
    { 0, 0 },
    { 495, -1 },
    { 500, 1 },
    { 695, -1 },
    { 700, 2 },
    { 880, -1 },
    { 900, 3 },
    { 9999, -1 }
};

static const struct levelmusic g2_death_music[] = {
    { 0, 1 },
    { 280, -1 },
    { 300, 2 },
    { 480, -1 },
    { 500, 3 },
    { 9999, -1 }
};

static const struct levelmusic g3_terror_music[] = {
    { 0, 2 },
    { 480, -1 },
    { 500, 3 },
    { 680, -1 },
    { 700, 4 },
    { 980, -1 },
    { 1000, 5 },
    { 9999, -1 }
};

static const struct levelmusic g1_music[] = {
    { 0, 0 },
    { 485, -1 },
    { 500, 1 },
    { 9999, -1 }
};

static int find_music(int level, const struct levelmusic *table)
{
    int music = -1;
    for (int i = 0; table[i].fromlevel <= level; ++i)
        music = table[i].musicindex;
    return music;
}

static void play_or_halt_music(qrsdata *q, coreState *cs, struct music *first_music, int desired_music)
{
    if (q->music == desired_music)
        return;

    q->music = desired_music;
    printf("music: %d\n", q->music);
    if (desired_music == -1)
        Mix_HaltMusic();
    else
        music_play(first_music + desired_music, cs);
}

static void update_music(qrsdata *q, coreState *cs)
{
    switch (q->mode_type) {
    case MODE_PENTOMINO:
        play_or_halt_music(q, cs, &cs->assets->track0, find_music(q->level, pentomino_music));
        break;

    case MODE_G2_MASTER:
        play_or_halt_music(q, cs, &cs->assets->g2_track0, find_music(q->level, g2_master_music));
        break;

    case MODE_G2_DEATH:
        play_or_halt_music(q, cs, &cs->assets->g2_track0, find_music(q->level, g2_death_music));
        break;

    case MODE_G3_TERROR:
        play_or_halt_music(q, cs, &cs->assets->g3_track0, find_music(q->level, g3_terror_music));
        break;

    case MODE_G1_MASTER:
    case MODE_G1_20G:
        play_or_halt_music(q, cs, &cs->assets->g1_track0, find_music(q->level, g1_music));
        break;

    default:
        log_warn("qrsdata->mode_type improperly set to %d\n", q->mode_type);
        break;
    }
}

game_t *qs_game_create(coreState *cs, int level, unsigned int flags, int replay_id)
{
    game_t *g = malloc(sizeof(game_t));
    qrsdata *q = NULL;
    qrs_player *p = NULL;

    g->origin = cs;
    g->field = qrsfield_create();

    g->init = qs_game_init;
    g->quit = qs_game_quit;
    g->preframe = qs_game_preframe;
    g->input = &qrs_input;
    g->frame = qs_game_frame;
    g->draw = gfx_drawqs;

    g->frame_counter = 0;

    g->data = malloc(sizeof(qrsdata));
    q = (qrsdata *)(g->data);

    q->mode_flags = flags;

    q->piecepool = qrspool_create();
    q->timer = nz_timer_create(60);
    q->p1 = malloc(sizeof(qrs_player));
    p = q->p1;
    p->def = NULL;
    p->speeds = NULL;
    p->state = PSINACTIVE;
    p->x = 0;
    p->y = 0;
    p->orient = FLAT;

    q->p1counters = malloc(sizeof(qrs_counters));
    q->p1counters->init = 0;
    q->p1counters->lock = 0;
    q->p1counters->are = 0;
    q->p1counters->lineare = 0;
    q->p1counters->lineclear = 0;
    q->p1counters->floorkicks = 0;
    q->p1counters->hold_flash = 0;

    q->randomizer = NULL;

    if (replay_id >= 0) {
        qrs_load_replay(g, replay_id);
        if (!q->replay) {
            free(q->p1);
            free(q->p1counters);
            nz_timer_destroy(q->timer);
            free(q->piecepool);
            free(q);
            grid_destroy(g->field);
            free(g);

            return NULL;
        }

        flags = q->replay->mode_flags;
        level = q->replay->starting_level;
    }
    else {
        q->replay = NULL;
    }

    q->recording = 0;
    q->playback = 0;
    q->playback_index = 0;

    q->is_practice = 0;

    //q->field_tex = SDL_CreateTexture(cs->screen.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 12*16, 20*16);
    //q->previous_field = NULL;
    //q->field_deltas = qrsfield_create();

    q->garbage = NULL;
    q->garbage_row_index = 0;
    q->garbage_counter = 0;
    q->garbage_delay = 0;

    q->previews[0] = NULL;
    q->previews[1] = NULL;
    q->previews[2] = NULL;
    q->hold = NULL;

    q->field_x = QRS_FIELD_X;
    q->field_y = QRS_FIELD_Y;

    q->field_w = 12;
    q->randomizer_type = RANDOMIZER_NORMAL;
    q->tetromino_only = 0;
    q->pentomino_only = 0;
    q->lock_delay_enabled = 1;
    q->lock_protect = 1;
    q->num_previews = 3;
    q->hold_enabled = 0;
    q->special_irs = 1;
    q->using_gems = false;

    q->piece_fade_rate = 300;
    q->stack_anim_counter = 0;
    // default credit roll is 60 seconds
    q->credit_roll_counter = 60 * 60;
    q->credit_roll_lineclears = 0;

    q->state_flags = 0;

    q->max_floorkicks = 2;
    q->lock_on_rotate = 0;

    request_fps(cs, 60);
    q->game_type = 0;
    q->mode_type = MODE_PENTOMINO;

    q->last_gradeup_timestamp = 0xFFFFFFFF;
    q->grade = GRADE_9;
    q->internal_grade = 0;
    q->grade_points = 0;
    q->grade_decay_counter = 0;

    q->mroll_unlocked = true;
    q->cur_section_timestamp = 0;

    for (int i = 0; i < MAX_SECTIONS; i++) {
        q->section_times[i] = -1;
        q->section_tetrises[i] = 0;
    }

    if (flags & MODE_G2_DEATH)
    {
        q->mode_type = MODE_G2_DEATH;
        q->grade = NO_GRADE;
        //q->field_x = QRS_FIELD_X - 28;
        //q->field_y = QRS_FIELD_Y - 16 + 2;
        q->lock_protect = 1;
        flags |= SIMULATE_G2;
        flags |= TETROMINO_ONLY;
        flags &= ~SIMULATE_G1;
        flags &= ~SIMULATE_G3;
    }
    else if (flags & MODE_G3_TERROR)
    {
        q->mode_type = MODE_G3_TERROR;
        q->grade = NO_GRADE;
        q->lock_protect = 1;
        flags |= SIMULATE_G3;
        flags |= TETROMINO_ONLY;
        flags &= ~SIMULATE_G1;
        flags &= ~SIMULATE_G2;
        if (level < 1000 && level >= 500) {
            q->state_flags |= GAMESTATE_RISING_GARBAGE;
        }
        else if (level >= 1000) {
            q->state_flags |= GAMESTATE_BRACKETS;
        }
    }
    else if (flags & MODE_G1_MASTER)
    {
        q->mode_type = MODE_G1_MASTER;
        q->grade = GRADE_9;
        flags |= SIMULATE_G1;
        flags |= TETROMINO_ONLY;
        flags &= ~SIMULATE_G2;
        flags &= ~SIMULATE_G3;
    }
    else if (flags & MODE_G1_20G)
    {
        q->mode_type = MODE_G1_20G;
        q->grade = GRADE_9;
        flags |= SIMULATE_G1;
        flags |= TETROMINO_ONLY;
        flags &= ~SIMULATE_G2;
        flags &= ~SIMULATE_G3;
    }
    else if (flags & MODE_G2_MASTER)
    {
        q->mode_type = MODE_G2_MASTER;
        q->grade = GRADE_9;
        flags |= SIMULATE_G2;
        flags |= TETROMINO_ONLY;
        flags &= ~SIMULATE_G1;
        flags &= ~SIMULATE_G3;

        // 61.68 "game seconds", or 60 realtime seconds
        q->credit_roll_counter = 3701;
    }

    if (flags & NIGHTMARE_MODE)
        q->randomizer_type = RANDOMIZER_NIGHTMARE;
    if (flags & NO_LOCK_DELAY)
        q->lock_delay_enabled = 0;
    if (flags & PENTOMINO_ONLY)
        q->pentomino_only = 1;

    if (flags & TETROMINO_ONLY) {
        q->field_w = 10;
        qrsfield_set_w(g->field, 10);
        q->tetromino_only = 1;
    }

    if (flags & SIMULATE_G1) {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G1;
        q->game_type = SIMULATE_G1;
        q->num_previews = 1;
        q->max_floorkicks = 0;
        q->special_irs = 0;
        q->lock_protect = 0;
        q->piecepool[QRS_I4]->flags &= ~PDNOWKICK;
    }

    if (flags & SIMULATE_G2) {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G2;
        q->game_type = SIMULATE_G2;
        q->num_previews = 1;
        q->max_floorkicks = 0;
        q->special_irs = 0;
        q->piecepool[QRS_I4]->flags &= ~PDNOWKICK;
        request_fps(cs, G2_FPS);
    }

    if (flags & SIMULATE_G3) {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G3;
        q->game_type = SIMULATE_G3;
        q->num_previews = 3;
        q->max_floorkicks = 1;
        q->special_irs = 0;
        q->lock_protect = 1;
        q->hold_enabled = 1;
    }

    if (q->game_type == 0)
        q->field_x = QRS_FIELD_X + 4;

    uint32_t randomizer_flags = 0;

    switch (q->randomizer_type) {
    case RANDOMIZER_NORMAL:
        if (q->pentomino_only)
            randomizer_flags |= PENTO_RAND_NOTETS;

        q->randomizer = pento_randomizer_create(randomizer_flags);
        break;

    case RANDOMIZER_NIGHTMARE:
        randomizer_flags |= PENTO_RAND_NIGHTMARE;
        q->randomizer = pento_randomizer_create(randomizer_flags);
        break;

    case RANDOMIZER_G1:
        q->randomizer = g1_randomizer_create(randomizer_flags);
        break;

    case RANDOMIZER_G2:
        q->randomizer = g2_randomizer_create(randomizer_flags);
        break;

    case RANDOMIZER_G3:
        q->randomizer = g2_randomizer_create(randomizer_flags);
        break;

    default:
        q->randomizer = pento_randomizer_create(randomizer_flags);
        break;
    }

    q->randomizer_seed = 0;

    q->level = 0;
    q->section = 0;
    q->rank = 0.0;
    q->score = 0;
    q->soft_drop_counter = 0;
    q->sonic_drop_height = 0;
    q->active_piece_time = 0;
    q->placement_speed = 0;
    q->levelstop_time = 0;

    q->lastclear = 0;
    q->locking_row = -1;
    q->lock_held = 0;
    q->lvlinc = 0;
    q->combo = 0;
    q->combo_simple = 0;

    q->singles = 0;
    q->doubles = 0;
    q->triples = 0;
    q->tetrises = 0;
    q->pentrises = 0;

    q->recoveries = 0;
    q->is_recovering = 0;

    q->last_medal_re_timestamp = 0xFFFFFFFF;
    q->last_medal_sk_timestamp = 0xFFFFFFFF;
    q->last_medal_st_timestamp = 0xFFFFFFFF;
    q->last_medal_co_timestamp = 0xFFFFFFFF;
    q->medal_re = 0;
    q->medal_sk = 0;
    q->medal_st = 0;
    q->medal_co = 0;

    q->speed_curve_index = 0;
    q->music = -1;

    if (flags & QRS_PRACTICE) {
        q->is_practice = 1;

        if (!cs->pracdata_mirror) {
            q->pracdata = malloc(sizeof(struct pracdata));

            q->pracdata->field_w = 10;
            q->pracdata->game_type = SIMULATE_G2;
            //q->pracdata->long_history = NULL;   // unused at the moment
            q->pracdata->usr_seq_expand_len = 0;
            q->pracdata->usr_seq_len = 0;
            q->pracdata->usr_field_undo = NULL;
            q->pracdata->usr_field_redo = NULL;
            q->pracdata->usr_field_undo_len = 0;
            q->pracdata->usr_field_redo_len = 0;
            q->pracdata->field_edit_in_progress = 0;
            q->pracdata->usr_field = qrsfield_create();
            q->pracdata->palette_selection = -5;
            q->pracdata->field_selection = 0;
            q->pracdata->field_selection_vertex1_x = 0;
            q->pracdata->field_selection_vertex1_y = 0;
            q->pracdata->field_selection_vertex2_x = 0;
            q->pracdata->field_selection_vertex2_y = 0;
            if (flags & TETROMINO_ONLY) {
                qrsfield_set_w(q->pracdata->usr_field, 10);
                q->field_w = 10;
            }

            q->pracdata->usr_timings = malloc(sizeof(qrs_timings));
            q->pracdata->usr_timings->level = 0;
            q->pracdata->usr_timings->are = 30;
            q->pracdata->usr_timings->grav = 4;
            q->pracdata->usr_timings->lock = 30;
            q->pracdata->usr_timings->das = 14;
            q->pracdata->usr_timings->lineare = 30;
            q->pracdata->usr_timings->lineclear = 40;
            q->pracdata->hist_index = 0;
            q->pracdata->paused = QRS_FIELD_EDIT;
            q->pracdata->grid_lines_shown = 0;
            q->pracdata->brackets = 0;
            q->pracdata->invisible = 0;
            q->pracdata->infinite_floorkicks = 0;
            q->pracdata->lock_protect = -1;
            if (flags & TETROMINO_ONLY)
                q->pracdata->piece_subset = SUBSET_TETS;
            else if (flags & PENTOMINO_ONLY)
                q->pracdata->piece_subset = SUBSET_PENTS;
            else
                q->pracdata->piece_subset = SUBSET_ALL;

            q->pracdata->randomizer_seed = 0;

            cs->pracdata_mirror = q->pracdata;
        }
        else {
            q->pracdata = cs->pracdata_mirror;
            q->field_w = q->pracdata->field_w;
            q->game_type = q->pracdata->game_type;
        }
    }
    else
        q->pracdata = NULL;

    if (!level)
        q->p1->speeds = &qs_curve[0];
    else {
        q->level = level;
        q->section = level / 100;
        q->p1->speeds = &qs_curve[8];

        if (q->mode_type == MODE_PENTOMINO)
        {
            if (q->level >= 1000) {
                double amount = pow(1.025, q->level - 1000);
                histrand_set_difficulty(q->randomizer, 15.0 + (amount > 85.0 ? 85.0 : amount));
            }
        }
    }

    return g;
}

int qs_game_init(game_t *g)
{
    if (!g)
        return -1;

    qrsdata *q = (qrsdata *)(g->data);
    qrs_player *p = q->p1;
    struct randomizer *qrand = q->randomizer;

    piece_id next1_id, next2_id, next3_id;
    int rc = 0;

    /*SDL_SetRenderTarget(g->origin->screen.renderer, q->field_tex);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(g->origin->screen.renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(g->origin->screen.renderer);
    SDL_SetRenderDrawBlendMode(g->origin->screen.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 255);
    SDL_SetRenderTarget(g->origin->screen.renderer, NULL);*/

    //gfx_createbutton(g->origin, "TEST", 31*16 - 6, 16 - 6, 0, toggle_obnoxious_text, NULL, NULL, RGBA_DEFAULT);

    /*int i = 0;
    int j = 0;
    int t = 0;*/

    if (q->replay) {
        *(qrand->seedp) = q->replay->seed;
        q->randomizer_seed = q->replay->seed;
    }
    else {
        q->randomizer_seed = *(qrand->seedp);
    }

    printf("Random seed: %ld\n", q->randomizer_seed);

    if (qrand) {
        qrand->init(qrand, NULL);

        next1_id = qrand->pull(qrand);
        next2_id = qrand->pull(qrand);
        next3_id = qrand->pull(qrand);

        if (qrand->num_pieces == 7) {
            next1_id = ars_to_qrs_id(next1_id);
            next2_id = ars_to_qrs_id(next2_id);
            next3_id = ars_to_qrs_id(next3_id);
        }
    }

    if (q->pracdata && q->pracdata->usr_seq_len) {
        q->previews[0] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 0));
        q->previews[1] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 1));
        q->previews[2] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 2));

        q->pracdata->hist_index = 2;
    }
    else {
        q->previews[0] = qrspiece_cpy(q->piecepool, next1_id);
        q->previews[1] = qrspiece_cpy(q->piecepool, next2_id);
        q->previews[2] = qrspiece_cpy(q->piecepool, next3_id);
    }

    if (q->state_flags & GAMESTATE_BRACKETS) {
        if (q->previews[0])
            q->previews[0]->flags |= PDBRACKETS;

        if (q->previews[1])
            q->previews[1]->flags |= PDBRACKETS;

        if (q->previews[2])
            q->previews[2]->flags |= PDBRACKETS;
    }

    if (q->cur_piece_qrs_id >= 18)
        p->y = ROWTOY(SPAWNY_QRS + 2);
    else
        p->y = ROWTOY(SPAWNY_QRS);

    p->def = NULL;
    p->x = SPAWNX_QRS;

    q->p1->state = PSFALL;

    if (!g->origin->assets->bg0.tex)   // used to check if we are in a testing environment
        return 0;

    int bgnumber = q->section;
    if (bgnumber > 12)
        bgnumber = 12;

    if (!q->pracdata) {
        g->origin->bg = (&g->origin->assets->bg0 + bgnumber)->tex;
        gfx_start_bg_fade_in(g->origin);

        if (q->mode_type == MODE_G2_DEATH) {
            //g->origin->anim_bg = g->origin->g2_bgs[q->section > 9 ? 9 : q->section];
            //g->origin->anim_bg_old = g->origin->anim_bg;
        }
    }
    else {
        q->p1->state = PSINACTIVE;
    }

    return 0;
}

int qs_game_pracinit(game_t *g, int val)
{
    coreState *cs = g->origin;
    g = cs->p1game;
    qrsdata *q = g->data;
    qrs_player *p = q->p1;
    qrs_counters *c = q->p1counters;

    struct randomizer *qrand = q->randomizer;
    piece_id next1_id, next2_id, next3_id;

    int i = 0;
    int j = 0;
    int cell = 0;

    cs->menu_input_override = 0;

    q->randomizer_seed = *(qrand->seedp);
    q->using_gems = false;

    gridcpy(q->pracdata->usr_field, g->field);

    for (i = 0; i < g->field->w; i++) {
        for (j = 0; j < g->field->h; j++) {
            cell = gridgetcell(g->field, i, j);
            if (cell < 0 || cell == GRID_OOB)
                continue;

            if (cell & QRS_PIECE_GEM)
                q->using_gems = true;
        }
    }

    c->init = 0;
    c->lock = 0;
    c->are = 0;
    c->lineare = 0;
    c->lineclear = 0;
    c->floorkicks = 0;
    q->timer->time = 0;

    q->p1->speeds = q->pracdata->usr_timings;
    if (q->pracdata->usr_timings->lock < 0)
        q->lock_delay_enabled = 0;
    else
        q->lock_delay_enabled = 1;

    q->state_flags = 0;

    if (q->pracdata->invisible)
        q->state_flags |= GAMESTATE_INVISIBLE;
    if (q->pracdata->brackets)
        q->state_flags |= GAMESTATE_BRACKETS;

    if (q->previews[0])
        piecedef_destroy(q->previews[0]);
    if (q->previews[1])
        piecedef_destroy(q->previews[1]);
    if (q->previews[2])
        piecedef_destroy(q->previews[2]);

    qrand->init(qrand, NULL);

    next1_id = qrand->pull(qrand);
    next2_id = qrand->pull(qrand);
    next3_id = qrand->pull(qrand);

    if (qrand->num_pieces == 7) {
        next1_id = ars_to_qrs_id(next1_id);
        next2_id = ars_to_qrs_id(next2_id);
        next3_id = ars_to_qrs_id(next3_id);
    }

    if (q->pracdata->usr_seq_len) {
        q->previews[0] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 0));
        q->previews[1] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 1));
        q->previews[2] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(q->pracdata, 2));

        q->pracdata->hist_index = 2;
    }
    else {
        q->previews[0] = qrspiece_cpy(q->piecepool, next1_id);
        q->previews[1] = qrspiece_cpy(q->piecepool, next2_id);
        q->previews[2] = qrspiece_cpy(q->piecepool, next3_id);
    }

    if (q->state_flags & GAMESTATE_BRACKETS) {
        if (q->previews[0])
            q->previews[0]->flags |= PDBRACKETS;

        if (q->previews[1])
            q->previews[1]->flags |= PDBRACKETS;

        if (q->previews[2])
            q->previews[2]->flags |= PDBRACKETS;
    }

    if (q->cur_piece_qrs_id >= 18)
        p->y = ROWTOY(SPAWNY_QRS + 2);
    else
        p->y = ROWTOY(SPAWNY_QRS);

    p->def = NULL;
    p->x = SPAWNX_QRS;

    q->p1->state = PSFALL;
    q->level = 0;
    q->pracdata->paused = 0;

    return 0;
}

int qs_game_quit(game_t *g)
{
    if (!g)
        return -1;

    qrsdata *q = g->data;

    if (g->field)
        grid_destroy(g->field);

    if (g->data)
        qrsdata_destroy(q);

    Mix_HaltMusic();

    // mostly a band-aid for quitting practice tool properly, so menu input does not take priority for regular modes
    g->origin->menu_input_override = 0;

    return 0;
}

int qs_game_preframe(game_t *g)
{
    return 0;
}

int qs_game_frame(game_t *g)
{
    if (!g)
        return -1;

    coreState *cs = g->origin;
    qrsdata *q = (qrsdata *)(g->data);

    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    if (q->pracdata) {
        if (q->pracdata->paused)
            return 0;
    }

    if (c->init < 120) {
        if (c->init == 0 || c->init == 60) {
            struct text_formatting *fmt = text_fmt_create(0, 0x00FF00FF, 0);
            fmt->size_multiplier = 2.0;
            fmt->outlined = false;
            if (q->pracdata)
                fmt->outlined = true;

            fmt->outline_rgba = 0x00000080;

            if (c->init == 0) {
                // Start recording/playback immediately
                if (!q->pracdata) {
                    if (q->replay) {
                        qrs_start_playback(g);
                    }
                    else {
                        qrs_start_record(g);
                    }
                }

                gfx_pushmessage(cs, "READY", (4 * 16 + 8 + q->field_x), (11 * 16 + q->field_y),
                    0, monofont_fixedsys, fmt, 60, qrs_game_is_inactive);

                sfx_play(&cs->assets->ready);
            }

            else if (c->init == 60) {
                fmt->rgba = 0xFF0000FF;
                gfx_pushmessage(cs, "GO", (6 * 16 + q->field_x), (11 * 16 + q->field_y),
                    0, monofont_fixedsys, fmt, 60, qrs_game_is_inactive);

                sfx_play(&cs->assets->go);
            }
        }

        (*s) = PSINACTIVE;
        c->init++;
        return 0;
    }
    else if (c->init == 120) {
        c->init++;
        qs_initnext(g, q->p1, 0);
        qrs_proc_initials(g);

        if (qrs_chkcollision(g, q->p1)) {
            qrs_lock(g, q->p1);
            (*s) = PSINACTIVE;
            Mix_HaltMusic();
            if (q->playback)
                qrs_end_playback(g);
            else if (q->recording)
                qrs_end_record(g);
            return 0;
        }

        if (!q->pracdata) {
            update_music(q, cs);
        }
    }

    if (!q->pracdata)
    {
        // handle speed curve and music updates (this runs every frame)
        // TODO: why does this need to run every frame and not only on level updates?
        update_music(q, cs);
        switch (q->mode_type)
        {
        case MODE_G2_MASTER:
            while (q->speed_curve_index < G2_MASTER_CURVE_MAX &&
                g2_master_curve[q->speed_curve_index].level <= q->level)
            {
                q->p1->speeds = &g2_master_curve[q->speed_curve_index];
                q->speed_curve_index++;
            }
            break;

        case MODE_G2_DEATH:
            while (q->speed_curve_index < G2_DEATH_CURVE_MAX &&
                g2_death_curve[q->speed_curve_index].level <= q->level)
            {
                q->p1->speeds = &g2_death_curve[q->speed_curve_index];
                q->speed_curve_index++;
            }
            break;

        case MODE_G3_TERROR:
            while (q->speed_curve_index < G3_TERROR_CURVE_MAX &&
                g3_terror_curve[q->speed_curve_index].level <= q->level)
            {
                q->p1->speeds = &g3_terror_curve[q->speed_curve_index];
                q->speed_curve_index++;
            }
            break;

        case MODE_G1_MASTER:
            while (q->speed_curve_index < G1_MASTER_CURVE_MAX &&
                g1_master_curve[q->speed_curve_index].level <= q->level)
            {
                q->p1->speeds = &g1_master_curve[q->speed_curve_index];
                q->speed_curve_index++;
            }
            break;

        case MODE_G1_20G:
            q->p1->speeds = &g1_master_curve[G1_MASTER_CURVE_MAX - 1];
            break;

        case MODE_PENTOMINO:
            while (q->speed_curve_index < QS_CURVE_MAX &&
                qs_curve[q->speed_curve_index].level <= q->level)
            {
                q->p1->speeds = &qs_curve[q->speed_curve_index];
                q->speed_curve_index++;
            }
            break;

        default:
            break;
        }
    }

    if (q->state_flags & GAMESTATE_TOPOUT_ANIM) {
        q->stack_anim_counter++;
        if (q->stack_anim_counter == 3 * 20)
        {
            q->stack_anim_counter = 0;
            q->state_flags &= ~GAMESTATE_TOPOUT_ANIM;
            q->state_flags |= GAMESTATE_GAMEOVER;
        }
        else if (q->stack_anim_counter % 3 == 1)
        {
            int row = 21 - (q->stack_anim_counter / 3);
            int start_i = (g->field->w - q->field_w) / 2;

            for (int i = start_i; i < g->field->w - start_i; i++) {
                if (gridgetcell(g->field, i, row))
                    gridsetcell(g->field, i, row, QRS_PIECE_GARBAGE);
            }
        }
    }

    if (q->state_flags & GAMESTATE_FADE_TO_CREDITS) {
        q->stack_anim_counter++;
        if (q->stack_anim_counter == 6 * 20) // 6 frames for each visible row
        {
            q->stack_anim_counter = 0;

            // just zeroing out the last two (invisible) rows
            int start_i = (g->field->w - q->field_w) / 2;
            for (int i = start_i; i < g->field->w - start_i; i++) {
                gridsetcell(g->field, i, 0, 0);
                gridsetcell(g->field, i, 1, 0);
            }

            q->state_flags &= ~GAMESTATE_FADE_TO_CREDITS;

            // for testing
            // q->mroll_unlocked = true;

            if (q->mroll_unlocked)
                q->state_flags |= GAMESTATE_INVISIBLE | GAMESTATE_CREDITS;
            else
                q->state_flags |= GAMESTATE_FADING | GAMESTATE_CREDITS;

            qs_initnext(g, q->p1, 0);
            qrs_proc_initials(g);
        }
        else if (q->stack_anim_counter % 6 == 1) {
            int row = 21 - (q->stack_anim_counter / 6);
            int start_i = (g->field->w - q->field_w) / 2;

            for (int i = start_i; i < g->field->w - start_i; i++) {
                gridsetcell(g->field, i, row, 0);
            }
        }
    }

    if (q->state_flags & GAMESTATE_CREDITS)
    {
        q->credit_roll_counter--;
        if (q->credit_roll_counter == 0)
        {
            switch (q->mode_type)
            {
            case MODE_G1_MASTER:
            case MODE_G1_20G:
            case MODE_G2_DEATH:
            case MODE_G3_TERROR:
            case MODE_PENTOMINO:
                q->state_flags &= ~GAMESTATE_CREDITS;
                (*s) = PSINACTIVE;
                break;

            case MODE_G2_MASTER:
                q->state_flags &= ~(GAMESTATE_CREDITS | GAMESTATE_FADING | GAMESTATE_INVISIBLE);
                if (q->mroll_unlocked)
                {
                    q->state_flags |= GAMESTATE_FIREWORKS_GM;

                    if (q->credit_roll_lineclears >= 32)
                        q->grade = GRADE_GM | ORANGE_LINE;
                    else
                        q->grade = GRADE_GM | GREEN_LINE;
                }
                else
                {
                    q->state_flags |= GAMESTATE_FIREWORKS;
                    q->grade |= ORANGE_LINE;
                }

                (*s) = PSINACTIVE;
                break;
            }

            if (q->playback)
                qrs_end_playback(g);
            else if (q->recording)
                qrs_end_record(g);
        }
    }

    if ((*s) == PSINACTIVE) {
        return 0;
    }

    if ((*s) & PSSPAWN) {
        qs_process_fall(g);
        (*s) &= ~PSSPAWN;
        //printf("First piece\n");
    }
    else {
        if (qs_process_are(g) == QSGAME_SHOULD_TERMINATE) {
            if (q->pracdata) {
                q->pracdata->paused = 1;
            }

            q->p1->state = PSINACTIVE;
        }

        if ((*s) & PSSPAWN) {
            qs_process_fall(g);
            (*s) &= ~PSSPAWN;
        }
        else {
            qs_process_prelockflash(g);
            qs_process_fall(g);
            qs_process_lock(g);
            qs_process_lockflash(g);
            qs_process_lineclear(g);
            if (qs_process_lineare(g) == QSGAME_SHOULD_TERMINATE) {
                if (q->pracdata) {
                    q->pracdata->paused = 1;
                }

                q->p1->state = PSINACTIVE;
            }

            if ((*s) & PSSPAWN) {
                qs_process_fall(g);
                (*s) &= ~PSSPAWN;
            }
        }
    }

    if (q->state_flags & GAMESTATE_FADING) {
        int fade_counter = 0;
        int val = 0;

        for (int i = 0; i < g->field->w; i++) {
            for (int j = 0; j < g->field->h; j++) {
                val = gridgetcell(g->field, i, j);
                if (val == QRS_WALL || val < 0)
                    continue;

                fade_counter = GET_PIECE_FADE_COUNTER(val);

                if (fade_counter > 0) {
                    fade_counter--;
                    SET_PIECE_FADE_COUNTER(val, fade_counter);
                    gridsetcell(g->field, i, j, val);
                }
            }
        }
    }

    // player state might have been set to INACTIVE, we want to react right away
    if ((*s) == PSINACTIVE)
        return 0;

    if (q->mode_type == MODE_G2_MASTER) {
        int decay_rate = grade_point_decays[q->internal_grade];
        if (((*s) & PSFALL || (*s) & PSLOCK) && q->combo_simple == 0)
            q->grade_decay_counter++;

        if (q->grade_decay_counter >= decay_rate && q->grade_points > 0) {
            q->grade_points--;
            q->grade_decay_counter = 0;
        }
    }

    if (!q->pracdata && q->is_recovering && q->game_type == 0) {
        if (grid_cells_filled(g->field) <= 85) {
            q->recoveries++;
            q->last_medal_re_timestamp = g->frame_counter;
            q->is_recovering = 0;
            switch (q->recoveries) {
            case 1:
                q->medal_re = BRONZE;
                sfx_play(&cs->assets->medal);
                break;
            case 2:
                q->medal_re = SILVER;
                sfx_play(&cs->assets->medal);
                break;
            case 3:
                q->medal_re = GOLD;
                sfx_play(&cs->assets->medal);
                break;
            case 5:
                q->medal_re = PLATINUM;
                sfx_play(&cs->assets->medal);
                break;
            default:
                break;
            }
        }
    }
    else if (grid_cells_filled(g->field) >= 170)
        q->is_recovering = 1;

    if (!q->pracdata && q->mode_type == MODE_G3_TERROR) {
        if (q->level >= 1000) {
            q->state_flags |= GAMESTATE_BRACKETS;
            q->state_flags &= ~GAMESTATE_RISING_GARBAGE;
        }
        else if (q->level >= 500) {
            q->state_flags |= GAMESTATE_RISING_GARBAGE;
            switch (q->section) {
            case 5:
                q->garbage_delay = 20;
                break;
            case 6:
                q->garbage_delay = 18;
                break;
            case 7:
                q->garbage_delay = 10;
                break;
            case 8:
                q->garbage_delay = 9;
                break;
            case 9:
                q->garbage_delay = 8;
                break;
            default:
                break;
            }
        }
    }

    if (!q->pracdata && q->mode_type == MODE_PENTOMINO) {
        if (q->level >= 1000) {
            // histrand_set_difficulty(q->randomizer, 5.0 + 0.2 * (q->level - 1000));
            double amount = pow(1.025, q->level - 1000);
            histrand_set_difficulty(q->randomizer, 15.0 + (amount > 85.0 ? 85.0 : amount));
        }

        // for testing
        // histrand_set_difficulty(q->randomizer, 100.0);
    }

    if (q->levelstop_time)
        q->levelstop_time++;

    if (!(q->state_flags & GAMESTATE_CREDITS))
        timeinc(q->timer);

    return 0;
}

static int qs_are_expired(game_t *g)
{
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    q->lastclear = 0;
    if (q->level % 100 != 99 && !(q->state_flags & GAMESTATE_CREDITS)) {
        switch (q->mode_type) {
        case MODE_G2_DEATH:
        case MODE_G1_20G:
        case MODE_G1_MASTER:
        case MODE_G2_MASTER:
            if (q->level != 998) {
                q->level++;
                q->lvlinc = 1;
            }
            else {
                q->lvlinc = 0;
                q->levelstop_time++;
            }

            break;

        default:
            q->level++;
            q->lvlinc = 1;
            break;
        }
    }
    else {
        q->lvlinc = 0;
        if (q->level % 100 == 99)
            q->levelstop_time++;
    }

    c->lock = 0;
    if (qs_initnext(g, q->p1, 0) == QSGAME_SHOULD_TERMINATE) {
        return QSGAME_SHOULD_TERMINATE;
    }

    qrs_proc_initials(g);

    if (qrs_chkcollision(g, q->p1)) {
        qrs_lock(g, q->p1);
        (*s) = PSINACTIVE;
        Mix_HaltMusic();
        if (q->playback)
            qrs_end_playback(g);
        else if (q->recording)
            qrs_end_record(g);

        // TODO: for lineare, this was different - intentional or copy/paste bug?
        /*
                if(q->state_flags & GAMESTATE_CREDITS)
                    q->state_flags |= GAMESTATE_CREDITS_TOPOUT;
                else
                    q->state_flags |= GAMESTATE_TOPOUT_ANIM;
        */
        if (q->state_flags & GAMESTATE_CREDITS) {
            q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE);
            q->state_flags |= GAMESTATE_CREDITS_TOPOUT;

            if (q->mode_type == MODE_G2_MASTER) {
                if (q->mroll_unlocked)
                    q->grade = GRADE_M | GREEN_LINE;
                else
                    q->grade |= GREEN_LINE;
            }
        }
        else
            q->state_flags |= GAMESTATE_TOPOUT_ANIM;

        return 0;
    }

    if (q->state_flags & GAMESTATE_RISING_GARBAGE)
        q->garbage_counter++;

    return 0;
}

int qs_process_are(game_t *g)
{
    //coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    if ((*s) & PSLOCKFLASH4)
        (*s) &= ~PSLOCKFLASH4;

    if ((*s) & PSLOCKFLASH3) {
        (*s) &= ~PSLOCKFLASH3;
        (*s) |= PSLOCKFLASH4;
    }

    if ((*s) & PSLOCKFLASH2) {
        (*s) &= ~PSLOCKFLASH2;
        (*s) |= PSLOCKFLASH3;

        if (!q->lastclear)
            (*s) |= PSARE;
    }

    if ((*s) & PSARE) {
        if (c->are == q->p1->speeds->are) {
            c->are = 0;
            return qs_are_expired(g);
        }
        else {
            c->are++;
            if (c->are == 1)
            {
                // TODO: why is this not in lineare too?
                if (q->state_flags & GAMESTATE_RISING_GARBAGE)
                {
                    if (q->garbage_counter >= q->garbage_delay)
                    {
                        qrs_spawn_garbage(g, GARBAGE_COPY_BOTTOM_ROW);
                        q->garbage_counter = 0;
                    }
                }
            }
        }
    }

    return 0;
}

int qs_process_lineare(game_t *g)
{
    //coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    if ((*s) & PSLINEARE) {
        if (c->lineare == q->p1->speeds->lineare) {
            c->lineare = 0;
            return qs_are_expired(g);
        }
        else
            c->lineare++;
    }

    return 0;
}

int qs_process_lineclear(game_t *g)
{
    coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    if ((*s) & PSLINECLEAR) {
        if (c->lineclear == q->p1->speeds->lineclear) {
            (*s) &= ~PSLINECLEAR;
            (*s) |= PSLINEARE;

            c->lineclear = 0;
            qrs_dropfield(g);
            sfx_play(&cs->assets->dropfield);

            switch (q->mode_type)
            {
            case MODE_G2_MASTER:
                if (q->level == 999 && !(q->state_flags & GAMESTATE_CREDITS)) {
                    q->state_flags |= GAMESTATE_FADE_TO_CREDITS;
                    (*s) = PSINACTIVE;
                }

                break;

            default:
                break;
            }
        }
        else
            c->lineclear++;
    }

    return 0;
}

int qs_process_lock(game_t *g)
{
    //coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    if ((*s) & PSLOCK && q->lock_on_rotate == 2)
    {
        if (q->pracdata) {
            if (!q->pracdata->infinite_floorkicks) {
                c->lock = 0;
                (*s) &= ~PSLOCK;
                (*s) |= PSPRELOCKED;
            }
        }
        else {
            c->lock = 0;
            (*s) &= ~PSLOCK;
            (*s) |= PSPRELOCKED;
        }

        q->lock_on_rotate = 0;
    }
    else if ((*s) & PSLOCK && q->lock_delay_enabled)
    {
        if (qrs_isonground(g, q->p1)) {
            if (YTOROW(q->p1->y) != q->locking_row) {
                q->locking_row = YTOROW(q->p1->y);
                c->lock = 0;
            }

            if (!q->p1->speeds->lock) {
                (*s) &= ~PSLOCK;
                (*s) |= PSPRELOCKED;
            }
            else {
                c->lock++;
                if (c->lock == q->p1->speeds->lock) {
                    c->lock = 0;
                    (*s) &= ~PSLOCK;
                    (*s) |= PSPRELOCKED;
                }
            }
        }
        else {
            (*s) &= ~PSLOCK;
            (*s) |= PSFALL;
        }
    }

    return 0;
}

int qs_process_fall(game_t *g)
{
    //coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    qrs_counters *c = q->p1counters;

    int row = 0;

    if ((*s) & PSFALL) {
        row = YTOROW(q->p1->y);
        qrs_fall(g, q->p1, 0);
        if (YTOROW(q->p1->y) < row) {
            c->lock = 0;
        }
    }

    return 0;
}

int qs_process_prelockflash(game_t *g)
{
    //coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    //qrs_counters *c = q->p1counters;

    if ((*s) & PSLOCKFLASH1) {
        (*s) &= ~PSLOCKFLASH1;
        (*s) |= PSLOCKFLASH2;
    }

    if ((*s) & PSPRELOCKED) {
        (*s) &= ~PSPRELOCKED;
        qrs_lock(g, q->p1);

        (*s) &= ~PSPRELOCKFLASH1;
        (*s) |= PSLOCKFLASH1;
    }

    if ((*s) & PSLOCKPRESSED) {
        (*s) &= ~PSLOCKPRESSED;
        (*s) |= PSPRELOCKED;
    }

    if ((*s) & PSPRELOCKFLASH1) {
        (*s) &= ~PSPRELOCKFLASH1;
        (*s) |= PSLOCKFLASH1;
    }

    return 0;
}

int qs_process_lockflash(game_t *g)
{
    coreState *cs = g->origin;
    qrsdata *q = g->data;
    unsigned int *s = &q->p1->state;
    //qrs_counters *c = q->p1counters;

    int n = 0;

    if ((*s) & PSLOCKFLASH1) {
        n = qrs_lineclear(g, q->p1);
        q->lastclear = n;

        q->placement_speed = q->p1->speeds->lock - q->active_piece_time;
        if (q->placement_speed < 0)
            q->placement_speed = 0;

        if (n) {
            (*s) |= PSLINECLEAR;

            q->combo += 2 * n - 2;
            bool bravo = false;

            int cells = grid_cells_filled(g->field);
            // slightly convoluted calculation, need to do this to account for the QRS_WALL blocks on the sides
            if (cells == (g->field->w * g->field->h) - (q->field_w * QRS_FIELD_H))
                bravo = true;

            switch (n) {
            case 5:
                q->pentrises++;
                break;
            case 4:
                q->tetrises++;
                if (!(q->state_flags & GAMESTATE_CREDITS))
                    q->section_tetrises[q->section]++;

                break;
            case 3:
                q->triples++;
                break;
            case 2:
                q->doubles++;
                break;
            case 1:
                q->singles++;
                break;
            default:
                break;
            }

            if (!(q->state_flags & GAMESTATE_CREDITS))
            {
                if (q->game_type == SIMULATE_G3 && n > 2)
                    q->lvlinc = 2 * n - 2;
                else
                    q->lvlinc = n;

                q->level += q->lvlinc;

                int pts;
                float combo_mult;
                bool gradeup = false;

                double n_val;

                // score/grade stuff
                switch (q->mode_type)
                {
                case MODE_G1_MASTER:
                case MODE_G1_20G:
                    pts = (ceil(q->level / 4) + q->soft_drop_counter) * n * q->combo * (bravo ? 4 : 1);
                    q->score += pts;
                    for (int i = 0; i < 17; i++) {
                        if (q->score - pts < g1_grade_score_reqs[i] && q->score >= g1_grade_score_reqs[i]) {
                            q->grade = GRADE_8 + i;
                            if (!gradeup) {
                                q->last_gradeup_timestamp = g->frame_counter;
                                sfx_play(&cs->assets->gradeup);
                                gradeup = true;
                            }
                        }
                    }

                    break;

                case MODE_G2_DEATH:
                    q->score += (q->level / 4 + q->soft_drop_counter + 2 * q->sonic_drop_height) * n * q->combo * (bravo ? 4 : 1)
                        + q->level / 2 + 7 * q->placement_speed;
                    break;

                case MODE_G3_TERROR:
                    break;

                case MODE_PENTOMINO:
                    n_val = ((double)n * 0.7);
                    if (n_val < 1.0)
                        n_val = 1.0;

                    pts = (double)(ceil((double)q->level / 3.0) + q->soft_drop_counter + q->sonic_drop_height
                        + 2 * q->placement_speed - (q->levelstop_time / 4))
                        * n_val * (double)q->combo * (bravo ? 4.0 : 1.0);
                    if (pts < 0)
                        pts = 0;

                    q->score += pts;

                    for (int i = 0; i < 17; i++) {
                        if (q->score - pts < g1_grade_score_reqs[i] && q->score >= g1_grade_score_reqs[i]) {
                            q->grade = GRADE_8 + i;
                            if (!gradeup) {
                                q->last_gradeup_timestamp = g->frame_counter;
                                sfx_play(&cs->assets->gradeup);
                                gradeup = true;
                            }
                        }
                    }

                    break;

                case MODE_G2_MASTER:
                    pts = grade_points_table[q->internal_grade][n - 1];
                    combo_mult = g2_grade_point_combo_table[q->combo_simple - 1][n - 1];

                    q->grade_points += (int)ceil(pts * combo_mult) * (1 + q->level / 250);

                    if (q->grade_points >= 100) {
                        int old_grade = q->grade;
                        q->grade_points = 0;
                        q->internal_grade++;
                        if (q->internal_grade > 31) q->internal_grade = 31;
                        q->grade = internal_to_displayed_grade(q->internal_grade);
                        if (old_grade != q->grade) {
                            q->last_gradeup_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->gradeup);
                        }
                    }

                    // Score = ((Level + Lines)/4 + Soft + (2 x Sonic)) x Lines x Combo x Bravo + (Level_After_Clear)/2 + (Speed x 7)
                    q->score += (q->level / 4 + q->soft_drop_counter + 2 * q->sonic_drop_height) * n * q->combo * (bravo ? 4 : 1)
                        + q->level / 2 + 7 * q->placement_speed;

                    break;
                }
            }
            else {
                q->lvlinc = 0;
            }

            if (q->state_flags & GAMESTATE_RISING_GARBAGE) {
                switch (q->mode_type) {
                case MODE_G3_TERROR:
                    q->garbage_counter -= n;
                    if (q->garbage_counter < 0)
                        q->garbage_counter = 0;
                    break;

                default:
                    break;
                }
            }

            if (!q->pracdata && q->game_type == 0) {
                switch (q->combo_simple) {
                case 3:
                    if (q->medal_co <= BRONZE) {
                        q->medal_co = BRONZE;
                        q->last_medal_co_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->medal);
                    }

                    break;
                case 4:
                    if (q->medal_co <= SILVER) {
                        q->medal_co = SILVER;
                        q->last_medal_co_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->medal);
                    }

                    break;
                case 5:
                    if (q->medal_co <= GOLD) {
                        q->medal_co = GOLD;
                        q->last_medal_co_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->medal);
                    }

                    break;
                case 6:
                    if (q->medal_co <= PLATINUM) {
                        q->medal_co = PLATINUM;
                        q->last_medal_co_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->medal);
                    }

                    break;
                default:
                    break;
                }

                if (n == 5 && q->mode_type == MODE_PENTOMINO) {
                    switch (q->pentrises) {
                    case 2:
                        if (q->medal_sk <= BRONZE) {
                            q->medal_sk = BRONZE;
                            q->last_medal_sk_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->medal);
                        }

                        break;
                    case 4:
                        if (q->medal_sk <= SILVER) {
                            q->medal_sk = SILVER;
                            q->last_medal_sk_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->medal);
                        }

                        break;
                    case 7:
                        if (q->medal_sk <= GOLD) {
                            q->medal_sk = GOLD;
                            q->last_medal_sk_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->medal);
                        }

                        break;
                    case 10:
                        if (q->medal_sk <= PLATINUM) {
                            q->medal_sk = PLATINUM;
                            q->last_medal_sk_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->medal);
                        }

                        break;
                    default:
                        break;
                    }
                }
            }

            sfx_play(&cs->assets->lineclear);

            if (((q->level - q->lvlinc) % 100) > 90 && (q->level % 100) < 10) {
                q->section_times[q->section] = q->timer->time - q->cur_section_timestamp;
                q->cur_section_timestamp = q->timer->time;
                q->section++;

                q->levelstop_time = 0;

                if (!q->pracdata) {
                    switch (q->mode_type) {
                    case MODE_PENTOMINO:
                        if (q->section == 5) {
                            if (q->score < 40000 || q->timer->time > 5 * 60 * 60 + 45 * 60) {
                                q->mroll_unlocked = false;
                            }
                        }
                        else if (q->section == 10) {
                            if (q->timer->time > 9 * 60 * 60 + 45 * 60) {
                                q->section--;

                                if (q->playback)
                                    qrs_end_playback(g);
                                else if (q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                            }
                            else {
                                if (q->score < 126000)
                                    q->mroll_unlocked = false;

                                if (q->mroll_unlocked) {
                                    q->grade = GRADE_M;
                                    q->last_gradeup_timestamp = g->frame_counter;
                                    sfx_play(&cs->assets->gradeup);
                                }
                            }
                        }
                        else if (q->level >= 1200 && q->level < 1300) {
                            q->level = 1200;

                            if (q->score < 150000)
                                q->mroll_unlocked = false;

                            if (q->mroll_unlocked) {
                                q->grade = GRADE_GM;
                                q->last_gradeup_timestamp = g->frame_counter;
                                sfx_play(&cs->assets->gradeup);
                            }

                            if (q->playback)
                                qrs_end_playback(g);
                            else if (q->recording)
                                qrs_end_record(g);
                            q->p1->state = PSINACTIVE;
                        }

                        break;

                    case MODE_G2_MASTER:
                        if (q->level >= 999) {
                            q->level = 999;
                            if (q->timer->time > 525 * 60 || q->grade < GRADE_S9)
                                q->mroll_unlocked = false;
                        }

                        switch (q->section - 1)
                        {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            if (q->section_tetrises[q->section - 1] < 2 ||
                                q->section_times[q->section - 1] > (65 * 60))
                            {
                                q->mroll_unlocked = false;
                            }

                            break;

                        case 5:
                            if (q->section_tetrises[q->section - 1] < 1 ||
                                q->section_times[q->section - 1] > (AVG_FIRST_FIVE(q->section_times)) + 2 * 60)
                            {
                                q->mroll_unlocked = false;
                            }

                            break;

                        case 6:
                        case 7:
                        case 8:
                            if (q->section_tetrises[q->section - 1] < 1 ||
                                q->section_times[q->section - 1] > (q->section_times[q->section - 2]) + 2 * 60)
                            {
                                q->mroll_unlocked = false;
                            }

                            break;

                        case 9:
                            if (q->section_times[q->section - 1] > (q->section_times[q->section - 2]) + 2 * 60)
                                q->mroll_unlocked = false;

                            break;

                        default:
                            break;
                        }

                        break;

                    case MODE_G2_DEATH:
                        if (q->section == 5) {
                            if (q->timer->time > G2_DEATH_TORIKAN) {
                                q->section--;

                                if (q->playback)
                                    qrs_end_playback(g);
                                else if (q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                            }
                            else {
                                q->grade = GRADE_M;
                                q->last_gradeup_timestamp = g->frame_counter;
                                sfx_play(&cs->assets->gradeup);
                            }
                        }
                        else if (q->level >= 999) {
                            q->level = 999;
                            q->grade = GRADE_GM;
                            q->last_gradeup_timestamp = g->frame_counter;
                            sfx_play(&cs->assets->gradeup);
                            if (q->playback)
                                qrs_end_playback(g);
                            else if (q->recording)
                                qrs_end_record(g);
                            q->p1->state = PSINACTIVE;
                        }

                        break;

                    case MODE_G3_TERROR:
                        if (q->grade == NO_GRADE)
                            q->grade = GRADE_S1;
                        else
                            q->grade++;

                        q->last_gradeup_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->gradeup);

                        if (q->section == 5) {
                            if (q->timer->time > G3_TERROR_TORIKAN) {
                                q->grade = GRADE_S5;

                                if (q->playback)
                                    qrs_end_playback(g);
                                else if (q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                            }
                            else {
                                q->grade = GRADE_S5;
                            }
                        }
                        else if (q->section == 10) {
                            if (q->timer->time > 2 * G3_TERROR_TORIKAN) {
                                q->section--;

                                if (q->playback)
                                    qrs_end_playback(g);
                                else if (q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                            }
                            else {
                                q->grade = GRADE_S10;
                            }
                        }
                        else if (q->section == 13) {
                            q->level = 1300;
                            q->grade = GRADE_S13;
                            if (q->playback)
                                qrs_end_playback(g);
                            else if (q->recording)
                                qrs_end_record(g);
                            q->p1->state = PSINACTIVE;
                        }

                        break;

                    case MODE_G1_20G:
                    case MODE_G1_MASTER:
                        // checking "mroll" requirements (actually just GM reqs)
                        if (q->section == 3 && (q->timer->time > (4 * 60 * 60 + 15 * 60) || q->score < 12000))
                            q->mroll_unlocked = false;

                        if (q->section == 5 && (q->timer->time > (7 * 60 * 60 + 30 * 60) || q->score < 40000))
                            q->mroll_unlocked = false;

                        if (q->level >= 999) {
                            q->level = 999;
                            if (q->timer->time >= (13 * 60 * 60 + 30 * 60) || q->score < 126000)
                                q->mroll_unlocked = false;

                            if (q->mroll_unlocked) {
                                q->grade = GRADE_GM;
                                q->last_gradeup_timestamp = g->frame_counter;
                                sfx_play(&cs->assets->gradeup);
                            }

                            if (q->playback)
                                qrs_end_playback(g);
                            else if (q->recording)
                                qrs_end_record(g);
                            q->p1->state = PSINACTIVE;
                        }

                        break;

                    default:
                        break;
                    }

                    sfx_play(&cs->assets->newsection);
                    if (q->section < 13) {
                        cs->bg = (&cs->assets->bg0 + q->section)->tex;
                    }
                }
            }
            else if (q->level == 999 && q->lvlinc) {
                switch (q->mode_type) {
                case MODE_G2_MASTER:
                    if (q->timer->time > 525 * 60 || q->grade < GRADE_S9)
                        q->mroll_unlocked = false;

                    if (q->section_times[9] > (q->section_times[8]) + 2 * 60)
                        q->mroll_unlocked = false;

                    break;

                case MODE_G2_DEATH:
                    q->grade = GRADE_GM;
                    q->last_gradeup_timestamp = g->frame_counter;
                    sfx_play(&cs->assets->gradeup);
                    if (q->playback)
                        qrs_end_playback(g);
                    else if (q->recording)
                        qrs_end_record(g);
                    q->p1->state = PSINACTIVE;

                    break;

                case MODE_G3_TERROR:
                    break;

                case MODE_G1_20G:
                case MODE_G1_MASTER:
                    if (q->timer->time >= (13 * 60 * 60 + 30 * 60) || q->score < 126000)
                        q->mroll_unlocked = false;

                    if (q->mroll_unlocked) {
                        q->grade = GRADE_GM;
                        q->last_gradeup_timestamp = g->frame_counter;
                        sfx_play(&cs->assets->gradeup);
                    }

                    if (q->playback)
                        qrs_end_playback(g);
                    else if (q->recording)
                        qrs_end_record(g);
                    q->p1->state = PSINACTIVE;

                    break;

                default:
                    break;
                }
            }

            q->combo_simple += (n > 1);
        }
        else {
            q->combo = 1;
            q->combo_simple = 1;

            if (q->mode_type == MODE_PENTOMINO && q->levelstop_time > 0 && q->levelstop_time < 500) {
                q->score -= (q->levelstop_time / 4 > 100) ? 100 : q->levelstop_time / 4;
                if (q->score < 0)
                    q->score = 0;

                int old_grade = q->grade;

                for (int i = 0; i < 17; i++) {
                    if (q->score >= g1_grade_score_reqs[i]) {
                        q->grade = GRADE_8 + i;
                    }
                }

                if (q->score < g1_grade_score_reqs[0])
                    q->grade = GRADE_9;

                if (old_grade != q->grade)
                    q->last_gradeup_timestamp = g->frame_counter;
            }
        }
    }

    return 0;
}

int qrs_game_is_inactive(coreState *cs)
{
    if (!cs->p1game)
        return 1;

    qrsdata *q = cs->p1game->data;

    if (!q)
        return 1;

    if (q->pracdata) {
        if (q->pracdata->paused == QRS_FIELD_EDIT)
            return 1;
    }

    return 0;
}

// TODO please fix this mess...
/*
need to move practool-related stuff to a separate game_t than the QRS game_t
so there aren't so many awkward overlapping functions

need to break this up into multiple functions which each update exactly one
thing

split up pieceseq parser into: parsing function, expansion function, and
get_usrseq_elem

get_elem should return failure if q->pracdata->usr_seq_expand is NULL and it
should not modify state
*/

int qs_update_pracdata(coreState *cs)
{
    if (!cs->p1game || !cs->menu)
        return 1;

    qrsdata *q = cs->p1game->data;
    struct pracdata *d = q->pracdata;
    menudata *md = cs->menu->data;
    bstring seq_bstr = NULL;
    char name_str[3] = { 0, 0, 0 };

    int piece_seq[3000];
    int num = 0;

    int i = 0;
    int j = 0;
    int k = 0;
    int t = 0;
    unsigned char c;

    int rpt_start = 0;
    int rpt_end = 0;
    int rpt = 0;
    int rpt_count = 0;
    int pre_rpt_count = 0;

    char rpt_count_strbuf[5];

    q->game_type = d->game_type;
    q->field_w = d->field_w;

    switch (q->game_type) {
    case 0:
        q->num_previews = 3;
        q->randomizer_type = RANDOMIZER_NORMAL;
        if (q->randomizer) randomizer_destroy(q->randomizer);
        q->randomizer = pento_randomizer_create(0);

        q->hold_enabled = 0;
        q->max_floorkicks = 2;
        q->lock_protect = 1;
        q->piecepool[QRS_I4]->flags &= ~PDNOWKICK;
        q->tetromino_only = 0;
        q->pentomino_only = 0;
        request_fps(cs, 60);
        break;
    case SIMULATE_G1:
        q->num_previews = 1;
        q->randomizer_type = RANDOMIZER_G1;
        if (q->randomizer) randomizer_destroy(q->randomizer);
        q->randomizer = g1_randomizer_create(0);

        q->hold_enabled = 0;
        q->max_floorkicks = 0;
        q->lock_protect = 0;
        q->piecepool[QRS_I4]->flags |= PDNOWKICK;
        q->tetromino_only = 1;
        q->pentomino_only = 0;
        request_fps(cs, 60);
        break;
    case SIMULATE_G2:
        q->num_previews = 1;
        q->randomizer_type = RANDOMIZER_G2;
        if (q->randomizer) randomizer_destroy(q->randomizer);
        q->randomizer = g2_randomizer_create(0);

        q->hold_enabled = 0;
        q->max_floorkicks = 0;
        q->lock_protect = 1;
        q->piecepool[QRS_I4]->flags |= PDNOWKICK;
        q->tetromino_only = 1;
        q->pentomino_only = 0;
        request_fps(cs, G2_FPS);
        break;
    case SIMULATE_G3:
        q->num_previews = 3;
        q->randomizer_type = RANDOMIZER_G3;
        if (q->randomizer) randomizer_destroy(q->randomizer);
        q->randomizer = g3_randomizer_create(0);

        q->hold_enabled = 1;
        q->max_floorkicks = 1;
        q->lock_protect = 1;
        q->piecepool[QRS_I4]->flags &= ~PDNOWKICK;
        q->tetromino_only = 1;
        q->pentomino_only = 0;
        request_fps(cs, 60);
        break;
    default:
        break;
    }

    q->hold = NULL;

    // and now for the hackiest check ever to see if we need to update the usr_seq

    if (md->numopts == MENU_PRACTICE_NUMOPTS && md->menu[md->numopts - 1]->type == MENU_TEXTINPUT) {
        seq_bstr = ((struct text_opt_data *)(md->menu[md->numopts - 1]->data))->text;
        for (i = 0; i < seq_bstr->slen; i++) {
            c = seq_bstr->data[i];
            if ((c < 'A' || c > 'Z') && !(c == '*' || c == '(' || c == ')')) {
                if (rpt_count) {
                    k = 0;
                    while (k < 4 && i < seq_bstr->slen && seq_bstr->data[i] >= '0' && seq_bstr->data[i] <= '9') {
                        rpt_count_strbuf[k] = seq_bstr->data[i];
                        rpt_count_strbuf[k + 1] = '\0';
                        i++;
                        k++;
                    }

                    i--;

                    num++;

                    if (k) {
                        piece_seq[num - 1] = (strtol(rpt_count_strbuf, NULL, 10) & 1023);
                    }
                    else {
                        piece_seq[num - 1] = 1;
                    }

                    rpt_count = 0;
                    continue;
                }
                else
                    continue;
            }

            if (rpt_count) {
                num++;

                if (i < seq_bstr->slen - 1) {
                    if (c == 'I' && seq_bstr->data[i + 1] == 'N' && seq_bstr->data[i + 2] == 'F') {
                        piece_seq[num - 1] = SEQUENCE_REPEAT_INF;
                        goto end_sequence_proc;
                    }
                    else {
                        piece_seq[num - 1] = 1;
                    }
                }
                else {
                    piece_seq[num - 1] = 1;
                }

                rpt_count = 0;
                continue;
            }

            if (c == '*') {
                if (rpt) {
                    if (!rpt_start) {
                        rpt_count = 1;
                        pre_rpt_count = 0;
                        rpt = 0;
                        if (!(piece_seq[num - 1] & SEQUENCE_REPEAT_END))
                            piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                    }

                    continue;
                }
                else {
                    if (num > 1) {
                        if (!(piece_seq[num - 2] & SEQUENCE_REPEAT_END)) {
                            rpt_count = 1;
                            pre_rpt_count = 0;
                            if (!(piece_seq[num - 1] & SEQUENCE_REPEAT_END)) {
                                piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                                piece_seq[num - 1] |= SEQUENCE_REPEAT_START;
                            }
                            continue;
                        }
                        else
                            continue;
                    }
                    else if (num) {
                        piece_seq[0] |= (SEQUENCE_REPEAT_START | SEQUENCE_REPEAT_END);
                        rpt_count = 1;
                        pre_rpt_count = 0;
                        continue;
                    }
                    else
                        continue;
                }
            }

            if (c == '(') {
                if (rpt)
                    continue;

                rpt_start = 1;
                rpt = 1;
                continue;
            }

            if (c == ')') {
                if (!rpt)
                    continue;

                if (num > 0) {
                    piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                    pre_rpt_count = 1;
                }
                continue;
            }

            if (pre_rpt_count) {
                num++;
                piece_seq[num - 1] = 1;
                pre_rpt_count = 0;
                i--;
                continue;
            }

            name_str[0] = seq_bstr->data[i];

            if (seq_bstr->data[i + 1] == '4') {
                name_str[1] = '4';
                name_str[2] = '\0';

                for (j = 0; j < 25; j++) {
                    if (strcmp(name_str, get_qrspiece_name(j)) == 0) {
                        t = j;
                        if (!q->pentomino_only) {
                            goto found;
                        }
                    }
                }
            }

            name_str[1] = '\0';

            for (j = 0; j < 25; j++) {
                if (strcmp(name_str, get_qrspiece_name(j)) == 0) {
                    t = j;
                    if (q->tetromino_only) {
                        switch (t) {
                        case QRS_I:
                            t += 18;
                            break;
                        case QRS_T:
                            t += 10;
                            break;
                        case QRS_J:
                        case QRS_L:
                        case QRS_S:
                        case QRS_Z:
                            t += 19;
                            break;
                        default:
                            break;
                        }

                        if (t >= 18)
                            goto found;
                    }
                    else if (q->pentomino_only) {
                        switch (t) {
                        case QRS_I4:
                            t -= 18;
                            break;
                        case QRS_T4:
                            t -= 10;
                            break;
                        case QRS_J4:
                        case QRS_L4:
                        case QRS_S4:
                        case QRS_Z4:
                            t -= 19;
                            break;
                        default:
                            break;
                        }

                        if (t < 18)
                            goto found;
                    }
                    else {
                        goto found;
                    }
                }
            }

            if (seq_bstr->data[i + 1] == 'a') {
                name_str[1] = 'a';
                name_str[2] = '\0';

                for (j = 0; j < 25; j++) {
                    if (strcmp(name_str, get_qrspiece_name(j)) == 0) {
                        t = j;
                        if (q->tetromino_only) {
                            switch (t) {
                            case QRS_I:
                                t += 18;
                                break;
                            case QRS_T:
                                t += 10;
                                break;
                            case QRS_J:
                            case QRS_L:
                            case QRS_S:
                            case QRS_Z:
                                t += 19;
                                break;
                            default:
                                break;
                            }

                            if (t >= 18)
                                goto found;
                        }
                        else if (q->pentomino_only) {
                            switch (t) {
                            case QRS_I4:
                                t -= 18;
                                break;
                            case QRS_T4:
                                t -= 10;
                                break;
                            case QRS_J4:
                            case QRS_L4:
                            case QRS_S4:
                            case QRS_Z4:
                                t -= 19;
                                break;
                            default:
                                break;
                            }

                            if (t < 18)
                                goto found;
                        }
                        else {
                            goto found;
                        }
                    }
                }
            }
            else if (seq_bstr->data[i + 1] == 'b') {
                name_str[1] = 'b';
                name_str[2] = '\0';

                for (j = 0; j < 25; j++) {
                    if (strcmp(name_str, get_qrspiece_name(j)) == 0) {
                        t = j;
                        if (q->tetromino_only) {
                            switch (t) {
                            case QRS_I:
                                t += 18;
                                break;
                            case QRS_T:
                                t += 10;
                                break;
                            case QRS_J:
                            case QRS_L:
                            case QRS_S:
                            case QRS_Z:
                                t += 19;
                                break;
                            default:
                                break;
                            }

                            if (t >= 18)
                                goto found;
                        }
                        else if (q->pentomino_only) {
                            switch (t) {
                            case QRS_I4:
                                t -= 18;
                                break;
                            case QRS_T4:
                                t -= 10;
                                break;
                            case QRS_J4:
                            case QRS_L4:
                            case QRS_S4:
                            case QRS_Z4:
                                t -= 19;
                                break;
                            default:
                                break;
                            }

                            if (t < 18)
                                goto found;
                        }
                        else {
                            goto found;
                        }
                    }
                }
            }

            continue;
        found:
            num++;
            piece_seq[num - 1] = t;

            if (rpt_start) {
                piece_seq[num - 1] |= SEQUENCE_REPEAT_START;
                rpt_start = 0;
            }
            else if (rpt_end) {
                piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                rpt_end = 0;
            }
        }
    }

    if (rpt_count) {
        num++;
        piece_seq[num - 1] = 1;
    }

end_sequence_proc:
    for (i = 0; i < num; i++)
        d->usr_sequence[i] = piece_seq[i];

    d->usr_seq_len = num;
    d->usr_seq_expand_len = 0;

    /**/

    qrsfield_set_w(cs->p1game->field, q->field_w);
    qrsfield_set_w(q->pracdata->usr_field, q->field_w);

    for (i = 0; i < d->usr_field_undo_len; i++)
        qrsfield_set_w(q->pracdata->usr_field_undo[i], q->field_w);

    for (i = 0; i < d->usr_field_redo_len; i++)
        qrsfield_set_w(q->pracdata->usr_field_redo[i], q->field_w);

    d->field_selection = 0;

    // process randomizer seed entry...

    // q->previews are expected to be destroyed and re-allocated as needed

    if (q->previews[0])
        piecedef_destroy(q->previews[0]);
    if (q->previews[1])
        piecedef_destroy(q->previews[1]);
    if (q->previews[2])
        piecedef_destroy(q->previews[2]);

    q->previews[0] = NULL;
    q->previews[1] = NULL;
    q->previews[2] = NULL;

    if (q->pracdata->usr_seq_len) {
        q->previews[0] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(d, 0));
        q->previews[1] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(d, 1));
        q->previews[2] = qrspiece_cpy(q->piecepool, qs_get_usrseq_elem(d, 2));
    }
    else {
        q->previews[0] = qrspiece_cpy(q->piecepool, q->randomizer->lookahead(q->randomizer, 1));
        q->previews[1] = qrspiece_cpy(q->piecepool, q->randomizer->lookahead(q->randomizer, 2));
        q->previews[2] = qrspiece_cpy(q->piecepool, q->randomizer->lookahead(q->randomizer, 3));
    }

    if (d->brackets)
        q->state_flags |= GAMESTATE_BRACKETS;
    else
        q->state_flags &= ~GAMESTATE_BRACKETS;

    if (d->invisible)
        q->state_flags |= GAMESTATE_INVISIBLE;
    else
        q->state_flags &= ~GAMESTATE_INVISIBLE;

    if (q->state_flags & GAMESTATE_BRACKETS) {
        if (q->previews[0])
            q->previews[0]->flags |= PDBRACKETS;

        if (q->previews[1])
            q->previews[1]->flags |= PDBRACKETS;

        if (q->previews[2])
            q->previews[2]->flags |= PDBRACKETS;
    }
    else {
        if (q->previews[0])
            q->previews[0]->flags &= ~PDBRACKETS;

        if (q->previews[1])
            q->previews[1]->flags &= ~PDBRACKETS;

        if (q->previews[2])
            q->previews[2]->flags &= ~PDBRACKETS;
    }

    return 0;
}

// TODO: clean this function up, especially the parser + expander, and use more established terminology
int qs_get_usrseq_elem(struct pracdata *d, int index)
{
    int *seq = d->usr_sequence;
    int expand[4000];
    int expand_count = 0;

    int rpt_start = 0;
    int rpt_end = 0;
    int rpt_len = 0;
    int rpt_count = 0;
    int rpt = 0;
    int inf_rpt_len = 0;
    int inf_start = 0;

    int inf = 0;

    int i = 0;
    int j = 0;
    int k = 0;
    int val = 0;
    int complex = 0;

    if (d->usr_seq_expand_len) {
        for (i = 0; i < d->usr_seq_expand_len; i++) {
            if (d->usr_seq_expand[i] & SEQUENCE_REPEAT_INF) {
                inf = 1;
                rpt_start = i;
                inf_rpt_len = d->usr_seq_expand_len - rpt_start;
            }
        }

        if (inf) {
            if (index < rpt_start) {
                val = d->usr_seq_expand[index] & 0b11111;
            }
            else {
                if (inf_rpt_len > 1)
                    index = (index - rpt_start) % inf_rpt_len;
                else
                    index = 0;

                val = d->usr_seq_expand[rpt_start + index] & 0b11111;
            }
        }
        else {
            if (index > d->usr_seq_expand_len - 1)
                val = USRSEQ_ELEM_OOB;
            else
                val = d->usr_seq_expand[index] & 0b11111;
        }

        return val;
    }

    if (!d->usr_seq_len) {
        d->usr_seq_expand_len = 0;
        return USRSEQ_ELEM_OOB;
    }

    for (i = 0; i < index && i < d->usr_seq_len; i++) {
        if (!IS_QRS_PIECE(seq[i]))
            complex = 1;
    }

    if (!complex && index < d->usr_seq_len) {
        if (IS_QRS_PIECE(seq[index]))
            return seq[index];
    }

    for (i = 0;; i++) {
        if (i >= d->usr_seq_len || expand_count > 3500) {
            // 3500 is an arbitrary limit to prevent taking up too much memory for little benefit
            break;
        }

        val = seq[i];
        if (rpt) {
            /* rpt = 1 implies we encountered the beginning of a grouped subsequence, where groups are assumed
            to have a rep count specified at the end */
            if (val & SEQUENCE_REPEAT_END || i == d->usr_seq_len - 1) {
                rpt = 0;
                rpt_end = i;
                rpt_len = rpt_end - rpt_start + 1; // length of grouped subsequence
                i++; // next element of the sequence is either a repetition count or beyond the end of the sequence

                if (i != d->usr_seq_len && seq[i] == SEQUENCE_REPEAT_INF) {
                    // ^^ bound check must come first to prevent seq[i] from creating UB
                    inf = 1;
                    inf_rpt_len = rpt_len;
                    inf_start = rpt_start;
                    expand_count += rpt_len;
                    for (k = 0; k < rpt_len; k++)
                    {
                        /* seq[blah] & 0b11111 zeroes other flags on the piece;
                        first repeated element gets repeat_inf flag;
                        expanded sequence is cut off after the infinitely repeating group and this expansion loop ends */

                        expand[expand_count - rpt_len + k] = (seq[rpt_start + k] & 0b11111) | (!k ? SEQUENCE_REPEAT_INF : 0);
                    }

                    break;
                }
                else {
                    // dealing with finite repetition or open-ended grouped subsequence

                    if (i == d->usr_seq_len) rpt_count = 1;
                    // ^^ force open-ended group to 1 repetition (same as if there was no grouping)

                    else {
                        if (seq[i] > USRSEQ_RPTCOUNT_MAX)   // last-minute bounding of repetition count to something reasonable
                            rpt_count = USRSEQ_RPTCOUNT_MAX;
                        else
                            rpt_count = seq[i];
                    }

                    for (k = 0; k < rpt_count; k++) {    // expand entire repetition to sequence of pieces with no special flags
                        for (j = 0; j < rpt_len; j++) {
                            expand[expand_count + k * rpt_len + j] = seq[rpt_start + j] & 0b11111;
                        }
                    }

                    expand_count += rpt_count * rpt_len;
                    rpt = 0;    // we are done with this grouped subsequence
                }
            }
        }
        else {
            if (val & SEQUENCE_REPEAT_START) {
                rpt_start = i;
                rpt = 1;
                if (val & SEQUENCE_REPEAT_END || i == d->usr_seq_len - 1) {
                    i--;        // hacky way for the loop to go into the if(rpt) branch and handle these edge cases
                    continue;   // the i-- ensures the loop reads the additional flags on this element/doesn't go OOB
                }
            }
            else {    // no expansion to be done
                expand_count++;
                expand[expand_count - 1] = seq[i] & 0b11111;
            }
        }
    }

    // end of expansion loop

    if (inf) {
        if (index < inf_start) { // index is of the piece we want to return to the function caller
            val = expand[index] & 0b11111;
        }
        else {
            if (inf_rpt_len > 1) {
                // collapse an arbitrarily large index value to a relative one that fits in the array bounds
                index = (index - inf_start) % inf_rpt_len;
            }
            else
                index = 0;

            val = expand[inf_start + index] & 0b11111;
        }
    }
    else {
        if (index < expand_count)
            val = expand[index] & 0b11111;
        else
            val = USRSEQ_ELEM_OOB;
    }

    // printf("Expanded sequence:");
    // for(i = 0; i < expand_count; i++) { printf(" %d", expand[i]); }
    // printf("\n");



    // updating the pracdata's expanded sequence field so we don't have to waste time expanding it later unless it changes
    for (i = 0; i < expand_count; i++)
        d->usr_seq_expand[i] = expand[i];

    d->usr_seq_expand_len = expand_count;


    return val;
}

// return value: 0 success, -1 invalid argument(s), 1 no next piece to deal to the player
int qs_initnext(game_t *g, qrs_player *p, unsigned int flags)
{
    if (!g || !p)
        return -1;

    coreState *cs = g->origin;
    qrsdata *q = (qrsdata *)(g->data);
    struct randomizer *qrand = q->randomizer;

    int i = 0;
    int j = 0;
    piece_id t = 0;
    int rc = 0;
    int cell = 0;

    q->lock_on_rotate = 0;
    q->p1counters->floorkicks = 0;
    q->soft_drop_counter = 0;
    q->sonic_drop_height = 0;
    q->active_piece_time = 0;
    q->placement_speed = 0;

    if (flags & INITNEXT_DURING_ACTIVE_PLAY)
    {
        // special check so we don't tell the game to terminate, even if there is no new piece to deal out
        // we don't want the game to terminate while the player is controlling a piece
        // edge case where this is relevant: player uses hold with no held piece, at last piece in user seq

        if (q->pracdata && q->pracdata->usr_seq_len)
        {
            if (qs_get_usrseq_elem(q->pracdata, q->pracdata->hist_index + 1) == USRSEQ_ELEM_OOB)
            {
                return 1;
            }
        }
    }

    if (q->using_gems) {
        bool gems_in_field = false;
        for (i = 0; i < g->field->w; i++) {
            for (j = 0; j < g->field->h; j++) {
                cell = gridgetcell(g->field, i, j);
                if (cell < 0 || cell == GRID_OOB)
                    continue;

                if (cell & QRS_PIECE_GEM)
                    gems_in_field = true;
            }
        }

        if (gems_in_field == false) {
            printf("No gems left, terminating.\n");
            return QSGAME_SHOULD_TERMINATE;
        }
    }

    if (q->pracdata && q->pracdata->usr_seq_len) {
        q->pracdata->hist_index++;
        rc = qs_get_usrseq_elem(q->pracdata, q->pracdata->hist_index);

        if (rc != USRSEQ_ELEM_OOB)
            t = (piece_id)(rc & 0xff);
        else
            t = PIECE_ID_INVALID;
    }
    else {
        t = qrand->pull(qrand);
        if (q->randomizer->num_pieces == 7)
            t = ars_to_qrs_id(t);
    }

    if (p->def) piecedef_destroy(p->def);
    p->def = q->previews[0];

    if (p->def)
        q->cur_piece_qrs_id = p->def->qrs_id;
    else
        q->cur_piece_qrs_id = PIECE_ID_INVALID;

    q->previews[0] = q->previews[1];
    q->previews[1] = q->previews[2];
    q->previews[2] = qrspiece_cpy(q->piecepool, t);
    //printf("New piecedef to deal out: %lx with ID %d\n", q->previews[2], t);

    if (q->state_flags & GAMESTATE_BRACKETS) {
        if (q->previews[2]) q->previews[2]->flags |= PDBRACKETS;
    }

    if (q->previews[0])
    {
        t = q->previews[0]->qrs_id;

        if (t != PIECE_ID_INVALID)
        {
            int ts = t;
            if (ts >= 18)
                ts -= 18;
            struct sfx *sfx = &cs->assets->piece0 + (ts % 7);
            sfx_play(sfx);
        }
    }

    if (q->cur_piece_qrs_id == PIECE_ID_INVALID || !p->def) {
        // pause the game if we can't deal a new piece
        return QSGAME_SHOULD_TERMINATE;
    }

    if (q->cur_piece_qrs_id >= 18)
        p->y = ROWTOY(SPAWNY_QRS + 2);
    else
        p->y = ROWTOY(SPAWNY_QRS);

    p->x = SPAWNX_QRS;
    p->orient = FLAT;
    p->state = PSFALL | PSSPAWN;

    q->p1counters->lock = 0;

    return 0;
}
