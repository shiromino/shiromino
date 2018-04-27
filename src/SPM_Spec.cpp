#include "SPM_Spec.hpp"

#include <iostream>
#include <vector>

#include "rotation_tables.h"

using namespace std;

SPM_Spec::SPM_Spec()
{
    vector<int *> tableArrs;

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

    for(int i = 0; i < 7; i++)
    {
        minoBehaviorList.push_back(0);
    }

    fieldW = 10;
    fieldH = 23;
    visualFieldH = 20;
    spawnX = 3;
    spawnY = 2;

    numPreviews = 1;
    allowHold = false;
}

int SPM_Spec::checkCollision(grid_t *field, ActivatedPolyomino& mino)
{
    grid_t *d = mino.currentRotationTable();

    int d_x = 0;
    int d_y = 0;
    int d_val = 0;

    int f_x = 0;
    int f_y = 0;
    int f_val = 0;

    int s = d->w * d->h;

    for(int i = 0; i < s; i++)
    {
        d_x = gridpostox(d, i);
        d_y = gridpostoy(d, i);
        f_x = mino.position.x + d_x;
        f_y = mino.position.y + d_y;

        d_val = gridgetcell(d, d_x, d_y);
        f_val = gridgetcell(field, f_x, f_y);

        if(d_val && f_val)
        {
            // gridgetcell returns 8128 on out of bounds, so it will default to collision = true

            // the +1 slightly confuses things, but is required for cases where the collision is at position = 0
            // this way we don't return 0 (== no collision) when there in fact was a collision
            // TODO put in a macro for COLLISION_FALSE, set it to some non-zero value?
            return i + 1;
        }
    }

    return 0;
}

bool SPM_Spec::isGrounded(grid_t *field, ActivatedPolyomino& mino)
{
    mino.position.y++;
    if(checkCollision(field, mino))
    {
        mino.position.y--;
        return true;
    }

    mino.position.y--;
    return false;
}

bool SPM_Spec::checkedShift(grid_t *field, ActivatedPolyomino& mino, SPM_offset offset)
{
    mino.position = mino.position + offset;
    if(checkCollision(field, mino))
    {
        mino.position = mino.position - offset;
        return false;
    }

    return true;
}

bool SPM_Spec::checkedRotate(grid_t *field, ActivatedPolyomino& mino, SPM_orientation dir)
{
    SPM_orientation old = mino.orientation;
    mino.orientation = static_cast<SPM_orientation>((static_cast<int>(mino.orientation) + static_cast<int>(dir)) % 4);

    if(checkCollision(field, mino))
    {
        mino.orientation = old;
        return false;
    }

    return true;
}

int SPM_Spec::checkedFall(grid_t *field, ActivatedPolyomino& mino, int subY)
{
    SPM_point backupPos = mino.position;

    SPM_offset grav = {0, (subY / SPM_SUBUNIT_SCALE), 0, (subY % SPM_SUBUNIT_SCALE)};
    SPM_point maximum = mino.position + grav;

    while((mino.position.y * SPM_SUBUNIT_SCALE + mino.position.subY) < (maximum.y * SPM_SUBUNIT_SCALE + maximum.subY))
    {
        mino.position.y++;
        if(checkCollision(field, mino))
        {
            mino.position.y--;
            mino.position.subY = 0;
            if(mino.physicState == spm_physic_falling)
            {
                // sfx_play()
            }

            mino.physicState = spm_physic_grounded;
            return -1;
        }
    }

    if((mino.position.y * SPM_SUBUNIT_SCALE + mino.position.subY) > (maximum.y * SPM_SUBUNIT_SCALE + maximum.subY))
    {
        mino.position = maximum;
    }

    return mino.position.y - backupPos.y;
}

ActivatedPolyomino *SPM_Spec::activatePolyomino(SPM_minoID ID)
{
    if(ID >= minoList.size())
    {
        return NULL;
    }

    ActivatedPolyomino *ap = new ActivatedPolyomino{*minoList[ID], ID, spawnX, spawnY};
    return ap;
}

void SPM_Spec::imprintMino(grid_t *field, ActivatedPolyomino& mino)
{
    grid_t *d = mino.currentRotationTable();

    for(int i = 0; i < (d->w * d->h); i++)
    {
        int from_x = gridpostox(d, i);
        int from_y = gridpostoy(d, i);
        int to_x = mino.position.x + from_x;
        int to_y = mino.position.y + from_y;

        if(gridgetcell(d, from_x, from_y))
        {
            gridsetcell(field, to_x, to_y, mino.ID + 1);
        }
    }

    // sfx_play(&g->origin->assets->lock);
}

int SPM_Spec::checkAndClearLines(grid_t *field)
{
    int n = 0;

    for(int i = 0; i < field->h; i++)
    {
        int cells = 0;

        for(int j = 0; j < field->w; j++)
        {
            if(gridgetcell(field, j, i))
            {
                cells++;
            }
        }

        if(cells == field->w)
        {
            n++;

            for(int j = 0; j < field->w; j++)
            {
                gridsetcell(field, j, i, SPM_CELL_CLEARED);
            }
        }
    }

    return n;
}

void SPM_Spec::dropField(grid_t *field)
{
    int n = 0;

    for(int i = field->h - 1; i >= 0; i--)
    {
        while(gridgetcell(field, 0, i - n) == SPM_CELL_CLEARED)
        {
            n++;
        }

        if(i - n >= 0)
        {
            gridrowcpy(field, NULL, i - n, i);
        }
        else
        {
            for(int j = 0; j < field->w; j++)
            {
                gridsetcell(field, j, i, 0);
            }
        }
    }
}
