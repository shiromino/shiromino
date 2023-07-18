#pragma once
#include "RotationTables.h"
#include "SPM_Structures.h"
#include "SPM_Spec.h"
#include <cstdint>
#include <vector>
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

/* convoluted QRS kick behavior flags */

#define QRS_KICK_NONE 0
#define QRS_KICK_WALL (1 << 1)
#define QRS_KICK_FLOOR (1 << 2)

#define QRS_KICK_PREFER_WALL (1 << 3)

#define QRS_FKICK_FLAT  (1 << 4)
#define QRS_FKICK_AIRBORNE (1 << 5)

/* */

enum QRS_variant
{
    qrs_variant_G1,
    qrs_variant_G2,
    qrs_variant_G3,
    qrs_variant_P
};

class QRS : public SPM_Spec
{
public:
    QRS(QRS_variant variant, bool doubles)
        : variant(variant)
    {
        if (variant == qrs_variant_P) {
            for (size_t i = 0; i < 18; i++) {
                polyominoes.push_back(Shiro::PentoRotationTables[i]);
                spawnPositions.push_back({ 4, 1 });
            }

            for (size_t i = 0; i < 7; i++) {
                spawnPositions.push_back({ 4, 2 });
            }
        }
        else {
            for (size_t i = 0; i < 7; i++) {
                spawnPositions.push_back({ 3, 2 });
            }
        }

        for (size_t i = 0; i < Shiro::TetroRotationTables.size(); i++) {
            polyominoes.push_back(Shiro::TetroRotationTables[i]);
        }


        allowHardDrop = true;

        switch(variant)
        {
            case qrs_variant_G1:
                allowHardDrop = false;
                [[fallthrough]];
            case qrs_variant_G2:
                fieldW = 10;
                numPreviews = 1;
                allowHold = false;

                break;

            case qrs_variant_G3:
                fieldW = 10;
                numPreviews = 3;
                allowHold = true;

                break;

            default:
            case qrs_variant_P:
                fieldW = 12;
                numPreviews = 4;
                allowHold = false;

                break;
        }

        if(doubles)
        {
            fieldW = 14;
        }

        fieldH = 23;
        visualFieldH = 20;

        softDropLock = true;
        hardDropLock = false;
    }

    // TODO: move sfx_play() calls from SPM_Spec functions to ShiroPhysoMino functions! TODO checkedFall, imprintMino, dropField(?)
    // using defaults for: everything other than checkedRotate lol

    bool wallkick(Shiro::Grid *field, ActivatedPolyomino& mino)
    {
        std::pair<int, int> pos;
        checkCollision(field, mino, pos);
        int x = pos.first;
        SPM_orientation o = mino.orientation;
        SPM_minoID shiftedID = mino.ID;

        if(variant != qrs_variant_P)
        {
            shiftedID += 18;
        }

        if(shiftedID == QRS_I4)
        {
            if(variant == qrs_variant_G1 || variant == qrs_variant_G2)
            {
                return false;
            }
        }

        switch(shiftedID)
        {
            case QRS_I:
            case QRS_I4:
                if(o == spm_clockwise || o == spm_counter_clockwise)
                {
                    return false;
                }

                break;

            case QRS_J:
                if((o == spm_clockwise || o == spm_counter_clockwise) && x == 2)
                {
                    return false;
                }

                break;

            case QRS_L:
            case QRS_L4:
            case QRS_J4:
            case QRS_T4:
                if((o == spm_clockwise || o == spm_counter_clockwise) && x == 1)
                {
                    return false;
                }

                break;

            case QRS_N:
            case QRS_G:
            case QRS_Ya:
            case QRS_Yb:
                if(o == spm_clockwise && x == 2)
                {
                    return false;
                }
                else if(o == spm_counter_clockwise && x == 1)
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
                switch(shiftedID)
                {
                    case QRS_I4:
                    case QRS_I:
                        break;

                    case QRS_J:
                    case QRS_L:
                    case QRS_Ya:
                    case QRS_Yb:
                        if(o == spm_clockwise || o == spm_counter_clockwise)
                        {
                            return false;
                        }

                        break;

                    default:
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

    bool floorkick(Shiro::Grid *field, ActivatedPolyomino& mino)
    {
        if(variant == qrs_variant_G1 || variant == qrs_variant_G2)
        {
            return false;
        }

        SPM_minoID shiftedID = mino.ID;

        if(variant == qrs_variant_G3)
        {
            shiftedID += 18;
        }

        switch(shiftedID)
        {
            case QRS_X:
            case QRS_S:
            case QRS_Z:
            case QRS_U:
            case QRS_Fa:
            case QRS_Fb:
            case QRS_P:
            case QRS_Q:
            case QRS_W:

            case QRS_J4:
            case QRS_L4:
            case QRS_O:
            case QRS_S4:
            case QRS_Z4:
                return false;

            default:
                break;
        }

        if(shiftedID == QRS_T || shiftedID == QRS_T4)
        {
            if(mino.orientation != spm_flip)
            {
                return false;
            }
        }
        else
        {
            if(mino.orientation == spm_flat || mino.orientation == spm_flip || mino.physicState != spm_physic_grounded)
            {
                return false;
            }
        }

        if(checkedShift(field, mino, {0, -1}) == true)
        {
            return true;
        }

        if(shiftedID == QRS_T4)
        {
            return false;
        }

        return checkedShift(field, mino, {0, -2});
    }

    virtual bool checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir) override
    {
        SPM_orientation old = mino.orientation;

        SPM_minoID shiftedID = mino.ID;

        if(variant != qrs_variant_P)
        {
            shiftedID += 18;
        }

        mino.orientation = static_cast<SPM_orientation>((static_cast<int>(mino.orientation) + static_cast<int>(dir)) % 4);

        if(checkCollision(field, mino))
        {
            if(shiftedID == QRS_T4)
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

protected:
    QRS_variant variant;
};