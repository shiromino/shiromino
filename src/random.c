#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "qrs.h"
#include "random.h"

static long double pento_piece_weights[25] =
{
    QRS_I_WEIGHT,
    QRS_J_WEIGHT,
    QRS_L_WEIGHT,
    QRS_X_WEIGHT,
    QRS_S_WEIGHT,
    QRS_Z_WEIGHT,
    QRS_N_WEIGHT,
    QRS_G_WEIGHT,
    QRS_U_WEIGHT,
    QRS_T_WEIGHT,
    QRS_Fa_WEIGHT,
    QRS_Fb_WEIGHT,
    QRS_P_WEIGHT,
    QRS_Q_WEIGHT,
    QRS_W_WEIGHT,
    QRS_Ya_WEIGHT,
    QRS_Yb_WEIGHT,
    QRS_V_WEIGHT,

    ARS_I_WEIGHT,
    ARS_T_WEIGHT,
    ARS_J_WEIGHT,
    ARS_L_WEIGHT,
    ARS_O_WEIGHT,
    ARS_S_WEIGHT,
    ARS_Z_WEIGHT
};

static long double pento_drought_coeffs[25] =
{
    QRS_I_DROUGHT_COEFF,
    QRS_L_DROUGHT_COEFF,
    QRS_J_DROUGHT_COEFF,
    QRS_X_DROUGHT_COEFF,
    QRS_S_DROUGHT_COEFF,
    QRS_Z_DROUGHT_COEFF,
    QRS_N_DROUGHT_COEFF,
    QRS_G_DROUGHT_COEFF,
    QRS_U_DROUGHT_COEFF,
    QRS_T_DROUGHT_COEFF,
    QRS_Fa_DROUGHT_COEFF,
    QRS_Fb_DROUGHT_COEFF,
    QRS_P_DROUGHT_COEFF,
    QRS_Q_DROUGHT_COEFF,
    QRS_W_DROUGHT_COEFF,
    QRS_Ya_DROUGHT_COEFF,
    QRS_Yb_DROUGHT_COEFF,
    QRS_V_DROUGHT_COEFF,

    ARS_I_DROUGHT_COEFF,
    ARS_T_DROUGHT_COEFF,
    ARS_J_DROUGHT_COEFF,
    ARS_L_DROUGHT_COEFF,
    ARS_O_DROUGHT_COEFF,
    ARS_S_DROUGHT_COEFF,
    ARS_Z_DROUGHT_COEFF
};

uint32_t g1_seed = 0;
uint32_t g1_bkp_seed = 0;

uint32_t g2_seed = 0;
uint32_t g2_bkp_seed = 0;

uint32_t g3_seed = 0;
uint32_t g3_bkp_seed = 0;

uint32_t pento_seed = 0;
uint32_t pento_bkp_seed = 0;

//piece_id g3_bag[35];
//piece_id *sakura_seq; TODO

piece_id ars_to_qrs_id(piece_id t)
{
    switch(t) {
        case 0:
            break;
        case 1:
            t = 6;
            break;
        case 2:
            t = 5;
            break;
        case 3:
            t = 2;
            break;
        case 4:
            t = 3;
            break;
        case 5:
            t = 4;
            break;
        case 6:
            t = 1;
            break;
        default:
            break;
    }

    return t + 18;
}

void g123_seeds_init()
{
    uint32_t s = time(0);
    srand(s);

    s = rand();
    g1_seed = g2_rand((s >> 12) % 23890);

    s = rand();
    g2_seed = g2_rand((s << 5) % 17622);

    s = rand();
    g3_seed = g2_rand((s << 8) % 8997);

    s = rand();
    pento_seed = g2_rand((s << 4) % 11456);
}

/*
int seeds_are_close(uint32_t s1, uint32_t s2, unsigned int max_gap)
{
    unsigned int i = 0;
    uint32_t s1_tmp_forward = s1;
    uint32_t s1_tmp_backward = s1;

    for(i = 0; i < max_gap; i++) {
        s1_tmp_forward = g2_rand(s1_tmp_forward);
        if(s1_tmp_forward == s2)
            return 1;

        s1_tmp_backward = g2_unrand(s1_tmp_backward);
        if(s1_tmp_backward == s2)
            return 1;
    }

    return 0;
}

int seed_is_after(uint32_t b, uint32_t a, unsigned int max_gap)
{
    unsigned int i = 0;
    for(i = 0; i < max_gap; i++) {
        a = g2_rand(a);
        if(a == b)
            return 1;
    }

    return 0;
}
*/

