#ifndef _spm_spec_hpp
#define _spm_spec_hpp

#include <cstdint>
#include "SGUIL/SGUIL.hpp"

struct SPM_Point
{
    SPM_Point() : x(0), y(0), subX(0), subY(0) {}
    SPM_Point(int x, int y) : x(x), y(y), subX(0), subY(0) {}
    SPM_Point(int x, int y, uint32_t subX, uint32_t subY) : x(x), y(y), subX(subX), subY(subY) {}

    int x;
    int y;

    int32_t subX;
    int32_t subY;
}

typedef SPM_Point SPM_Offset;

SPM_Point operator + (SPM_Point& a, SPM_Offset& b)
{
    SPM_Point result {}

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    if(b.subX > 0xFFFF)
    {
        b.subX = 0xFFFF;
    }
    else if(b.subX < -0xFFFF)
    {
        b.subX = -0xFFFF;
    }

    if(b.subY > 0xFFFF)
    {
        b.subY = 0xFFFF;
    }
    else if(b.subY < -0xFFFF)
    {
        b.subY = -0xFFFF;
    }

    result.subX = a.subX + b.subX;
    result.subY = a.subY + b.subY;

    if(result.subX > 0xFFFF)
    {
        result.subX -= 0x10000;
        result.x++;
    }
    else if(result.subX < 0)
    {
        result.subX += 0x10000;
        result.x--;
    }

    if(result.subY > 0xFFFF)
    {
        result.subY -= 0x10000;
        result.y++;
    }
    else if(result.subY < 0)
    {
        result.subY += 0x10000;
        result.y--;
    }

    return result;
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
    spm_piece_control,
    spm_line_clear
};

enum SPM_physicState
{
    spm_physic_locked = -1,
    spm_physic_absent = 0,
    spm_physic_falling = 1,
    spm_physic_grounded
};


SPM_physicState operator bool (SPM_physicState& a)
{ return static_cast<int>(a) != 0; }

SPM_physicState operator > (SPM_physicState& a, int val)
{ return static_cast<int>(a) > val; }


enum SPM_orientation
{
    spm_flat = 0,
    spm_clockwise = 1,
    spm_flip = 2,
    spm_counter_clockwise = 3
};


SPM_orientation operator ++ (SPM_orientation& a)
{
    int o = static_cast<int>(a)++;
    return (o == 4) ? spm_flat : static_cast<SPM_orientation>(o);
}

SPM_orientation operator -- (SPM_orientation& a)
{
    int o = static_cast<int>(a)--;
    return (o == -1) ? spm_counter_clockwise : static_cast<SPM_orientation>(o);
}


struct SPM_FrameTimings
{
    SPM_FrameTimings() : gravity(0), lockDelay(0), das(0), are(0), lineAre(0), lineClear(0) {}

    int gravity;
    int lockDelay;
    int das;
    int dasInterval;
    int are;
    int lineAre;
    int lineClear;
};

struct SPM_FrameCounters
{
    SPM_FrameCounters() : transition(0), lock(0), are(0), lineAre(0), lineClear(0) {}

    int frameWait; // could be used for slow-down: if(frameWait != 0) {frameWait--; return;}
    int transition;

    int grounded;
    int are;
    int lineAre;
    int lineClear;

    /* the below are not fundamental, can be implemented in a derived struct */

    // unsigned int floorkicks;
    // int holdFlash;
};

class Polyomino
{
public:
    Polyomino(int **tableArrs, unsigned int size)
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

    virtual draw();

protected:
    grid_t *rotationTables[4]; // these grids technically don't have to be the same size
};

class ActivePolyomino : public Polyomino
{
public:
    ActivePolyomino(Polyomino& p, int ID, int x, int y)
        : ID(ID), position(x, y), physicState(spm_physic_absent), orientation(spm_flat)
    {
        for(int i = 0; i < 4; i++)
        {
            this->rotationTables[i] = gridcpy(p.rotationTables[i], NULL);
        }
    }

    void freeRotateCW() { orientation++; }
    void freeRotateCCW() { orientation-- }
    void freeRotateFlip() { orientation++; orientation++; }

    void freeMoveLeft() { position.x--; }
    void freeMoveRight() { position.x++; }
    void freeMoveUp() { position.y--; }
    void freeMoveDown() { position.y++; }

    void freeMoveOffset(SPM_Offset& offset) { position = position + offset; }

    int ID;

protected:
    SPM_Point position;
    SPM_physicState physicState;
    SPM_orientation orientation;
};

#endif
