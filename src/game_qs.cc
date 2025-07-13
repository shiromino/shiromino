#include "game_qs.h"
#include "SDL_mixer.h"
#include <string.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include "CoreState.h"
#include "Credits.h"
#include "Debug.h"
#include "DisplayMode.h"
#include "GameType.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "QRS0.h"
#include "Records.h"
#include "RefreshRates.h"
#include "Timer.h"
#include "asset/Font.h"
#include "asset/Image.h"
#include "asset/Music.h"
#include "asset/Sfx.h"
#include "game_menu.h"
#include "gfx_old.h"
#include "gfx_qs.h"
#include "gfx_structures.h"
#include "input/KeyFlags.h"
#include "menu/ElementType.h"
#include "menu/Option.h"
#include "menu/TextOption.h"
#include "random.h"
#include "replay.h"
#include "video/Background.h"
#include "video/Gfx.h"
#include "video/MessageEntity.h"
using namespace Shiro;
using namespace std;

// clang-format off
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

static int qs_grade_score_reqs[17] =
{
    1000,
    1400,
    2000,
    3000,
    5000,
    8000,
    13000,
    19000,

    26000, // S1
    33000,
    41000,
    50000, // S4 and score requirement at 500
    60000,
    72000,
    87000,
    114000,
    132000 // S9
};

static QRS_Timings qs_curve[QS_CURVE_MAX] =
{
    {0u, 4, 40, 12, 25, 25, 12},
    {100u, 16, 40, 12, 25, 25, 12},
    {120u, 32, 40, 12, 25, 25, 12},
    {140u, 48, 40, 12, 25, 25, 12},
    {160u, 64, 40, 12, 25, 25, 12},
    {180u, 96, 40, 12, 25, 25, 12},
    {200u, 128, 45, 12, 25, 25, 12},
    {250u, 192, 45, 12, 25, 25, 12},
    {300u, 256, 45, 12, 25, 25, 12},
    {400u, 512, 45, 12, 25, 25, 12},
    {450u, 768, 45, 12, 25, 25, 12},
    {480u, 5 * 256, 45, 12, 25, 25, 12},

    {500u, 20 * 256, 40, 10, 20, 16, 12},
    {600u, 20 * 256, 36, 10, 18, 16, 8},
    {700u, 20 * 256, 32, 8, 16, 16, 6},
    {800u, 20 * 256, 30, 8, 12, 12, 6},
    {900u, 20 * 256, 27, 8, 12, 12, 6},
    {1000u, 20 * 256, 22, 7, 10, 10, 4},
    {1100u, 20 * 256, 20, 7, 8, 8, 4},
    {1700u, 20 * 256, 17, 6, 6, 6, 4}
};

static QRS_Timings g1_master_curve[G1_MASTER_CURVE_MAX] =
{
    {0u, 4, 30, 14, 30, 30, 41},
    {30u, 6, 30, 14, 30, 30, 41},
    {35u, 8, 30, 14, 30, 30, 41},
    {40u, 10, 30, 14, 30, 30, 41},
    {50u, 12, 30, 14, 30, 30, 41},
    {60u, 16, 30, 14, 30, 30, 41},
    {70u, 32, 30, 14, 30, 30, 41},
    {80u, 48, 30, 14, 30, 30, 41},
    {90u, 64, 30, 14, 30, 30, 41},
    {100u, 80, 30, 14, 30, 30, 41},
    {120u, 96, 30, 14, 30, 30, 41},
    {140u, 112, 30, 14, 30, 30, 41},
    {160u, 128, 30, 14, 30, 30, 41},
    {170u, 144, 30, 14, 30, 30, 41},
    {200u, 4, 30, 14, 30, 30, 41},
    {220u, 32, 30, 14, 30, 30, 41},
    {230u, 64, 30, 14, 30, 30, 41},
    {233u, 96, 30, 14, 30, 30, 41},
    {236u, 128, 30, 14, 30, 30, 41},
    {239u, 160, 30, 14, 30, 30, 41},
    {243u, 192, 30, 14, 30, 30, 41},
    {247u, 224, 30, 14, 30, 30, 41},
    {251u, 256, 30, 14, 30, 30, 41},
    {300u, 512, 30, 14, 30, 30, 41},
    {330u, 768, 30, 14, 30, 30, 41},
    {360u, 1024, 30, 14, 30, 30, 41},
    {400u, 1280, 30, 14, 30, 30, 41},
    {420u, 1024, 30, 14, 30, 30, 41},
    {450u, 768, 30, 14, 30, 30, 41},
    {500u, 5120, 30, 14, 30, 30, 41}
};

static QRS_Timings g2_master_curve[G2_MASTER_CURVE_MAX] =
{
    {0u, 4, 30, 14, 25, 25, 40},
    {30u, 6, 30, 14, 25, 25, 40},
    {35u, 8, 30, 14, 25, 25, 40},
    {40u, 10, 30, 14, 25, 25, 40},
    {50u, 12, 30, 14, 25, 25, 40},
    {60u, 16, 30, 14, 25, 25, 40},
    {70u, 32, 30, 14, 25, 25, 40},
    {80u, 48, 30, 14, 25, 25, 40},
    {90u, 64, 30, 14, 25, 25, 40},
    {100u, 80, 30, 14, 25, 25, 40},
    {120u, 96, 30, 14, 25, 25, 40},
    {140u, 112, 30, 14, 25, 25, 40},
    {160u, 128, 30, 14, 25, 25, 40},
    {170u, 144, 30, 14, 25, 25, 40},
    {200u, 4, 30, 14, 25, 25, 40},
    {220u, 32, 30, 14, 25, 25, 40},
    {230u, 64, 30, 14, 25, 25, 40},
    {233u, 96, 30, 14, 25, 25, 40},
    {236u, 128, 30, 14, 25, 25, 40},
    {239u, 160, 30, 14, 25, 25, 40},
    {243u, 192, 30, 14, 25, 25, 40},
    {247u, 224, 30, 14, 25, 25, 40},
    {251u, 256, 30, 14, 25, 25, 40},
    {300u, 512, 30, 14, 25, 25, 40},
    {330u, 768, 30, 14, 25, 25, 40},
    {360u, 1024, 30, 14, 25, 25, 40},
    {400u, 1280, 30, 14, 25, 25, 40},
    {420u, 1024, 30, 14, 25, 25, 40},
    {450u, 768, 30, 14, 25, 25, 40},
    {500u, 5120, 30, 8, 25, 25, 25},
    {601u, 5120, 30, 8, 25, 16, 16},
    {701u, 5120, 30, 8, 16, 12, 12},
    {801u, 5120, 30, 8, 12, 6, 6},
    {900u, 5120, 30, 6, 12, 6, 6},
    {901u, 5120, 17, 6, 12, 6, 6}
};

static QRS_Timings g2_death_curve[G2_DEATH_CURVE_MAX] =
{
    {0u, 5120, 30, 10, 16, 12, 12},
    {101u, 5120, 26, 10, 12, 6, 6},
    {200u, 5120, 26, 9, 12, 6, 6},
    {201u, 5120, 22, 9, 12, 6, 6},
    {300u, 5120, 22, 8, 12, 6, 6},
    {301u, 5120, 18, 8, 6, 6, 6},
    {400u, 5120, 18, 6, 6, 6, 6},
    {401u, 5120, 15, 6, 5, 5, 5},
    {500u, 5120, 15, 6, 4, 4, 4}
};

static QRS_Timings g3_terror_curve[G3_TERROR_CURVE_MAX] =
{
    {0u, 5120, 18, 8, 10, 6, 6},
    {100u, 5120, 18, 6, 10, 5, 5},
    {200u, 5120, 17, 6, 10, 4, 4},
    {300u, 5120, 15, 6, 4, 4, 4},
    {500u, 5120, 13, 4, 4, 3, 3},
    {600u, 5120, 12, 4, 4, 3, 3},
    {1100u, 5120, 10, 4, 4, 3, 3},
    {1200u, 5120, 8, 4, 4, 3, 3},
    {1300u, 5120, 15, 4, 4, 4, 6}
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
    {1.0f, 1.0f, 1.0f, 1.0f},
    {1.2f, 1.4f, 1.5f, 1.0f},
    {1.2f, 1.5f, 1.8f, 1.0f},
    {1.4f, 1.6f, 2.0f, 1.0f},
    {1.4f, 1.7f, 2.2f, 1.0f},
    {1.4f, 1.8f, 2.3f, 1.0f},
    {1.4f, 1.9f, 2.4f, 1.0f},
    {1.5f, 2.0f, 2.5f, 1.0f},
    {1.5f, 2.1f, 2.6f, 1.0f},
    {2.0f, 2.5f, 3.0f, 1.0f}
};
// clang-format on

const char *get_grade_name(int grade)
{
    grade &= 0xff;
    if(grade > 36)
        return " --";

    else
        return grade_names[grade];
}

const char *get_internal_grade_name(int index)
{
    if(index < 0 || index > 31)
        return "N/A";

    else
        return internal_grade_names[index];
}