/*
char *sprintf_rngstate(char *strbuf, rngstate s)
{
    sprintf(strbuf, "%d%d%d%d,%lx", state_hN(s, 0), state_hN(s, 1), state_hN(s, 2), state_hN(s, 3), (long unsigned int)(s & 0xffffffff));
    return strbuf;
}
*/

/* */

struct randomizer *g1_randomizer_create(uint32_t flags)
{
    struct randomizer *r = malloc(sizeof(struct randomizer));
    struct histrand_data *d = NULL;

    r->num_pieces = 7;
    r->seedp = &g1_seed;
    r->type = HISTRAND;

    r->init = g1_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    d = r->data;

    d->hist_len = 4;
    d->rerolls = 3;
    d->history = malloc(d->hist_len * sizeof(piece_id));
    d->history[0] = PIECE_ID_INVALID;
    d->history[1] = PIECE_ID_INVALID;
    d->history[2] = PIECE_ID_INVALID;
    d->history[3] = PIECE_ID_INVALID;

    d->difficulty = 0.0;

    d->piece_weights = NULL;
    d->drought_protection_coefficients = NULL;
    d->drought_times = NULL;

    return r;
}

struct randomizer *g2_randomizer_create(uint32_t flags)
{
    struct randomizer *r = malloc(sizeof(struct randomizer));
    struct histrand_data *d = NULL;

    r->num_pieces = 7;
    r->seedp = &g2_seed;
    r->type = HISTRAND;

    r->init = g2_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    d = r->data;

    d->hist_len = 4;
    d->rerolls = 5;
    d->history = malloc(d->hist_len * sizeof(piece_id));
    d->history[0] = PIECE_ID_INVALID;
    d->history[1] = PIECE_ID_INVALID;
    d->history[2] = PIECE_ID_INVALID;
    d->history[3] = PIECE_ID_INVALID;

    d->difficulty = 0.0;

    d->piece_weights = NULL;
    d->drought_protection_coefficients = NULL;
    d->drought_times = NULL;

    return r;
}

struct randomizer *g3_randomizer_create(uint32_t flags)
{
    struct randomizer *r = malloc(sizeof(struct randomizer));
    struct g3rand_data *d = NULL;
    int i = 0;

    r->num_pieces = 7;
    r->seedp = &g3_seed;
    r->type = G3RAND;

    r->init = g3_randomizer_init;
    r->pull = g3rand_pull;
    r->lookahead = g3rand_lookahead;

    r->data = malloc(sizeof(struct g3rand_data));
    d = r->data;

    for(i = 0; i < 4; i++)
        d->history[i] = PIECE_ID_INVALID;

    for(i = 0; i < 35; i++)
        d->bag[i] = PIECE_ID_INVALID;

    for(i = 0; i < 7; i++)
        d->histogram[0] = 0;

    return r;
}

struct randomizer *pento_randomizer_create(uint32_t flags)
{
    struct randomizer *r = malloc(sizeof(struct randomizer));
    struct histrand_data *d = NULL;
    unsigned int i = 0;

    r->num_pieces = 25;
    r->seedp = &pento_seed;
    r->type = HISTRAND;

    r->init = pento_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    d = r->data;

    d->hist_len = 6;
    d->rerolls = 0;
    d->history = malloc(d->hist_len * sizeof(piece_id));
    d->history[0] = PIECE_ID_INVALID;
    d->history[1] = PIECE_ID_INVALID;
    d->history[2] = PIECE_ID_INVALID;
    d->history[3] = PIECE_ID_INVALID;
    d->history[4] = PIECE_ID_INVALID;
    d->history[5] = PIECE_ID_INVALID;

    d->difficulty = 0.0;

    d->piece_weights = malloc(r->num_pieces * sizeof(double));
    d->drought_protection_coefficients = malloc(r->num_pieces * sizeof(double));
    d->drought_times = malloc(r->num_pieces * sizeof(unsigned int));

    for(i = 0; i < r->num_pieces; i++) {
        d->piece_weights[i] = pento_piece_weights[i];
        d->drought_protection_coefficients[i] = pento_drought_coeffs[i];
        d->drought_times[i] = 0;
    }

    return r;
}

