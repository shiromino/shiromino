#ifndef _spm_spec_hpp
#define _spm_spec_hpp

#include <cstdint>
#include <vector>
#include "SGUIL/SGUIL.hpp"

#include "core.h"
#include "random.h"
#include "grid.h"

#define SPM_SUBUNIT_SCALE 65536

#define MINO_ID_INVALID 0xFFFFFFFF

#define SPM_CELL_CLEARED -2

typedef unsigned int SPM_minoID;
typedef uint64_t SPM_randomSeed;

class SPM_Randomizer
{
public:
    SPM_Randomizer(unsigned int numMinoes) : numMinoes(numMinoes) {}
    virtual ~SPM_Randomizer() {}

    virtual void init(SPM_randomSeed) {}

    virtual SPM_minoID pull() = 0;
    virtual SPM_minoID lookahead(unsigned int) = 0;

protected:
    unsigned int numMinoes;
};

class SPM_NonRandomizer : public SPM_Randomizer
{
public:
    SPM_NonRandomizer(unsigned int num) : SPM_Randomizer(num) { index = 0; }

    SPM_minoID pull()
    {
        SPM_minoID val = index;
        index = (index + 1) % 7;
        return val;
    }

    SPM_minoID lookahead(unsigned int distance) { return (index + distance) % 7; }

protected:
    SPM_minoID index;
};

struct SPM_point
{
    SPM_point() : x(0), y(0), subX(0), subY(0) {}
    SPM_point(int x, int y) : x(x), y(y), subX(0), subY(0) {}
    SPM_point(int x, int y, uint32_t subX, uint32_t subY) : x(x), y(y), subX(subX), subY(subY) {}

    int x;
    int y;

    int32_t subX;
    int32_t subY;
};

typedef SPM_point SPM_offset;

inline SPM_point operator + (SPM_point& a, SPM_offset& b)
{
    SPM_point result {};

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    if(b.subX >= SPM_SUBUNIT_SCALE)
    {
        b.subX = SPM_SUBUNIT_SCALE - 1;
    }
    else if(b.subX < 1 - SPM_SUBUNIT_SCALE)
    {
        b.subX = 1 - SPM_SUBUNIT_SCALE;
    }

    if(b.subY >= SPM_SUBUNIT_SCALE)
    {
        b.subY = SPM_SUBUNIT_SCALE - 1;
    }
    else if(b.subY < 1 - SPM_SUBUNIT_SCALE)
    {
        b.subY = 1 - SPM_SUBUNIT_SCALE;
    }

    result.subX = a.subX + b.subX;
    result.subY = a.subY + b.subY;

    if(result.subX >= SPM_SUBUNIT_SCALE)
    {
        result.subX -= SPM_SUBUNIT_SCALE;
        result.x++;
    }
    else if(result.subX < 0)
    {
        result.subX += SPM_SUBUNIT_SCALE;
        result.x--;
    }

    if(result.subY >= SPM_SUBUNIT_SCALE)
    {
        result.subY -= SPM_SUBUNIT_SCALE;
        result.y++;
    }
    else if(result.subY < 0)
    {
        result.subY += SPM_SUBUNIT_SCALE;
        result.y--;
    }

    return result;
}

inline SPM_point operator - (SPM_point& a, SPM_offset& b)
{
    SPM_offset b_ = {-b.x, -b.y, -b.subX, -b.subY};
    return operator + (a, b_);
}

enum SPM_gamePhase
{
    spm_paused = 0,
    spm_gameplay,
    spm_transition,
    spm_game_over
};

enum SPM_playPhase
{
    spm_spawn_delay,
    spm_player_control,
    spm_line_clear
};

enum SPM_physicState
{
    spm_physic_locked = -1,
    spm_physic_absent = 0,
    spm_physic_spawned = 1,
    spm_physic_falling,
    spm_physic_grounded
};

// operator bool (SPM_physicState& a) { return static_cast<int>(a) != 0; }

inline bool operator > (SPM_physicState& a, int val)
{ return static_cast<int>(a) > val; }


enum SPM_orientation
{
    spm_flat = 0,
    spm_clockwise = 1,
    spm_flip = 2,
    spm_counter_clockwise = 3
};


inline SPM_orientation operator ++ (SPM_orientation& a)
{
    int o = static_cast<int>(a);
    o++;
    return (o == 4) ? spm_flat : static_cast<SPM_orientation>(o);
}

inline SPM_orientation operator -- (SPM_orientation& a)
{
    int o = static_cast<int>(a);
    o--;
    return (o == -1) ? spm_counter_clockwise : static_cast<SPM_orientation>(o);
}

inline SPM_orientation operator + (SPM_orientation& a, SPM_orientation& b)
{
    SPM_orientation result = a;
    int n = static_cast<int>(b);

    for(int i = 0; i < n; i++)
    {
        ++a;
    }

    return result;
}

inline SPM_orientation operator - (SPM_orientation& a, SPM_orientation& b)
{
    SPM_orientation result = a;
    int n = static_cast<int>(b);

    for(int i = 0; i < n; i++)
    {
        --a;
    }

    return result;
}

struct SPM_frameTimings
{
    SPM_frameTimings() : gravity(0), lockDelay(0), das(0), are(0), lineAre(0), lineClear(0) {}

