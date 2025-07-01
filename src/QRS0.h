#pragma once
#include <SDL_render.h>
#include <array>
#include <cstdint>
#include <fstream>
#include <vector>
#include <utility>
#include <memory>
#include "Game.h"
#include "GameType.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "Timer.h"

#define MAX_SECTIONS 30

#define PSINACTIVE         0x0000

#define PSARE             0x0001
#define PSLINEARE         0x0002
#define PSFALL             0x0004
// #define PSDASLEFT        0x0008
// #define PSDASRIGHT        0x0010
#define PSLOCK             0x0020
#define PSLINECLEAR        0x0040
#define PSFKICKED        0x0080
#define PSLOCKED        0x0100
#define PSPRELOCKED        0x0200
#define PSLOCKFLASH1    0x0400
#define PSLOCKFLASH2    0x0800
// #define PSLOCKPROTECT    0x1000
#define PSLOCKFLASH3    0x2000
#define PSLOCKFLASH4    0x4000
#define PSPRELOCKFLASH1 0x8000
#define PSSPAWN            0x10000
#define PSLOCKPRESSED    0x20000
#define PSUSEDHOLD        0x40000
// #define PSHOLDFLASH        0x80000

#define MOVE_LEFT        -1
#define MOVE_RIGHT        1

#define QRS_FIELD_EDIT  2

// IJLXSZNGUTFaFbPQWYaYbV I4T4J4L4OS4Z4 - felicity's arbitrary pentomino+tetromino ordering
#define QRS_I 0
#define QRS_J 1
#define QRS_L 2
#define QRS_X 3
#define QRS_S 4
#define QRS_Z 5
#define QRS_N 6
#define QRS_G 7
#define QRS_U 8
#define QRS_T 9
#define QRS_Fa 10
#define QRS_Fb 11
#define QRS_P 12
#define QRS_Q 13
#define QRS_W 14
#define QRS_Ya 15
#define QRS_Yb 16
#define QRS_V 17
#define QRS_I4 18
#define QRS_T4 19
#define QRS_J4 20
#define QRS_L4 21
#define QRS_O 22
#define QRS_S4 23
#define QRS_Z4 24

// ITJLOSZ - felicity's arbitrary tetromino ordering
#define QRS_ARS_I 0
#define QRS_ARS_T 1
#define QRS_ARS_J 2
#define QRS_ARS_L 3
#define QRS_ARS_O 4
#define QRS_ARS_S 5
#define QRS_ARS_Z 6

// IZSJLOT - Arika's arbitrary tetromino ordering
#define ARS_I 0
#define ARS_Z 1
#define ARS_S 2
#define ARS_J 3
#define ARS_L 4
#define ARS_O 5
#define ARS_T 6

#define PIECE_ID_INVALID 255

#define SUBSET_ALL 0
#define SUBSET_TETS 1
#define SUBSET_PENTS 2

#define QRS_PIECE_GARBAGE -5
//#define QRS_PIECE_RAINBOW 512
#define QRS_PIECE_BRACKETS (1 << 24)
#define QRS_PIECE_GEM (1 << 25)
#define QRS_FIELD_W_LIMITER -30
#define QRS_WALL QRS_FIELD_W_LIMITER

#define SEQUENCE_REPEAT_START 2048
#define SEQUENCE_REPEAT_END    4096
#define SEQUENCE_REPEAT_INF 8192

#define QRS_FIELD_W 12
#define QRS_FIELD_H 24