void randomizer_destroy(struct randomizer *r)
{
    if(!r)
        return;

    struct histrand_data *d = NULL;

    switch(r->type) {
        case HISTRAND:
            d = r->data;

            if(d->history) free(d->history);
            if(d->piece_weights) free(d->piece_weights);
            if(d->drought_protection_coefficients) free(d->drought_protection_coefficients);
            if(d->drought_times) free(d->drought_times);

            free(r->data);
            break;

        case G3RAND:
            free(r->data);
            break;

        default:
            // try to free it even if r->type has not been set properly
            free(r->data);
            break;
    }

    free(r);
}

// ------ //

int g1_randomizer_init(struct randomizer *r, uint32_t *seed)
{
    struct histrand_data *d = r->data;
    int i = 0;
    int num_generated = 0;

    if(seed) {
        g1_bkp_seed = g1_seed;
        g1_seed = *seed;
    }

    d->history[0] = ARS_Z;
    d->history[1] = ARS_Z;
    d->history[2] = ARS_Z;
    d->history[3] = g123_get_init_piece(&g1_seed);
    num_generated = 1;

    for(i = 0; i < 3; i++) // move init piece to history[0] (first preview/next piece) and fill in 3 pieces ahead
    {
        histrand_pull(r);
        num_generated++;
    }

    return num_generated;
}

int g2_randomizer_init(struct randomizer *r, uint32_t *seed)
{
    struct histrand_data *d = r->data;
    int i = 0;
    int num_generated = 0;

    if(seed) {
        g2_bkp_seed = g2_seed;
        g2_seed = *seed;
    }

    d->history[0] = ARS_S;
    d->history[1] = ARS_S;
    d->history[2] = ARS_Z;
    d->history[3] = g123_get_init_piece(&g2_seed);
    num_generated = 1;

    for(i = 0; i < 3; i++)
    {
        // move init piece to history[0] (first preview/next piece) and fill in 3 pieces ahead
        // the S,S,Z get pulled into the void
        histrand_pull(r);
        num_generated++;
    }

    return num_generated;
}

int g3_randomizer_init(struct randomizer *r, uint32_t *seed)
{
    struct g3rand_data *d = r->data;
    int i = 0;
    int num_generated = 0;

    if(seed) {
        g3_bkp_seed = g3_seed;
        g3_seed = *seed;
    }

    for(i = 0; i < 35; i++)
      d->bag[i] = i/5;

    d->history[0] = ARS_S;
    d->history[1] = ARS_S;
    d->history[2] = ARS_Z;
    d->history[3] = g123_get_init_piece(r->seedp);
    num_generated = 1;

    for(i = 0; i < 3; i++)
    {
        // move init piece to history[0] (first preview/next piece) and fill in 3 pieces ahead
        // the S,S,Z get pulled into the void
        g3rand_pull(r);
        num_generated++;
    }

    return num_generated;
}

int pento_randomizer_init(struct randomizer *r, uint32_t *seed)
{
    struct histrand_data *d = r->data;
    int t = 0;
    int i = 0;
    int num_generated = 0;

    if(seed) {
        pento_bkp_seed = pento_seed;
        pento_seed = *seed;
    }

    d->history[0] = QRS_Fb;
    d->history[1] = QRS_Fa;
    d->history[2] = QRS_X;
    d->history[3] = QRS_S;
    d->history[4] = QRS_Z;

    t = pento_read_rand(r->seedp) % 5;

    switch(t) {
        case 0:
            d->history[5] = QRS_I;
            break;
        case 1:
            d->history[5] = QRS_J;
            break;
        case 2:
            d->history[5] = QRS_L;
            break;
        case 3:
            d->history[5] = QRS_Ya;
            break;
        case 4:
            d->history[5] = QRS_Yb;
            break;
        default:
            d->history[5] = QRS_I;
            break;
    }

    num_generated = 1;

    for(i = 0; i < 5; i++) {
        histrand_pull(r); // Fb, Fa, X, S, Z get pulled into the void
        num_generated++;
    }

    return num_generated;
}

// ----- //

void history_push(piece_id *history, unsigned int hist_len, piece_id t)
{
    unsigned int i = 0;

    for(i = 0; i < hist_len - 1; i++) {
        history[i] = history[i+1];
    }

    history[hist_len - 1] = t;
}

piece_id history_pop(piece_id *history)
{
    piece_id t = history[0];
    history[0] = PIECE_ID_INVALID;

    return t;
}

