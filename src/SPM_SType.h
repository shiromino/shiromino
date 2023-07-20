#pragma once
#include "Game.h"
#include "ShiroPhysoMino.h"
#include "replay.h"
#include "SPM_Spec.h"
#include "SPM_Randomizer.h"
#include <memory>
#include <vector>
#include <fstream>

struct SPM_SPlayer : public SPM_Player
{
    SPM_SPlayer() : SPM_Player(),
        level(0),
        score(0),
        rotations(0),
        pieces_dealt(0),
        bravos(0),
        soft_drop_counter(0),
        sonic_drop_height(0),
        active_piece_time(0),
        placement_speed(0),
        levelstop_time(0),
        last_gradeup_timestamp(0),
        grade(0),
        internal_grade(0),
        grade_points(0),
        grade_decay_counter(0),
        lock_on_rotate(0),
        lock_held(false),
        locking_row(0),
        lvlinc(0),
        lastclear(0),
        combo(0),
        combo_simple(0),
        singles(0),
        doubles(0),
        triples(0),
        quadruples(0),
        quintuples(0) {}

    int level;
    int score;

    int rotations;
    int pieces_dealt;
    int bravos;

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
    int lock_on_rotate;

    bool lock_held;
    int locking_row;
    int lvlinc;
    int lastclear;
    int combo;
    int combo_simple;

    int singles;
    int doubles;
    int triples;
    int quadruples;
    int quintuples;
};

class SPM_SType : public ShiroPhysoMino
{
public:
    SPM_SType(CoreState&, SPM_Spec *);

    int init() override;
    int input() override;
    int frame() override;
    int draw() override;

protected:
    bool spawnDelayExpired(SPM_SPlayer&);
    bool lockDelayExpired(SPM_SPlayer&);
    bool lineClearExpired(SPM_SPlayer&);
    bool initNextMino(SPM_SPlayer&);
    ActivatedPolyomino *activateMino(SPM_minoID ID);

    SPM_SPlayer player;

    SPM_gameCounters gameCounters;
    
    std::ifstream credits;
    int credit_roll_length;
    int credit_roll_counter;
    int credit_roll_lineclears;
    
    bool cleared_game;
    bool topped_out;

    SPM_randomSeed seed;

    int starting_level;
    int section;
    unsigned long cur_section_timestamp;

    std::vector<long> section_times;
    std::vector<long> best_section_times;
    std::vector<int> section_quadruples;

    std::vector<std::pair<int, SPM_frameTimings>> *speed_curve;
    int speed_curve_index;
    int musicTrack;
};