#define GAMESTATE_BIGMODE               0x40000000
//#define GAMESTATE_INACTIVE            0x80000000
#define GAMESTATE_INVISIBLE                 0x0001
#define GAMESTATE_BRACKETS                  0x0002
#define GAMESTATE_RISING_GARBAGE            0x0004
#define GAMESTATE_CREDITS                   0x0008
#define GAMESTATE_FADING                    0x0010
#define GAMESTATE_TOPOUT_ANIM               0x0020
#define GAMESTATE_FADE_TO_CREDITS           0x0040
//#define GAMESTATE_READYGO                 0x0080
#define GAMESTATE_GAMEOVER                  0x0100
//#define GAMESTATE_PRESS_START             0x0200
#define GAMESTATE_FIREWORKS_GM              0x0400
#define GAMESTATE_FIREWORKS                 0x0800
#define GAMESTATE_BETTER_NEXT_TIME          0x1000
//#define GAMESTATE_RESULTS_SCREEN          0x2000
#define GAMESTATE_CREDITS_TOPOUT_ANIM       0x4000
#define GAMESTATE_HALTING               0x00020000
#define GAMESTATE_SOFT_CREDITS_TOPOUT   0x00040000
#define GAMESTATE_STACK_REVEAL_DELAY    0x00080000
//#define GAMESTATE_PRE_PROMO_EXAM          0x8000
//#define GAMESTATE_PROMO_RESULTS       0x00010000

//#define GAMESTATE_REWIND                  0x4000
//#define GAMESTATE_FAST_FORWARD            0x8000

#define GARBAGE_COPY_BOTTOM_ROW     0x0001

#define SPAWNX_QRS 5
#define SPAWNY_QRS 3

#define ANCHORX_QRS        1
#define ANCHORY_QRS        1
#define SPAWNX_ARS        4
#define SPAWNY_ARS        5

#define TAP_ARS 0
#define TI_ARS 1

#define BRONZE 1
#define SILVER 2
#define GOLD 3
#define PLATINUM 4

#define YTOROW(n)   (n/256)
#define ROWTOY(n)   (n*256)
#define IS_STACK(n)    ((!n || n == -2 || n == GRID_OOB || n == QRS_FIELD_W_LIMITER) ? 0 : 1)
#define IS_INBOUNDS(n) (IS_STACK(n) || !n || n == -2)
#define IS_QRS_PIECE(n) (n >= 0 && n < 25)
#define GET_PIECE_FADE_COUNTER(n) ((n >> 8) & 0xffff)
#define SET_PIECE_FADE_COUNTER(n, f) n = ((n & 0xff0000ff) | ((f & 0xffff) << 8))

extern const char *qrs_piece_names[25];

typedef uint8_t piece_id;

enum {
    MODE_INVALID,
    QUINTESSE
};

struct QRS_Timings {
    QRS_Timings();
    QRS_Timings(unsigned level, int grav, int lock, int das, int are, int lineare, int lineclear);

    unsigned level;

    int grav;
    int lock;
    int das;
    int are;
    int lineare;
    int lineclear;
};

struct QRS_Counters {
    QRS_Counters();

    int init;

    int lock;
    int are;
    int lineare;
    int lineclear;
    unsigned int floorkicks;
    int hold_flash;
};

struct qrs_player
{
    Shiro::PieceDefinition* def;
    QRS_Timings* speeds;
    unsigned int state;        // rename?

    int x;
    int y;
    int* old_xs;
    int* old_ys;
    int num_olds;
    int orient;
};

struct pracdata
{
    pracdata() :
        game_type_int(0),
        game_type(Shiro::GameType::SIMULATE_QRS),
        field_w(12),
        goal_level(0),
        goal_time(0),
        usr_seq_len(0),
        usr_seq_expand_len(0),
        field_edit_in_progress(false),
        usr_field_fumen(false),
        usr_field_locked(false),
        palette_selection(0),
        field_selection(0),
        field_selection_vertex1_x(0),
        field_selection_vertex1_y(0),
        field_selection_vertex2_x(0),
        field_selection_vertex2_y(0),
        usr_timings(nullptr),
        paused(0),
        grid_lines_shown(true),
        brackets(false),
        invisible(false),
        fading(false),
        garbage_delay(0),
        hist_index(0),
        lock_protect(0),
        infinite_floorkicks(false),
        piece_subset(SUBSET_ALL),
        randomizer_seed(0),
        using_seed(false)
    {
        for (int& item : usr_sequence) {
            item = 0;
        }
        for (int& item : usr_seq_expand) {
            item = 0;
        }
    }

    int game_type_int;
    Shiro::GameType game_type;    // mirrors of values in qrsdata; these are just here so that..
    int field_w;    // ..backed up pracdata structs can be used to restore their values

