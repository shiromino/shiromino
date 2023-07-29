#pragma once
#include "Game.h"
#include "ShiroPhysoMino.h"
#include "SPM_Structures.h"
#include "SPM_Graphics.h"
#include "replay.h"
#include "SPM_Spec.h"
#include "SPM_Randomizer.h"
#include "MinoCanonicalID.h"
#include <vector>
#include <fstream>
#include <cstdint>
#include <unordered_map>

enum class SFieldEncoding : int32_t
{
    empty = 0,
    mono = -1,
    bone = -2,
    clearedCell = -3,
    fieldLimiterLeft = -100,
    fieldLimiterRight = -101,
    fieldLimiterBlock = -102
};

struct SType_FieldEncoding_GfxMap : public Shiro::SPM::FieldEncoding_GfxMap
{
    static std::unordered_map<int32_t, Shiro::Gfx::Rect> createMap()
    {
        std::unordered_map<int32_t, Shiro::Gfx::Rect> m;

        // note: currently these rect coordinates correspond to "pieces_256x256.png"
        /* in the future, there could be additional structure to specify aspects of a piece texture atlas such as
         * cell size (currently this assumes 256x256). that can come when it becomes relevant
        */
        
        m[static_cast<int32_t>(SFieldEncoding::empty)] = {6 * 256, 5 * 256, 256, 256}; // nothing... usually
        m[static_cast<int32_t>(SFieldEncoding::mono)] = {5 * 256, 2 * 256, 256, 256}; // monochrome (grey) blocks
        m[static_cast<int32_t>(SFieldEncoding::bone)] = {6 * 256, 2 * 256, 256, 256}; // square brackets []
        m[static_cast<int32_t>(SFieldEncoding::clearedCell)] = {6 * 256, 5 * 256, 256, 256}; // nothing... usually
        m[static_cast<int32_t>(SFieldEncoding::fieldLimiterLeft)] = {3 * 256, 6 * 256, 256, 256}; // grey "wall" block (edge on right)
        m[static_cast<int32_t>(SFieldEncoding::fieldLimiterRight)] = {2 * 256, 6 * 256, 256, 256}; // grey "wall" block (edge on left)
        m[static_cast<int32_t>(SFieldEncoding::fieldLimiterBlock)] = {1 * 256, 6 * 256, 256, 256}; // grey "wall" block (solid, no edge outline)

        m[static_cast<int32_t>(Shiro::Mino::O1)] = {3 * 256, 0, 256, 256}; // same as X pentomino (temp)
        m[static_cast<int32_t>(Shiro::Mino::I2)] = {5 * 256, 2 * 256, 256, 256}; // same as mono (temp)
        m[static_cast<int32_t>(Shiro::Mino::I3)] = {1 * 256, 2 * 256, 256, 256}; // same as Ya pentomino (temp)
        m[static_cast<int32_t>(Shiro::Mino::V3)] = {1 * 256, 2 * 256, 256, 256}; // same as Ya pentomino (temp)

        m[static_cast<int32_t>(Shiro::Mino::I4)] = {0, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::T4)] = {1 * 256, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::J4)] = {2 * 256, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::L4)] = {3 * 256, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::O4)] = {4 * 256, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::S4)] = {5 * 256, 3 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Z4)] = {6 * 256, 3 * 256, 256, 256};

        m[static_cast<int32_t>(Shiro::Mino::I5)] = {1 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::J5)] = {2 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::L5)] = {3 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::X5)] = {4 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::S5)] = {5 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Z5)] = {6 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::N5)] = {1 * 256, 0, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::G5)] = {2 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::U5)] = {3 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::T5)] = {4 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Fa5)] = {5 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Fb5)] = {6 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::P5)] = {1 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Q5)] = {2 * 256, 1 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::W5)] = {3 * 256, 2 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Ya5)] = {4 * 256, 2 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::Yb5)] = {5 * 256, 2 * 256, 256, 256};
        m[static_cast<int32_t>(Shiro::Mino::V5)] = {6 * 256, 2 * 256, 256, 256};

        return m;
    }

    const Shiro::Gfx::Rect& get(int32_t i) override
    {
        return gfxMap.at(i);
    }

    static const std::unordered_map<int32_t, Shiro::Gfx::Rect> gfxMap;
};

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
    virtual bool spawnDelayExpired(SPM_SPlayer&);
    virtual bool lockDelayExpired(SPM_SPlayer&);
    virtual bool lineClearExpired();
    virtual bool initNextMino(SPM_SPlayer&);
    virtual ActivatedPolyomino *activateMino(SPM_minoID);

    SPM_Spec *spec;

    SPM_SPlayer player;

    SPM_gameCounters gameCounters;
    
    std::ifstream credits;
    int credit_roll_length; // time alotted to credit roll, in frames
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