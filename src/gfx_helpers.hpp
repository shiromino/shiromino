#ifndef _gfx_helpers_hpp
#define _gfx_helpers_hpp

#include <vector>
#include <SDL2/SDL.h>

#include "grid.h"

struct paletteMapEntry
{
    paletteMapEntry(int mappedVal, bool isFlag) : mappedVal(mappedVal), isFlag(isFlag) {}

    int mappedVal;
    bool isFlag;
};

void gfx_drawCodedGrid(SDL_Renderer *renderer, grid_t *grid, SDL_Texture *paletteTex, SDL_Point position,
    int cellW, int cellH, std::vector<paletteMapEntry>& paletteValMap);



#endif // _gfx_helpers_hpp