    //int *long_history;        // old idea, just history of all pieces placed,
                                // going to re-add long_history but with placement locations included,
                                // as well as game time/level, to allow piece-by-piece rewinds

    int goal_level; // when goal is reached, the game will end (0 = disable)
    int goal_time; // count timer backward from this many frames, and when timer reaches zero, game ends (0 = disable)

    int usr_sequence[2000];
    int usr_seq_expand[4000];
    std::size_t usr_seq_len;
    std::size_t usr_seq_expand_len;

    std::vector<Shiro::Grid> usr_field_undo;
    std::vector<Shiro::Grid> usr_field_redo;
    bool field_edit_in_progress;

    bool usr_field_fumen; // true -> every piece placed in-game is saved as a new usr_field state
    bool usr_field_locked;

    Shiro::Grid usr_field;
    int palette_selection;
    int field_selection;
    int field_selection_vertex1_x;
    int field_selection_vertex1_y;
    int field_selection_vertex2_x;
    int field_selection_vertex2_y;

    QRS_Timings *usr_timings;

    int paused;
    bool grid_lines_shown;
    bool brackets;
    bool invisible;
    bool fading;

    int garbage_delay;

    int hist_index;
    int lock_protect;
    bool infinite_floorkicks;
    int piece_subset;

    uint32_t randomizer_seed;
    bool using_seed;
};

struct qrsdata
{
    std::vector<Shiro::PieceDefinition> piecepool;
    struct randomizer *randomizer;
    struct pracdata *pracdata;
    struct replay *replay;
    Shiro::Grid garbage;
    piece_id *piece_seq;

    Shiro::Timer timer;
    qrs_player *p1;
    QRS_Counters *p1counters;
    std::vector<Shiro::PieceDefinition> previews;
    Shiro::PieceDefinition* hold;

    std::ifstream credits;
    SDL_Texture *credits_tex;
    int credits_tex_height;
    int num_credits_lines;
    int credit_roll_length;

    // counts down to 0 (measured in frames)
    int credit_roll_counter;
    int credit_roll_lineclears;

    bool cleared_game;
    bool topped_out;

// fields which are assumed to be read-only during normal gameplay

    unsigned int piece_seq_len;
    bool using_piece_seq;

    bool is_practice;
    bool recording;
    bool playback;

    Shiro::GameType game_type;
    int mode_type;
    unsigned int mode_flags;
    int randomizer_type;

    uint32_t randomizer_seed;
    bool tetromino_only;
    bool pentomino_only;

    int field_x, field_y; // in pixels (position of top-left of field)
    int field_w; // in cells (only player-accessible ones counted here)

    unsigned int max_floorkicks;
    int num_previews;

    bool lock_delay_enabled;
    bool lock_protect;
    bool hold_enabled;
    bool special_irs;
    bool using_gems;

    unsigned int piece_fade_rate;

// fields which are assumed to be mutated during normal gameplay

    piece_id cur_piece_qrs_id;

    unsigned int state_flags;    // things like invisible mode, bracket mode, etc. go here

    int piece_seq_index;
    int garbage_row_index;    // which row of the garbage cells to spawn next
    int playback_index;        // equivalent to number of frames that input has been handled in the game so far

    // increments for each piece that doesnt clear lines (shirase: for each piece spawned & decrements for each line cleared)
    int garbage_counter;
    // = 13 - q->section for TGM+. shirase: 500-599=20, 600-699=18, 700-799=10, 800-899=9, 900-999=8; checks during ARE
    int garbage_delay;

    // general purpose frame counter to keep track of top out animation, etc.
    int stack_anim_counter;

    int starting_level;
    unsigned level;
    int section;
    double rank;

    int rotations;
    int pieces_dealt;
    int bravos;

    int score;
    int soft_drop_counter;
    int sonic_drop_height;
    int active_piece_time;
    int placement_speed;
    int levelstop_time;

    unsigned long last_gradeup_timestamp;
    int grade;
    int internal_grade;
    int grade_points;
    int grade_decay_counter;

    bool mroll_unlocked;
    long cur_section_timestamp;
    int section_times[MAX_SECTIONS];
    int section_tetrises[MAX_SECTIONS];

