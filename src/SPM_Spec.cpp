#include "SPM_Spec.hpp"

#include <iostream>
#include <vector>

using namespace Shiro;
using namespace std;

bool SPM_Spec::checkCollision(Grid* field, ActivatedPolyomino& mino) {
    pair<int, int> pos;
    return checkCollision(field, mino, pos);
}
bool SPM_Spec::checkCollision(Grid *field, ActivatedPolyomino& mino, pair<int, int>& pos) {
    Grid d = mino.currentRotationTable();

    int d_x = 0;
    int d_y = 0;

    int f_x = 0;
    int f_y = 0;

    for (d_y = 0, f_y = mino.position.y; d_y < d.getHeight(); d_y++, f_y++) {
        for (d_x = 0, f_x = mino.position.x; d_x < d.getWidth(); d_x++, f_x++) {
            if (d.getCell(d_x, d_y) && field->getCell(f_x, f_y)) {
                pos = pair<int, int>(d_x, d_y);
                return true;
            }
        }
    }

    return false;
}

bool SPM_Spec::isGrounded(Grid *field, ActivatedPolyomino& mino)
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

bool SPM_Spec::checkedShift(Grid *field, ActivatedPolyomino& mino, SPM_offset offset)
{
    mino.position = mino.position + offset;
    if(checkCollision(field, mino))
    {
        mino.position = mino.position - offset;
        return false;
    }

    return true;
}

bool SPM_Spec::checkedRotate(Grid *field, ActivatedPolyomino& mino, SPM_orientation dir)
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

int SPM_Spec::checkedFall(Grid *field, ActivatedPolyomino& mino, int subY)
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

            mino.physicState = spm_physic_grounded;
            return SPM_FALL_LANDED;
        }
    }

    if((mino.position.y * SPM_SUBUNIT_SCALE + mino.position.subY) > (maximum.y * SPM_SUBUNIT_SCALE + maximum.subY))
    {
        mino.position = maximum;
    }

    return mino.position.y - backupPos.y;
}

void SPM_Spec::imprintMino(Grid *field, ActivatedPolyomino& mino)
{
    Grid d = mino.currentRotationTable();

    for (int from_y = 0, to_y = mino.position.y; from_y < d.getHeight(); from_y++, to_y++) {
        for (int from_x = 0, to_x = mino.position.x; from_x < d.getWidth(); from_x++, to_x++) {
            if (d.getCell(from_x, from_y)) {
                field->cell(to_x, to_y) = mino.codedCellValue();
            }
        }
    }

    // sfx_play(&g->origin->assets->lock);
}

int SPM_Spec::checkAndClearLines(Grid *field, int bound)
{
    int n = 0;

    for(int i = 0; i < field->getHeight() && i < bound; i++)
    {
        int cells = 0;

        for(int j = 0; j < field->getWidth(); j++)
        {
            if (field->getCell(j, i)) {
                cells++;
            }
        }

        if(cells == field->getWidth())
        {
            n++;

            for(int j = 0; j < field->getWidth(); j++)
            {
                field->cell(j, i) = SPM_CELL_CLEARED;
            }
        }
    }

    return n;
}

void SPM_Spec::dropField(Grid *field)
{
    int n = 0;

    for(int i = (int)field->getHeight() - 1; i >= 0; i--)
    {
        while (field->getCell(0, i - n) == SPM_CELL_CLEARED) {
            n++;
        }

        if(i - n >= 0)
        // if the row to copy downward is within bounds
        {
            field->copyRow(i - n, i);
        }
        else
        // make an empty row
        {
            for(int j = 0; j < field->getWidth(); j++)
            {
                field->cell(j, i) = 0;
            }
        }
    }
}
