#include "random.h"
#include "SDL_stdinc.h"
#include "SDL_timer.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "QRS0.h"

// clang-format off
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
// clang-format on

static Uint64 old_seed_counter = 0u;

static uint32_t g1_seed = 0;
static uint32_t g1_bkp_seed = 0;

static uint32_t g2_seed = 0;
static uint32_t g2_bkp_seed = 0;

static uint32_t g3_seed = 0;
static uint32_t g3_bkp_seed = 0;

static uint32_t pento_seed = 0;
static uint32_t pento_bkp_seed = 0;

piece_id ars_to_qrs_id(piece_id t)
{
    switch(t)
    {
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
    unsigned s = (unsigned)time(0);
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

void g123_seeds_preupdate() {
    old_seed_counter = SDL_GetPerformanceCounter();
}

void g123_seeds_update() {
    const Uint64 new_seed_counter = SDL_GetPerformanceCounter();
    const uint32_t seed_increment = uint32_t(new_seed_counter - old_seed_counter);

    g1_seed = (g1_seed * 1497079797u) + seed_increment;
    g2_seed = (g2_seed * 4185316896u) + seed_increment;
    g3_seed = (g3_seed * 2525892343u) + seed_increment;
    pento_seed = (pento_seed * 1119304217u) + seed_increment;
}

struct randomizer *g1_randomizer_create()
{
    struct randomizer *r = (struct randomizer *)malloc(sizeof(struct randomizer));
    assert(r != nullptr);
    struct histrand_data *d = NULL;

    r->num_pieces = 7;
    r->seedp = &g1_seed;
    r->type = HISTRAND;

    r->init = g1_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    assert(r->data != nullptr);
    d = (struct histrand_data *)r->data;

    d->hist_len = 4;
    d->rerolls = 3;
    d->history = (piece_id *)malloc(d->hist_len * sizeof(piece_id));
    assert(d->history != nullptr);
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

struct randomizer *g2_randomizer_create()
{
    struct randomizer *r = (struct randomizer *)malloc(sizeof(struct randomizer));
    assert(r != nullptr);
    struct histrand_data *d = NULL;

    r->num_pieces = 7;
    r->seedp = &g2_seed;
    r->type = HISTRAND;

    r->init = g2_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    assert(r->data != nullptr);
    d = (struct histrand_data *)r->data;

    d->hist_len = 4;
    d->rerolls = 5;
    d->history = (piece_id *)malloc(d->hist_len * sizeof(piece_id));
    assert(d->history != nullptr);
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

struct randomizer *g3_randomizer_create()
{
    struct randomizer *r = (struct randomizer *)malloc(sizeof(struct randomizer));
    assert(r != nullptr);
    struct g3rand_data *d = NULL;
    int i = 0;

    r->num_pieces = 7;
    r->seedp = &g3_seed;
    r->type = G3RAND;

    r->init = g3_randomizer_init;
    r->pull = g3rand_pull;
    r->lookahead = g3rand_lookahead;

    r->data = malloc(sizeof(struct g3rand_data));
    assert(r->data != nullptr);
    d = (struct g3rand_data *)r->data;

    for(i = 0; i < 4; i++)
        d->history[i] = PIECE_ID_INVALID;

    for(i = 0; i < 35; i++)
        d->bag[i] = PIECE_ID_INVALID;

    for(i = 0; i < 7; i++)
        d->histogram[0] = 0;

    return r;
}

struct randomizer *pento_randomizer_create()
{
    struct randomizer *r = (struct randomizer *)malloc(sizeof(struct randomizer));
    assert(r != nullptr);
    struct histrand_data *d = NULL;
    unsigned int i = 0;

    r->num_pieces = 25;
    r->seedp = &pento_seed;
    r->type = HISTRAND;

    r->init = pento_randomizer_init;
    r->pull = histrand_pull;
    r->lookahead = histrand_lookahead;

    r->data = malloc(sizeof(struct histrand_data));
    assert(r->data != nullptr);
    d = (struct histrand_data *)r->data;

    d->hist_len = 6;
    d->rerolls = 0;
    d->history = (piece_id *)malloc(d->hist_len * sizeof(piece_id));
    assert(d->history != nullptr);
    d->history[0] = PIECE_ID_INVALID;
    d->history[1] = PIECE_ID_INVALID;
    d->history[2] = PIECE_ID_INVALID;
    d->history[3] = PIECE_ID_INVALID;
    d->history[4] = PIECE_ID_INVALID;
    d->history[5] = PIECE_ID_INVALID;

    d->difficulty = 0.0;

    d->piece_weights = (double *)malloc(r->num_pieces * sizeof(double));
    assert(d->piece_weights != nullptr);
    d->drought_protection_coefficients = (double *)malloc(r->num_pieces * sizeof(double));
    assert(d->drought_protection_coefficients != nullptr);
    d->drought_times = (unsigned int *)malloc(r->num_pieces * sizeof(unsigned int));
    assert(d->drought_times != nullptr);

    for(i = 0; i < r->num_pieces; i++)
    {
        d->piece_weights[i] = double(pento_piece_weights[i]);
        d->drought_protection_coefficients[i] = double(pento_drought_coeffs[i]);
        d->drought_times[i] = 0u;
    }

    return r;
}

void randomizer_destroy(struct randomizer *r)
{
    if(!r)
        return;

    struct histrand_data *d = NULL;

    switch(r->type)
    {
        case HISTRAND:
            d = (struct histrand_data *)r->data;

            if(d->history)
                free(d->history);
            if(d->piece_weights)
                free(d->piece_weights);
            if(d->drought_protection_coefficients)
                free(d->drought_protection_coefficients);
            if(d->drought_times)
                free(d->drought_times);

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
    struct histrand_data *d = (struct histrand_data *)r->data;
    int i = 0;
    int num_generated = 0;

    if(seed)
    {
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
    struct histrand_data *d = (struct histrand_data *)r->data;
    int i = 0;
    int num_generated = 0;

    if(seed)
    {
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
    struct g3rand_data *d = (struct g3rand_data *)r->data;
    int i = 0;
    int num_generated = 0;

    if(seed)
    {
        g3_bkp_seed = g3_seed;
        g3_seed = *seed;
    }

    for(i = 0; i < 35; i++)
        d->bag[i] = i / 5;

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
    struct histrand_data *d = (struct histrand_data *)r->data;
    int t = 0;
    int i = 0;
    int num_generated = 0;

    if(seed)
    {
        pento_bkp_seed = pento_seed;
        pento_seed = *seed;
    }

    for(unsigned j = 0; j < r->num_pieces; j++)
    {
        d->drought_times[j] = pento_read_rand(r->seedp) % 30;
    }

    d->history[0] = QRS_Fb;
    d->history[1] = QRS_Fa;
    d->history[2] = QRS_X;
    d->history[3] = QRS_S;
    d->history[4] = QRS_Z;

    t = pento_read_rand(r->seedp) % 5;

    switch(t)
    {
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

    for(i = 0; i < 5; i++)
    {
        histrand_pull(r); // Fb, Fa, X, S, Z get pulled into the void
        num_generated++;
    }

    return num_generated;
}

// ----- //

void history_push(piece_id *history, unsigned int hist_len, piece_id t)
{
    unsigned int i = 0;

    for(i = 0; i < hist_len - 1; i++)
    {
        history[i] = history[i + 1];
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

    for(i = 0; i < hist_len; i++)
    {
        if(history[i] == t)
            return true;
    }

    return false;
}

piece_id histrand_pull(struct randomizer *r)
{
    struct histrand_data *d = (struct histrand_data *)r->data;
    piece_id t = histrand_get_next(r);
    piece_id result = history_pop(d->history);
    unsigned int i = 0;

    history_push(d->history, d->hist_len, t);

    if(d->drought_times)
    { // update drought times if we are using them
        for(i = 0; i < r->num_pieces; i++)
        {
            if(i == t)
                d->drought_times[i] = 0;
            else
                d->drought_times[i]++;
        }
    }

    return result;
}

piece_id g3rand_pull(struct randomizer *r)
{
    struct g3rand_data *d = (struct g3rand_data *)r->data;
    piece_id t = g3rand_get_next(r);
    piece_id piece = history_pop(d->history);

    history_push(d->history, 4, t);

    return piece;
}

piece_id histrand_get_next(struct randomizer *r)
{
    struct histrand_data *d = (struct histrand_data *)r->data;
    uint32_t *seedp = r->seedp;
    unsigned int i = 0;
    unsigned int j = 0;
    bool in_hist = false;
    piece_id t = PIECE_ID_INVALID;

    long double p = 0.0;
    double oldSum = 0.0;
    double sum = 0.0;
    unsigned int *histogram = (unsigned int *)malloc(r->num_pieces * sizeof(unsigned int));
    const auto weights = static_cast<float *>(malloc(r->num_pieces * sizeof(float)));

    if(!seedp) {
        seedp = &g2_seed;
    }

    if(!d->piece_weights) // if we are using rerolls and not weighted calculation
    {
        for(i = 0; i < d->rerolls; i++)
        {
            t = g123_read_rand(seedp) % 7;

            in_hist = 0;
            for(j = 0; j < d->hist_len; j++)
            {
                if(d->history[j] == t)
                    in_hist = true;
            }

            if(!in_hist)
                break;

            t = g123_read_rand(seedp) % 7;
        }

        free(weights);
        free(histogram);

        return t;
    }

    else
    {
        // starts at 1 and counts up, bad pieces' weights are divided by this
        int belowHighThreshold = 1;
        const float difficultyThreshold = 30;
        for(i = 0; i < r->num_pieces; i++)
        {
            weights[i] = d->piece_weights[i];
            // histogram values are all offset by one from how many times the piece is actually in the history
            histogram[i] = 1;

            for(j = 0; j < d->hist_len; j++)
            {
                if(d->history[j] == i)
                {
                    histogram[i]++;
                    if(d->piece_weights[i] < QRS_WEIGHT_HIGHTIER_THRESHOLD)
                    {
                        belowHighThreshold++;
                    }
                }
            }

            // reduce piece weights for pieces that have been generated recently, and increase weights for pieces that haven't
            // if difficulty is above 130.0 (past uniform randomness and into evil territory), don't perform any of these extra calculations
            if(d->difficulty <= difficultyThreshold + 100.0)
            {
                weights[i] /= double(histogram[i] * histogram[i]); // OLD: * (i < 18 ? pieces[i] : 1)
                if(d->drought_protection_coefficients && d->drought_times)
                {
                    // multiply by coefficient^(t/BASELINE)
                    // e.g. for coeff 2 and drought time BASELINE, weight *= 2
                    weights[i] *= pow(d->drought_protection_coefficients[i], (double)(d->drought_times[i]) / QRS_DROUGHT_BASELINE);

                    if(d->piece_weights[i] >= QRS_WEIGHT_HIGHTIER_THRESHOLD && QRS_DROUGHT_HIGHTIER_SOFTLIMIT >= 0)
                    {
                        if(d->drought_times[i] >= QRS_DROUGHT_HIGHTIER_SOFTLIMIT)
                        {
                            weights[i] *= pow(1.3, (double)d->drought_times[i] - QRS_DROUGHT_HIGHTIER_SOFTLIMIT + 1.0);
                        }
                    }
                    else if(d->difficulty >= difficultyThreshold && d->piece_weights[i] >= QRS_WEIGHT_MIDTIER_THRESHOLD && QRS_DROUGHT_MIDTIER_SOFTLIMIT >= 0)
                    {
                        if(d->drought_times[i] >= QRS_DROUGHT_MIDTIER_SOFTLIMIT)
                        {
                            weights[i] *= pow(1.3, (double)d->drought_times[i] - QRS_DROUGHT_MIDTIER_SOFTLIMIT + 1.0);
                        }
                    }
                    else if(d->difficulty >= difficultyThreshold && QRS_DROUGHT_LOWTIER_SOFTLIMIT >= 0)
                    {
                        if(int(d->drought_times[i]) >= QRS_DROUGHT_LOWTIER_SOFTLIMIT)
                        {
                            weights[i] *= pow(1.3, (double)d->drought_times[i] - QRS_DROUGHT_LOWTIER_SOFTLIMIT + 1.0);
                        }
                    }
                }
            }
        }

        // reduce piece weights for mid- and low-tier pieces collectively for each of them that has appeared recently
        // if difficulty is below 30.0 (randomizer is extra nice), increase piece weights for all top-tier and high-tier pieces with separate uniform calculations
        // if difficulty is above 130.0 (past uniform randomness and into evil territory), don't perform any of these extra calculations
        if(d->difficulty <= difficultyThreshold + 100.0)
        {
            for(i = 0; i < r->num_pieces; i++)
            {
                if(d->piece_weights[i] < QRS_WEIGHT_HIGHTIER_THRESHOLD)
                {
                    weights[i] /= belowHighThreshold;
                }

                if(d->difficulty < difficultyThreshold && d->piece_weights[i] >= QRS_WEIGHT_TOPTIER_THRESHOLD)
                {
                    weights[i] *= pow(1.1, (difficultyThreshold - d->difficulty) / 3);
                }

                if(d->difficulty < difficultyThreshold && d->piece_weights[i] >= QRS_WEIGHT_HIGHTIER_THRESHOLD && d->piece_weights[i] < QRS_WEIGHT_TOPTIER_THRESHOLD)
                {
                    weights[i] *= pow(1.1, (difficultyThreshold - d->difficulty) / 6);
                }


            }
        }

        for(i = 0; i < r->num_pieces; i++)
        {
            sum += weights[i];
        }

        // modulate piece weights to bring them closer to the average (if 30.0 <= difficulty <= 130.0)
        // ...or make them more extreme in favor of worse pieces (if difficulty > 130.0)
        if(d->difficulty > difficultyThreshold)
        {
            oldSum = sum;
            sum = 0.0;

            for(i = 0; i < r->num_pieces; i++)
            {
                // take the difference from the average and multiply by difficulty/100, then add that to the weight
                // note that if the weight was above average, a negative value is added
                weights[i] += ((d->difficulty - difficultyThreshold) / 100.0) * ((oldSum / r->num_pieces) - weights[i]);
                sum += weights[i];
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
            if(p >= (long double)(sum) && p < (long double)(sum + weights[i]))
            {
                free(weights);
                free(histogram);
                return i;
            }
            else
                sum += weights[i];
        }

        free(weights);
        free(histogram);

        return 0;
    }

    // fallback piece_id return value, just to be safe
    return 0;
}

double histrand_get_difficulty(struct randomizer *r)
{
    struct histrand_data *d = (struct histrand_data *)r->data;

    return d->difficulty;
}

int histrand_set_difficulty(struct randomizer *r, double difficulty)
{
    struct histrand_data *d = (struct histrand_data *)r->data;
    d->difficulty = difficulty;

    return 0;
}

piece_id g3rand_get_next(struct randomizer *r)
{
    struct g3rand_data *d = (struct g3rand_data *)r->data;

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
    for(i = 0; i < 7; i++)
    {
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
    struct histrand_data *d = (struct histrand_data *)r->data;

    if(distance > d->hist_len || distance == 0)
        return PIECE_ID_INVALID;

    return d->history[distance - 1];
}

piece_id g3rand_lookahead(struct randomizer *r, unsigned int distance)
{
    struct g3rand_data *d = (struct g3rand_data *)r->data;

    if(distance > 4 || distance == 0)
        return PIECE_ID_INVALID;

    return d->history[distance - 1];
}

uint32_t g2_get_seed() { return g2_seed; }

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

uint32_t g2_rand(uint32_t n) { return (n * 0x41c64e6d + 12345); }

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

uint32_t g2_unrand(uint32_t n) { return ((n - 12345) * 0xeeb9eb65); }

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
uint32_t pento_read_rand(uint32_t *seedp) { return g123_read_rand(seedp); }

piece_id g123_get_init_piece(uint32_t *seedp)
{
    piece_id t = 1;

    if(!seedp)
        seedp = &g2_seed;

    while(t == 1 || t == 2 || t == 5)
    {
        t = g123_read_rand(seedp) % 7;
    }

    return t;
}

piece_id g3_most_droughted_piece(int *histogram)
{
    size_t i = 0;
    int n = 0;
    size_t maxi = 0;

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