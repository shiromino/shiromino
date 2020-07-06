#include "SGUIL.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

SDL_Renderer *Gui_SDL_Renderer = NULL;
SDL_Texture *Gui_ThemeTexture = NULL;

bool Gui_Init(SDL_Renderer *theRenderer, const char *themePath)
{
    if(theRenderer != NULL)
    {
        SDL_RendererInfo info;
        int rc;
        rc = SDL_GetRendererInfo(theRenderer, &info);

        if(rc < 0)
        {
            std::cout << "Gui_Init(): Error: SDL_GetRendererInfo(): " << SDL_GetError() << std::endl;
            return false;
        }

        if((info.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
        {
            std::cout << "Gui_Init(): Error: SDL_Renderer flags must include SDL_RENDERER_TARGETTEXTURE!" << std::endl;
            return false;
        }

        if(themePath)
        {
            SDL_Surface *sur = IMG_Load(themePath);
            if(sur != NULL)
            {
                Gui_ThemeTexture = SDL_CreateTextureFromSurface(theRenderer, sur);
            } else
            {
                std::cout << "Gui_Init(): Error: IMG_Load(): " << IMG_GetError() << std::endl;
                return false;
            }
        } else
        {
            /*
            cout << "Gui_Init(): Error: Must provide path to theme files!" << endl;
            return false;
            */
        }

        std::cout << "Finished initializing SGUIL, version " << SGUIL_VERSION_STR << std::endl;
        Gui_SDL_Renderer = theRenderer;
        return true;
    } else
    {
        std::cout << "Gui_Init(): Error: Must provide valid SDL_Renderer!" << std::endl;
        return false;
    }

    return false;
}

BitFont::BitFont(const char *sheetFname, const char *outlineSheetFname, unsigned int charW, unsigned int charH)
{
    this->charW = 0;
    this->charH = 0;
    this->isValid = false;

    bool sheetValid = false;
    bool outlineSheetValid = false;

    SDL_Surface *sheetSur = IMG_Load(sheetFname);
    if(sheetSur != NULL)
    {
        sheet = SDL_CreateTextureFromSurface(Gui_SDL_Renderer, sheetSur);
        if(sheet == NULL)
        {
            std::cout << "BitFont::BitFont(): SDL_CreateTextureFromSurface(): Error: " << SDL_GetError() << std::endl;
        } else
        {
            sheetValid = true;
        }
    } else
    {
        std::cout << "BitFont::BitFont(): IMG_Load(sheetFname.c_str()): Error: " << IMG_GetError() << std::endl;
        sheet = NULL;
    }

    SDL_Surface *outlineSheetSur = NULL;

    if(outlineSheetFname)
    {
        outlineSheetSur = IMG_Load(outlineSheetFname);
        if(outlineSheetSur != NULL)
        {
            outlineSheet = SDL_CreateTextureFromSurface(Gui_SDL_Renderer, outlineSheetSur);
            if(outlineSheet == NULL)
            {
                std::cout << "BitFont::BitFont(): SDL_CreateTextureFromSurface(): Error: " << SDL_GetError() << std::endl;
            } else
            {
                outlineSheetValid = true;
            }
        } else
        {
            std::cout << "BitFont::BitFont(): IMG_Load(outlineSheetFname.c_str()): Error: " << IMG_GetError() << std::endl;
            outlineSheet = NULL;
        }
    } else
    {
        // no outline sheet specified, but the font is still valid because it doesn't need outlines to be usable
        outlineSheet = NULL;
        outlineSheetValid = true;
    }

    if(sheetValid && outlineSheetValid)
    {
        this->charW = charW;
        this->charH = charH;
        this->isValid = true;
    } else
    {
        if(sheet)
        {
            SDL_DestroyTexture(sheet);
        }

        if(outlineSheet)
        {
            SDL_DestroyTexture(outlineSheet);
        }
    }

    if(sheetSur)
    {
        SDL_FreeSurface(sheetSur);
    }

    if(outlineSheetSur)
    {
        SDL_FreeSurface(outlineSheetSur);
    }
}

BitFont::~BitFont()
{
    if(isValid)
    {
        if(sheet)
        {
            SDL_DestroyTexture(sheet);
        }

        if(outlineSheet)
        {
            SDL_DestroyTexture(outlineSheet);
        }
    }
}

GuiText::GuiText(std::string text, BitFont& font, SDL_Rect& relativeDestRect)
    : text(text), font(font)
{
    this->relativeDestRect = relativeDestRect;
    this->containingWindow = NULL;
    this->updatePositionalValues = true;
}

void GuiText::draw()
{
    this->prepareRenderTarget(false);

    if(updatePositionalValues)
    {
        Gui_GenerateTextPositionalValues(text, &fmt, font, relativeDestRect, textPositionalValues, false, false);
        updatePositionalValues = false;
    }

    Gui_DrawText_PV(text, &fmt, font, textPositionalValues, 0, 0);
}

bool GuiInteractable::canInteractAt(int x, int y)
{
    return (enabled &&
            x >= relativeDestRect.x &&
            x < (relativeDestRect.x + relativeDestRect.w) &&
            y >= relativeDestRect.y &&
            y < (relativeDestRect.y + relativeDestRect.h)
    );
}

void GuiInteractable::handleEvent(GuiEvent& event)
{
    switch(event.type)
    {
        case mouse_clicked:
            this->mouseClicked(event.mouseClickedEvent->x, event.mouseClickedEvent->y, event.mouseClickedEvent->button);
            break;
        case mouse_released:
            this->mouseReleased(event.mouseReleasedEvent->x, event.mouseReleasedEvent->y, event.mouseReleasedEvent->button);
            break;
        case mouse_dragged:
            this->mouseDragged(event.mouseDraggedEvent->x, event.mouseDraggedEvent->y, event.mouseDraggedEvent->button);
            break;
        case key_pressed:
            this->keyPressed(event.keyPressedEvent->key);
            break;
        default:
            break;
    }
}

void Gui_DrawBorder(SDL_Rect& rect, int width, rgba_t rgba)
{
    if(width <= 0)
    {
        return;
    }

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_GetRenderDrawColor(Gui_SDL_Renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(Gui_SDL_Renderer, rgba_R(rgba), rgba_G(rgba), rgba_B(rgba), rgba_A(rgba));

    if(width == 1)
    {
        SDL_Point points[5] = {
            {rect.x, rect.y},
            {rect.x + rect.w - 1, rect.y},
            {rect.x + rect.w - 1, rect.y + rect.h - 1},
            {rect.x, rect.y + rect.h - 1},
            {rect.x, rect.y}
        };


        SDL_RenderDrawLines(Gui_SDL_Renderer, points, 5);
    } else
    {
        SDL_Rect left = {
            rect.x,
            rect.y + width,
            width,
            rect.h - 2 * width
        };
        SDL_Rect right = {
            rect.x + rect.w - width,
            rect.y + width,
            width,
            rect.h - 2 * width
        };
        SDL_Rect top = {
            rect.x,
            rect.y,
            rect.w,
            width
        };
        SDL_Rect bottom = {
            rect.x,
            rect.y + rect.h - width,
            rect.w,
            width
        };

        SDL_Rect rects[4] = { left, right, top, bottom };

        SDL_RenderFillRects(Gui_SDL_Renderer, rects, 4);
    }

    SDL_SetRenderDrawColor(Gui_SDL_Renderer, r, g, b, a);
}

void Gui_GenerateTextPositionalValues(std::string& text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox,
    std::vector<std::pair<int, int>>& values, bool horizontalScroll, bool verticalScroll)
{
    Gui_GenerateTextPositionalValuesPartial(text, 0, (unsigned)text.size(), fmt, font, destBox, values, horizontalScroll, verticalScroll);
}

void Gui_GenerateTextPositionalValuesPartial(std::string& text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, SDL_Rect& destBox, std::vector<std::pair<int, int>>& values, bool horizontalScroll, bool verticalScroll)
{
    TextFormat fmtDefault {};

    if(fmt == NULL)
    {
        fmt = &fmtDefault;
    }

    std::stringstream ss {text};
    std::string to;
    std::vector<std::string> lines;
    while(getline(ss, to, '\n'))
    {
        lines.push_back(to);
    }

    int linefeeds = 0;
    int lastWrapLinePos = 0;
    int lastWrapPos = 0;

    values.clear();
    values.reserve(text.size());

    int currentX = destBox.x;
    int currentY = destBox.y;

    for(unsigned int i = pos; i < text.size() && i < pos + len; i++)
    {
        if(i == 0 ||
           (fmt->wrapLen != 0 && (i - lastWrapPos) == fmt->wrapLen) ||
           text[i] == '\n' ||
           (destBox.w != 0 && ((int)(currentX + ((float)font.charW * fmt->sizeMult)) > destBox.x + destBox.w))
        )
        {
            if(text[i] == '\n')
            {
                linefeeds++;
                values.push_back( {currentX, currentY} );
                lastWrapLinePos = i - lastWrapPos + lastWrapLinePos;
                lastWrapLinePos = 0;
                lastWrapPos = i + 1;
            } else
            {
                lastWrapLinePos = i - lastWrapPos + lastWrapLinePos;
                lastWrapPos = i;
            }

            if(i != 0 || text[i] == '\n')
            {
                currentY += fmt->lineSpacing * fmt->sizeMult * (float)font.charH;
            }

            unsigned int width = (unsigned int)((float)(destBox.w) / (float)(font.charW) / fmt->sizeMult);
            if((fmt->wrapLen != 0 && fmt->wrapLen < width) || width == 0)
            {
                width = fmt->wrapLen;
            }

            switch(fmt->alignment)
            {
                default:
                case enumAlignment::left:
                    currentX = destBox.x;
                    break;

                case enumAlignment::right:
                    if(lines[linefeeds].size() - lastWrapLinePos < width)
                    {
                        currentX = destBox.x + destBox.w - (int)((float)font.charW * fmt->sizeMult * (float)(lines[linefeeds].size() - lastWrapLinePos));
                    } else
                    {
                        currentX = destBox.x + destBox.w - (int)(width * font.charW * fmt->sizeMult);
                    }

                    break;

                case enumAlignment::center:
                    if(lines[linefeeds].size() - lastWrapLinePos < width)
                    {
                        currentX = destBox.x + (fmt->sizeMult * (float)font.charW / 2.0) * (width - (lines[linefeeds].size() - lastWrapLinePos));
                    } else
                    {
                        currentX = destBox.x;
                    }

                    break;
            }

            if(currentX < destBox.x)
            {
                currentX = destBox.x;
            }

            if(text[i] == '\n')
            {
                continue;
            }
        }

        // if(currentX > destBox.x + destBox.w || currentY > destBox.y + destBox.h) {break;}

        values.push_back( {currentX, currentY} );

        currentX += fmt->sizeMult * (float)font.charW;
    }
}

void Gui_DrawText_PV(std::string text, TextFormat *fmt, BitFont& font, std::vector<std::pair<int, int>>& positionalValues,
    unsigned int scrollPosX, unsigned int scrollPosY)
{
    Gui_DrawTextPartial_PV(text, 0, (unsigned)text.size(), fmt, font, positionalValues, scrollPosX, scrollPosY);
}

void Gui_DrawTextPartial_PV(std::string text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, std::vector<std::pair<int, int>>& positionalValues, unsigned int scrollPosX, unsigned int scrollPosY)
{
    if(positionalValues.size() == 0)
    {
        return;
    }

    TextFormat fmtDefault {};

    if(fmt == NULL)
    {
        fmt = &fmtDefault;
    }

    Gui_SetTextureRGBA(font.sheet, fmt->rgba);

    if(font.outlineSheet)
    {
        Gui_SetTextureRGBA(font.outlineSheet, fmt->rgbaOutline);
    }

    SDL_Rect src = { 0, 0, (int) font.charW, (int) font.charH };
    SDL_Rect dest = { 0, 0, (int) (fmt->sizeMult * (float) font.charW), (int) (fmt->sizeMult * (float) font.charH) };

    bool using_target_tex = false;

    if(SDL_GetRenderTarget(Gui_SDL_Renderer) != NULL)
    {
        using_target_tex = true;
    }

    for(unsigned int i = pos; i < positionalValues.size() && i < pos + len; i++)
    {
        if(text[i] == '\n')
        {
            continue;
        }

        auto p = positionalValues[i];
        dest.x = std::get<0>(p);
        dest.y = std::get<1>(p);

        if(fmt->highlight || (fmt->outline && font.outlineSheet == NULL))
        {
            rgba_t rgba = fmt->highlight ? fmt->rgbaHighlight : fmt->rgbaOutline;
            src.x = 31*font.charW;
            src.y = 3*font.charH;

            Gui_SetTextureRGBA(font.sheet, rgba);
            SDL_RenderCopy(Gui_SDL_Renderer, font.sheet, &src, &dest);
            Gui_SetTextureRGBA(font.sheet, fmt->rgba);
        }

        src.x = font.charW * (text[i] % 32);
        src.y = font.charH * ((int)(text[i] / 32) - 1);
        if(src.y < 0) {
            src.x = 31 * font.charW;
            src.y = 2 * font.charH;
        }

        if(fmt->shadow) {
            dest.x -= 2.0 * fmt->sizeMult;
            dest.y += 2.0 * fmt->sizeMult;

            SDL_SetTextureAlphaMod(font.sheet, (Uint8)((float)rgba_A(fmt->rgba) / 3.5));
            if(font.outlineSheet)
                SDL_SetTextureAlphaMod(font.outlineSheet, (Uint8)((float)rgba_A(fmt->rgba) / 3.5));

            SDL_RenderCopy(Gui_SDL_Renderer, font.sheet, &src, &dest);

            if(fmt->outline && font.outlineSheet) {
                SDL_RenderCopy(Gui_SDL_Renderer, font.outlineSheet, &src, &dest);
            }

            dest.x += 2.0 * fmt->sizeMult;
            dest.y -= 2.0 * fmt->sizeMult;

            SDL_SetTextureAlphaMod(font.sheet, rgba_A(fmt->rgba));
            if(font.outlineSheet)
                SDL_SetTextureAlphaMod(font.outlineSheet, rgba_A(fmt->rgba));
        }

        SDL_RenderCopy(Gui_SDL_Renderer, font.sheet, &src, &dest);

        if(fmt->outline && font.outlineSheet)
        {
            SDL_RenderCopy(Gui_SDL_Renderer, font.outlineSheet, &src, &dest);
        }
    }

    Gui_SetTextureRGBA(font.sheet, GUI_RGBA_DEFAULT);

    if(font.outlineSheet)
    {
        Gui_SetTextureRGBA(font.outlineSheet, GUI_RGBA_DEFAULT);
    }
}

void Gui_DrawText(std::string text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
{
    std::vector<std::pair<int, int>> textPositionalValues;
    Gui_GenerateTextPositionalValues(text, fmt, font, destBox, textPositionalValues, false, false);
    Gui_DrawText_PV(text, fmt, font, textPositionalValues, 0, 0);
}

void Gui_DrawTextPartial(std::string text, unsigned int pos, unsigned int len, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
// destBox's w and h fields can be 0, which results in the text being rendered without strict positional bounds
{
    std::vector<std::pair<int, int>> textPositionalValues;
    Gui_GenerateTextPositionalValuesPartial(text, pos, len, fmt, font, destBox, textPositionalValues, false, false);
    Gui_DrawTextPartial_PV(text, pos, len, fmt, font, textPositionalValues, 0, 0);
}