int internal_to_displayed_grade(int internal_grade)
{
    if(internal_grade < INTERNAL_GRADE_9)
        return -1;

    else
        switch(internal_grade)
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

// clang-format off
static const struct levelmusic pentomino_music[] = {
    { 0, 0 },
    { 490, -1 },
    { 500, 1 },
    { 690, -1 },
    { 700, 2 },
    { 980, -1 },
    { 1000, 3 },
    { QS_LEVEL_CREDITS, 4 },
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
    { QS_LEVEL_CREDITS, 4 },
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
    { QS_LEVEL_CREDITS, 6 },
    { 9999, -1 }
};

static const struct levelmusic g1_music[] = {
    { 0, 0 },
    { 485, -1 },
    { 500, 1 },
    { QS_LEVEL_CREDITS, 2 },
    { 9999, -1 }
};
// clang-format on

static int find_music(int level, const struct levelmusic *table)
{
    int music = -1;
    for(int i = 0; table[i].fromlevel <= level; ++i)
        music = table[i].musicindex;
    return music;
}

static void play_or_halt_music(qrsdata *q, CoreState *cs, const std::filesystem::path& tracks, int desired_music)
{
    if(q->music == desired_music)
        return;

    q->music = desired_music;
    std::cerr << "Music: " << q->music << std::endl;
    if(desired_music == -1)
        Mix_HaltMusic();
    else
        MusicAsset::get(cs->assetMgr, tracks, desired_music).play(cs->settings);
}

static void update_music(qrsdata *q, CoreState *cs)
{
    switch(q->mode_type)
    {
        case MODE_PENTOMINO:
            play_or_halt_music(q, cs, "tracks", find_music(q->level, pentomino_music));
            break;

        case MODE_G2_MASTER:
            play_or_halt_music(q, cs, "g2_tracks", find_music(q->level, g2_master_music));
            break;

        case MODE_G2_DEATH:
            play_or_halt_music(q, cs, "g2_tracks", find_music(q->level, g2_death_music));
            break;

        case MODE_G3_TERROR:
            play_or_halt_music(q, cs, "g3_tracks", find_music(q->level, g3_terror_music));
            break;

        case MODE_G1_MASTER:
        case MODE_G1_20G:
            play_or_halt_music(q, cs, "g1_tracks", find_music(q->level, g1_music));
            break;

        default:
            log_warn("qrsdata->mode_type improperly set to %d\n", q->mode_type);
            break;
    }
}

double pentomino_c_difficulty(int level)
{
    double difficulty = 0;

    if(level >= 1700)
    {
        // difficulty hits 230.0 at level 1800 exactly: 1.0472327 = 101.0 ^ (1/100)
        double amount = pow(1.0472327, level - 1700);
        difficulty = 129.0 + (amount > 101.0 ? 101.0 : amount);
    }
    else if(level >= 1500)
    {
        // difficulty hits 130.0 at level 1700 exactly: 1.0222154 = 81.0 ^ (1/200)
        double amount = pow(1.0222154, level - 1500);
        difficulty = 49.0 + (amount > 81.0 ? 81.0 : amount);
    }
    else if(level >= 1000)
    {
        // difficulty jumps to 30.0
        // difficulty hits 50.0 at level 1160 exactly: 1.0192105 = 21.0 ^ (1/160)
        double amount = pow(1.0192105, level - 1000);
        difficulty = 29.0 + (amount > 21.0 ? 21.0 : amount);
    }
    else if(level >= 900)
    {
        // difficulty hits 21.0 at level 1000 exactly: 1.0139595 = 4.0 ^ (1/100)
        double amount = pow(1.0139595, level - 900);
        difficulty = 17.0 + (amount > 4.0 ? 4.0 : amount);
    }
    else if(level >= 700)
    {
        // difficulty hits 18.0 at level 900 exactly: 1.0069556 = 4.0 ^ (1/200)
        double amount = pow(1.0069556, level - 700);
        difficulty = 14.0 + (amount > 4.0 ? 4.0 : amount);
    }
    else if(level >= 500)
    {
        // difficulty drops to 10.0
        // difficulty hits 15.0 at level 700 exactly: 1.0089990 = 6.0 ^ (1/200)
        double amount = pow(1.0089990, level - 500);
        difficulty = 9.0 + (amount > 6.0 ? 6.0 : amount);
    }
    else if(level >= 300)
    {
        // difficulty hits 30.0 at level 480 exactly: 1.0134108 = 11.0 ^ (1/180)
        double amount = pow(1.0134108, level - 300);
        difficulty = 19.0 + (amount > 11.0 ? 11.0 : amount);
    }
    else
    {
        // difficulty starts at 15.0
        // difficulty hits 20.0 at level 300 exactly: 1.0059904 = 6.0 ^ (1/300)
        double amount = pow(1.0059904, level);
        difficulty = 14.0 + (amount > 6.0 ? 6.0 : amount);
    }

    return difficulty;
}

game_t *qs_game_create(CoreState *cs, int level, unsigned int flags, int replay_id)
{
    game_t *g = (game_t *)malloc(sizeof(game_t));
    qrsdata *q = NULL;
    qrs_player *p = NULL;

    g->origin = cs;
    g->field = new Shiro::Grid(QRS_FIELD_W, QRS_FIELD_H);

    g->init = qs_game_init;
    g->quit = qs_game_quit;
    g->preframe = qs_game_preframe;
    g->input = &qrs_input;
    g->frame = qs_game_frame;
    g->draw = gfx_drawqs;

    g->frame_counter = 0;

    g->data = new qrsdata;
    q = (qrsdata *)(g->data);

    /*
    const auto executableDirectory = OS::getExecutablePath().remove_filename();
    const std::vector<std::optional<fs::path>> configurationPrefixes = {
        executableDirectory,
        std::filesystem::weakly_canonical(executableDirectory / ".." / "etc")
    };
    for (const auto &prefix : configurationPrefixes) {
        if (!prefix) {
            continue;
        }
        const auto candidate = prefix.value() / "credits.txt";
        if (std::filesystem::exists(candidate)) {
            q->credits.open(candidate.string());
            break;
        }
    }

    if(q->credits.is_open())
    {
        q->num_credits_lines = 1 + std::count(std::istreambuf_iterator<char>(q->credits), std::istreambuf_iterator<char>(), '\n');
        q->credits_tex = gfx_create_credits_tex(cs, q->credits, q->num_credits_lines);
        q->credits_tex_height = 16 * q->num_credits_lines;
        q->credits.close();
    }
    else
    {
        q->credits_tex = nullptr;
        q->credits_tex_height = 0;
    }
    */

    q->num_credits_lines = 1 + std::count(Shiro::Credits::creditsString.begin(), Shiro::Credits::creditsString.end(), '\n');
    q->credits_tex = gfx_create_credits_tex(cs, q->num_credits_lines);
    q->credits_tex_height = 16 * q->num_credits_lines;

    q->mode_flags = flags;

    q->piecepool = qrspool_create();
    q->timer = Shiro::Timer(60.0);
    q->p1 = (qrs_player *)malloc(sizeof(qrs_player));
    p = q->p1;
    p->def = NULL;
    p->speeds = NULL;
    p->state = PSINACTIVE;
    p->x = 0;
    p->y = 0;
    p->num_olds = 10;
    p->old_xs = (int *)malloc(10 * sizeof(int));
    p->old_ys = (int *)malloc(10 * sizeof(int));
    for(int i = 0; i < 10; i++)
    {
        p->old_xs[i] = 0;
        p->old_ys[i] = 0;
    }
    p->orient = Shiro::FLAT;

    q->p1counters = new QRS_Counters;

    q->randomizer = NULL;

    if(replay_id >= 0)
    {
        qrs_load_replay(g, replay_id);
        if(!q->replay)
        {
            free(q->p1);
            delete q->p1counters;
            delete q;
            delete g->field;
            free(g);

            return NULL;
        }

        flags = q->replay->mode_flags;
        level = q->replay->starting_level;
    }
    else
    {
        q->replay = NULL;
    }

    q->recording = 0;
    q->playback = 0;
    q->playback_index = 0;

    q->is_practice = 0;

    // q->field_tex = SDL_CreateTexture(cs->screen.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 12*16, 20*16);
    // q->previous_field.clear();
    // q->field_deltas = Grid(QRS_FIELD_W, QRS_FIELD_H);

    q->garbage_row_index = 0;
    q->garbage_counter = 0;
    q->garbage_delay = 0;

    q->previews.clear();
    q->hold = NULL;

    q->field_x = QRS_FIELD_X;
    q->field_y = QRS_FIELD_Y;
    q->field_w = 12;

    if((cs->displayMode == Shiro::DisplayMode::CENTERED) && !(flags & QRS_PRACTICE))
    {
        q->field_x = 192;
    }

    q->randomizer_type = RANDOMIZER_NORMAL;
    q->tetromino_only = 0;
    q->pentomino_only = 0;
    q->lock_delay_enabled = 1;
    q->lock_protect = 1;
    q->num_previews = 4;
    q->hold_enabled = 0;
    q->special_irs = 1;
    q->using_gems = false;

    q->piece_fade_rate = 300;
    q->stack_anim_counter = 0;
    // default credit roll is 60 seconds
    q->credit_roll_length = q->credit_roll_counter = 60 * 60;
    q->credit_roll_lineclears = 0;

    q->cleared_game = false;
    q->topped_out = false;

    q->state_flags = 0;

    q->max_floorkicks = 2;
    q->lock_on_rotate = 0;

    cs->request_fps(Shiro::RefreshRates::pentomino);
    q->game_type = Shiro::GameType::SIMULATE_QRS;
    q->mode_type = MODE_PENTOMINO;

    q->last_gradeup_timestamp = 0xFFFFFFFF;
    q->grade = GRADE_9;
    q->internal_grade = 0;
    q->grade_points = 0;
    q->grade_decay_counter = 0;

    q->mroll_unlocked = true;
    q->cur_section_timestamp = 0;

    for(int i = 0; i < MAX_SECTIONS; i++)
    {
        q->section_times[i] = -1;
        q->section_tetrises[i] = 0;
        q->best_section_times[i] = -1;
    }

    if(flags & MODE_G2_DEATH)
    {
        q->mode_type = MODE_G2_DEATH;
        q->grade = NO_GRADE;
        // q->field_x = QRS_FIELD_X - 28;
        // q->field_y = QRS_FIELD_Y - 16 + 2;
        q->lock_protect = 1;
        flags |= static_cast<int>(Shiro::GameType::SIMULATE_G2);
        flags |= TETROMINO_ONLY;
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G1);
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G3);

        // 61.68 "game seconds", or 60 realtime seconds
        q->credit_roll_length = q->credit_roll_counter = 3701;
    }
    else if(flags & MODE_G3_TERROR)
    {
        q->mode_type = MODE_G3_TERROR;
        q->grade = NO_GRADE;
        q->lock_protect = 1;
        flags |= static_cast<int>(Shiro::GameType::SIMULATE_G3);
        flags |= TETROMINO_ONLY;
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G1);
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G2);
        if(level < 1000 && level >= 500)
        {
            q->state_flags |= GAMESTATE_RISING_GARBAGE;
        }
        else if(level >= 1000)
        {
            q->state_flags |= GAMESTATE_BRACKETS;
        }

        q->credit_roll_length = q->credit_roll_counter = 54 * 60;
    }
    else if(flags & MODE_G1_MASTER)
    {
        q->mode_type = MODE_G1_MASTER;
        q->grade = GRADE_9;
        flags |= static_cast<int>(Shiro::GameType::SIMULATE_G1);
        flags |= TETROMINO_ONLY;
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G2);
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G3);
    }
    else if(flags & MODE_G1_20G)
    {
        q->mode_type = MODE_G1_20G;
        q->grade = GRADE_9;
        flags |= static_cast<int>(Shiro::GameType::SIMULATE_G1);
        flags |= TETROMINO_ONLY;
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G2);
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G3);
    }
    else if(flags & MODE_G2_MASTER)
    {
        q->mode_type = MODE_G2_MASTER;
        q->grade = GRADE_9;
        flags |= static_cast<int>(Shiro::GameType::SIMULATE_G2);
        flags |= TETROMINO_ONLY;
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G1);
        flags &= ~static_cast<int>(Shiro::GameType::SIMULATE_G3);

        // 61.68 "game seconds", or 60 realtime seconds
        q->credit_roll_length = q->credit_roll_counter = 3701;
    }

    if(flags & NIGHTMARE_MODE)
        q->randomizer_type = RANDOMIZER_NIGHTMARE;
    if(flags & NO_LOCK_DELAY)
        q->lock_delay_enabled = 0;
    if(flags & PENTOMINO_ONLY)
        q->pentomino_only = 1;

    if(flags & TETROMINO_ONLY)
    {
        q->field_w = 10;
        qrsfield_set_w(g->field, 10);
        q->tetromino_only = 1;
    }

    if(flags & static_cast<int>(Shiro::GameType::SIMULATE_G1))
    {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G1;
        q->game_type = Shiro::GameType::SIMULATE_G1;
        q->num_previews = 1;
        q->max_floorkicks = 0;
        q->special_irs = 0;
        q->lock_protect = 0;
        q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags | Shiro::PDNOWKICK);
        cs->request_fps(Shiro::RefreshRates::g1);
    }

    if(flags & static_cast<int>(Shiro::GameType::SIMULATE_G2))
    {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G2;
        q->game_type = Shiro::GameType::SIMULATE_G2;
        q->num_previews = 1;
        q->max_floorkicks = 0;
        q->special_irs = 0;
        q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags | Shiro::PDNOWKICK);
        cs->request_fps(Shiro::RefreshRates::g2);
    }

    if(flags & static_cast<int>(Shiro::GameType::SIMULATE_G3))
    {
        q->tetromino_only = 1;
        q->randomizer_type = RANDOMIZER_G3;
        q->game_type = Shiro::GameType::SIMULATE_G3;
        q->num_previews = 3;
        q->max_floorkicks = 1;
        q->special_irs = 0;
        q->lock_protect = 1;
        q->hold_enabled = 1;
    }

    switch(q->randomizer_type)
    {
        case RANDOMIZER_NORMAL:
            q->randomizer = pento_randomizer_create();
            break;

        case RANDOMIZER_NIGHTMARE:
            q->randomizer = pento_randomizer_create();
            break;

        case RANDOMIZER_G1:
            q->randomizer = g1_randomizer_create();
            break;

        case RANDOMIZER_G2:
            q->randomizer = g2_randomizer_create();
            break;

        case RANDOMIZER_G3:
            q->randomizer = g2_randomizer_create();
            break;

        default:
            q->randomizer = pento_randomizer_create();
            break;
    }

    q->randomizer_seed = 0;

    q->rotations = 0;
    q->pieces_dealt = 0;
    q->bravos = 0;

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
    q->last_medal_ac_timestamp = 0xFFFFFFFF;
    q->last_medal_ro_timestamp = 0xFFFFFFFF;
    q->medal_ac = 0;
    q->medal_sk = 0;
    q->medal_ro = 0;
    q->medal_st = 0;
    q->medal_re = 0;
    q->medal_co = 0;

    q->speed_curve_index = 0;
    q->music = -1;

    /* for testing */
    // flags |= BIG_MODE;

    if(flags & static_cast<int>(Shiro::GameType::BIG_MODE) && !(flags & QRS_PRACTICE))
    {
        q->state_flags |= GAMESTATE_BIGMODE;

        int w = q->field_w / 2;

        g->field->fill(0);
        for(int i = 0; i <= w; i++)
        {
            g->field->cell(i, QRS_FIELD_H - 10) = QRS_WALL;
        }

        for(int j = QRS_FIELD_H - 10; j >= 0; j--)
        {
            g->field->cell(w, j) = QRS_WALL;
        }

        q->field_w = w;
    }

    if(flags & QRS_PRACTICE)
    {
        q->is_practice = 1;

        q->mode_type = -1;

        if(!cs->pracdata_mirror)
        {
            q->pracdata = new pracdata;

            q->pracdata->field_w = 10;
            q->pracdata->game_type = Shiro::GameType::SIMULATE_G2;
            // q->pracdata->long_history = NULL;   // unused at the moment
            q->pracdata->usr_seq_expand_len = 0;
            q->pracdata->usr_seq_len = 0;
            q->pracdata->usr_field_undo.clear();
            q->pracdata->usr_field_redo.clear();
            q->pracdata->field_edit_in_progress = 0;
            q->pracdata->usr_field = Shiro::Grid(QRS_FIELD_W, QRS_FIELD_H);
            q->pracdata->palette_selection = -5;
            q->pracdata->field_selection = 0;
            q->pracdata->field_selection_vertex1_x = 0;
            q->pracdata->field_selection_vertex1_y = 0;
            q->pracdata->field_selection_vertex2_x = 0;
            q->pracdata->field_selection_vertex2_y = 0;
            if(flags & TETROMINO_ONLY)
            {
                qrsfield_set_w(&q->pracdata->usr_field, 10);
                q->field_w = 10;
            }

            q->pracdata->usr_timings = new QRS_Timings;
            /*
            q->pracdata->usr_timings->level = 0;
            q->pracdata->usr_timings->are = 30;
            q->pracdata->usr_timings->grav = 4;
            q->pracdata->usr_timings->lock = 30;
            q->pracdata->usr_timings->das = 14;
            q->pracdata->usr_timings->lineare = 30;
            q->pracdata->usr_timings->lineclear = 40;
            */
            q->pracdata->hist_index = 0;
            q->pracdata->paused = QRS_FIELD_EDIT;
            q->pracdata->grid_lines_shown = 0;
            q->pracdata->brackets = 0;
            q->pracdata->invisible = 0;
            q->pracdata->infinite_floorkicks = 0;
            q->pracdata->lock_protect = -1;
            if(flags & TETROMINO_ONLY)
                q->pracdata->piece_subset = SUBSET_TETS;
            else if(flags & PENTOMINO_ONLY)
                q->pracdata->piece_subset = SUBSET_PENTS;
            else
                q->pracdata->piece_subset = SUBSET_ALL;

            q->pracdata->randomizer_seed = 0;

            cs->pracdata_mirror = q->pracdata;
        }
        else
        {
            q->pracdata = cs->pracdata_mirror;
            q->field_w = q->pracdata->field_w;
            q->game_type = q->pracdata->game_type;
        }
    }
    else
        q->pracdata = NULL;

    if(!level)
    {
        q->p1->speeds = &qs_curve[0];
        q->starting_level = 0;
    }
    else
    {
        q->level = q->starting_level = level;

        if(q->starting_level != 0)
        {
            q->mroll_unlocked = false;
        }

        q->section = level / 100;
        q->p1->speeds = &qs_curve[8];
    }

    if(q->mode_type == MODE_PENTOMINO)
    {
        histrand_set_difficulty(q->randomizer, pentomino_c_difficulty(q->level));
    }

    if(!(flags & QRS_PRACTICE))
    {
        for(int i = 0; i < MAX_SECTIONS; i++)
        {
            q->best_section_times[i] = scoredb_get_sectiontime(&g->origin->records, q->mode_type, i * 100);
        }
    }

    return g;
}

