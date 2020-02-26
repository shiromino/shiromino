#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>

#include "Grid.hpp"
#include "piecedef.h"

using namespace Shiro;

piecedef *piecedef_create()
{
    piecedef* pd = new piecedef;

    pd->qrs_id = 0;
    pd->flags = 0;
    pd->anchorx = 0;
    pd->anchory = 0;

    for (int i = 0; i < 4; i++) {
        pd->rotation_tables[i] = Grid(4, 4);
    }

    return pd;
}

void piecedef_destroy(piecedef *pd)
{
    delete pd;
}

piecedef *piecedef_cpy(piecedef *pd)
{
    if(!pd)
        return NULL;

    piecedef* pd_new = new piecedef;

    pd_new->qrs_id = pd->qrs_id;
    pd_new->flags = pd->flags;
    pd_new->anchorx = pd->anchorx;
    pd_new->anchory = pd->anchory;

    int i = 0;
    for(i = 0; i < 4; i++)
        pd_new->rotation_tables[i] = pd->rotation_tables[i];

    return pd_new;
}

int pdsetw(piecedef *pd, int w)
{
    if(!pd)
        return -1;
    if(w < 1)
        return 1;
    if(pd->rotation_tables[0].getWidth() == w)
        return 0;

    int i = 0;

    for(i = 0; i < 4; i++)
    {
        pd->rotation_tables[i].setWidth(w);
    }

    return 0;
}

int pdseth(piecedef *pd, int h)
{
    if(!pd)
        return -1;
    if(h < 1)
        return 1;
    if(pd->rotation_tables[0].getHeight() == h)
        return 0;

    int i = 0;

    for(i = 0; i < 4; i++)
    {
        pd->rotation_tables[i].setHeight(h);
    }

    return 0;
}

int pdsetcell(piecedef *pd, int orientation, int x, int y)
{
    if(!pd)
        return -1;

    if(x < 0 || y < 0 || x >= pd->rotation_tables[0].getWidth() || y >= pd->rotation_tables[0].getHeight())
        return 1;

    Grid& g = pd->rotation_tables[orientation & 3];
    if (g.cell(x, y) ^= 1) {
        return 1;
    }

    return 0;
}
/*
int pdchkflags(piecedef *pd, unsigned int tflags)
{
   if(!pd)
      return -1;

   if((pd->flags & tflags) == tflags)
      return 1;

   return 0;
}*/
