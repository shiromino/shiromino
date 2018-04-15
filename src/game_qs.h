#ifndef _game_qs_h
#define _game_qs_h

#include "core.h"
#include "qrs.h"

#define QS_CURVE_MAX 19
#define G1_MASTER_CURVE_MAX 30
#define G2_MASTER_CURVE_MAX 35
#define G2_DEATH_CURVE_MAX 9
#define G3_TERROR_CURVE_MAX 9

#define MODE_PENTOMINO 0

#define NIGHTMARE_MODE 0x0001
#define PENTOMINO_ONLY 0x0002
#define TETROMINO_ONLY 0x0004
#define NO_LOCK_DELAY  0x0008

// these are defined in core.h now
// #define SIMULATE_G1    0x0010
// #define SIMULATE_G2    0x0020
// #define SIMULATE_G3    0x0040

// mode IDs
#define QRS_PRACTICE   0x0080
#define MODE_G2_DEATH  0x0100
#define MODE_G3_TERROR 0x0200
#define MODE_G1_MASTER 0x0400
#define MODE_G1_20G    0x0800
#define MODE_G2_MASTER 0x1000
// should add "variation" codes that change mode behavior (unique code interpretations for each mode maybe)

#define RANDOMIZER_NORMAL 0
#define RANDOMIZER_NIGHTMARE 1
#define RANDOMIZER_G1 2
#define RANDOMIZER_G2 3
#define RANDOMIZER_G3 4

#define GRADE_9 0
#define GRADE_8 1
#define GRADE_7 2
#define GRADE_6 3
#define GRADE_5 4
#define GRADE_4 5
#define GRADE_3 6
#define GRADE_2 7
#define GRADE_1 8
#define GRADE_S1 9
#define GRADE_S2 10
#define GRADE_S3 11
#define GRADE_S4 12
#define GRADE_S5 13
#define GRADE_S6 14
#define GRADE_S7 15
#define GRADE_S8 16
#define GRADE_S9 17
#define GRADE_S10 18
#define GRADE_S11 19
#define GRADE_S12 20
#define GRADE_S13 21
#define GRADE_M1 22
#define GRADE_M2 23
#define GRADE_M3 24
#define GRADE_M4 25
#define GRADE_M5 26
#define GRADE_M6 27
#define GRADE_M7 28
#define GRADE_M8 29
#define GRADE_M9 30
#define GRADE_M 31
#define GRADE_MK 32
#define GRADE_MV 33
#define GRADE_MO 34
#define GRADE_MM 35
#define GRADE_GM 36

#define NO_GRADE 255

#define GREEN_LINE 0x100
#define ORANGE_LINE 0x200

#define INTERNAL_GRADE_9        0
#define INTERNAL_GRADE_8        1
#define INTERNAL_GRADE_7        2
#define INTERNAL_GRADE_6        3
#define INTERNAL_GRADE_5        4
#define INTERNAL_GRADE_4        5
#define INTERNAL_GRADE_4_PLUS   6
#define INTERNAL_GRADE_3        7
#define INTERNAL_GRADE_3_PLUS   8
#define INTERNAL_GRADE_2_MINUS  9
#define INTERNAL_GRADE_2        10
#define INTERNAL_GRADE_2_PLUS   11
#define INTERNAL_GRADE_1_MINUS  12
#define INTERNAL_GRADE_1        13
#define INTERNAL_GRADE_1_PLUS   14
#define INTERNAL_GRADE_S1_MINUS 15
#define INTERNAL_GRADE_S1       16
#define INTERNAL_GRADE_S1_PLUS  17
#define INTERNAL_GRADE_S2       18
#define INTERNAL_GRADE_S3       19
#define INTERNAL_GRADE_S4_MINUS 20
#define INTERNAL_GRADE_S4       21
#define INTERNAL_GRADE_S4_PLUS  22
#define INTERNAL_GRADE_S5       23
#define INTERNAL_GRADE_S5_PLUS  24
#define INTERNAL_GRADE_S6       25
#define INTERNAL_GRADE_S6_PLUS  26
#define INTERNAL_GRADE_S7       27
#define INTERNAL_GRADE_S7_PLUS  28
#define INTERNAL_GRADE_S8       29
#define INTERNAL_GRADE_S8_PLUS  30
#define INTERNAL_GRADE_S9       31

#define G2_DEATH_TORIKAN (3*60*60 + 25*60)
#define G3_TERROR_TORIKAN (2*60*60 + 28*60)

#define AVG_FIRST_FIVE(T) ((T[0] + T[1] + T[2] + T[3] + T[4]) / 5)

#define USRSEQ_ELEM_OOB -1
#define USRSEQ_RPTCOUNT_MAX 1023

#define QSGAME_SHOULD_TERMINATE 2

#define INITNEXT_DURING_ACTIVE_PLAY 0x0001

const char *get_grade_name(int grade);
const char *get_internal_grade_name(int index);
int internal_to_displayed_grade(int internal_grade);

game_t *qs_game_create(coreState *cs, int level, unsigned int flags, int replay_id);
int qs_game_init(game_t *g);
int qs_game_pracinit(game_t *g, int val);
int qs_game_quit(game_t *g);
int qs_game_preframe(game_t *g);
int qs_game_frame(game_t *g);

int qs_process_are(game_t *g);
int qs_process_prelockflash(game_t *g);
int qs_process_fall(game_t *g);
int qs_process_lock(game_t *g);
int qs_process_lockflash(game_t *g);
int qs_process_lineclear(game_t *g);
int qs_process_lineare(game_t *g);

int qrs_game_is_inactive(coreState *cs);
int qs_update_pracdata(coreState *cs);
int qs_get_usrseq_elem(struct pracdata *d, int index);

int qs_initnext(game_t *g, qrs_player *p, unsigned int flags);
int qs_init_randomize(game_t *g);
int qs_randomize(game_t *g);

int qs_normal_randomize(int *history, long double *weights);
int qs_g2_randomize(int *history);
int qs_g1_randomize(int *history);

#endif
