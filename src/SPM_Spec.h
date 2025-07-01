#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>
#include "Grid.h"
#include "RotationTables.h"

#define SPM_SUBUNIT_SCALE 65536

#define MINO_ID_INVALID 0xFFFFFFFF
#define SPM_FALL_LANDED -1

#define SPM_CELL_CLEARED -2

typedef unsigned int SPM_minoID;

struct SPM_point
{
    SPM_point() : x(0), y(0), subX(0), subY(0) {}
    SPM_point(int x, int y) : x(x), y(y), subX(0), subY(0) {}
    SPM_point(int x, int y, int32_t subX, int32_t subY) : x(x), y(y), subX(subX), subY(subY) {}

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
    SPM_frameTimings() : gravity(0), lockDelay(0), das(0), dasInterval(0), are(0), lineAre(0), lineClear(0) {}

    int gravity;
    int lockDelay;
    int das;
    int dasInterval;
    int are;
    int lineAre;
    int lineClear;
};

struct SPM_gameCounters
{
    SPM_gameCounters()
    {
        frameWait = 0;
        frameWaitExpirePoint = 0;

        gamePhase = 0;
        gamePhaseExpirePoint = 0;

        transition = 0;
        transitionExpirePoint = 0;
    }

    int frameWait; // could be used for slow-down: if(frameWait != 0) {frameWait--; return;}
    int frameWaitExpirePoint;

    int gamePhase; // counter for how long to stay in the current game phase
    int gamePhaseExpirePoint;

    int transition;
    int transitionExpirePoint;
};

struct SPM_frameCounters
{
    SPM_frameCounters()
    {
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

// TODO: Templatize the 2D dimensions of the rotations.
class Polyomino {
public:
    Polyomino() {}
    virtual ~Polyomino() {}

    template<const std::size_t rotationWidth, const std::size_t rotationHeight>
    Polyomino(const std::array<std::array<std::array<bool, rotationWidth>, rotationHeight>, 4>& rotationTables) {
        for (int i = 0; i < 4; i++) {
            this->rotationTables[i] = rotationTables[i];
        }
    }

    virtual void draw() {}

    std::array<Shiro::Grid, 4> rotationTables; // these grids technically don't have to be the same size
};

class ActivatedPolyomino : public Polyomino {
public:
    ActivatedPolyomino(Polyomino& p, SPM_minoID ID, SPM_point position)
        : ID(ID), position(position), physicState(spm_physic_absent), orientation(spm_flat) {
        rotationTables = p.rotationTables;
    }

    ~ActivatedPolyomino() {}

    const Shiro::Grid& currentRotationTable()
    {
        return rotationTables[static_cast<std::size_t>(orientation)];
    }

    virtual int codedCellValue()
    {
        return ID + 1;
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

class SPM_Spec {
public:
    SPM_Spec() :
        fieldW(0),
        fieldH(0),
        visualFieldH(0),
        numPreviews(0u),
        allowHold(false),
        allowHardDrop(false),
        softDropLock(false),
        hardDropLock(false) {}
    virtual ~SPM_Spec() {}

    virtual bool checkCollision(Shiro::Grid* field, ActivatedPolyomino& mino);
    virtual bool checkCollision(Shiro::Grid* field, ActivatedPolyomino& mino, std::pair<int, int>& pos);
    virtual bool isGrounded(Shiro::Grid *field, ActivatedPolyomino& mino);

    virtual bool checkedShift(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_offset offset);
    virtual bool checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir);
    virtual int checkedFall(Shiro::Grid *field, ActivatedPolyomino& mino, int subY);

    virtual void imprintMino(Shiro::Grid *field, ActivatedPolyomino& mino);
    virtual int checkAndClearLines(Shiro::Grid *field, int bound);
    virtual void dropField(Shiro::Grid *field);

    int fieldW;
    int fieldH;
    int visualFieldH;

    std::vector<SPM_point> spawnPositions;
    std::vector<Polyomino> polyominoes;

    // TODO: no reset vs. step reset vs. move reset, as well as possibility for other options?

    unsigned int numPreviews;
    bool allowHold;
    bool allowHardDrop;

    bool softDropLock;
    bool hardDropLock;
};

class SPM_TestSpec : public SPM_Spec {
public:
    SPM_TestSpec() {
        for (int i = 0; i < 7; i++) {
            polyominoes.push_back(Shiro::TetroRotationTables[i]);

            spawnPositions.push_back( {3, 2} );
        }

        fieldW = 10;
        fieldH = 23;
        visualFieldH = 20;

        numPreviews = 1;
        allowHold = false;
        allowHardDrop = true;

        softDropLock = true;
        hardDropLock = false;
    }
};