    int best_section_times[MAX_SECTIONS];

    // values: 1 = set to 2 next time a rotate happens.
    //           2 = lock during THIS frame ( handled by qs_process_lock() )
    int lock_on_rotate;

    bool lock_held;
    int locking_row;
    int lvlinc;
    int lastclear;
    int combo;
    int combo_simple;
    int singles;    //
    int doubles;    //
    int triples;    //
    int tetrises;    //
    int pentrises;
    int recoveries;
    bool is_recovering;

    unsigned long last_medal_ac_timestamp;
    unsigned long last_medal_sk_timestamp;
    unsigned long last_medal_ro_timestamp;
    unsigned long last_medal_st_timestamp;
    unsigned long last_medal_re_timestamp;
    unsigned long last_medal_co_timestamp;

    // medal values range from 0 to BRONZE (1) to PLATINUM (4)
    int medal_ac;
    int medal_sk;
    int medal_ro;
    int medal_st;
    int medal_re;
    int medal_co;

    int speed_curve_index;
    int music;
    //int history[10];    // 0, 1, 2, 3, 4, 5 are in the past; 6 is the current piece; 7, 8, 9 are previews
};

const std::string get_qrspiece_name(std::size_t n);

void qrsdata_destroy(qrsdata *q);
void pracdata_destroy(struct pracdata *d);

std::vector<Shiro::PieceDefinition> qrspool_create();

Shiro::Grid* qrsfield_create();
int qrsfield_set_w(Shiro::Grid* field, int w);
int qrsfield_clear(Shiro::Grid* field);

int undo_clear_button_should_activate(CoreState *cs);
int undo_clear_button_should_deactivate(CoreState *cs);

int usr_field_undo_history_exists(CoreState *cs);
int usr_field_undo_history_not_exists(CoreState *cs);

int usr_field_redo_history_exists(CoreState *cs);
int usr_field_redo_history_not_exists(CoreState *cs);

int lock_usr_field(CoreState *cs, void *data);
int unlock_usr_field(CoreState *cs, void *data);

int usr_field_is_locked(CoreState *cs);
int usr_field_is_unlocked(CoreState *cs);

int usr_field_bkp(CoreState *cs, struct pracdata *d);
int usr_field_undo(CoreState *cs, struct pracdata *d);
int usr_field_redo(CoreState *cs, struct pracdata *d);
int push_undo_clear_confirm(CoreState *cs, void *data);
int undo_clear_confirm_yes(CoreState *cs, void *data);
int undo_clear_confirm_no(CoreState *cs, void *data);
int usr_field_undo_clear(CoreState *cs, void *data);

int usr_field_undo_button_action(CoreState *cs, void *data);
int usr_field_redo_button_action(CoreState *cs, void *data);

int qrs_input(game_t *g);

int qrs_start_record(game_t *g);
int qrs_end_record(game_t *g);

int qrs_load_replay(game_t *g, int replay_id);
int qrs_start_playback(game_t *g);
int qrs_end_playback(game_t *g);

int qrs_move(game_t *g, qrs_player *p, int offset);
int qrs_rotate(game_t *g, qrs_player *p, int direction);
int qrs_proc_initials(game_t *g);
int qrs_irs(game_t *g);
int qrs_wallkick(game_t *g, qrs_player *p);
int qrs_floorkick(game_t *g, qrs_player *p);
int qrs_ceilingkick(game_t *g, qrs_player *p);
int qrs_hold(game_t *g, qrs_player *p);
//int qrs_ihs(game_t *g, int playerid);

int qrs_fall(game_t *g, qrs_player *p, int grav);
int qrs_lock(game_t *g, qrs_player *p);
bool qrs_chkcollision(game_t& g, qrs_player& p);
bool qrs_chkcollision(game_t& g, qrs_player& p, std::pair<int, int>& pos);
int qrs_isonground(game_t *g, qrs_player *p);

int qrs_lineclear(game_t *g, qrs_player *p);
int qrs_dropfield(game_t *g);
int qrs_spawn_garbage(game_t *g, unsigned int flags);

void qrs_embiggen(Shiro::PieceDefinition& p);