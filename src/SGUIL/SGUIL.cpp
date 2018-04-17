#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "SGUIL.hpp"

using namespace std;


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
            cout << "Gui_Init(): Error: SDL_GetRendererInfo(): " << SDL_GetError() << endl;
            return false;
        }

        if((info.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
        {
            cout << "Gui_Init(): Error: SDL_Renderer flags must include SDL_RENDERER_TARGETTEXTURE!" << endl;
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
                cout << "Gui_Init(): Error: IMG_Load(): " << IMG_GetError() << endl;
                return false;
            }
        } else
        {
            /*
            cout << "Gui_Init(): Error: Must provide path to theme files!" << endl;
            return false;
            */
        }

        cout << "Finished initializing SGUIL, version " << SGUIL_VERSION_STR << endl;
        Gui_SDL_Renderer = theRenderer;
        return true;
    } else
    {
        cout << "Gui_Init(): Error: Must provide valid SDL_Renderer!" << endl;
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
            cout << "BitFont::BitFont(): SDL_CreateTextureFromSurface(): Error: " << SDL_GetError() << endl;
        } else
        {
            sheetValid = true;
        }
    } else
    {
        cout << "BitFont::BitFont(): IMG_Load(sheetFname.c_str()): Error: " << IMG_GetError() << endl;
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
                cout << "BitFont::BitFont(): SDL_CreateTextureFromSurface(): Error: " << SDL_GetError() << endl;
            } else
            {
                outlineSheetValid = true;
            }
        } else
        {
            cout << "BitFont::BitFont(): IMG_Load(outlineSheetFname.c_str()): Error: " << IMG_GetError() << endl;
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

GuiText::GuiText(string text, BitFont& font, SDL_Rect& relativeDestRect)
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

void GuiText::addEventHook(function<void(GuiText&, GuiEvent&)> callback, enumGuiEventType type)
{
    eventHooks.push_back( GuiEventHook<GuiText>{callback, type} );
}

void GuiText::addRenderHook(function<void(GuiText&)> callback)
{
    renderHooks.push_back( GuiRenderHook<GuiText>{callback} );
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
            this->mouseClicked(event.mouseClickedEvent->x, event.mouseClickedEvent->y);
            break;
        case mouse_released:
            this->mouseReleased(event.mouseReleasedEvent->x, event.mouseReleasedEvent->y);
            break;
        case mouse_dragged:
            this->mouseDragged(event.mouseDraggedEvent->x, event.mouseReleasedEvent->y);
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

        SDL_Rect rects[4] = {left, right, top, bottom};

        SDL_RenderFillRects(Gui_SDL_Renderer, rects, 4);
    }

    SDL_SetRenderDrawColor(Gui_SDL_Renderer, r, g, b, a);
}

void Gui_GenerateTextPositionalValues(string& text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox,
    vector<pair<int, int>>& values, bool horizontalScroll, bool verticalScroll)
{
    Gui_GenerateTextPositionalValuesPartial(text, 0, text.length(), fmt, font, destBox, values, horizontalScroll, verticalScroll);
}

void Gui_GenerateTextPositionalValuesPartial(string& text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, SDL_Rect& destBox, vector<pair<int, int>>& values, bool horizontalScroll, bool verticalScroll)
{
    TextFormat fmtDefault {};

    if(fmt == NULL)
    {
        fmt = &fmtDefault;
    }

    stringstream ss {text};
    string to;
    vector<string> lines;
    while(getline(ss, to, '\n'))
    {
        lines.push_back(to);
    }

    int linefeeds = 0;
    int lastWrapLinePos = 0;
    int lastWrapPos = 0;

    values.clear();
    values.reserve(text.length());

    unsigned int currentX = destBox.x;
    unsigned int currentY = destBox.y;

    for(unsigned int i = pos; i < text.length() && i < pos + len; i++)
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
                    if(lines[linefeeds].length() - lastWrapLinePos < width)
                    {
                        currentX = destBox.x + destBox.w - (int)((float)font.charW * fmt->sizeMult * (float)(lines[linefeeds].length() - lastWrapLinePos));
                    } else
                    {
                        currentX = destBox.x + destBox.w - (int)(width * font.charW * fmt->sizeMult);
                    }

                    break;

                case enumAlignment::center:
                    if(lines[linefeeds].length() - lastWrapLinePos < width)
                    {
                        currentX = destBox.x + (fmt->sizeMult * (float)font.charW / 2.0) * (width - (lines[linefeeds].length() - lastWrapLinePos));
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

void Gui_DrawText_PV(string text, TextFormat *fmt, BitFont& font, vector<pair<int, int>>& positionalValues,
    unsigned int scrollPosX, unsigned int scrollPosY)
{
    Gui_DrawTextPartial_PV(text, 0, text.length(), fmt, font, positionalValues, scrollPosX, scrollPosY);
}

void Gui_DrawTextPartial_PV(string text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, vector<pair<int, int>>& positionalValues, unsigned int scrollPosX, unsigned int scrollPosY)
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

    SDL_Rect src = {.x = 0, .y = 0, .w = (int)font.charW, .h = (int)font.charH};
    SDL_Rect dest = {.x = 0, .y = 0, .w = (int)(fmt->sizeMult * (float)font.charW), .h = (int)(fmt->sizeMult * (float)font.charH)};

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
        dest.x = get<0>(p);
        dest.y = get<1>(p);

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

void Gui_DrawText(string text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
{
    vector<pair<int, int>> textPositionalValues;
    Gui_GenerateTextPositionalValues(text, fmt, font, destBox, textPositionalValues, false, false);
    Gui_DrawText_PV(text, fmt, font, textPositionalValues, 0, 0);
}

void Gui_DrawTextPartial(string text, unsigned int pos, unsigned int len, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
// destBox's w and h fields can be 0, which results in the text being rendered without strict positional bounds
{
    vector<pair<int, int>> textPositionalValues;
    Gui_GenerateTextPositionalValuesPartial(text, pos, len, fmt, font, destBox, textPositionalValues, false, false);
    Gui_DrawTextPartial_PV(text, pos, len, fmt, font, textPositionalValues, 0, 0);
}