bool in_history(piece_id *history, unsigned int hist_len, piece_id t)
{
    unsigned int i = 0;

    for(i = 0; i < hist_len; i++) {
        if(history[i] == t)
            return true;
    }

    return false;
}

piece_id histrand_pull(struct randomizer *r)
{
    struct histrand_data *d = r->data;
    piece_id t = histrand_get_next(r);
    piece_id result = history_pop(d->history);
    unsigned int i = 0;

    history_push(d->history, d->hist_len, t);

    if(d->drought_times) {  // update drought times if we are using them
        for(i = 0; i < r->num_pieces; i++) {
            if(i == t) d->drought_times[i] = 0;
            else d->drought_times[i]++;
        }
    }

    return result;
}

piece_id g3rand_pull(struct randomizer *r)
{
    struct g3rand_data *d = r->data;
    piece_id t = g3rand_get_next(r);
    piece_id piece = history_pop(d->history);

   history_push(d->history, 4, t);

   return piece;
}

piece_id histrand_get_next(struct randomizer *r)
{
    struct histrand_data *d = r->data;
    uint32_t *seedp = r->seedp;
    unsigned int i = 0;
    unsigned int j = 0;
    bool in_hist = false;
    piece_id t = PIECE_ID_INVALID;

    long double p = 0.0;
    double old_sum = 0.0;
    double sum = 0.0;
    unsigned int *histogram = malloc(r->num_pieces * sizeof(unsigned int));
    double *temp_weights = malloc(r->num_pieces * sizeof(double));

    if(!seedp)
        seedp = &g2_seed;

    if(!d->piece_weights) // if we are using rerolls and not weighted calculation
    {
        for(i = 0; i < d->rerolls; i++) {
            t = g123_read_rand(seedp) % 7;

            in_hist = 0;
            for(j = 0; j < d->hist_len; j++) {
                if(d->history[j] == t) 
                    in_hist = true;
            }

            if(!in_hist)
                break;

            t = g123_read_rand(seedp) % 7;
        }

        free(temp_weights);
        free(histogram);

        return t;
    }

    else
    {
        // starts at 1 and counts up, bad pieces' weights are divided by this
        int below_threshold = 1;

        for(i = 0; i < r->num_pieces; i++) {
            temp_weights[i] = d->piece_weights[i];
            // histogram values are all offset by one from how many times the piece is actually in the history
            histogram[i] = 1;

            for(j = 0; j < d->hist_len; j++) {
                if(d->history[j] == i) {
                    histogram[i]++;
                    if(d->piece_weights[i] < QRS_WEIGHT_TIER_THRESHOLD)
                        below_threshold++;
                }
            }

            temp_weights[i] /= (double)(histogram[i] * histogram[i]); // OLD: * (i < 18 ? pieces[i] : 1)
            if(d->drought_protection_coefficients && d->drought_times) {
                // multiply by coefficient^(t/BASELINE)
                // e.g. for coeff 2 and drought time BASELINE, weight *= 2
                temp_weights[i] *= pow(d->drought_protection_coefficients[i],
                                       (double)(d->drought_times[i]) / QRS_DROUGHT_BASELINE );
            }
        }

        for(i = 0; i < r->num_pieces; i++) {
            if(d->piece_weights[i] < QRS_WEIGHT_TIER_THRESHOLD) {
                temp_weights[i] /= (double)(below_threshold);
            }

            sum += temp_weights[i];
        }

        if(d->difficulty > 0.0)
        {
            old_sum = sum;
            sum = 0.0;

            for(i = 0; i < r->num_pieces; i++) {
                // final factor: difficulty, which brings weights closer to being equal to each other
                // takes the difference from the average and multiplies by difficulty/100, then adds that to the weight
                // note that if the weight was above average, a negative value is added
                temp_weights[i] += (d->difficulty / 100.0) * ((old_sum / r->num_pieces) - temp_weights[i]);

                sum += temp_weights[i];
            }
        }

        p = (long double)(pento_read_rand(seedp)) / (long double)(PENTO_READ_RAND_MAX);
        // produce value between 0 and the sum of weights
        // each weight is like an segment of this interval, and p is a point in the interval,
        // so p falls within one of the segments (each of which corresponds to a piece)
        p = p * (long double)(sum);
        sum = 0.0;

        for(i = 0; i < r->num_pieces; i++)
        {
            // find which segment p is in
            if(p >= (long double)(sum) && p < (long double)(sum + temp_weights[i]) ) {
                return i;
            } else sum += temp_weights[i];
        }

        free(temp_weights);
        free(histogram);

        return 0;
    }

    // fallback piece_id return value, just to be safe
    return 0;
}