int qs_game_init(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)(g->data);
    qrs_player *p = q->p1;
    struct randomizer *qrand = q->randomizer;

    piece_id next1_id = 0u, next2_id = 0u, next3_id = 0u, next4_id = 0u;
    //int rc = 0;

    /*SDL_SetRenderTarget(g->origin->screen.renderer, q->field_tex);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(g->origin->screen.renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(g->origin->screen.renderer);
    SDL_SetRenderDrawBlendMode(g->origin->screen.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 255);
    SDL_SetRenderTarget(g->origin->screen.renderer, NULL);*/

    // gfx_createbutton(g->origin, "TEST", 31*16 - 6, 16 - 6, 0, toggle_obnoxious_text, NULL, NULL, RGBA_DEFAULT);

    /*int i = 0;
    int j = 0;
    int t = 0;*/

    if(q->replay)
    {
        *(qrand->seedp) = q->replay->seed;
        q->randomizer_seed = q->replay->seed;
    }
    else
    {
        q->randomizer_seed = *(qrand->seedp);
    }

    std::cerr << "Random seed: " << q->randomizer_seed << std::endl;

    if(qrand)
    {
        qrand->init(qrand, NULL);

        next1_id = qrand->pull(qrand);
        next2_id = qrand->pull(qrand);
        next3_id = qrand->pull(qrand);
        next4_id = qrand->pull(qrand);

        if(qrand->num_pieces == 7)
        {
            next1_id = ars_to_qrs_id(next1_id);
            next2_id = ars_to_qrs_id(next2_id);
            next3_id = ars_to_qrs_id(next3_id);
            next4_id = ars_to_qrs_id(next4_id);
        }
    }

    q->previews.clear();
    if(q->pracdata && q->pracdata->usr_seq_len)
    {
        for (size_t i = 0; i < 4; i++) {
            int elem = qs_get_usrseq_elem(q->pracdata, i);
            if (elem != USRSEQ_ELEM_OOB) {
                q->previews.push_back(q->piecepool[i]);
            }
        }

        q->pracdata->hist_index = 3;
    }
    else
    {
        q->previews.push_back(q->piecepool[next1_id]);
        q->previews.push_back(q->piecepool[next2_id]);
        q->previews.push_back(q->piecepool[next3_id]);
        q->previews.push_back(q->piecepool[next4_id]);
    }

    if(q->state_flags & GAMESTATE_BRACKETS)
    {
        for (auto& preview : q->previews) {
            preview.flags = static_cast<Shiro::PieceDefinitionFlag>(preview.flags | Shiro::PDBRACKETS);
        }
    }

    if(q->cur_piece_qrs_id >= 18)
    {
        p->y = ROWTOY(SPAWNY_QRS + 4);
    }
    else
    {
        p->y = ROWTOY(SPAWNY_QRS);
    }

    p->def = NULL;
    p->x = SPAWNX_QRS;
    for(int i = 0; i < p->num_olds; i++)
    {
        p->old_xs[i] = p->x;
        p->old_ys[i] = p->y;
    }

    q->p1->state = PSFALL;

    if(!ImageAsset::get(g->origin->assetMgr, "bg", 0).loaded()) // used to check if we are in a testing environment
        return 0;

    int bgnumber = q->section;
    if(bgnumber > 12)
        bgnumber = 12;

    if(!q->pracdata)
    {
        g->origin->bg.transition(ImageAsset::get(g->origin->assetMgr, "bg", bgnumber));

        if(q->mode_type == MODE_G2_DEATH)
        {
            // g->origin->anim_bg = g->origin->g2_bgs[q->section > 9 ? 9 : q->section];
            // g->origin->anim_bg_old = g->origin->anim_bg;
        }
    }
    else
    {
        q->p1->state = PSINACTIVE;
    }

    return 0;
}

int qs_game_pracinit(game_t *g, int)
{
    CoreState *cs = g->origin;
    g = cs->p1game;
    qrsdata *q = (qrsdata *)g->data;
    qrs_player *p = q->p1;
    QRS_Counters *c = q->p1counters;

    struct randomizer *qrand = q->randomizer;
    piece_id next1_id, next2_id, next3_id, next4_id;

    int cell = 0;

    cs->menu_input_override = 0;

    q->randomizer_seed = *(qrand->seedp);
    q->using_gems = false;

    //q->pracdata->usr_field = *g->field;
    *g->field = q->pracdata->usr_field;

    if(!q->pracdata->usr_field_locked && q->pracdata->usr_field_fumen)
    {
        undo_clear_confirm_yes(cs, nullptr);
    }

    for(size_t i = 0; i < g->field->getWidth(); i++)
    {
        for(size_t j = 0; j < g->field->getHeight(); j++)
        {
            cell = g->field->getCell(i, j);
            if(cell < 0 || cell == GRID_OOB)
                continue;

            if(cell & QRS_PIECE_GEM)
                q->using_gems = true;
        }
    }

    c->init = 0;
    c->lock = 0;
    c->are = 0;
    c->lineare = 0;
    c->lineclear = 0;
    c->floorkicks = 0;
    q->timer.time = 0;

    q->combo_simple = 0;
    q->rotations = 0;
    q->pieces_dealt = 0;

    q->singles = 0;
    q->doubles = 0;
    q->triples = 0;
    q->tetrises = 0;
    q->pentrises = 0;

    q->medal_ac = 0;
    q->medal_sk = 0;
    q->medal_ro = 0;
    q->medal_st = 0;
    q->medal_re = 0;
    q->medal_co = 0;

    for(int i = 0; i < MAX_SECTIONS; i++)
    {
        q->section_times[i] = -1;
        q->section_tetrises[i] = 0;
        q->best_section_times[i] = -1;
    }

    q->section = 0;
    q->cur_section_timestamp = 0;

    q->p1->speeds = q->pracdata->usr_timings;
    if(q->pracdata->usr_timings->lock < 0)
        q->lock_delay_enabled = 0;
    else
        q->lock_delay_enabled = 1;

    q->state_flags = 0;
    q->topped_out = false;

    if(q->pracdata->invisible)
        q->state_flags |= GAMESTATE_INVISIBLE;
    if(q->pracdata->brackets)
        q->state_flags |= GAMESTATE_BRACKETS;

    if(q->pracdata->garbage_delay >= 0)
    {
        q->garbage_delay = q->pracdata->garbage_delay;
        q->state_flags |= GAMESTATE_RISING_GARBAGE;
    }

    q->previews.clear();

    if(q->pracdata->using_seed)
    {
        qrand->init(qrand, &q->pracdata->randomizer_seed);
    }
    else
    {
        qrand->init(qrand, NULL);
    }

    next1_id = qrand->pull(qrand);
    next2_id = qrand->pull(qrand);
    next3_id = qrand->pull(qrand);
    next4_id = qrand->pull(qrand);

    if(qrand->num_pieces == 7)
    {
        next1_id = ars_to_qrs_id(next1_id);
        next2_id = ars_to_qrs_id(next2_id);
        next3_id = ars_to_qrs_id(next3_id);
        next4_id = ars_to_qrs_id(next4_id);
    }

    if(q->pracdata->usr_seq_len)
    {
        for (size_t i = 0; i < 4; i++) {
            int elem = qs_get_usrseq_elem(q->pracdata, i);
            if (elem != USRSEQ_ELEM_OOB) {
                q->previews.push_back(q->piecepool[elem]);
            }
        }

        q->pracdata->hist_index = 3;
    }
    else
    {
        q->previews.push_back(q->piecepool[next1_id]);
        q->previews.push_back(q->piecepool[next2_id]);
        q->previews.push_back(q->piecepool[next3_id]);
        q->previews.push_back(q->piecepool[next4_id]);
    }

    if(q->state_flags & GAMESTATE_BRACKETS)
    {
        for (auto& preview : q->previews) {
            preview.flags = static_cast<Shiro::PieceDefinitionFlag>(preview.flags | Shiro::PDBRACKETS);
        }
    }

    if(q->cur_piece_qrs_id >= 18)
    {
        p->y = ROWTOY(SPAWNY_QRS + 4);
    }
    else
    {
        p->y = ROWTOY(SPAWNY_QRS);
    }

    p->def = NULL;
    p->x = SPAWNX_QRS;
    for(int i = 0; i < p->num_olds; i++)
    {
        p->old_xs[i] = p->x;
        p->old_ys[i] = p->y;
    }

    q->p1->state = PSFALL;
    q->level = 0;
    q->pracdata->paused = 0;

    if(q->pracdata->goal_time > 0)
    {
        q->timer.time = q->pracdata->goal_time;
        q->cur_section_timestamp = q->pracdata->goal_time;
    }
    else
    {
        q->timer.time = 0;
    }

    if(q->game_type == Shiro::GameType::SIMULATE_QRS)
    {
        histrand_set_difficulty(q->randomizer, pentomino_c_difficulty(q->level));
    }

    scoredb_clear_live_sectiontimes(&g->origin->records);

    return 0;
}

int qs_game_quit(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)g->data;

    int numBestSections = 0;

    if(!q->pracdata && !(q->state_flags & GAMESTATE_BIGMODE) && q->starting_level == 0)
    {
        for(int i = 0; i < MAX_SECTIONS; i++)
        {
            if(q->section_times[i] > 0)
            {
                if((q->best_section_times[i] < 0) || (q->section_times[i] < q->best_section_times[i]))
                {
                    int startlevel = i * 100;
                    int endlevel = startlevel + 100;

                    if(i == 9)
                    {
                        switch(q->mode_type)
                        {
                            case MODE_G2_MASTER:
                            case MODE_G2_DEATH:
                            case MODE_G1_20G:
                            case MODE_G1_MASTER:
                                endlevel = 999;
                                break;

                            default:
                                break;
                        }
                    }

                    scoredb_add_sectiontime(&g->origin->records, &g->origin->player, q->mode_type, q->grade, startlevel, endlevel, q->section_times[i]);

                    numBestSections++;
                }
            }
        }
    }

    scoredb_clear_live_sectiontimes(&g->origin->records);

    /*
    if(q->credits.is_open())
    {
        q->credits.close();
    }
    */

    if(numBestSections > 0)
    {
        std::cout << "Wrote " << numBestSections << " new best section times" << std::endl;
    }

    free(q->p1->old_ys);
    free(q->p1->old_xs);

    if (g->field) {
        delete g->field;
    }

    if(q)
    {
        qrsdata_destroy(q);
    }

    Mix_HaltMusic();

    // mostly a band-aid for quitting practice tool properly, so menu input does not take priority for regular modes
    g->origin->menu_input_override = 0;

    return 0;
}

int qs_game_preframe(game_t *) { return 0; }

