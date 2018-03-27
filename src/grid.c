#include <stdlib.h>
#include <stdint.h>

#include "grid.h"

grid_t *grid_create(int w, int h)    // allocate first horizontally, then vertically
{
    if(w < 1 || h < 1)
        return NULL;

    int i = 0;
    int j = 0;

    grid_t *g = malloc(sizeof(grid_t));
    g->w = w;
    g->h = h;
    g->grid = malloc(w * sizeof(int *));

    for(i = 0; i < w; i++)
        g->grid[i] = malloc(h * sizeof(int));

    for(i = 0; i < w; i++) {
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

int gridfill(grid_t *g, int val)
{
    if(!g)
        return -1;

    int i = 0;
    int j = 0;

    for(i = 0; i < g->w; i++) {
        for(j = 0; j < g->h; j++)
            g->grid[i][j] = val;
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

    if(dest) {
        w = (src->w < dest->w ? src->w : dest->w);
        h = (src->h < dest->h ? src->h : dest->h);
        cpy = dest;
    } else
        cpy = grid_create(w, h);

    for(i = 0; i < w; i++) {
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

int gridxytopos(grid_t *g, int x, int y)
{
    return ((y * g->w) + x);
}

int gridpostox(grid_t *g, int pos)
{
    return (pos % g->w);
}

int gridpostoy(grid_t *g, int pos)
{
    return (pos / g->w);
}

int grid_cells_filled(grid_t *g)
{
    if(!g)
        return -1;

    int i = 0;
    int j = 0;
    int n = 0;

    for(i = 0; i < g->w; i++) {
        for(j = 0; j < g->h; j++) {
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

    grid_t *h = malloc(sizeof(grid_t));
    int i = 0;
    int j = 0;

    h->w = g->h;
    h->h = g->w;

    for(i = 0; i < g->w; i++) {
        for(j = 0; j < g->h; j++) {
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

    for(i = 0; i < w*h; i++) {
        x = i % w;
        y = i / w;

        g->grid[x][y] = arr[i];
    }

    return g;
}