double histrand_get_difficulty(struct randomizer *r)
{
    struct histrand_data *d = r->data;

    return d->difficulty;
}

int histrand_set_difficulty(struct randomizer *r, double difficulty)
{
    if(difficulty < 0.0 || difficulty > 100.0)
        return 1;

    struct histrand_data *d = r->data;
    d->difficulty = difficulty;

    return 0;
}

piece_id g3rand_get_next(struct randomizer *r)
{
    struct g3rand_data *d = r->data;

    piece_id piece = PIECE_ID_INVALID;
    unsigned int bagpos = 0;
    int i = 0;

   for(i = 0; i < 6; i++)
   {
      bagpos = g123_read_rand(r->seedp) % 35;
      piece = d->bag[bagpos];

      // piece is not in the history, this is fine
      if(!in_history(d->history, 4, piece))
         break;

      // piece is already in the history, churn the bag a little and reroll
      d->bag[bagpos] = g3_most_droughted_piece(d->histogram);

      // We might be about to fall out of the loop, pick something at random
      bagpos = g123_read_rand(r->seedp) % 35;
      piece = d->bag[bagpos];
   }

   // make the pieces we didn't pick more likely in future, and this piece less
    for(i = 0; i < 7; i++) {
        if(i == (int)(piece))
            d->histogram[i] = 0;
        else
            d->histogram[i]++;
    }

   // piece has finally be chosen, histogram is up to date, put the rarest piece back into the bag and return.
   d->bag[bagpos] = g3_most_droughted_piece(d->histogram);

   return piece;
}

piece_id histrand_lookahead(struct randomizer *r, unsigned int distance)
{
    struct histrand_data *d = r->data;

    if(distance > d->hist_len || distance == 0)
        return PIECE_ID_INVALID;

    return d->history[distance - 1];
}

piece_id g3rand_lookahead(struct randomizer *r, unsigned int distance)
{
    struct g3rand_data *d = r->data;

    if(distance > 4 || distance == 0)
        return PIECE_ID_INVALID;

    return d->history[distance - 1];
}

//
/* */
//

/*
uint32_t g2_state_read_rand(rngstate s)
{
    uint32_t seed = s;
    return (g2_rand(seed) >> 10) & 0x7fff;
}

rngstate g2_state_rand(rngstate s)
{
    uint32_t oldseed = s & 0xffffffff;

    uint64_t seed = g2_rand(oldseed);
    uint64_t hist_only = (s >> 32) << 32;

    return hist_only | seed;
}

uint8_t state_hN(rngstate s, uint8_t n)
{
    n &= 3;
    return (s >> (56 - 8*n));
}

rngstate state_set_hN(rngstate s, uint8_t n, uint8_t piece)
{
    rngstate new = 0;

    piece = piece % 7;
    n = n % 4;

    uint64_t piecemask = (uint64_t)(piece) << (56 - 8*n);
    uint64_t mask = 0xffffffffffffffff;

    mask ^= (uint64_t)(0xff) << (56 - 8*n);
    new = (s & mask) | piecemask;

    return new;
}

// TODO

rngstate state_push(rngstate s, uint8_t piece)
{
    return s;
}

rngstate from_hist_seed(char *history_str, uint32_t seed)
{
    uint64_t seed_ = (uint64_t)(seed);
    uint64_t history[4];
    uint64_t vals[5];
    uint64_t result = 0;
    int i = 0;

    for(i = 0; i < 4; i++) {
        switch(history_str[i]) {
            default:
            case 'I':
                history[i] = 0;
                break;
            case 'Z':
                history[i] = 1;
                break;
            case 'S':
                history[i] = 2;
                break;
            case 'J':
                history[i] = 3;
                break;
            case 'L':
                history[i] = 4;
                break;
            case 'O':
                history[i] = 5;
                break;
            case 'T':
                history[i] = 6;
                break;
        }

        vals[i] = (history[i] << (56 - 8*i));
    }

    vals[4] = seed_;

    for(i = 0; i < 5; i++) {
        result |= vals[i];
    }

    return result;
}
*/

uint32_t g2_get_seed()
{
    return g2_seed;
}

