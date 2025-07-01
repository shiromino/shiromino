#pragma once
#include "QRS0.h"
#include <stdint.h>

#define RNGSTATE(STR, SEED) ((uint64_t)(STR[0]) << 56
#define RNGSTATE_STRLEN 14

#define PENTO_READ_RAND_MAX 0x7fff

#define PENTO_RAND_NOTETS 0x01
#define PENTO_RAND_NIGHTMARE 0x02
#define PENTO_RAND_RANKED 0x04

#define QRS_I_WEIGHT 1.2
#define QRS_J_WEIGHT 0.8
#define QRS_L_WEIGHT 0.8
#define QRS_X_WEIGHT 0.18
#define QRS_S_WEIGHT 0.4
#define QRS_Z_WEIGHT 0.4
#define QRS_N_WEIGHT 0.9
#define QRS_G_WEIGHT 0.9
#define QRS_U_WEIGHT 0.8
#define QRS_T_WEIGHT 0.55
#define QRS_Fa_WEIGHT 0.26
#define QRS_Fb_WEIGHT 0.26
#define QRS_P_WEIGHT 1.3
#define QRS_Q_WEIGHT 1.3
#define QRS_W_WEIGHT 0.45
#define QRS_Ya_WEIGHT 0.75
#define QRS_Yb_WEIGHT 0.75
#define QRS_V_WEIGHT 0.4

#define ARS_I_WEIGHT 1.7
#define ARS_T_WEIGHT 1.9
#define ARS_J_WEIGHT 1.7
#define ARS_L_WEIGHT 1.7
#define ARS_O_WEIGHT 1.6
#define ARS_S_WEIGHT 1.7
#define ARS_Z_WEIGHT 1.7

#define QRS_DROUGHT_BASELINE 16.0
#define QRS_DROUGHT_COEFF_DEFAULT 1.1
// threshold: weights below this value are considered "bad" pieces and have collective flood protection
#define QRS_WEIGHT_TOPTIER_THRESHOLD 1.0
#define QRS_WEIGHT_HIGHTIER_THRESHOLD 0.45
#define QRS_WEIGHT_MIDTIER_THRESHOLD 0.25

#define QRS_DROUGHT_HIGHTIER_SOFTLIMIT 20
#define QRS_DROUGHT_MIDTIER_SOFTLIMIT 42
#define QRS_DROUGHT_LOWTIER_SOFTLIMIT -1

#define QRS_I_DROUGHT_COEFF 4.0
#define QRS_L_DROUGHT_COEFF 1.3
#define QRS_J_DROUGHT_COEFF 1.3
#define QRS_X_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_S_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_Z_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_N_DROUGHT_COEFF 1.1
#define QRS_G_DROUGHT_COEFF 1.1
#define QRS_U_DROUGHT_COEFF 1.5
#define QRS_T_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_Fa_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_Fb_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_P_DROUGHT_COEFF 2.5
#define QRS_Q_DROUGHT_COEFF 2.5
#define QRS_W_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_Ya_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_Yb_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT
#define QRS_V_DROUGHT_COEFF QRS_DROUGHT_COEFF_DEFAULT

#define ARS_I_DROUGHT_COEFF 5.0
#define ARS_T_DROUGHT_COEFF 3.5
#define ARS_J_DROUGHT_COEFF 3.0
#define ARS_L_DROUGHT_COEFF 3.0
#define ARS_O_DROUGHT_COEFF 3.0
#define ARS_S_DROUGHT_COEFF 2.5
#define ARS_Z_DROUGHT_COEFF 2.5

enum { HISTRAND, G3RAND };

typedef uint32_t seed32_t;
typedef uint64_t seed64_t;

struct randomizer
{
    unsigned int num_pieces;
    uint32_t *seedp;
    int type;

    // TODO: implement PCG and make a read_rand() archetype to put here
    // this way we don't need to read r->type (to determine which read_rand() to use)

    int (*init)(struct randomizer *, uint32_t *);
    piece_id (*pull)(struct randomizer *); // pop + push
    piece_id (*lookahead)(struct randomizer *, unsigned int);

    void *data;
};

struct histrand_data
{
    unsigned int hist_len;
    unsigned int rerolls;

    // if hist_len is 0 this is NULL
    piece_id *history;

    // range from 0.0 to 100.0
    double difficulty;

    // these three are not guaranteed to be filled in (set to NULL if not used)
    double *piece_weights;
    double *drought_protection_coefficients;
    unsigned int *drought_times;
};

// sakura_seq will be handled elsewhere as a non-randomizer-related piece_seq
struct g3rand_data
{
    piece_id history[4];
    piece_id bag[35];
    int histogram[7];
};

piece_id ars_to_qrs_id(piece_id t);

int seeds_are_close(uint32_t s1, uint32_t s2, unsigned int max_gap);
int seed_is_after(uint32_t b, uint32_t a, unsigned int max_gap);
void g123_seeds_init();

// Only call preupdate before processing a non-gameplay frame, then call update
// after processing. By spacing the two function calls that way, a somewhat
// random amount of time will pass between them, which is used for updating the
// seeds.
//
// Don't *EVER* call these during gameplay! The game manages the seeds, and
// would break if other code changed them while a game is going.
// -Brandon McGriff
void g123_seeds_preupdate();
void g123_seeds_update();

// TODO NULL-proof the functions which take struct randomizer * parameters

// _create functions fill in all fields; piece_id arrays are filled with PIECE_ID_INVALID
struct randomizer *g1_randomizer_create();
struct randomizer *g2_randomizer_create();
struct randomizer *g3_randomizer_create();
struct randomizer *pento_randomizer_create();
void randomizer_destroy(struct randomizer *r);

/* _init functions prepare a randomizer for the beginning of a new game
   i.e. they generate the first piece and move it to the beginning of the
   history (if applicable), and fill it in completely */
int g1_randomizer_init(struct randomizer *r, uint32_t *seed);
int g2_randomizer_init(struct randomizer *r, uint32_t *seed);
int g3_randomizer_init(struct randomizer *r, uint32_t *seed);
int pento_randomizer_init(struct randomizer *r, uint32_t *seed);

// helpers
void history_push(piece_id *history, unsigned int hist_len, piece_id t);
piece_id history_pop(piece_id *history);
bool in_history(piece_id *history, unsigned int hist_len, piece_id t);
piece_id g3_most_droughted_piece(int *histogram);

piece_id histrand_pull(struct randomizer *r);
piece_id histrand_get_next(struct randomizer *r);
piece_id histrand_lookahead(struct randomizer *r, unsigned int distance);
double histrand_get_difficulty(struct randomizer *r);
int histrand_set_difficulty(struct randomizer *r, double difficulty);

piece_id g3rand_pull(struct randomizer *r);
piece_id g3rand_get_next(struct randomizer *r);
piece_id g3rand_lookahead(struct randomizer *r, unsigned int distance);

uint32_t g2_get_seed();
uint32_t g2_seed_rand(uint32_t seed);
uint32_t g2_seed_bkp();
uint32_t g2_seed_restore();

uint32_t g2_rand(uint32_t n);
uint32_t g2_rand_rep(uint32_t n, uint32_t reps);
uint32_t g2_unrand_rep(uint32_t n, uint32_t reps);
uint32_t g2_unrand(uint32_t n);

uint32_t g123_read_rand(uint32_t *seedp);
uint32_t pento_read_rand(uint32_t *seedp);
piece_id g123_get_init_piece(uint32_t *seedp);