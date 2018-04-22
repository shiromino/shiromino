#include <stdint.h>
#include <stdlib.h>

#include "grid.h"

grid_t *grid_create(int w, int h) // allocate first horizontally, then vertically
{
    if(w < 1 || h < 1)
        return NULL;

    int i = 0;
    int j = 0;

    grid_t *g = (grid_t *)malloc(sizeof(grid_t));
    g->w = w;
    g->h = h;
    g->grid = (int **)malloc(w * sizeof(int *));

    for(i = 0; i < w; i++)
        g->grid[i] = (int *)malloc(h * sizeof(int));

    for(i = 0; i < w; i++)
    {
        for(j = 0; j < h; j++)
            g->grid[i][j] = 0;
    }

    return g;
}

void grid_destroy(grid_t *g)
{
    if(!g)
        return;

    int i = 0;

    for(i = 0; i < g->w; i++)
        free(g->grid[i]);

    free(g->grid);
    free(g);
}

int gridsetw(grid_t *g, int w)
{
    if(!g)
        return -1;
    if(w < 1)
        return 1;

    grid_t *tmp = grid_create(g->w, g->h);
    gridcpy(g, tmp);
    grid_destroy(g);

    g = grid_create(w, tmp->h);
    gridcpy(tmp, g);
    grid_destroy(tmp);

    return 0;
}

int gridseth(grid_t *g, int h)
{
    if(!g)
        return -1;
    if(h < 1)
        return 1;

    grid_t *tmp = grid_create(g->w, g->h);
    gridcpy(g, tmp);
    grid_destroy(g);

    g = grid_create(tmp->w, h);
    gridcpy(tmp, g);
    grid_destroy(tmp);

    return 0;
}

int gridsetcell(grid_t *g, int x, int y, int val)
{
    if(!g)
        return -1;
    if(x < 0 || y < 0 || x >= g->w || y >= g->h)
        return GRID_OOB;
    if(val == GRID_OOB)
        return 1;

    g->grid[x][y] = val;

    return 0;
}

int gridxorcell(grid_t *g, int x, int y, int xorVal)
{
    if(!g)
        return -1;
    if(x < 0 || y < 0 || x >= g->w || y >= g->h)
        return GRID_OOB;
    if(xorVal == GRID_OOB)
        return 1;

    g->grid[x][y] ^= xorVal;

    return 0;
}

int gridfillrect(grid_t *g, grid_rect *rect, int val)
{
    if(!g)
    {
        return -1;
    }

    grid_rect defaultRect = {0, 0, g->w, g->h};

    if(!rect)
    {
        rect = &defaultRect;
    }

    int startX = rect->x;
    int endX = rect->x + rect->w;
    int startY = rect->y;
    int endY = rect->y + rect->h;

    if(startX < 0)
    {
        startX = 0;
    }
    if(endX > g->w)
    {
        endX = g->w;
    }
    if(startY < 0)
    {
        startY = 0;
    }
    if(endY > g->h)
    {
        endY = g->h;
    }

    for(int i = startX; i < endX; i++)
    {
        for(int j = startY; j < endY; j++)
        {
            g->grid[i][j] = val;
        }
    }

    return 0;
}

grid_t *gridfromsrcrect(grid_t *src, grid_rect& rect)
{
    if(!src)
    {
        return NULL;
    }

    int startX = rect.x;
    int endX = rect.x + rect.w;
    int startY = rect.y;
    int endY = rect.y + rect.h;

    if(startX < 0)
    {
        startX = 0;
    }
    if(endX > src->w)
    {
        endX = src->w;
    }
    if(startY < 0)
    {
        startY = 0;
    }
    if(endY > src->h)
    {
        endY = src->h;
    }

    grid_t *g = grid_create(endX - startX, endY - startY);
    int x = 0;
    int y = 0;

    for(int i = startX; i < endX; i++)
    {
        for(int j = startY; j < endY; j++)
        {
            g->grid[x][y] = src->grid[i][j];

            y++;
        }

        x++;
        y = 0;
    }

    return g;
}

