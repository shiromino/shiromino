#include "gfx_helpers.hpp"

#include <vector>
#include <SDL2/SDL.h>

#include "grid.h"

using namespace std;

void gfx_drawCodedGrid(SDL_Renderer *renderer, grid_t *grid, SDL_Texture *paletteTex, SDL_Point position,
    int cellW, int cellH, vector<paletteMapEntry>& paletteValMap)
{
    for(int i = 0; i < grid->w; i++)
    {
        for(int j = 0; j < grid->h; j++)
        {
            vector<unsigned int> paletteList;
            int val = gridgetcell(grid, i, j);

            int destX = position.x + (i * cellW);
            int destY = position.y + (j * cellH);

            SDL_Rect src = {0, 0, cellW, cellH};
            SDL_Rect dest = {destX, destY, cellW, cellH};

            if(!paletteValMap.empty())
            {
                paletteList.clear();

                int baseVal = -1;

                for(unsigned int i = 1; i < paletteValMap.size(); i++)
                {
                    paletteMapEntry m = paletteValMap[i];
                    if(m.isFlag)
                    {
                        if(val & m.mappedVal)
                        {
                            paletteList.push_back(i - 1);
                        }
                    }
                    else
                    {
                        if(val & m.mappedVal)
                        {
                            baseVal = (int)(i);
                        }
                    }
                }

                if(baseVal >= 0)
                {
                    paletteList.insert(paletteList.begin(), baseVal);
                }

                for(auto m : paletteList)
                {
                    src.x = m * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderCopy(renderer, paletteTex, &src, &dest);
                    }
                }
            }
            else
            {
                val--;
                if(val >= 0)
                {
                    src.x = val * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderCopy(renderer, paletteTex, &src, &dest);
                    }
                }
            }
        }
    }
}