uint32_t g2_seed_rand(uint32_t seed)
{
    g2_seed = seed;

    return 0;
}

uint32_t g2_seed_bkp()
{
    g2_bkp_seed = g2_seed;
    return 0;
}

uint32_t g2_seed_restore()
{
    g2_seed = g2_bkp_seed;
    return 0;
}

uint32_t g2_rand(uint32_t n)
{
    return (n * 0x41c64e6d + 12345);
}

uint32_t g2_rand_rep(uint32_t n, uint32_t reps)
{
    if(reps == 0)
        return n;

    if(reps == 1)
        return g2_rand(n);

    uint32_t rr = g2_rand(n);
    uint32_t result = g2_rand_rep(rr, reps - 1);

    return result;
}

uint32_t g2_unrand(uint32_t n)
{
    return ((n - 12345) * 0xeeb9eb65);
}

uint32_t g2_unrand_rep(uint32_t n, uint32_t reps)
{
    if(reps == 0)
        return n;

    if(reps == 1)
        return g2_unrand(n);

    uint32_t un = g2_unrand(n);
    uint32_t result = g2_unrand_rep(un, reps - 1);

    return result;
}

uint32_t g123_read_rand(uint32_t *seedp)
{
    if(!seedp)
        seedp = &g2_seed;

    *seedp = g2_rand(*seedp);
    return (*seedp >> 10) & 0x7fff;
}

// TODO
uint32_t pento_read_rand(uint32_t *seedp)
{
    return g123_read_rand(seedp);
}

piece_id g123_get_init_piece(uint32_t *seedp)
{
    piece_id t = 1;

    if(!seedp)
        seedp = &g2_seed;

    while(t == 1 || t == 2 || t == 5) {
        t = g123_read_rand(seedp) % 7;
    }

    return t;
}

/*
uint8_t g2_std_randomize(uint8_t *history)
{
    uint8_t t = 0;
    int i = 0;
    int j = 0;

    int is_in_history = 0;

    for(i = 0; i < 5; i++) {
        t = (uint8_t)(g123_read_rand(NULL) % 7);

        for(j = 0; j < 4; j++) {
            if(history[j] == t)
                is_in_history = 1;
        }

        if(!is_in_history)
            return t;

        is_in_history = 0;
        t = g123_read_rand(NULL) % 7;
    }

    return t;
}

rngstate g2_state_randomize(rngstate s)
{
    uint8_t t = 0;
    int i = 0;
    int j = 0;

    int is_in_history = 0;

    for(i = 0; i < 5; i++) {
        t = (uint8_t)(g2_state_read_rand(s) % 7);
        s = g2_state_rand(s);

        for(j = 0; j < 4; j++) {
            if(state_hN(s, j) == t)
                is_in_history = 1;
        }

        if(!is_in_history) {
            s = state_set_hN(s, 0, state_hN(s, 1));
            s = state_set_hN(s, 1, state_hN(s, 2));
            s = state_set_hN(s, 2, state_hN(s, 3));
            s = state_set_hN(s, 3, t);

            return s;
        }

        t = (uint8_t)(g2_state_read_rand(s) % 7);
        s = g2_state_rand(s);
    }

    s = state_set_hN(s, 0, state_hN(s, 1));
    s = state_set_hN(s, 1, state_hN(s, 2));
    s = state_set_hN(s, 2, state_hN(s, 3));
    s = state_set_hN(s, 3, t);

    return s;
}

rngstate g2_state_init_randomize(rngstate s)
{
    uint8_t t = 1;
    char pieces[7] = "IZSJLOT";

    while(t == 1 || t == 2 || t == 5) {
        t = (uint8_t)(g2_state_read_rand(s) % 7);
        s = g2_state_rand(s);
    }

    printf("init piece = %c\n", pieces[t]);

    s = state_set_hN(s, 0, state_hN(s, 1));
    s = state_set_hN(s, 1, state_hN(s, 2));
    s = state_set_hN(s, 2, state_hN(s, 3));
    s = state_set_hN(s, 3, t);

    return s;
}
*/

piece_id g3_most_droughted_piece(int *histogram)
{
    unsigned int i = 0;
    unsigned int n = 0;
    unsigned int maxi = 0;

   for(i = 0; i < 7; i++)
   {
      if(n < histogram[i])
      {
         n = histogram[i];
         maxi = i;
      }
   }

    return (piece_id)(maxi);
}
