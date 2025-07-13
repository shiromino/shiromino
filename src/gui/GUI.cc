#include "GUI.h"
#include "SDL_error.h"
#include "SDL_surface.h"
#include <cstddef>
#include <iostream>
#include <sstream>
#include "SDL_image.h"

SDL_Renderer *guiSDLRenderer = NULL;
SDL_Texture *guiThemeTexture = NULL;
bool initializeGUI(SDL_Renderer *theRenderer, const char *themePath)
{
    if(theRenderer != NULL)
    {
        SDL_RendererInfo info;
        int rc;
        rc = SDL_GetRendererInfo(theRenderer, &info);

        if(rc < 0)
        {
            std::cout << "initializeGUI(): Error: SDL_GetRendererInfo(): " << SDL_GetError() << std::endl;
            return false;
        }

        if((info.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
        {
            std::cout << "initializeGUI(): Error: SDL_Renderer flags must include SDL_RENDERER_TARGETTEXTURE!" << std::endl;
            return false;
        }

        if(themePath)
        {
            SDL_Surface *sur = IMG_Load(themePath);
            if(sur != NULL)
            {
                guiThemeTexture = SDL_CreateTextureFromSurface(theRenderer, sur);
            } else
            {
                std::cout << "initializeGUI(): Error: IMG_Load(): " << IMG_GetError() << std::endl;
                return false;
            }
        } else
        {
            /*
            cout << "initializeGUI(): Error: Must provide path to theme files!" << endl;
            return false;
            */
        }
        guiSDLRenderer = theRenderer;
        return true;
    } else
    {
        std::cout << "initializeGUI(): Error: Must provide valid SDL_Renderer!" << std::endl;
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
        sheet = SDL_CreateTextureFromSurface(guiSDLRenderer, sheetSur);
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
            outlineSheet = SDL_CreateTextureFromSurface(guiSDLRenderer, outlineSheetSur);
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

GUIText::GUIText(std::string text, BitFont& font, SDL_Rect& relativeDestRect)
    : text(text), font(font)
{
    this->relativeDestRect = relativeDestRect;
    this->containingWindow = NULL;
    this->updatePositionalValues = true;
}

void GUIText::draw()
{
    this->prepareRenderTarget();

    if(updatePositionalValues)
    {
        generateGUITextPositionalValues(text, &fmt, font, relativeDestRect, textPositionalValues);
        updatePositionalValues = false;
    }

    drawGUITextPV(text, &fmt, font, textPositionalValues);
}

bool GUIInteractable::canInteractAt(int x, int y)
{
    return (enabled &&
            x >= relativeDestRect.x &&
            x < (relativeDestRect.x + relativeDestRect.w) &&
            y >= relativeDestRect.y &&
            y < (relativeDestRect.y + relativeDestRect.h)
    );
}

void GUIInteractable::handleEvent(GUIEvent& event)
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

void GUIDrawBorder(SDL_Rect& rect, int width, Shiro::GUI::rgba_t rgba)
{
    if(width <= 0)
    {
        return;
    }

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_GetRenderDrawColor(guiSDLRenderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(guiSDLRenderer, rgba_R(rgba), rgba_G(rgba), rgba_B(rgba), rgba_A(rgba));

    if(width == 1)
    {
        SDL_Point points[5] = {
            {rect.x, rect.y},
            {rect.x + rect.w - 1, rect.y},
            {rect.x + rect.w - 1, rect.y + rect.h - 1},
            {rect.x, rect.y + rect.h - 1},
            {rect.x, rect.y}
        };


        SDL_RenderDrawLines(guiSDLRenderer, points, 5);
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

        SDL_RenderFillRects(guiSDLRenderer, rects, 4);
    }

    SDL_SetRenderDrawColor(guiSDLRenderer, r, g, b, a);
}

void generateGUITextPositionalValues(std::string& text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox,
    std::vector<std::pair<int, int>>& values)
{
    generateGUITextPositionalValuesPartial(text, 0, (unsigned)text.size(), fmt, font, destBox, values);
}

void generateGUITextPositionalValuesPartial(std::string& text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, SDL_Rect& destBox, std::vector<std::pair<int, int>>& values)
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
                currentY += static_cast<int>(fmt->lineSpacing * fmt->sizeMult * (float)font.charH);
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
                        currentX = static_cast<int>(destBox.x + (fmt->sizeMult * (float)font.charW / 2.0f) * (width - (lines[linefeeds].size() - lastWrapLinePos)));
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

        currentX += static_cast<int>(fmt->sizeMult * (float)font.charW);
    }
}

void drawGUITextPV(std::string text, TextFormat *fmt, BitFont& font, std::vector<std::pair<int, int>>& positionalValues)
{
    drawGUITextPartialPV(text, 0, (unsigned)text.size(), fmt, font, positionalValues);
}

void drawGUITextPartialPV(std::string text, unsigned int pos, unsigned int len, TextFormat *fmt,
    BitFont& font, std::vector<std::pair<int, int>>& positionalValues)
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

    setGUITextureRGBA(font.sheet, fmt->rgba);

    if(font.outlineSheet)
    {
        setGUITextureRGBA(font.outlineSheet, fmt->rgbaOutline);
    }

    SDL_Rect src = { 0, 0, (int) font.charW, (int) font.charH };
    SDL_Rect dest = { 0, 0, (int) (fmt->sizeMult * (float) font.charW), (int) (fmt->sizeMult * (float) font.charH) };

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
            Shiro::GUI::rgba_t rgba = fmt->highlight ? fmt->rgbaHighlight : fmt->rgbaOutline;
            src.x = 31*font.charW;
            src.y = 3*font.charH;

            setGUITextureRGBA(font.sheet, rgba);
            SDL_RenderCopy(guiSDLRenderer, font.sheet, &src, &dest);
            setGUITextureRGBA(font.sheet, fmt->rgba);
        }

        src.x = font.charW * (text[i] % 32);
        src.y = font.charH * ((int)(text[i] / 32) - 1);
        if(src.y < 0) {
            src.x = 31 * font.charW;
            src.y = 2 * font.charH;
        }

        if(fmt->shadow) {
            dest.x -= static_cast<int>(2.0f * fmt->sizeMult);
            dest.y += static_cast<int>(2.0f * fmt->sizeMult);

            SDL_SetTextureAlphaMod(font.sheet, (Uint8)((float)rgba_A(fmt->rgba) / 3.5));
            if(font.outlineSheet)
                SDL_SetTextureAlphaMod(font.outlineSheet, (Uint8)((float)rgba_A(fmt->rgba) / 3.5));

            SDL_RenderCopy(guiSDLRenderer, font.sheet, &src, &dest);

            if(fmt->outline && font.outlineSheet) {
                SDL_RenderCopy(guiSDLRenderer, font.outlineSheet, &src, &dest);
            }

            dest.x += static_cast<int>(2.0f * fmt->sizeMult);
            dest.y -= static_cast<int>(2.0f * fmt->sizeMult);

            SDL_SetTextureAlphaMod(font.sheet, rgba_A(fmt->rgba));
            if(font.outlineSheet)
                SDL_SetTextureAlphaMod(font.outlineSheet, rgba_A(fmt->rgba));
        }

        SDL_RenderCopy(guiSDLRenderer, font.sheet, &src, &dest);

        if(fmt->outline && font.outlineSheet)
        {
            SDL_RenderCopy(guiSDLRenderer, font.outlineSheet, &src, &dest);
        }
    }

    setGUITextureRGBA(font.sheet, Shiro::GUI::RGBA_DEFAULT);

    if(font.outlineSheet)
    {
        setGUITextureRGBA(font.outlineSheet, Shiro::GUI::RGBA_DEFAULT);
    }
}

void drawGUIText(std::string text, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
{
    std::vector<std::pair<int, int>> textPositionalValues;
    generateGUITextPositionalValues(text, fmt, font, destBox, textPositionalValues);
    drawGUITextPV(text, fmt, font, textPositionalValues);
}

void drawGUITextPartial(std::string text, unsigned int pos, unsigned int len, TextFormat *fmt, BitFont& font, SDL_Rect& destBox)
// destBox's w and h fields can be 0, which results in the text being rendered without strict positional bounds
{
    std::vector<std::pair<int, int>> textPositionalValues;
    generateGUITextPositionalValuesPartial(text, pos, len, fmt, font, destBox, textPositionalValues);
    drawGUITextPartialPV(text, pos, len, fmt, font, textPositionalValues);
}