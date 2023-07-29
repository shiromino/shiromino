#include "SPM_Structures.h"
#include "SPM_Spec.h"
#include "MinoCanonicalID.h"
#include "Grid.h"
#include "ARS.h"
#include <cstdint>
#include <vector>

ARS::ARS(bool isG2) : SPM_Spec()
{
    softDropLock = true;
    hardDropLock = false;

    if(isG2)
    {
        randomizer = new G2_Randomizer;
        allowHardDrop = true;
    }
    else
    {
        randomizer = new G1_Randomizer;
        allowHardDrop = false;
    }

    fieldW = 10;
    fieldH = 23;
    visualFieldH = 20;

    numPreviews = 1;

    useLockDelay = true;
    initialRotate = true;

    rotateA = spm_counter_clockwise;
    rotateB = spm_clockwise;
    rotateC = spm_counter_clockwise;
    buttonDAction = spm_do_nothing;

    for(size_t i = 0; i < Shiro::TetroRotationTables.size(); i++)
    {
        polyominoes.push_back(Shiro::TetroRotationTables[i]);
        spawnPositions.push_back({ 3, 2 });
    }
}

ARS3::ARS3() : SPM_Spec()
{
    randomizer = new G3_Randomizer;
    allowHardDrop = true;
    softDropLock = true;
    hardDropLock = false;

    fieldW = 10;
    fieldH = 23;
    visualFieldH = 20;

    numPreviews = 3;

    useLockDelay = true;
    initialRotate = true;
    initialHold = true;

    rotateA = spm_counter_clockwise;
    rotateB = spm_clockwise;
    rotateC = spm_counter_clockwise;
    buttonDAction = spm_hold;

    for(size_t i = 0; i < Shiro::TetroRotationTables.size(); i++)
    {
        polyominoes.push_back(Shiro::TetroRotationTables[i]);
        spawnPositions.push_back({ 3, 2 });
    }
}

bool ARS::wallkick(Shiro::Grid *field, ActivatedPolyomino& mino)
{
    if(mino.ID == Shiro::Mino::I4)
    {
        return false;
    }

    std::pair<int, int> pos;
    checkCollision(field, mino, pos);
    int x = pos.first;

    switch(mino.ID)
    {
        case Shiro::Mino::L4:
        case Shiro::Mino::J4:
        case Shiro::Mino::T4:
            // rule for 3-wide pieces: don't allow a kick if collision is on piece's center column
            if(x == 1 && (mino.orientation == spm_clockwise || mino.orientation == spm_counter_clockwise))
            {
                return false;
            }

            break;

        default:
            break;
    }

    if(checkedShift(field, mino, {1, 0}) == false)
    {
        if(checkedShift(field, mino, {-1, 0}) == false)
        {
            return false;
        }
    }

    return true;
}

bool ARS::checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir)
{
    SPM_orientation old = mino.orientation;
    mino.orientation = mino.orientation + dir;

    if(checkCollision(field, mino))
    {
        if(wallkick(field, mino) == false)
        {
            mino.orientation = old;
            return false;
        }
    }

    return true;
}

bool ARS3::wallkick(Shiro::Grid *field, ActivatedPolyomino& mino)
{
    std::pair<int, int> pos;
    checkCollision(field, mino, pos);
    int x = pos.first;

    switch(mino.ID)
    {
        case Shiro::Mino::I4:
            if(mino.orientation == spm_clockwise || mino.orientation == spm_counter_clockwise)
            {
                return false;
            }

            break;

        case Shiro::Mino::L4:
        case Shiro::Mino::J4:
        case Shiro::Mino::T4:
            // rule for 3-wide pieces: don't allow a kick if collision is on piece's center column
            if(x == 1 && (mino.orientation == spm_clockwise || mino.orientation == spm_counter_clockwise))
            {
                return false;
            }

            break;
        
        case Shiro::Mino::O4:
            return false;

        default:
            break;
    }

    if(checkedShift(field, mino, {1, 0}) == false)
    {
        if(checkedShift(field, mino, {-1, 0}) == false)
        {
            if(mino.ID != Shiro::Mino::I4)
            {
                return false;
            }

            if(checkedShift(field, mino, {2, 0}) == false)
            {
                return checkedShift(field, mino, {-2, 0});
            }
        }
    }

    return true;
}

bool ARS3::floorkick(Shiro::Grid *field, ActivatedPolyomino& mino)
{
    if(mino.ID != Shiro::Mino::T4 && mino.ID != Shiro::Mino::I4)
    {
        return false;
    }

    if(mino.ID == Shiro::Mino::T4 && mino.orientation != spm_flip)
    {
        return false;
    }

    if(mino.ID == Shiro::Mino::I4 && (mino.orientation == spm_flat || mino.orientation == spm_flip || mino.physicState != spm_physic_grounded))
    {
        return false;
    }

    if(checkedShift(field, mino, {0, -1}) == true)
    {
        return true;
    }

    if(mino.ID == Shiro::Mino::T4)
    {
        return false;
    }

    return checkedShift(field, mino, {0, -2});
}

bool ARS3::checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir)
{
    SPM_orientation old = mino.orientation;
    mino.orientation = mino.orientation + dir;

    if(checkCollision(field, mino))
    {
        if(mino.ID == Shiro::Mino::T4)
        // T tetromino prefers a wallkick; everything else prefers a floorkick
        {
            if(wallkick(field, mino) == false)
            {
                if(floorkick(field, mino) == false)
                {
                    mino.orientation = old;
                    return false;
                }
            }
        }
        else
        {
            if(floorkick(field, mino) == false)
            {
                if(wallkick(field, mino) == false)
                {
                    mino.orientation = old;
                    return false;
                }
            }
        }
    }

    return true;
}