int qs_game_frame(game_t *g)
{
    if(!g)
        return -1;

    CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)(g->data);

    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    if(q->pracdata)
    {
        if(q->pracdata->paused)
        {
            return 0;
        }
    }
    else
    {
        switch(cs->displayMode)
        {
            default:
            case Shiro::DisplayMode::DEFAULT:
            case Shiro::DisplayMode::DETAILED:
                q->field_x = QRS_FIELD_X;
                break;

            case Shiro::DisplayMode::CENTERED:
                q->field_x = 192;
                break;
        }
    }

    for(int i = q->p1->num_olds - 1; i > 0; i--)
    {
        q->p1->old_xs[i] = q->p1->old_xs[i - 1];
        q->p1->old_ys[i] = q->p1->old_ys[i - 1];
    }

    q->p1->old_xs[0] = q->p1->x;
    q->p1->old_ys[0] = q->p1->y;

    if(q->state_flags & GAMESTATE_GAMEOVER)
    {
        return 0;
    }

    if(c->init < 120)
    {
        if(c->init == 0 || c->init == 60)
        {
            struct text_formatting fmt = text_fmt_create(0, 0x00FF00FF, 0);
            fmt.size_multiplier = 2.0;
            fmt.outlined = false;
            if(q->pracdata)
                fmt.outlined = true;

            fmt.outline_rgba = 0x00000080;

            const auto gameIsInactive = [cs]() {
                if (cs->p1game == nullptr || cs->p1game->data == nullptr) {
                    return true;
                }

                const qrsdata* const q = static_cast<qrsdata*>(cs->p1game->data);
                return q->pracdata != nullptr && q->pracdata->paused == QRS_FIELD_EDIT;
            };

            if(c->init == 0)
            {
                // Start recording/playback immediately
                if(!q->pracdata)
                {
                    if(q->replay)
                    {
                        qrs_start_playback(g);
                    }
                    else
                    {
                        qrs_start_record(g);
                    }
                }

                MessageEntity::push(cs->gfx, cs->screen,
                    FontAsset::get(cs->assetMgr, "fixedsys"),
                    "READY",
                    q->field_x + 4 * 16 + 8,
                    q->field_y + 11 * 16,
                    2.0f,
                    fmt.rgba,
                    60u,
                    GfxLayer::messages,
                    gameIsInactive
                );

                SfxAsset::get(cs->assetMgr, "ready").play(cs->settings);
            }

            else if(c->init == 60)
            {
                fmt.rgba = 0xFF0000FF;
                MessageEntity::push(cs->gfx, cs->screen,
                    FontAsset::get(cs->assetMgr, "fixedsys"),
                    "GO",
                    q->field_x + 6 * 16,
                    q->field_y + 11 * 16,
                    2.0f,
                    fmt.rgba,
                    60u,
                    GfxLayer::messages,
                    gameIsInactive
                );

                SfxAsset::get(cs->assetMgr, "go").play(cs->settings);
            }
        }

        (*s) = PSINACTIVE;
        c->init++;
        return 0;
    }
    else if(c->init == 120)
    {
        c->init++;
        qs_initnext(g, q->p1, 0);
        qrs_proc_initials(g);

        if(qrs_chkcollision(*g, *q->p1))
        {
            qrs_lock(g, q->p1);
            (*s) = PSINACTIVE;

            q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE | GAMESTATE_CREDITS);

            if(!q->pracdata)
                Mix_HaltMusic();

            if(q->playback)
                qrs_end_playback(g);
            else if(q->recording)
                qrs_end_record(g);

            q->topped_out = true;

            return 0;
        }

        if(!q->pracdata)
        {
            update_music(q, cs);
        }
    }

    if(!q->pracdata && !(q->state_flags & GAMESTATE_FADE_TO_CREDITS))
    {
        // handle speed curve and music updates (this runs every frame)
        // TODO: why does this need to run every frame and not only on level updates?
        update_music(q, cs);
        switch(q->mode_type)
        {
            case MODE_G2_MASTER:
                while(q->speed_curve_index < G2_MASTER_CURVE_MAX && g2_master_curve[q->speed_curve_index].level <= q->level)
                {
                    q->p1->speeds = &g2_master_curve[q->speed_curve_index];
                    q->speed_curve_index++;
                }
                break;

            case MODE_G2_DEATH:
                while(q->speed_curve_index < G2_DEATH_CURVE_MAX && g2_death_curve[q->speed_curve_index].level <= q->level)
                {
                    q->p1->speeds = &g2_death_curve[q->speed_curve_index];
                    q->speed_curve_index++;
                }
                break;

            case MODE_G3_TERROR:
                while(q->speed_curve_index < G3_TERROR_CURVE_MAX && g3_terror_curve[q->speed_curve_index].level <= q->level)
                {
                    q->p1->speeds = &g3_terror_curve[q->speed_curve_index];
                    q->speed_curve_index++;
                }
                break;

            case MODE_G1_MASTER:
                while(q->speed_curve_index < G1_MASTER_CURVE_MAX && g1_master_curve[q->speed_curve_index].level <= q->level)
                {
                    q->p1->speeds = &g1_master_curve[q->speed_curve_index];
                    q->speed_curve_index++;
                }
                break;

            case MODE_G1_20G:
                q->p1->speeds = &g1_master_curve[G1_MASTER_CURVE_MAX - 1];
                break;

            case MODE_PENTOMINO:
                while(q->speed_curve_index < QS_CURVE_MAX && qs_curve[q->speed_curve_index].level <= q->level)
                {
                    q->p1->speeds = &qs_curve[q->speed_curve_index];
                    q->speed_curve_index++;
                }
                break;

            default:
                break;
        }
    }

    if(q->state_flags & GAMESTATE_STACK_REVEAL_DELAY)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter == 3 * 60)
        {
            q->stack_anim_counter = 0;

            q->state_flags &= ~GAMESTATE_STACK_REVEAL_DELAY;
            q->state_flags |= GAMESTATE_TOPOUT_ANIM;
        }
    }

    if(q->state_flags & GAMESTATE_TOPOUT_ANIM)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter == 6 * 20)
        {
            q->stack_anim_counter = 0;
            q->state_flags &= ~GAMESTATE_TOPOUT_ANIM;
            q->state_flags |= GAMESTATE_GAMEOVER;
        }
        // TODO: move this to gfx_drawqrsfield and make it purely visual
        /*else if(q->stack_anim_counter % 3 == 1)
        {
            int row = QRS_FIELD_H - 1 - (q->stack_anim_counter / 3);
            int start_i = static_cast<int>((g->field->getWidth() - q->field_w) / 2);

            if(q->state_flags & GAMESTATE_BIGMODE)
            {
                start_i = 0;
            }

            for(int i = start_i; i < start_i + q->field_w; i++)
            {
                if (g->field->getCell(i, row) && g->field->getCell(i, row) != QRS_WALL) {
                    g->field->cell(i, row) = QRS_PIECE_GARBAGE;
                }
            }
        }*/
    }

    if(q->state_flags & GAMESTATE_CREDITS_TOPOUT_ANIM)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter == 6 * 20)
        {
            q->stack_anim_counter = 0;
            q->state_flags &= ~GAMESTATE_CREDITS_TOPOUT_ANIM;
            q->state_flags |= GAMESTATE_GAMEOVER;
        }
    }

    if(q->state_flags & GAMESTATE_GAMEOVER)
    {
        if(q->cleared_game)
        {
            SfxAsset::get(cs->assetMgr, "newsection").play(cs->settings);
        }

        return 0;
    }

    if(q->state_flags & GAMESTATE_SOFT_CREDITS_TOPOUT)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter == 6 * 20)
        {
            q->stack_anim_counter = 0;

            q->state_flags &= ~GAMESTATE_SOFT_CREDITS_TOPOUT;
        }
    }

    if(q->state_flags & GAMESTATE_FADE_TO_CREDITS)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter >= 5 * 20) // 6 frames for each visible row
        {
            if(q->stack_anim_counter == 5 * 20)
            {
                // just zeroing out the last two (invisible) rows
                int start_i = static_cast<int>((g->field->getWidth() - q->field_w) / 2);

                if(q->state_flags & GAMESTATE_BIGMODE)
                {
                    start_i = 0;
                }

                for(int i = start_i; i < start_i + q->field_w; i++)
                {
                    g->field->cell(i, 0) = 0;
                    g->field->cell(i, 1) = 0;
                }
            }
            else if(q->stack_anim_counter == (5 * 20) + 30)
            {
                q->stack_anim_counter = 0;
                q->state_flags &= ~GAMESTATE_FADE_TO_CREDITS;

                /* for testing */
                // q->mroll_unlocked = true;

                if(q->mode_type == MODE_G2_MASTER)
                {
                    if(q->mroll_unlocked)
                    {
                        q->state_flags |= GAMESTATE_INVISIBLE | GAMESTATE_CREDITS;
                    }
                    else
                    {
                        q->state_flags |= GAMESTATE_FADING | GAMESTATE_CREDITS;
                    }
                }
                else if(q->mode_type == MODE_G3_TERROR)
                {
                    q->state_flags |= GAMESTATE_BIGMODE | GAMESTATE_CREDITS;

                    int w = q->field_w / 2;

                    g->field->fill(0);
                    for(int i = 0; i <= w; i++)
                    {
                        g->field->cell(i, QRS_FIELD_H - 10) = QRS_WALL;
                    }

                    for(int j = QRS_FIELD_H - 10; j >= 0; j--)
                    {
                        g->field->cell(w, j) = QRS_WALL;
                    }

                    q->field_w = w;
                }

                qs_initnext(g, q->p1, 0);
                qrs_proc_initials(g);
            }
        }
        else if(q->stack_anim_counter % 5 == 1)
        {
            int row = QRS_FIELD_H - 1 - (q->stack_anim_counter / 5);
            int start_i = static_cast<int>((g->field->getWidth() - q->field_w) / 2);

            if(q->state_flags & GAMESTATE_BIGMODE)
            {
                start_i = 0;
            }

            gfx_qs_lineclear(g, row);

            for(int i = start_i; i < start_i + q->field_w; i++)
            {
                g->field->cell(i, row) = 0;
            }
        }
    }

    if(q->state_flags & GAMESTATE_CREDITS)
    {
        q->credit_roll_counter--;
        if(q->credit_roll_counter == 0)
        {
            q->cleared_game = true;

            switch(q->mode_type)
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
                    if(q->p1->state != PSINACTIVE)
                    {
                        if(q->mroll_unlocked)
                        {
                            q->state_flags |= GAMESTATE_FIREWORKS_GM;

                            if(q->credit_roll_lineclears >= 32)
                                q->grade = GRADE_GM | ORANGE_LINE;
                            else
                                q->grade = GRADE_GM | GREEN_LINE;
                        }
                        else
                        {
                            q->state_flags |= GAMESTATE_FIREWORKS;
                            q->grade |= ORANGE_LINE;
                        }
                    }

                    (*s) = PSINACTIVE;
                    break;
            }

            q->state_flags &= ~GAMESTATE_CREDITS;
            q->state_flags |= GAMESTATE_FIREWORKS;

            if(q->playback)
                qrs_end_playback(g);
            else if(q->recording)
                qrs_end_record(g);
        }
    }

    if(q->state_flags & GAMESTATE_FIREWORKS)
    {
        q->stack_anim_counter++;
        if(q->stack_anim_counter == 4 * 60)
        {
            q->stack_anim_counter = 0;

            q->state_flags &= ~GAMESTATE_FIREWORKS;

            if(!q->topped_out)
            {
                q->state_flags |= GAMESTATE_CREDITS_TOPOUT_ANIM;
            }
            else
            {
                q->state_flags |= GAMESTATE_GAMEOVER;
            }
        }
    }

    if(q->state_flags & GAMESTATE_HALTING)
    {
        if((*s) & PSLINECLEAR)
        {
            qs_process_lineclear(g);
        }
        else
        {
            q->p1->state = PSINACTIVE;
            q->state_flags &= ~GAMESTATE_HALTING;
        }

        return 0;
    }

    if((*s) == PSINACTIVE)
    {
        return 0;
    }

    if((*s) & PSSPAWN)
    {
        qs_process_fall(g, false);
        (*s) &= ~PSSPAWN;
        std::cerr << "First piece" << std::endl;
    }
    else
    {
        if(qs_process_are(g) == QSGAME_SHOULD_TERMINATE)
        {
            if(q->pracdata)
            {
                q->pracdata->paused = 1;
            }

            q->p1->state = PSINACTIVE;
        }

        if((*s) & PSSPAWN)
        {
            qs_process_fall(g, false);
            (*s) &= ~PSSPAWN;
        }
        else
        {
            qs_process_prelockflash(g);
	    // TODO: Change "false" to be true when down is currently pressed.
            qs_process_fall(g, g->origin->keys.down != 0);
            qs_process_lock(g);
            qs_process_lockflash(g);
            qs_process_lineclear(g);
            if(qs_process_lineare(g) == QSGAME_SHOULD_TERMINATE)
            {
                if(q->pracdata)
                {
                    q->pracdata->paused = 1;
                }

                q->p1->state = PSINACTIVE;
            }

            if((*s) & PSSPAWN)
            {
                qs_process_fall(g, false);
                (*s) &= ~PSSPAWN;
            }
        }
    }

    if(q->state_flags & GAMESTATE_FADING)
    {
        int fade_counter = 0;
        int val = 0;

        for(int i = 0; i < int(g->field->getWidth()); i++)
        {
            for(int j = 0; j < int(g->field->getHeight()); j++)
            {
                val = g->field->getCell(i, j);
                if(val == QRS_WALL || val < 0)
                    continue;

                fade_counter = GET_PIECE_FADE_COUNTER(val);

                if(fade_counter > 0)
                {
                    fade_counter--;
                    SET_PIECE_FADE_COUNTER(val, fade_counter);
                    g->field->cell(i, j) = val;
                }
            }
        }
    }

    // player state might have been set to INACTIVE, we want to react right away
    if((*s) == PSINACTIVE)
        return 0;

    if(q->mode_type == MODE_G2_MASTER)
    {
        int decay_rate = grade_point_decays[q->internal_grade];
        if(((*s) & PSFALL || (*s) & PSLOCK) && q->combo_simple == 0)
            q->grade_decay_counter++;

        if(q->grade_decay_counter >= decay_rate && q->grade_points > 0)
        {
            q->grade_points--;
            q->grade_decay_counter = 0;
        }
    }

    int recoveryThreshold = q->game_type == Shiro::GameType::SIMULATE_QRS ? 172 : (q->game_type == Shiro::GameType::SIMULATE_G3 ? 160 : 150);

    int cellsFilled = 0;

    for(size_t i = 0; i < g->field->getWidth(); i++)
    {
        for(size_t j = 0; j < g->field->getHeight(); j++)
        {
            int c = g->field->cell(i, j);
            if(c != 0 && c != -2 && c != QRS_FIELD_W_LIMITER)
            {
                cellsFilled++;
            }
        }
    }

    if(cellsFilled >= recoveryThreshold)
    {
        q->is_recovering = 1;
    }

    if(!q->pracdata && q->mode_type == MODE_G3_TERROR)
    {
        if(q->level >= 1000)
        {
            q->state_flags |= GAMESTATE_BRACKETS;
            q->state_flags &= ~GAMESTATE_RISING_GARBAGE;
        }
        else if(q->level >= 500)
        {
            q->state_flags |= GAMESTATE_RISING_GARBAGE;
            switch(q->section)
            {
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

    if(q->mode_type == MODE_PENTOMINO || (q->pracdata && q->game_type == Shiro::GameType::SIMULATE_QRS))
    {
        histrand_set_difficulty(q->randomizer, pentomino_c_difficulty(q->level));

        // for testing
        // histrand_set_difficulty(q->randomizer, 100.0);
    }

    if(q->levelstop_time)
    {
        if(q->levelstop_time == 1)
        {
            switch(q->game_type)
            {
                case Shiro::GameType::SIMULATE_G3:
                case Shiro::GameType::SIMULATE_QRS:
                    SfxAsset::get(cs->assetMgr, "levelstop").play(cs->settings);
                    break;

                default:
                    break;
            }
        }

        q->levelstop_time++;
    }

    if(!(q->state_flags & GAMESTATE_CREDITS))
    {
        if(!q->pracdata)
        {
            q->timer++;
        }
        else
        {
            if(q->pracdata->goal_level != 0)
            {
                if(static_cast<int>(q->level) >= q->pracdata->goal_level)
                {
                    q->level = q->pracdata->goal_level;

                    q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE);
                    q->state_flags |= GAMESTATE_HALTING;

                    if(q->playback)
                        qrs_end_playback(g);
                    else if(q->recording)
                        qrs_end_record(g);
                }
            }

            if(q->pracdata->goal_time != 0)
            {
                if(q->timer.time > 0)
                {
                    q->timer--;
                    if(q->timer.time == 0)
                    {
                        q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE);
                        q->state_flags |= GAMESTATE_HALTING;

                        if(q->playback)
                            qrs_end_playback(g);
                        else if(q->recording)
                            qrs_end_record(g);
                    }
                }
            }
            else
            {
                q->timer++;
            }
        }
    }

    return 0;
}

