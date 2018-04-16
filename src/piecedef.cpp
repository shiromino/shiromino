#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "piecedef.h"
#include "grid.h"

piecedef *piecedef_create()
{
   piecedef *pd = (piecedef *) malloc(sizeof(piecedef));

   pd->qrs_id = 0;
   pd->flags = 0;
   pd->anchorx = 0;
   pd->anchory = 0;

   int i = 0;
   for(i = 0; i < 4; i++)
      pd->rotation_tables[i] = grid_create(4, 4);

   return pd;
}

void piecedef_destroy(piecedef *pd)
{
   if(!pd)
      return;

   int i = 0;
   for(i = 0; i < 4; i++)
      grid_destroy(pd->rotation_tables[i]);

   free(pd);
}

piecedef *piecedef_cpy(piecedef *pd)
{
    if(!pd)
        return NULL;

    piecedef *pd_new = (piecedef *) malloc(sizeof(piecedef));

    pd_new->qrs_id = pd->qrs_id;
    pd_new->flags = pd->flags;
    pd_new->anchorx = pd->anchorx;
    pd_new->anchory = pd->anchory;

    int i = 0;
    for(i = 0; i < 4; i++)
        pd_new->rotation_tables[i] = gridcpy(pd->rotation_tables[i], NULL);

    return pd_new;
}

int pdsetw(piecedef *pd, int w)
{
   if(!pd)
      return -1;
   if(w < 1)
      return 1;
   if(pd->rotation_tables[0]->w == w)
      return 0;

   int i = 0;

   for(i = 0; i < 4; i++) {
      if(gridsetw(pd->rotation_tables[i], w))
         return 1;
   }

   return 0;
}

int pdseth(piecedef *pd, int h)
{
   if(!pd)
      return -1;
   if(h < 1)
      return 1;
   if(pd->rotation_tables[0]->h == h)
      return 0;

   int i = 0;

   for(i = 0; i < 4; i++) {
      if(gridseth(pd->rotation_tables[i], h))
         return 1;
   }

   return 0;
}

int pdsetcell(piecedef *pd, int orientation, int x, int y)
{
   if(!pd)
      return -1;

   if(x < 0 || y < 0 || x >= pd->rotation_tables[0]->w || y >= pd->rotation_tables[0]->h)
      return 1;

   grid_t *g = pd->rotation_tables[orientation & 3];
   int val = gridgetcell(g, x, y);

   if( gridsetcell(g, x, y, (val ^ 1)) )
      return 1;

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