int gridcpyrect(grid_t *src, grid_t *dest, grid_rect *srcrect, grid_rect *destrect)
{
    if(!src || !dest)
    {
        return -1;
    }

    grid_rect defaultSrcRect = {0, 0, src->w, src->h};
    grid_rect defaultDestRect = {0, 0, dest->w, dest->h};

    if(!srcrect)
    {
        srcrect = &defaultSrcRect;
    }

    if(!destrect)
    {
        destrect = &defaultDestRect;
    }

    int startX = srcrect->x;
    int endX = srcrect->x + srcrect->w;
    int startY = srcrect->y;
    int endY = srcrect->y + srcrect->h;

    if(startX < 0)
    {
        startX = 0;
    }
    if(endX > src->w)
    {
        endX = src->w;
    }
    if(startY < 0)
    {
        startY = 0;
    }
    if(endY > src->h)
    {
        endY = src->h;
    }

    int destStartX = destrect->x;
    int destEndX = destrect->x + destrect->w;
    int destStartY = destrect->y;
    int destEndY = destrect->y + destrect->h;

    if(destStartX < 0)
    {
        destStartX = 0;
    }
    if(destEndX > dest->w)
    {
        destEndX = dest->w;
    }
    if(destStartY < 0)
    {
        destStartY = 0;
    }
    if(destEndY > dest->h)
    {
        destEndY = dest->h;
    }

    if((endX - startX) < (destEndX - destStartX))
    {
        destEndX = destStartX + (endX - startX);
    }
    else if((destEndX - destStartX) < (endX - startX))
    {
        endX = startX + (destEndX - destStartX);
    }

    if((endY - startY) < (destEndY - destStartY))
    {
        destEndY = destStartY + (endY - startY);
    }
    else if((destEndY - destStartY) < (endY - startY))
    {
        endY = startY + (destEndY - destStartY);
    }

    int destX = destStartX;
    int destY = destStartY;

    for(int i = startX; i < endX; i++)
    {
        for(int j = startY; j < endY; j++)
        {
            dest->grid[destX][destY] = src->grid[i][j];

            destY++;
        }

        destX++;
        destY = destStartY;
    }

    return 0;
}

int gridgetcell(grid_t *g, int x, int y)
{
    if(!g)
        return -1;
    if(x < 0 || y < 0 || x >= g->w || y >= g->h)
        return GRID_OOB;

    return g->grid[x][y];
}

grid_t *gridcpy(grid_t *src, grid_t *dest)
{
    if(!src)
        return NULL;

    int i = 0;
    int j = 0;
    int w = src->w;
    int h = src->h;

    grid_t *cpy = NULL;

    if(dest)
    {
        w = (src->w < dest->w ? src->w : dest->w);
        h = (src->h < dest->h ? src->h : dest->h);
        cpy = dest;
    }
    else
        cpy = grid_create(w, h);

    for(i = 0; i < w; i++)
    {
        for(j = 0; j < h; j++)
            cpy->grid[i][j] = src->grid[i][j];
    }

    return cpy;
}

int gridrowcpy(grid_t *src, grid_t *dest, int srcrow, int destrow)
{
    if(!src && !dest)
        return -1;
    if(!src)
        src = dest;
    else if(!dest)
        dest = src;

    if(src == dest && srcrow == destrow)
        return 0;

    int i = 0;
    int w = src->w;
    int val = 0;

    for(i = 0; i < w; i++)
    {
        val = gridgetcell(src, i, srcrow);
        gridsetcell(dest, i, destrow, val);
    }

    return 0;
}

int gridxytopos(grid_t *g, int x, int y) { return ((y * g->w) + x); }

int gridpostox(grid_t *g, int pos) { return (pos % g->w); }

int gridpostoy(grid_t *g, int pos) { return (pos / g->w); }

int grid_cells_filled(grid_t *g)
{
    if(!g)
        return -1;

    int i = 0;
    int j = 0;
    int n = 0;

    for(i = 0; i < g->w; i++)
    {
        for(j = 0; j < g->h; j++)
        {
            if(gridgetcell(g, i, j))
                n++;
        }
    }

    return n;
}

grid_t *grid_yx_to_xy(grid_t *g)
{
    if(!g)
        return NULL;

    grid_t *h = (grid_t *)malloc(sizeof(grid_t));
    int i = 0;
    int j = 0;

    h->w = g->h;
    h->h = g->w;

    for(i = 0; i < g->w; i++)
    {
        for(j = 0; j < g->h; j++)
        {
            h->grid[j][i] = g->grid[i][j];
        }
    }

    return h;
}

grid_t *grid_from_1d_int_array(int *arr, int w, int h)
{
    if(!arr)
        return NULL;

    grid_t *g = grid_create(w, h);
    int i = 0;
    int x = 0;
    int y = 0;

    for(i = 0; i < w * h; i++)
    {
        x = i % w;
        y = i / w;

        g->grid[x][y] = arr[i];
    }

    return g;
}