static int qs_are_expired(game_t *g)
{
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    q->lastclear = 0;
    if(q->level % 100 != 99 && !(q->state_flags & GAMESTATE_CREDITS) && !q->pracdata)
    {
        switch(q->mode_type)
        {
            case MODE_G2_DEATH:
            case MODE_G1_20G:
            case MODE_G1_MASTER:
            case MODE_G2_MASTER:
                if(q->level != 998)
                {
                    q->level++;
                    q->lvlinc = 1;
                }
                else
                {
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
    else if(q->pracdata)
    {
        if(q->level % 100 != 99 && !(q->state_flags & GAMESTATE_CREDITS))
        {
            if(q->level == 998 && q->pracdata->goal_level == 999)
            {
                q->lvlinc = 0;
                q->levelstop_time++;
            }
            else
            {
                q->level++;
                q->lvlinc = 1;
            }
        }
        else
        {
            q->lvlinc = 0;
            if(q->level % 100 == 99)
                q->levelstop_time++;
        }
    }
    else
    {
        q->lvlinc = 0;
        if(q->level % 100 == 99)
            q->levelstop_time++;
    }

    c->lock = 0;
    if(qs_initnext(g, q->p1, 0) == QSGAME_SHOULD_TERMINATE)
    {
        return QSGAME_SHOULD_TERMINATE;
    }

    qrs_proc_initials(g);

    if(qrs_chkcollision(*g, *q->p1))
    {
        qrs_lock(g, q->p1);
        (*s) = PSINACTIVE;

        bool wasInvisible = (q->state_flags & GAMESTATE_INVISIBLE) != 0 || (q->state_flags & GAMESTATE_FADING) != 0;

        q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE);

        if(!q->pracdata)
            Mix_HaltMusic();

        if(q->playback)
            qrs_end_playback(g);
        else if(q->recording)
            qrs_end_record(g);

        q->topped_out = true;

        // TODO: for lineare, this was different - intentional or copy/paste bug?
        /*
                if(q->state_flags & GAMESTATE_CREDITS)
                    q->state_flags |= GAMESTATE_CREDITS_TOPOUT;
                else
                    q->state_flags |= GAMESTATE_TOPOUT_ANIM;
        */
        if(q->state_flags & GAMESTATE_CREDITS)
        {
            q->state_flags &= ~(GAMESTATE_FADING | GAMESTATE_INVISIBLE);

            switch(q->mode_type)
            {
                case MODE_G1_MASTER:
                case MODE_G1_20G:
                case MODE_G2_DEATH:
                case MODE_PENTOMINO:
                    q->state_flags |= GAMESTATE_SOFT_CREDITS_TOPOUT;
                    break;

                default:
                    q->state_flags |= GAMESTATE_STACK_REVEAL_DELAY;
                    q->state_flags &= ~GAMESTATE_CREDITS;
                    break;
            }

            if(q->mode_type == MODE_G2_MASTER)
            {
                if(q->mroll_unlocked)
                    q->grade = GRADE_M | GREEN_LINE;
                else
                    q->grade |= GREEN_LINE;


            }

            if(q->mode_type == MODE_G3_TERROR)
            {
                q->grade |= GREEN_LINE;
            }
        }
        else
        {
            if(!wasInvisible)
            {
                q->state_flags |= GAMESTATE_TOPOUT_ANIM;
            }
            else
            {
                q->state_flags |= GAMESTATE_STACK_REVEAL_DELAY;
            }
        }

        return 0;
    }

    if(q->state_flags & GAMESTATE_RISING_GARBAGE)
        q->garbage_counter++;

    return 0;
}

int qs_process_are(game_t *g)
{
    // CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    if((*s) & PSLOCKFLASH4)
        (*s) &= ~PSLOCKFLASH4;

    if((*s) & PSLOCKFLASH3)
    {
        (*s) &= ~PSLOCKFLASH3;
        (*s) |= PSLOCKFLASH4;
    }

    if((*s) & PSLOCKFLASH2)
    {
        (*s) &= ~PSLOCKFLASH2;
        (*s) |= PSLOCKFLASH3;

        if(!q->lastclear)
            (*s) |= PSARE;
    }

    if((*s) & PSARE)
    {
        if(c->are == q->p1->speeds->are)
        {
            c->are = 0;
            return qs_are_expired(g);
        }
        else
        {
            c->are++;
            if(c->are == 1)
            {
                // TODO: why is this not in lineare too?
                if(q->state_flags & GAMESTATE_RISING_GARBAGE)
                {
                    if(q->garbage_counter >= q->garbage_delay)
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
    // CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    if((*s) & PSLINEARE)
    {
        if(c->lineare == q->p1->speeds->lineare)
        {
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
    CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    if((*s) & PSLINECLEAR)
    {
        if(c->lineclear == q->p1->speeds->lineclear)
        {
            (*s) &= ~PSLINECLEAR;
            (*s) |= PSLINEARE;

            c->lineclear = 0;
            if(!(q->mode_type & (MODE_G1_MASTER | MODE_G1_20G) && q->grade == GRADE_GM))
            {
                qrs_dropfield(g);
            }
            SfxAsset::get(cs->assetMgr, "dropfield").play(cs->settings);

            switch(q->mode_type)
            {
                case MODE_G2_MASTER:
                    if(q->level == 999 && !(q->state_flags & GAMESTATE_CREDITS))
                    {
                        q->state_flags |= GAMESTATE_FADE_TO_CREDITS;
                        (*s) = PSINACTIVE;
                    }

                    break;

                case MODE_G3_TERROR:
                    if(q->level == 1300 && !(q->state_flags & GAMESTATE_CREDITS))
                    {
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
    // CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    if((*s) & PSLOCK && q->lock_on_rotate == 2)
    {
        if(q->pracdata)
        {
            if(!q->pracdata->infinite_floorkicks)
            {
                c->lock = 0;
                (*s) &= ~PSLOCK;
                (*s) |= PSPRELOCKED;
            }
        }
        else
        {
            c->lock = 0;
            (*s) &= ~PSLOCK;
            (*s) |= PSPRELOCKED;
        }

        q->lock_on_rotate = 0;
    }
    else if((*s) & PSLOCK && q->lock_delay_enabled)
    {
        if(qrs_isonground(g, q->p1))
        {
            if(YTOROW(q->p1->y) != q->locking_row)
            {
                q->locking_row = YTOROW(q->p1->y);
                c->lock = 0;
            }

            if(!q->p1->speeds->lock)
            {
                (*s) &= ~PSLOCK;
                (*s) |= PSPRELOCKED;
            }
            else
            {
                c->lock++;
                if(c->lock == q->p1->speeds->lock)
                {
                    c->lock = 0;
                    (*s) &= ~PSLOCK;
                    (*s) |= PSPRELOCKED;
                }
            }
        }
        else
        {
            (*s) &= ~PSLOCK;
            (*s) |= PSFALL;
        }
    }

    return 0;
}

int qs_process_fall(game_t *g, bool soft_drop) {
    // CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    QRS_Counters *c = q->p1counters;

    int row = 0;

    const bool lock_protect_enabled = q->is_practice ? q->pracdata->lock_protect : q->lock_protect;
    const bool should_lock_protect = lock_protect_enabled && q->lock_held;

    if ((*s) & PSFALL) {
        row = YTOROW(q->p1->y);

        int grav;
        if (q->p1->speeds->grav >= 256 || !soft_drop) {
            grav = 0;
        }
	else {
            grav = 256;
        }
        qrs_fall(g, q->p1, grav);

	if (YTOROW(q->p1->y) < row) {
            c->lock = 0;
        }

	if (soft_drop) {
            q->p1->y &= ~0xFF;
            q->soft_drop_counter++;
            if (qrs_isonground(g, q->p1) && !should_lock_protect) {
                q->lock_held = 1;
                q->p1->state &= ~PSLOCK;
                q->p1->state &= ~PSFALL;
                q->p1->state |= PSLOCKPRESSED;
            }
	}
    }
    else if (soft_drop && (*s) & PSLOCK && !should_lock_protect) {
        q->lock_held = 1;
        q->p1->state &= ~PSLOCK;
        q->p1->state |= PSLOCKPRESSED;
    }

    return 0;
}

int qs_process_prelockflash(game_t *g)
{
    // CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    // qrs_counters *c = q->p1counters;

    if((*s) & PSLOCKFLASH1)
    {
        (*s) &= ~PSLOCKFLASH1;
        (*s) |= PSLOCKFLASH2;
    }

    if((*s) & PSPRELOCKED)
    {
        (*s) &= ~PSPRELOCKED;
        qrs_lock(g, q->p1);

        if(q->previews.size() == 0)
        {
            q->state_flags |= GAMESTATE_HALTING;
        }

        (*s) &= ~PSPRELOCKFLASH1;
        (*s) |= PSLOCKFLASH1;
    }

    if((*s) & PSLOCKPRESSED)
    {
        (*s) &= ~PSLOCKPRESSED;
        (*s) |= PSPRELOCKED;
    }

    if((*s) & PSPRELOCKFLASH1)
    {
        (*s) &= ~PSPRELOCKFLASH1;
        (*s) |= PSLOCKFLASH1;
    }

    return 0;
}

int qs_process_lockflash(game_t *g)
{
    CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)g->data;
    unsigned int *s = &q->p1->state;
    // qrs_counters *c = q->p1counters;

    int n = 0;
    bool bravo = false;
    int sectionTimeDelta = -1;

    if((*s) & PSLOCKFLASH1)
    {
        n = qrs_lineclear(g, q->p1);
        q->lastclear = n;

        q->placement_speed = q->p1->speeds->lock - q->active_piece_time;
        if(q->placement_speed < 0)
            q->placement_speed = 0;

        if(n)
        {
            (*s) |= PSLINECLEAR;

            q->combo += 2 * n - 2;

            int cells = 0;

            for(size_t i = 0; i < g->field->getWidth(); i++)
            {
                for(size_t j = 0; j < g->field->getHeight(); j++)
                {
                    int c = g->field->cell(i, j);
                    if(c != 0 && c != -2 && c != QRS_FIELD_W_LIMITER)
                    {
                        cells++;
                    }
                }
            }

            if(cells == 0)
            {
                bravo = true;
                q->bravos++;
            }

            switch(n)
            {
                case 5:
                    q->pentrises++;
                    break;
                case 4:
                    q->tetrises++;
                    if(!(q->state_flags & GAMESTATE_CREDITS))
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
                    if(q->mode_type == MODE_PENTOMINO)
                    {
                        //qrs_embiggen(q->previews[0]);
                        //qrs_embiggen(q->previews[1]);
                        //qrs_embiggen(q->previews[2]);
                    }
                    break;
                default:
                    break;
            }

            if(!(q->state_flags & GAMESTATE_CREDITS))
            {
                if(q->game_type == Shiro::GameType::SIMULATE_G3 && n > 2)
                    q->lvlinc = 2 * n - 2;
                else
                    q->lvlinc = n;

                q->level += q->lvlinc;

                int pts;
                float combo_mult;
                bool gradeup = false;

                double n_val;

                // score/grade stuff
                switch(q->mode_type)
                {
                    default:
                        break;

                    case MODE_G1_MASTER:
                    case MODE_G1_20G:
                        pts = (static_cast<int>(ceil(q->level / 4.0)) + q->soft_drop_counter) * n * q->combo * (bravo ? 4 : 1);
                        q->score += pts;
                        for(int i = 0; i < 17; i++)
                        {
                            if(q->score - pts < g1_grade_score_reqs[i] && q->score >= g1_grade_score_reqs[i])
                            {
                                q->grade = GRADE_8 + i;
                                if(!gradeup)
                                {
                                    q->last_gradeup_timestamp = g->frame_counter;
                                    SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                    gradeup = true;
                                }
                            }
                        }

                        break;

                    case MODE_G2_DEATH:
                        q->score += (q->level / 4 + q->soft_drop_counter + 2 * q->sonic_drop_height) * n * q->combo * (bravo ? 4 : 1) + q->level / 2 +
                                    7 * q->placement_speed;
                        break;

                    case MODE_G3_TERROR:
                        break;

                    case MODE_PENTOMINO:
                        n_val = ((double)n * 0.7);
                        if(n_val < 1.0)
                            n_val = 1.0;

                        pts = static_cast<int>(
                            static_cast<double>(static_cast<int>(ceil(q->level / 3.0)) + q->soft_drop_counter + q->sonic_drop_height + 2 * q->placement_speed - (q->levelstop_time / 4)) *
                            n_val *
                            (static_cast<double>(q->combo) * (bravo ? 4.0 : 1.0))
                        );
                        if(pts < 0)
                            pts = 0;

                        q->score += pts;

                        // rank control: if you're playing well, the game gets harder; playing poorly, the game eases up
                        /*switch(n)
                        {
                            case 1:
                                histrand_set_difficulty(q->randomizer, histrand_get_difficulty(q->randomizer) - 0.2);
                                break;
                            case 2:
                                histrand_set_difficulty(q->randomizer, histrand_get_difficulty(q->randomizer) + 0.4);
                                break;
                            case 3:
                                histrand_set_difficulty(q->randomizer, histrand_get_difficulty(q->randomizer) + 0.8);
                                break;
                            case 4:
                                histrand_set_difficulty(q->randomizer, histrand_get_difficulty(q->randomizer) + 1.6);
                                break;
                            case 5:
                                histrand_set_difficulty(q->randomizer, histrand_get_difficulty(q->randomizer) + 2.0);
                                break;
                        }
                        */

                        for(int i = 0; i < 17; i++)
                        {
                            if(q->score - pts < qs_grade_score_reqs[i] && q->score >= qs_grade_score_reqs[i])
                            {
                                q->grade = GRADE_8 + i;
                                if(!gradeup)
                                {
                                    q->last_gradeup_timestamp = g->frame_counter;
                                    SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                    gradeup = true;
                                }
                            }
                        }

                        break;

                    case MODE_G2_MASTER:
                        pts = grade_points_table[q->internal_grade][n - 1];
                        combo_mult = g2_grade_point_combo_table[q->combo_simple][n - 1];

                        q->grade_points += (int)ceil(pts * combo_mult) * (1 + q->level / 250);

                        if(q->grade_points >= 100)
                        {
                            int old_grade = q->grade;
                            q->grade_points = 0;
                            q->internal_grade++;
                            if(q->internal_grade > 31)
                                q->internal_grade = 31;
                            q->grade = internal_to_displayed_grade(q->internal_grade);
                            if(old_grade != q->grade)
                            {
                                q->last_gradeup_timestamp = g->frame_counter;
                                SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                            }
                        }

                        // Score = ((Level + Lines)/4 + Soft + (2 x Sonic)) x Lines x Combo x Bravo + (Level_After_Clear)/2 + (Speed x 7)
                        q->score += (q->level / 4 + q->soft_drop_counter + 2 * q->sonic_drop_height) * n * q->combo * (bravo ? 4 : 1) + q->level / 2 +
                                    7 * q->placement_speed;

                        break;
                }
            }
            else
            {
                q->lvlinc = 0;
            }

            if(q->state_flags & GAMESTATE_RISING_GARBAGE)
            {
                switch(q->mode_type)
                {
                    case MODE_G3_TERROR:
                        q->garbage_counter -= n;
                        if(q->garbage_counter < 0)
                            q->garbage_counter = 0;
                        break;

                    default:
                        break;
                }
            }

            SfxAsset::get(cs->assetMgr, "lineclear").play(cs->settings);

            if(((q->level - q->lvlinc) % 100) > 90 && (q->level % 100) < 10)
            {
                q->section_times[q->section] = abs(static_cast<int>(q->timer.time - q->cur_section_timestamp));

                int startlevel = q->section * 100;
                int endlevel = startlevel + 100;

                if(q->section == 9)
                {
                    switch(q->mode_type)
                    {
                        case MODE_G1_MASTER:
                        case MODE_G1_20G:
                        case MODE_G2_MASTER:
                        case MODE_G2_DEATH:
                            endlevel = 999;
                            break;

                        default:
                            break;
                    }
                }

                scoredb_add_live_sectiontime(&g->origin->records, &g->origin->player, q->mode_type, startlevel, endlevel, q->section_times[q->section]);

                q->cur_section_timestamp = static_cast<long>(q->timer.time);

                sectionTimeDelta = -1;

                if(q->best_section_times[q->section] > 0)
                {
                    sectionTimeDelta = q->section_times[q->section] - q->best_section_times[q->section];
                }

                q->section++;

                q->levelstop_time = 0;

                SfxAsset::get(cs->assetMgr, "newsection").play(cs->settings);

                if(!q->pracdata)
                {
                    switch(q->mode_type)
                    {
                        case MODE_PENTOMINO:
                            if(q->section == 5)
                            {
                                if(q->score < 50000 || q->timer.time > 5 * 60 * 60 + 30 * 60)
                                {
                                    q->mroll_unlocked = false;
                                }
                            }
                            else if(q->section == 10)
                            {
                                if(q->timer.time > 10 * 60 * 60)
                                {
                                    q->section--;

                                    if(q->playback)
                                        qrs_end_playback(g);
                                    else if(q->recording)
                                        qrs_end_record(g);
                                    q->p1->state = PSINACTIVE;
                                }
                                else
                                {
                                    if(q->score < 132000)
                                        q->mroll_unlocked = false;

                                    if(q->mroll_unlocked)
                                    {
                                        q->grade = GRADE_M;
                                        q->last_gradeup_timestamp = g->frame_counter;
                                        SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                    }
                                }
                            }
                            else if(q->level >= 1200 && q->level < 1300)
                            {
                                q->level = 1200;

                                if(q->score < 132000)
                                    q->mroll_unlocked = false;

                                if(q->mroll_unlocked)
                                {
                                    q->grade = GRADE_GM;
                                    q->last_gradeup_timestamp = g->frame_counter;
                                    SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                }

                                /*
                                if(q->playback)
                                    qrs_end_playback(g);
                                else if(q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                                */

                                q->state_flags |= GAMESTATE_CREDITS;
                            }

                            break;

                        case MODE_G2_MASTER:
                            if(q->level >= 999)
                            {
                                q->level = 999;
                                if(q->timer.time > 525 * 60 || q->grade < GRADE_S9)
                                    q->mroll_unlocked = false;
                            }

                            switch(q->section - 1)
                            {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                case 4:
                                    if(q->section_tetrises[q->section - 1] < 2 || q->section_times[q->section - 1] > (65 * 60))
                                    {
                                        q->mroll_unlocked = false;
                                    }

                                    break;

                                case 5:
                                    if(q->section_tetrises[q->section - 1] < 1 ||
                                       q->section_times[q->section - 1] > (AVG_FIRST_FIVE(q->section_times)) + 2 * 60)
                                    {
                                        q->mroll_unlocked = false;
                                    }

                                    break;

                                case 6:
                                case 7:
                                case 8:
                                    if(q->section_tetrises[q->section - 1] < 1 ||
                                       q->section_times[q->section - 1] > (q->section_times[q->section - 2]) + 2 * 60)
                                    {
                                        q->mroll_unlocked = false;
                                    }

                                    break;

                                case 9:
                                    if(q->section_times[q->section - 1] > (q->section_times[q->section - 2]) + 2 * 60)
                                        q->mroll_unlocked = false;

                                    break;

                                default:
                                    break;
                            }

                            break;

                        case MODE_G2_DEATH:
                            if(q->section == 5)
                            {
                                if(q->timer.time > G2_DEATH_TORIKAN)
                                {
                                    q->section--;

                                    if(q->playback)
                                        qrs_end_playback(g);
                                    else if(q->recording)
                                        qrs_end_record(g);
                                    q->p1->state = PSINACTIVE;
                                }
                                else
                                {
                                    if(q->starting_level == 0)
                                        q->grade = GRADE_M;

                                    q->last_gradeup_timestamp = g->frame_counter;
                                    SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                }
                            }
                            else if(q->level >= 999)
                            {
                                q->level = 999;
                                if(q->starting_level == 0)
                                    q->grade = GRADE_GM;

                                q->last_gradeup_timestamp = g->frame_counter;
                                SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                /*
                                if(q->playback)
                                    qrs_end_playback(g);
                                else if(q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                                */

                                q->state_flags |= GAMESTATE_CREDITS;
                            }

                            break;

                        case MODE_G3_TERROR:
                            if(q->grade == NO_GRADE)
                            {
                                q->grade = GRADE_S1;
                            }
                            else
                            {
                                q->grade++;
                            }

                            q->last_gradeup_timestamp = g->frame_counter;
                            SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);

                            if(q->section == 5)
                            {
                                if(q->timer.time > G3_TERROR_TORIKAN)
                                {
                                    if(q->playback)
                                        qrs_end_playback(g);
                                    else if(q->recording)
                                        qrs_end_record(g);
                                    q->p1->state = PSINACTIVE;
                                }
                            }
                            else if(q->section == 10)
                            {
                                if(q->timer.time > 2 * G3_TERROR_TORIKAN)
                                {
                                    q->section--;

                                    if(q->playback)
                                        qrs_end_playback(g);
                                    else if(q->recording)
                                        qrs_end_record(g);
                                    q->p1->state = PSINACTIVE;
                                }
                            }
                            else if(q->section == 13)
                            {
                                q->level = 1300;
                            }

                            break;

                        case MODE_G1_20G:
                        case MODE_G1_MASTER:
                            // checking "mroll" requirements (actually just GM reqs)
                            if(q->section == 3 && (q->timer.time > (4 * 60 * 60 + 15 * 60) || q->score < 12000))
                                q->mroll_unlocked = false;

                            if(q->section == 5 && (q->timer.time > (7 * 60 * 60 + 30 * 60) || q->score < 40000))
                                q->mroll_unlocked = false;

                            if(q->level >= 999)
                            {
                                q->level = 999;
                                if(q->timer.time >= (13 * 60 * 60 + 30 * 60) || q->score < 126000)
                                    q->mroll_unlocked = false;

                                if(q->mroll_unlocked)
                                {
                                    q->grade = GRADE_GM;
                                    q->last_gradeup_timestamp = g->frame_counter;
                                    SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                                }

                                /*
                                if(q->playback)
                                    qrs_end_playback(g);
                                else if(q->recording)
                                    qrs_end_record(g);
                                q->p1->state = PSINACTIVE;
                                */

                                q->state_flags |= GAMESTATE_CREDITS;
                            }

                            break;

                        default:
                            break;
                    }

                    if(q->section < 13)
                    {
                        if(!(q->section == 10 && q->level == 999))
                        {
                            cs->bg.transition(ImageAsset::get(cs->assetMgr, "bg", q->section));
                        }
                    }
                }
            }
            else if(q->level == 999 && q->lvlinc)
            {
                sectionTimeDelta = -1;

                switch(q->mode_type)
                {
                    case MODE_G2_MASTER:
                        SfxAsset::get(cs->assetMgr, "newsection").play(cs->settings);
                        q->section_times[q->section] = abs(static_cast<int>(q->timer.time - q->cur_section_timestamp));

                        scoredb_add_live_sectiontime(&g->origin->records, &g->origin->player, q->mode_type, 900, 999, q->section_times[q->section]);

                        q->cur_section_timestamp = static_cast<long>(q->timer.time);

                        if(q->best_section_times[q->section] > 0)
                        {
                            sectionTimeDelta = q->section_times[q->section] - q->best_section_times[q->section];
                        }

                        q->section++;

                        if(q->timer.time > 525 * 60 || q->grade < GRADE_S9)
                            q->mroll_unlocked = false;

                        if(q->section_times[9] > (q->section_times[8]) + 2 * 60)
                            q->mroll_unlocked = false;

                        break;

                    case MODE_G2_DEATH:
                        SfxAsset::get(cs->assetMgr, "newsection").play(cs->settings);
                        q->section_times[q->section] = abs(static_cast<int>(q->timer.time - q->cur_section_timestamp));

                        scoredb_add_live_sectiontime(&g->origin->records, &g->origin->player, q->mode_type, 900, 999, q->section_times[q->section]);

                        q->cur_section_timestamp = static_cast<long>(q->timer.time);

                        if(q->best_section_times[q->section] > 0)
                        {
                            sectionTimeDelta = q->section_times[q->section] - q->best_section_times[q->section];
                        }

                        q->section++;

                        if(q->starting_level == 0)
                            q->grade = GRADE_GM;

                        q->last_gradeup_timestamp = g->frame_counter;
                        SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                        /*
                        if(q->playback)
                            qrs_end_playback(g);
                        else if(q->recording)
                            qrs_end_record(g);
                        q->p1->state = PSINACTIVE;
                        */

                        q->state_flags |= GAMESTATE_CREDITS;

                        break;

                    case MODE_G3_TERROR:
                        break;

                    case MODE_G1_20G:
                    case MODE_G1_MASTER:
                        SfxAsset::get(cs->assetMgr, "newsection").play(cs->settings);
                        q->section_times[q->section] = abs(static_cast<int>(q->timer.time - q->cur_section_timestamp));

                        scoredb_add_live_sectiontime(&g->origin->records, &g->origin->player, q->mode_type, 900, 999, q->section_times[q->section]);

                        q->cur_section_timestamp = static_cast<long>(q->timer.time);

                        if(q->best_section_times[q->section] > 0)
                        {
                            sectionTimeDelta = q->section_times[q->section] - q->best_section_times[q->section];
                        }

                        q->section++;

                        if(q->timer.time >= (13 * 60 * 60 + 30 * 60) || q->score < 126000)
                            q->mroll_unlocked = false;

                        if(q->mroll_unlocked)
                        {
                            q->grade = GRADE_GM;
                            q->last_gradeup_timestamp = g->frame_counter;
                            SfxAsset::get(cs->assetMgr, "gradeup").play(cs->settings);
                        }

                        /*
                        if(q->playback)
                            qrs_end_playback(g);
                        else if(q->recording)
                            qrs_end_record(g);
                        q->p1->state = PSINACTIVE;
                        */

                        q->state_flags |= GAMESTATE_CREDITS;

                        break;

                    default:
                        break;
                }
            }

            q->combo_simple += (n > 1 ? 1 : 0);

            if(q->using_gems)
            {
                bool gems_in_field = false;
                int cell = 0;

                for(size_t i = 0; i < g->field->getWidth(); i++)
                {
                    for(size_t j = 0; j < g->field->getHeight(); j++)
                    {
                        cell = g->field->getCell(i, j);
                        if(cell < 0 || cell == GRID_OOB)
                            continue;

                        if(cell & QRS_PIECE_GEM)
                            gems_in_field = true;
                    }
                }

                if(gems_in_field == false)
                {
                    std::cerr << "No gems left, terminating." << std::endl;
                    q->state_flags |= GAMESTATE_HALTING;
                }
            }
        }
        else
        {
            q->combo = 1;
            q->combo_simple = 0;

            if(q->mode_type == MODE_PENTOMINO && q->levelstop_time > 0 && q->levelstop_time < 500)
            {
                q->score -= (q->levelstop_time / 4 > 100) ? 100 : q->levelstop_time / 4;
                if(q->score < 0)
                    q->score = 0;

                int old_grade = q->grade;

                for(int i = 0; i < 17; i++)
                {
                    if(q->score >= qs_grade_score_reqs[i])
                    {
                        q->grade = GRADE_8 + i;
                    }
                }

                if(q->score < qs_grade_score_reqs[0])
                    q->grade = GRADE_9;

                if(old_grade != q->grade)
                    q->last_gradeup_timestamp = g->frame_counter;
            }
        }
    }

    int sectionTimeDeltaBronze = (10 * 60);
    int sectionTimeDeltaSilver = (5 * 60);
    int sectionTimeDeltaGold = 0;
    int sectionTimeDeltaPlatinum = 0 - (2 * 60);

    int skillValueBronze = 4;
    int skillValueSilver = 7;
    int skillValueGold = 11;
    int skillValuePlatinum = 15;

    int comboValueBronze = 4;
    int comboValueSilver = 5;
    int comboValueGold = 7;
    int comboValuePlatinum = 8;

    int recoveryValueBronze = 1;
    int recoveryValueSilver = 2;
    int recoveryValueGold = 4;
    int recoveryValuePlatinum = 5;

    int recoveryLowerThreshold = q->game_type == Shiro::GameType::SIMULATE_QRS ? 85 : 70;

    float rotationMedalThreshold = 6.0 / 5.0; // rotations per piece

    bool usingPlatinumMedals = false;

    bool is999mode = false;

    if((*s) & PSLOCKFLASH1)
    {
        switch(q->game_type)
        {
            case Shiro::GameType::SIMULATE_G1:
            case Shiro::GameType::SIMULATE_G2:
                if(q->mode_type == MODE_G2_DEATH)
                {
                    skillValueBronze = 5;
                    skillValueSilver = 10;
                    skillValueGold = 17;
                    skillValuePlatinum = 22;
                }
                else
                {
                    skillValueBronze = 10;
                    skillValueSilver = 20;
                    skillValueGold = 35;
                    skillValuePlatinum = 40;
                }

                is999mode = true;

                break;

            case Shiro::GameType::SIMULATE_G3:
                sectionTimeDeltaBronze = (7 * 60);
                sectionTimeDeltaSilver = (3 * 60);

                if(q->mode_type == MODE_G3_TERROR)
                {
                    skillValueBronze = 5;
                    skillValueSilver = 10;
                    skillValueGold = 15;
                    skillValuePlatinum = 35;
                }

                break;

            case Shiro::GameType::SIMULATE_QRS:
                usingPlatinumMedals = true;
                comboValueBronze = 3;
                comboValueSilver = 4;
                comboValueGold = 5;
                comboValuePlatinum = 6;
                break;

            default:
                break;
        }

        if(q->state_flags & GAMESTATE_BIGMODE)
        {
            skillValueBronze = 1;
            skillValueSilver = 2;
            skillValueGold = 4;
            skillValuePlatinum = 5;

            comboValueBronze = 2;
            comboValueSilver = 3;
            comboValueGold = 4;
            comboValuePlatinum = 5;
        }

        if(n)
        {
            if( (((q->level - q->lvlinc) % 100) > 90 && (q->level % 100) < 10) ||
                (q->level == 999 && q->lvlinc && is999mode) )
            {
                if(!(q->state_flags & GAMESTATE_BIGMODE) && q->starting_level == 0 && !q->pracdata)
                {
                    bool medalAwarded = false;

                    if(usingPlatinumMedals && (sectionTimeDelta <= sectionTimeDeltaPlatinum))
                    {
                        q->medal_st = PLATINUM;
                        medalAwarded = true;
                    }
                    else if(sectionTimeDelta <= sectionTimeDeltaGold)
                    {
                        if(q->medal_st <= GOLD)
                        {
                            q->medal_st = GOLD;
                            medalAwarded = true;
                        }
                    }
                    else if(sectionTimeDelta <= sectionTimeDeltaSilver)
                    {
                        if(q->medal_st < SILVER)
                        {
                            q->medal_st = SILVER;
                            medalAwarded = true;
                        }
                    }
                    else if(sectionTimeDelta <= sectionTimeDeltaBronze)
                    {
                        if(q->medal_st < BRONZE)
                        {
                            q->medal_st = BRONZE;
                            medalAwarded = true;
                        }
                    }

                    if(medalAwarded)
                    {
                        q->last_medal_st_timestamp = g->frame_counter;
                        SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                    }
                }

                bool aboveThreshold = float(q->rotations) / float(q->pieces_dealt) > rotationMedalThreshold;
                int medal = 0;

                switch(q->section)
                {
                    case 3:
                        q->rotations = 0;
                        q->pieces_dealt = 0;
                        medal = BRONZE;
                        break;

                    case 7:
                        q->rotations = 0;
                        q->pieces_dealt = 0;
                        medal = SILVER;
                        break;

                    case 10:
                        q->rotations = 0;
                        q->pieces_dealt = 0;
                        medal = GOLD;
                        break;

                    case 12:
                        q->rotations = 0;
                        q->pieces_dealt = 0;
                        if(usingPlatinumMedals)
                        {
                            medal = PLATINUM;
                        }

                        break;

                    default:
                        break;
                }

                if((medal != 0) && aboveThreshold)
                {
                    q->medal_ro = medal;
                    q->last_medal_ro_timestamp = g->frame_counter;
                    SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                }
            }

            if(n == 4 && q->game_type != Shiro::GameType::SIMULATE_QRS)
            {
                bool medalAwarded = false;

                if(usingPlatinumMedals && q->tetrises == skillValuePlatinum)
                {
                    q->medal_sk = PLATINUM;
                    medalAwarded = true;
                }
                else if(q->tetrises == skillValueGold)
                {
                    q->medal_sk = GOLD;
                    medalAwarded = true;
                }
                else if(q->tetrises == skillValueSilver)
                {
                    q->medal_sk = SILVER;
                    medalAwarded = true;
                }
                else if(q->tetrises == skillValueBronze)
                {
                    q->medal_sk = BRONZE;
                    medalAwarded = true;
                }

                if(medalAwarded)
                {
                    q->last_medal_sk_timestamp = g->frame_counter;
                    SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                }
            }

            if(n == 5 && q->game_type == Shiro::GameType::SIMULATE_QRS)
            {
                bool medalAwarded = false;

                if(usingPlatinumMedals && q->pentrises == skillValuePlatinum)
                {
                    q->medal_sk = PLATINUM;
                    medalAwarded = true;
                }
                else if(q->pentrises == skillValueGold)
                {
                    q->medal_sk = GOLD;
                    medalAwarded = true;
                }
                else if(q->pentrises == skillValueSilver)
                {
                    q->medal_sk = SILVER;
                    medalAwarded = true;
                }
                else if(q->pentrises == skillValueBronze)
                {
                    q->medal_sk = BRONZE;
                    medalAwarded = true;
                }

                if(medalAwarded)
                {
                    q->last_medal_sk_timestamp = g->frame_counter;
                    SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                }
            }

            if(n > 1)
            {
                bool medalAwarded = false;

                if(usingPlatinumMedals && q->combo_simple == comboValuePlatinum)
                {
                    q->medal_co = PLATINUM;
                    medalAwarded = true;
                }
                else if(q->combo_simple == comboValueGold)
                {
                    q->medal_co = GOLD;
                    medalAwarded = true;
                }
                else if(q->combo_simple == comboValueSilver)
                {
                    q->medal_co = SILVER;
                    medalAwarded = true;
                }
                else if(q->combo_simple == comboValueBronze)
                {
                    q->medal_co = BRONZE;
                    medalAwarded = true;
                }

                if(medalAwarded)
                {
                    q->last_medal_co_timestamp = g->frame_counter;
                    SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                }
            }

            if(q->is_recovering)
            {
                int cellsFilled = 0;

                for(size_t i = 0; i < g->field->getWidth(); i++)
                {
                    for(size_t j = 0; j < g->field->getHeight(); j++)
                    {
                        int c = g->field->cell(i, j);
                        if(c != 0 && c != -2 && c != QRS_FIELD_W_LIMITER)
                        {
                            cellsFilled++;
                        }
                    }
                }

                if(cellsFilled <= recoveryLowerThreshold)
                {
                    q->is_recovering = 0;
                    q->recoveries++;

                    bool medalAwarded = false;

                    if(usingPlatinumMedals && q->recoveries == recoveryValuePlatinum)
                    {
                        q->medal_re = PLATINUM;
                        medalAwarded = true;
                    }
                    else if(q->recoveries == recoveryValueGold)
                    {
                        q->medal_re = GOLD;
                        medalAwarded = true;
                    }
                    else if(q->recoveries == recoveryValueSilver)
                    {
                        q->medal_re = SILVER;
                        medalAwarded = true;
                    }
                    else if(q->recoveries == recoveryValueBronze)
                    {
                        q->medal_re = BRONZE;
                        medalAwarded = true;
                    }

                    if(medalAwarded)
                    {
                        q->last_medal_re_timestamp = g->frame_counter;
                        SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
                    }
                }
            }

            if(bravo)
            {
                if(q->medal_ac < GOLD || (q->medal_ac < PLATINUM && usingPlatinumMedals))
                {
                    q->medal_ac++;
                }

                if(q->game_type == Shiro::GameType::SIMULATE_QRS)
                {
                    q->medal_ac = PLATINUM;
                }

                q->last_medal_ac_timestamp = g->frame_counter;
                SfxAsset::get(cs->assetMgr, "medal").play(cs->settings);
            }
        }
    }

    return 0;
}

int qrs_game_is_inactive(CoreState *cs)
{
    if(!cs->p1game)
        return 1;

    qrsdata *q = (qrsdata *)cs->p1game->data;

    if(!q)
        return 1;

    if(q->pracdata)
    {
        if(q->pracdata->paused == QRS_FIELD_EDIT)
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

int qs_update_pracdata(CoreState *cs)
{
    if(!cs->p1game || !cs->menu)
        return 1;

    qrsdata *q = (qrsdata *)cs->p1game->data;
    pracdata *d = q->pracdata;
    menudata *md = (menudata *)cs->menu->data;
    std::string seqStr;
    char name_str[3] = {0, 0, 0};

    int piece_seq[3000];
    std::size_t num = 0;

    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t k = 0;
    std::size_t t = 0;
    unsigned char c;

    int rpt_start = 0;
    int rpt_end = 0;
    bool rpt = false;
    int rpt_count = 0;
    int pre_rpt_count = 0;

    char rpt_count_strbuf[5];

    switch(d->game_type_int)
    {
        default:
        case 0:
            d->game_type = Shiro::GameType::SIMULATE_QRS;
            break;

        case 1:
            d->game_type = Shiro::GameType::SIMULATE_G1;
            break;

        case 2:
            d->game_type = Shiro::GameType::SIMULATE_G2;
            break;

        case 3:
            d->game_type = Shiro::GameType::SIMULATE_G3;
            break;
    }

    q->game_type = d->game_type;
    q->field_w = d->field_w;

    switch(q->game_type)
    {
        case Shiro::GameType::SIMULATE_QRS:
            q->num_previews = 4;
            q->randomizer_type = RANDOMIZER_NORMAL;
            if(q->randomizer)
                randomizer_destroy(q->randomizer);
            q->randomizer = pento_randomizer_create();

            q->hold_enabled = 0;
            q->max_floorkicks = 2;
            q->lock_protect = 1;
            q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags & ~Shiro::PDNOWKICK);
            q->tetromino_only = 0;
            q->pentomino_only = 0;
            cs->request_fps(Shiro::RefreshRates::pentomino);
            break;
        case Shiro::GameType::SIMULATE_G1:
            q->num_previews = 1;
            q->randomizer_type = RANDOMIZER_G1;
            if(q->randomizer)
                randomizer_destroy(q->randomizer);
            q->randomizer = g1_randomizer_create();

            q->hold_enabled = 0;
            q->max_floorkicks = 0;
            q->lock_protect = 0;
            q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags | Shiro::PDNOWKICK);
            q->tetromino_only = 1;
            q->pentomino_only = 0;
            cs->request_fps(Shiro::RefreshRates::g1);
            break;
        case Shiro::GameType::SIMULATE_G2:
            q->num_previews = 1;
            q->randomizer_type = RANDOMIZER_G2;
            if(q->randomizer)
                randomizer_destroy(q->randomizer);
            q->randomizer = g2_randomizer_create();

            q->hold_enabled = 0;
            q->max_floorkicks = 0;
            q->lock_protect = 1;
            q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags | Shiro::PDNOWKICK);
            q->tetromino_only = 1;
            q->pentomino_only = 0;
            cs->request_fps(Shiro::RefreshRates::g2);
            break;
        case Shiro::GameType::SIMULATE_G3:
            q->num_previews = 3;
            q->randomizer_type = RANDOMIZER_G3;
            if(q->randomizer)
                randomizer_destroy(q->randomizer);
            q->randomizer = g3_randomizer_create();

            q->hold_enabled = 1;
            q->max_floorkicks = 1;
            q->lock_protect = 1;
            q->piecepool[QRS_I4].flags = static_cast<Shiro::PieceDefinitionFlag>(q->piecepool[QRS_I4].flags & ~Shiro::PDNOWKICK);
            q->tetromino_only = 1;
            q->pentomino_only = 0;
            cs->request_fps(Shiro::RefreshRates::g3);
            break;
        default:
            break;
    }

    q->hold = NULL;

    if(menu_is_practice(cs->menu))
    {
        for(int menu_i = 0; menu_i < md->numopts; menu_i++)
        {
            Shiro::MenuOption *menu_opt = &md->menu[menu_i];
            if(menu_opt->type == Shiro::ElementType::MENU_TEXTINPUT && menu_opt->label == "RANDOMIZER SEED")
            {
                std::string text = ((Shiro::TextOptionData*)(menu_opt->data))->text;

                if(text.length() > 0)
                {
                    int rc = std::sscanf(text.c_str(), "%u", &q->pracdata->randomizer_seed);

                    if(rc == 1)
                    {
                        q->pracdata->using_seed = true;
                    }
                    else
                    {
                        q->pracdata->using_seed = false;
                    }
                }
                else
                {
                    q->pracdata->using_seed = false;
                }

                break;
            }
        }
    }

    // check to see if we need to update the usr_seq

    if(menu_is_practice(cs->menu))
    {
        for(int menu_i = 0; menu_i < md->numopts; menu_i++)
        {
            Shiro::MenuOption *menu_opt = &md->menu[menu_i];
            if(menu_opt->type == Shiro::ElementType::MENU_TEXTINPUT && menu_opt->label == "PIECE SEQUENCE")
            {
                std::string seqStr = ((Shiro::TextOptionData*)(menu_opt->data))->text;

                for(i = 0; i < seqStr.size(); i++)
                {
                    c = seqStr[i];
                    if((c < 'A' || c > 'Z') && !(c == '*' || c == '(' || c == ')'))
                    {
                        if(rpt_count)
                        {
                            k = 0;
                            while(k < 4 && i < seqStr.size() && seqStr[i] >= '0' && seqStr[i] <= '9')
                            {
                                rpt_count_strbuf[k] = seqStr[i];
                                rpt_count_strbuf[k + 1] = '\0';
                                i++;
                                k++;
                            }

                            i--;

                            num++;

                            if(k)
                            {
                                piece_seq[num - 1] = (strtol(rpt_count_strbuf, NULL, 10) & 1023);
                            }
                            else
                            {
                                piece_seq[num - 1] = 1;
                            }

                            rpt_count = 0;
                            continue;
                        }
                        else
                            continue;
                    }

                    if(rpt_count)
                    {
                        num++;

                        if(i < seqStr.size() - 1)
                        {
                            if(c == 'I' && seqStr[i + 1] == 'N' && seqStr[i + 2] == 'F')
                            {
                                piece_seq[num - 1] = SEQUENCE_REPEAT_INF;
                                goto end_sequence_proc;
                            }
                            else
                            {
                                piece_seq[num - 1] = 1;
                            }
                        }
                        else
                        {
                            piece_seq[num - 1] = 1;
                        }

                        rpt_count = 0;
                        continue;
                    }

                    if(c == '*')
                    {
                        if(rpt)
                        {
                            if(!rpt_start)
                            {
                                rpt_count = 1;
                                pre_rpt_count = 0;
                                rpt = false;
                                if(!(piece_seq[num - 1] & SEQUENCE_REPEAT_END))
                                    piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                            }

                            continue;
                        }
                        else
                        {
                            if(num > 1)
                            {
                                if(!(piece_seq[num - 2] & SEQUENCE_REPEAT_END))
                                {
                                    rpt_count = 1;
                                    pre_rpt_count = 0;
                                    if(!(piece_seq[num - 1] & SEQUENCE_REPEAT_END))
                                    {
                                        piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                                        piece_seq[num - 1] |= SEQUENCE_REPEAT_START;
                                    }
                                    continue;
                                }
                                else
                                    continue;
                            }
                            else if(num)
                            {
                                piece_seq[0] |= (SEQUENCE_REPEAT_START | SEQUENCE_REPEAT_END);
                                rpt_count = 1;
                                pre_rpt_count = 0;
                                continue;
                            }
                            else
                                continue;
                        }
                    }

                    if(c == '(')
                    {
                        if(rpt)
                            continue;

                        rpt_start = 1;
                        rpt = true;
                        continue;
                    }

                    if(c == ')')
                    {
                        if(!rpt)
                            continue;

                        if(num > 0)
                        {
                            piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                            pre_rpt_count = 1;
                        }
                        continue;
                    }

                    if(pre_rpt_count)
                    {
                        num++;
                        piece_seq[num - 1] = 1;
                        pre_rpt_count = 0;
                        i--;
                        continue;
                    }

                    name_str[0] = seqStr[i];

                    if(seqStr[i + 1] == '5')
                    {
                        name_str[1] = '5';
                        name_str[2] = '\0';

                        for(j = 0; j < 25; j++)
                        {
                            if(strcmp(name_str, get_qrspiece_name(j).c_str()) == 0)
                            {
                                t = j;
                                if(!q->tetromino_only)
                                {
                                    goto found;
                                }
                            }
                        }
                    }

                    name_str[1] = '\0';

                    for(j = 0; j < 25; j++)
                    {
                        if(strcmp(name_str, get_qrspiece_name(j).c_str()) == 0)
                        {
                            t = j;
                            if(q->tetromino_only)
                            {
                                switch(t)
                                {
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

                                if(t >= 18)
                                    goto found;
                            }
                            else if(q->pentomino_only)
                            {
                                switch(t)
                                {
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

                                if(t < 18)
                                    goto found;
                            }
                            else
                            {
                                goto found;
                            }
                        }
                    }

                    if(seqStr[i + 1] == 'a')
                    {
                        name_str[1] = 'a';
                        name_str[2] = '\0';

                        for(j = 0; j < 25; j++)
                        {
                            if(strcmp(name_str, get_qrspiece_name(j).c_str()) == 0)
                            {
                                t = j;
                                if(q->tetromino_only)
                                {
                                    switch(t)
                                    {
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

                                    if(t >= 18)
                                        goto found;
                                }
                                else if(q->pentomino_only)
                                {
                                    switch(t)
                                    {
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

                                    if(t < 18)
                                        goto found;
                                }
                                else
                                {
                                    goto found;
                                }
                            }
                        }
                    }
                    else if(seqStr[i + 1] == 'b')
                    {
                        name_str[1] = 'b';
                        name_str[2] = '\0';

                        for(j = 0; j < 25; j++)
                        {
                            if(strcmp(name_str, get_qrspiece_name(j).c_str()) == 0)
                            {
                                t = j;
                                if(q->tetromino_only)
                                {
                                    switch(t)
                                    {
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

                                    if(t >= 18)
                                        goto found;
                                }
                                else if(q->pentomino_only)
                                {
                                    switch(t)
                                    {
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

                                    if(t < 18)
                                        goto found;
                                }
                                else
                                {
                                    goto found;
                                }
                            }
                        }
                    }

                    continue;
                found:
                    num++;
                    piece_seq[num - 1] = (int)t;

                    if(rpt_start)
                    {
                        piece_seq[num - 1] |= SEQUENCE_REPEAT_START;
                        rpt_start = 0;
                    }
                    else if(rpt_end)
                    {
                        piece_seq[num - 1] |= SEQUENCE_REPEAT_END;
                        rpt_end = 0;
                    }
                }

                break;
            }
        }
    }

    if(rpt_count)
    {
        num++;
        piece_seq[num - 1] = 1;
    }

end_sequence_proc:
    for(i = 0; i < num; i++)
        d->usr_sequence[i] = piece_seq[i];

    d->usr_seq_len = num;
    d->usr_seq_expand_len = 0;

    /**/

    qrsfield_set_w(cs->p1game->field, q->field_w);
    qrsfield_set_w(&q->pracdata->usr_field, q->field_w);

    for(i = 0; i < d->usr_field_undo.size(); i++)
        qrsfield_set_w(&q->pracdata->usr_field_undo[i], q->field_w);

    for(i = 0; i < d->usr_field_redo.size(); i++)
        qrsfield_set_w(&q->pracdata->usr_field_redo[i], q->field_w);

    d->field_selection = 0;

    // process randomizer seed entry...

    // q->previews is expected to be cleared and re-allocated as needed

    q->previews.clear();

    if(q->pracdata->usr_seq_len)
    {
        for (std::size_t i = 0; i < 4; i++) {
            int elem = qs_get_usrseq_elem(d, i);
            if (elem != USRSEQ_ELEM_OOB) {
                q->previews.push_back(q->piecepool[elem]);
            }
        }
    }
    else
    {
        if(q->pracdata->using_seed)
        {
            q->randomizer->init(q->randomizer, &q->pracdata->randomizer_seed);
        }

        for (std::size_t i = 0; i < 4; i++)
        {
            piece_id piece = q->randomizer->lookahead(q->randomizer, static_cast<unsigned>(i + 1));

            if(q->randomizer->num_pieces == 7)
            {
                piece = ars_to_qrs_id(piece);
            }

            if(piece != PIECE_ID_INVALID)
            {
                q->previews.push_back(q->piecepool[piece]);
            }
        }
    }

    if(d->brackets)
        q->state_flags |= GAMESTATE_BRACKETS;
    else
        q->state_flags &= ~GAMESTATE_BRACKETS;

    if(d->invisible)
        q->state_flags |= GAMESTATE_INVISIBLE;
    else
        q->state_flags &= ~GAMESTATE_INVISIBLE;

    if(q->state_flags & GAMESTATE_BRACKETS)
    {
        for (auto& preview : q->previews) {
            preview.flags = static_cast<Shiro::PieceDefinitionFlag>(preview.flags | Shiro::PDBRACKETS);
        }
    }
    else
    {
        for (auto& preview : q->previews) {
            preview.flags = static_cast<Shiro::PieceDefinitionFlag>(preview.flags & ~Shiro::PDBRACKETS);
        }
    }

    if(q->pracdata->goal_level > 9999 || q->pracdata->goal_level < 0)
        q->pracdata->goal_level = 0;

    return 0;
}

// TODO: clean this function up, especially the parser + expander, and use more established terminology
int qs_get_usrseq_elem(pracdata *d, std::size_t index)
{
    int *seq = d->usr_sequence;
    int expand[4000];
    std::size_t expand_count = 0;

    std::size_t rpt_start = 0;
    std::size_t rpt_end = 0;
    std::size_t rpt_len = 0;
    std::size_t rpt_count = 0;
    bool rpt = false;
    std::size_t inf_rpt_len = 0;
    std::size_t inf_start = 0;

    int inf = 0;

    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t k = 0;
    int val = 0;
    int complex = 0;

    if(d->usr_seq_expand_len)
    {
        for(i = 0; i < d->usr_seq_expand_len; i++)
        {
            if(d->usr_seq_expand[i] & SEQUENCE_REPEAT_INF)
            {
                inf = 1;
                rpt_start = i;
                inf_rpt_len = d->usr_seq_expand_len - rpt_start;
            }
        }

        if(inf)
        {
            if(index < rpt_start)
            {
                val = d->usr_seq_expand[index] & 0b11111;
            }
            else
            {
                if(inf_rpt_len > 1)
                    index = (index - rpt_start) % inf_rpt_len;
                else
                    index = 0;

                val = d->usr_seq_expand[rpt_start + index] & 0b11111;
            }
        }
        else
        {
            if(index > d->usr_seq_expand_len - 1)
                val = USRSEQ_ELEM_OOB;
            else
                val = d->usr_seq_expand[index] & 0b11111;
        }

        return val;
    }

    if(!d->usr_seq_len)
    {
        d->usr_seq_expand_len = 0;
        return USRSEQ_ELEM_OOB;
    }

    for(i = 0; i < index && i < d->usr_seq_len; i++)
    {
        if(!IS_QRS_PIECE(seq[i]))
            complex = 1;
    }

    if(!complex && index < d->usr_seq_len)
    {
        if(IS_QRS_PIECE(seq[index]))
            return seq[index];
    }

    for(i = 0;; i++)
    {
        if(i >= d->usr_seq_len || expand_count > 3500)
        {
            // 3500 is an arbitrary limit to prevent taking up too much memory for little benefit
            break;
        }

        val = seq[i];
        if(rpt)
        {
            /* rpt = true implies we encountered the beginning of a grouped subsequence, where groups are assumed
            to have a rep count specified at the end */
            if(val & SEQUENCE_REPEAT_END || i == d->usr_seq_len - 1)
            {
                rpt = false;
                rpt_end = i;
                rpt_len = rpt_end - rpt_start + 1; // length of grouped subsequence
                i++;                               // next element of the sequence is either a repetition count or beyond the end of the sequence

                if(i != d->usr_seq_len && seq[i] == SEQUENCE_REPEAT_INF)
                {
                    // ^^ bound check must come first to prevent seq[i] from creating UB
                    inf = 1;
                    inf_rpt_len = rpt_len;
                    inf_start = rpt_start;
                    expand_count += rpt_len;
                    for(k = 0; k < rpt_len; k++)
                    {
                        /* seq[blah] & 0b11111 zeroes other flags on the piece;
                        first repeated element gets repeat_inf flag;
                        expanded sequence is cut off after the infinitely repeating group and this expansion loop ends */

                        expand[expand_count - rpt_len + k] = (seq[rpt_start + k] & 0b11111) | (!k ? SEQUENCE_REPEAT_INF : 0);
                    }

                    break;
                }
                else
                {
                    // dealing with finite repetition or open-ended grouped subsequence

                    if(i == d->usr_seq_len)
                        rpt_count = 1;
                    // ^^ force open-ended group to 1 repetition (same as if there was no grouping)

                    else
                    {
                        if(seq[i] > USRSEQ_RPTCOUNT_MAX) // last-minute bounding of repetition count to something reasonable
                            rpt_count = USRSEQ_RPTCOUNT_MAX;
                        else
                            rpt_count = seq[i];
                    }

                    for(k = 0; k < rpt_count; k++)
                    { // expand entire repetition to sequence of pieces with no special flags
                        for(j = 0; j < rpt_len; j++)
                        {
                            expand[expand_count + k * rpt_len + j] = seq[rpt_start + j] & 0b11111;
                        }
                    }

                    expand_count += rpt_count * rpt_len;
                    rpt = false; // we are done with this grouped subsequence
                }
            }
        }
        else
        {
            if(val & SEQUENCE_REPEAT_START)
            {
                rpt_start = i;
                rpt = true;
                if(val & SEQUENCE_REPEAT_END || i == d->usr_seq_len - 1)
                {
                    i--;      // hacky way for the loop to go into the if(rpt) branch and handle these edge cases
                    continue; // the i-- ensures the loop reads the additional flags on this element/doesn't go OOB
                }
            }
            else
            { // no expansion to be done
                expand_count++;
                expand[expand_count - 1] = seq[i] & 0b11111;
            }
        }
    }

    // end of expansion loop

    if(inf)
    {
        if(index < inf_start)
        { // index is of the piece we want to return to the function caller
            val = expand[index] & 0b11111;
        }
        else
        {
            if(inf_rpt_len > 1)
            {
                // collapse an arbitrarily large index value to a relative one that fits in the array bounds
                index = (index - inf_start) % inf_rpt_len;
            }
            else
                index = 0;

            val = expand[inf_start + index] & 0b11111;
        }
    }
    else
    {
        if(index < expand_count)
            val = expand[index] & 0b11111;
        else
            val = USRSEQ_ELEM_OOB;
    }

//     std::cerr << "Expanded sequence:" << std::endl;
//     for (i = 0; i < expand_count; i++) {
//         std::cerr << " " << expand[i];
//
//     }
//     std::cerr << std::endl;

    // updating the pracdata's expanded sequence field so we don't have to waste time expanding it later unless it changes
    for(i = 0; i < expand_count; i++)
        d->usr_seq_expand[i] = expand[i];

    d->usr_seq_expand_len = expand_count;

    return val;
}

// return value: 0 success, -1 invalid argument(s), 1 no next piece to deal to the player
int qs_initnext(game_t *g, qrs_player *p, unsigned int flags)
{
    if(!g || !p)
        return -1;

    CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)(g->data);
    struct randomizer *qrand = q->randomizer;

    piece_id t = 0;
    int rc = 0;

    q->lock_on_rotate = 0;
    q->p1counters->floorkicks = 0;
    q->soft_drop_counter = 0;
    q->sonic_drop_height = 0;
    q->active_piece_time = 0;
    q->placement_speed = 0;

    if(flags & INITNEXT_DURING_ACTIVE_PLAY)
    {
        // special check so we don't tell the game to terminate, even if there is no new piece to deal out
        // we don't want the game to terminate while the player is controlling a piece
        // edge case where this is relevant: player uses hold with no held piece, at last piece in user seq

        if(q->pracdata && q->pracdata->usr_seq_len)
        {
            if(qs_get_usrseq_elem(q->pracdata, static_cast<size_t>(q->pracdata->hist_index) + 1) == USRSEQ_ELEM_OOB)
            {
                return 1;
            }
        }
    }

    if(q->pracdata && q->pracdata->usr_seq_len)
    {
        q->pracdata->hist_index++;
        rc = qs_get_usrseq_elem(q->pracdata, q->pracdata->hist_index);

        if(rc != USRSEQ_ELEM_OOB)
            t = (piece_id)(rc & 0xff);
        else
            t = PIECE_ID_INVALID;
    }
    else
    {
        t = qrand->pull(qrand);
        if(q->randomizer->num_pieces == 7)
        {
            t = ars_to_qrs_id(t);
        }

        if((p->speeds->grav >= 512) && (t == QRS_X))
        {
            t = qrand->pull(qrand);
        }
    }

    if (p->def) {
        delete p->def;
        p->def = NULL;
    }

    if(q->previews.size() > 0)
        p->def = new Shiro::PieceDefinition(q->previews[0]);

    if(p->def)
        q->cur_piece_qrs_id = p->def->qrsID;
    else
        q->cur_piece_qrs_id = PIECE_ID_INVALID;

    if(q->previews.size() > 0)
    {
        auto previews = q->previews;
        q->previews.pop_back();
        for (size_t i = 0; i < previews.size() - 1; i++) {
            q->previews[i] = previews[i + 1];
        }
    }

    if (t != PIECE_ID_INVALID) {
        q->previews.push_back(q->piecepool[t]);
    }
//     std::cerr << "New piece definition to deal out: " << &q->previews[3] << " with ID " << t << std::endl;

    if(q->state_flags & GAMESTATE_BRACKETS)
    {
        if (q->previews.size() > 3) {
            q->previews[3].flags = static_cast<Shiro::PieceDefinitionFlag>(q->previews[3].flags | Shiro::PDBRACKETS);
        }
    }

    if(q->previews.size() > 0)
    {
        t = q->previews[0].qrsID;

        if(t != PIECE_ID_INVALID)
        {
            int ts = t;
            if(ts >= 18)
                ts -= 18;
            /*Shiro::Sfx* sfx = cs->assets->pieces[ts % 7];
            sfx->play(cs->settings);
            */
            Shiro::SfxAsset::get(cs->assetMgr, "pieces", ts % 7).play(cs->settings);
        }
    }

    if(q->cur_piece_qrs_id == PIECE_ID_INVALID || !p->def)
    {
        // pause the game if we can't deal a new piece
        return QSGAME_SHOULD_TERMINATE;
    }

    if(q->cur_piece_qrs_id >= 18)
    {
        p->y = ROWTOY(SPAWNY_QRS + 4);
    }
    else
    {
        p->y = ROWTOY(SPAWNY_QRS);
    }

    if(q->state_flags & GAMESTATE_BIGMODE)
    {
        p->x = 2;
    }
    else
    {
        p->x = SPAWNX_QRS;
    }

    p->old_xs[0] = p->x;
    p->old_ys[0] = p->y;

    for(int i = 1; i < p->num_olds; i++)
    {
        p->old_xs[i] = p->old_xs[i - 1];
        p->old_ys[i] = p->old_ys[i - 1];
    }

    p->orient = Shiro::FLAT;
    p->state = PSFALL | PSSPAWN;

    q->p1counters->lock = 0;

    q->pieces_dealt++;

    return 0;
}