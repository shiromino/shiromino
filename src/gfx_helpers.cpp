#include "gfx_helpers.hpp"
#include <SDL.h>
#include <vector>
void gfx_drawCodedGrid(SDL_Renderer *renderer, Shiro::Grid *cells, SDL_Texture *paletteTex, SDL_Point position,
    int cellW, int cellH, std::vector<paletteMapEntry>& paletteValMap)
{
    for(int i = 0; i < cells->getWidth(); i++)
    {
        for(int j = 0; j < cells->getHeight(); j++)
        {
            std::vector<unsigned int> paletteList;
            int val = cells->getCell(i, j);

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