    int gravity;
    int lockDelay;
    int das;
    int dasInterval;
    int are;
    int lineAre;
    int lineClear;
};

struct SPM_frameCounters
{
    SPM_frameCounters()
    {
        frameWait = 0;
        frameWaitExpirePoint = 0;

        gamePhase = 0;
        gamePhaseExpirePoint = 0;

        transition = 0;
        transitionExpirePoint = 0;

        lockDelay = 0;
        lockDelayExpirePoint = 0;

        spawnDelay = 0;
        spawnDelayExpirePoint = 0;
        usingLineSpawnDelay = false;

        lineClear = 0;
        lineClearExpirePoint = 0;

        dasInterval = 0;
        dasIntervalExpirePoint = 0;
    }

    int frameWait; // could be used for slow-down: if(frameWait != 0) {frameWait--; return;}
    int frameWaitExpirePoint;

    int gamePhase; // counter for how long to stay in the current game phase
    int gamePhaseExpirePoint;

    int transition;
    int transitionExpirePoint;

    int lockDelay;
    int lockDelayExpirePoint;

    int spawnDelay;
    int spawnDelayExpirePoint;
    bool usingLineSpawnDelay;

    int lineClear;
    int lineClearExpirePoint;

    int dasInterval;
    int dasIntervalExpirePoint;

    /* the below are not fundamental, can be implemented in a derived struct */

    // int postLock;
    // unsigned int floorkicks;
    // int holdFlash;
};

class Polyomino
{
public:
    Polyomino()
    {
        for(int i = 0; i < 4; i++)
        {
            rotationTables[i] = NULL;
        }
    }

    Polyomino(std::vector<int *>& tableArrs, unsigned int size)
    {
        for(int i = 0; i < 4; i++)
        {
            rotationTables[i] = grid_from_1d_int_array(tableArrs[i], size, size);
        }
    }

    virtual ~Polyomino()
    {
        for(auto g : rotationTables)
        {
            grid_destroy(g);
        }
    }

    virtual void draw() {}

//protected:
    grid_t *rotationTables[4]; // these grids technically don't have to be the same size
};

class ActivatedPolyomino : public Polyomino
{
public:
    ActivatedPolyomino(Polyomino& p, int ID, int x, int y)
        : ID(ID), position(x, y), physicState(spm_physic_absent), orientation(spm_flat)
    {
        for(int i = 0; i < 4; i++)
        {
            this->rotationTables[i] = gridcpy(p.rotationTables[i], NULL);
        }
    }

    grid_t *currentRotationTable()
    {
        return rotationTables[static_cast<int>(orientation)];
    }

    void freeRotateCW() { ++orientation; }
    void freeRotateCCW() { --orientation; }
    void freeRotateFlip() { ++orientation; ++orientation; }

    void freeMoveLeft() { position.x--; }
    void freeMoveRight() { position.x++; }
    void freeMoveUp() { position.y--; }
    void freeMoveDown() { position.y++; }

    void freeMoveOffset(SPM_offset& offset) { position = position + offset; }

    SPM_minoID ID;
    SPM_point position;
    SPM_physicState physicState;
    SPM_orientation orientation;
};

struct SPM_Player
{
    SPM_Player()
    {
        randomizer = new SPM_NonRandomizer{7};
        mino = NULL;
        hold = NULL;
        playPhase = spm_player_control;
        timings.gravity = 8 * 256;
        timings.lockDelay = 30;
        timings.das = 12;
        timings.dasInterval = 0;
        timings.are = 25;
        timings.lineAre = 25;
        timings.lineClear = 40;
        minoSeqIndex = 0;
    }

    SPM_Randomizer *randomizer;

    ActivatedPolyomino *mino;
    std::vector<ActivatedPolyomino *> previews;
    ActivatedPolyomino *hold;

    SPM_playPhase playPhase;

    SPM_frameTimings timings;
    SPM_frameCounters counters;

    std::vector<SPM_minoID> minoSequence;
    unsigned int minoSeqIndex;
};

class SPM_Spec
{
public:
    SPM_Spec();
    virtual ~SPM_Spec()
    {
        for(auto p : minoList)
        {
            delete p;
        }
    }

    virtual int checkCollision(grid_t *field, ActivatedPolyomino& mino);
    virtual bool isGrounded(grid_t *field, ActivatedPolyomino& mino);

    virtual bool checkedShift(grid_t *field, ActivatedPolyomino& mino, SPM_offset offset);
    virtual bool checkedRotate(grid_t *field, ActivatedPolyomino& mino, SPM_orientation dir); // handles kicks internally via checkedShift
    virtual int checkedFall(grid_t *field, ActivatedPolyomino& mino, int subY);

    virtual ActivatedPolyomino *activatePolyomino(SPM_minoID ID);
    // virtual bool spawnMino(grid_t *field, ActivatedPolyomino& mino, struct keyflags& keys);

    virtual void imprintMino(grid_t *field, ActivatedPolyomino& mino);
    virtual int checkAndClearLines(grid_t *field);
    virtual void dropField(grid_t *field);

// protected:
    int fieldW;
    int fieldH;
    int visualFieldH;

    int spawnX;
    int spawnY;

    std::vector<Polyomino *> minoList;
    std::vector<uint32_t> minoBehaviorList;
    // SPM_Randomizer *randomizer;

    unsigned int numPreviews;
    bool allowHold;
};

#endif
