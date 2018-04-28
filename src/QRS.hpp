#ifndef _qrs_hpp
#define _qrs_hpp

#include <cstdint>

#include "SPM_Spec.hpp"
#include "rotation_tables.h"

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

// IZSJLOT - arkia's arbitrary tetromino ordering
#define ARS_I 0
#define ARS_Z 1
#define ARS_S 2
#define ARS_J 3
#define ARS_L 4
#define ARS_O 5
#define ARS_T 6

#define QRS_KICK_NONE 0
#define QRS_KICK_WALL (1 << 1)
#define QRS_KICK_FLOOR (1 << 2)

#define QRS_KICK_PREFER_WALL (1 << 3)

#define QRS_FKICK_FLAT  (1 << 4)
#define QRS_FKICK_AIRBORNE (1 << 5)

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
        vector<int *> tableArrs;

        if(variant == qrs_variant_P)
        {
            for(int i = 0; i < 18; i++)
            {
                tableArrs.clear();

                for(int j = 0; j < 4; j++)
                {
                    tableArrs.push_back( (int *)(qrspent_yx_rotation_tables[i][j]) );
                }

                Polyomino *p = new Polyomino{tableArrs, 5};
                minoList.push_back(p);
            }
        }

        for(int i = 0; i < 7; i++)
        {
            tableArrs.clear();

            for(int j = 0; j < 4; j++)
            {
                tableArrs.push_back( (int *)(qrstet_yx_rotation_tables[i][j]) );
            }

            Polyomino *p = new Polyomino{tableArrs, 4};
            minoList.push_back(p);
        }

        allowHardDrop = true;

        switch(variant)
        {
            case qrs_variant_G1:
                allowHardDrop = false;
            case qrs_variant_G2:
                fieldW = 10;
                numPreviews = 1;
                allowHold = false;

                for(int i = 0; i < 7; i++)
                {
                    minoFloorkickHeights.push_back(0);
                }

                break;

            case qrs_variant_G3:
                fieldW = 10;
                numPreviews = 3;
                allowHold = true;

                for(int i = 0; i < 7; i++)
                {
                    minoFloorkickHeights.push_back(0);
                }

                minoFloorkickHeights[QRS_ARS_I] = 2;
                minoFloorkickHeights[QRS_ARS_T] = 1;

                break;

            case qrs_variant_P:
                fieldW = 12;
                numPreviews = 4;
                allowHold = false;

                for(int i = 0; i < 25; i++)
                {
                    minoFloorkickHeights.push_back()
                }
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
    // using defaults for: checkCollision, isGrounded, checkedShift, checkedFall, imprintMino, checkAndClearLines, dropField

    bool checkedRotate(grid_t *field, ActivatedPolyomino& mino, SPM_orientation dir)
    {

    }

protected:
    QRS_variant variant;

    std::vector<uint32_t> minoKickBehavior;
    std::vector<int> minoFloorkickHeights;
};

#endif // _qrs_hpp
