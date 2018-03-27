#ifndef _grid_h
#define _grid_h

#include <stdint.h>

#define GRID_OOB    8128

typedef struct {
    int w;
    int h;
    int **grid;        // grid[w][h] / grid[column #][row #]
} grid_t;

typedef grid_t yx_grid_t;

grid_t *grid_create(int w, int h);    // malloc(w * sizeof(int *));
void grid_destroy(grid_t *g);

int gridsetw(grid_t *g, int w);
int gridseth(grid_t *g, int h);
int gridsetcell(grid_t *g, int x, int y, int val);
int gridfill(grid_t *g, int val);
int gridgetcell(grid_t *g, int x, int y);

grid_t *gridcpy(grid_t *src, grid_t *dest);
int gridrowcpy(grid_t *src, grid_t *dest, int srcrow, int destrow);

int gridgetsize(grid_t *g);
int gridxytopos(grid_t *g, int x, int y);
int gridpostox(grid_t *g, int pos);
int gridpostoy(grid_t *g, int pos);

int grid_cells_filled(grid_t *g);
// int grids_equal(grid_t *g, grid_t *h); TODO
grid_t *grid_yx_to_xy(grid_t *g); // i.e. grid[y][x] value is moved to grid[x][y] location in a new grid, and w and h are interchanged

grid_t *grid_from_1d_int_array(int *arr, int w, int h);

#endif
