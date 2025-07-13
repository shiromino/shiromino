#include "SDL_clipboard.h"
#include "SDL_error.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include <chrono>
#include <stdlib.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "GUI.h"

#define TEXT_POSITION_NONE 0xFFFFFFFFu

TextField::TextField(int ID, BindableString *var, BitFont& font, SDL_Rect relativeDestRect)
    : font(font)
{
    this->ID = ID;
    this->var = var;
    this->relativeDestRect = relativeDestRect;

    displayTexture = NULL;
    updateDisplayStringPVs = false;

    containingWindow = NULL;

    enabled = true;
    canHoldKeyboardFocus = true;
    hasDefaultKeyboardFocus = false;
    selected = false;
    hasKeyboardFocus = false;

    cursor = 0;
    scrollPosX = 0;
    scrollPosY = 0;
    selectionStart = 0;
    selectionEnd = 0;

    lastEventTime = 0;
    typing = false;

    updatePositionalValues = true;
    enableNewline = true;
    horizontalScroll = true;
    verticalScroll = true;
}

TextField::TextField(int ID, BindableString *var, std::string valueDefault, BitFont& font, SDL_Rect relativeDestRect)
    : TextField(ID, var, font, relativeDestRect)
{
    if(!valueDefault.empty())
    {
        this->value = valueDefault;
    }
}

TextField::~TextField()
{

}

void TextField::setTextFormat(TextFormat& fmt)
{
    this->fmt = fmt;
}

void TextField::draw()
{
    this->prepareRenderTarget();

    if(updatePositionalValues)
    {
        generateGUITextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues);
        updatePositionalValues = false;
    }

    Shiro::GUI::rgba_t rgbaBorder = this->hasKeyboardFocus ? 0xFF2020FF : 0x8080FFFF;
    GUIDrawBorder(relativeDestRect, 1, rgbaBorder);

    if(selectionStart == selectionEnd)
    {
        drawGUITextPV(value, &fmt, font, textPositionalValues);
    } else
    {
        TextFormat selectionFmt = fmt;
        selectionFmt.rgba = 0x000000FF;
        selectionFmt.highlight = true;

        unsigned int start = selectionStart < selectionEnd ? selectionStart : selectionEnd;
        unsigned int end = selectionStart < selectionEnd ? selectionEnd : selectionStart;

        drawGUITextPartialPV(value, 0, start, &fmt, font, textPositionalValues);
        drawGUITextPartialPV(value, start, end - start, &selectionFmt, font, textPositionalValues);
        drawGUITextPartialPV(value, end, (unsigned)value.size() - end, &fmt, font, textPositionalValues);
    }

    bool cursorBlinkOn = false;
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if((ms % 1000) < 500)
    {
        cursorBlinkOn = true;
    }

    if(ms - lastEventTime < 400)
    {
        cursorBlinkOn = false;
    } else
    {
        typing = false;
    }

    if(typing)
    {
        cursorBlinkOn = true;
    }

    if(cursorBlinkOn && this->hasKeyboardFocus)
    {
        SDL_Rect cursorRect;
        if(cursor < textPositionalValues.size())
        {
            cursorRect.x = std::get<0>(textPositionalValues[cursor]);
            cursorRect.y = std::get<1>(textPositionalValues[cursor]);
        } else if(textPositionalValues.size() != 0)
        {
            if(value[textPositionalValues.size() - 1] == '\n')
            {
                cursorRect.x = relativeDestRect.x;
                cursorRect.y = std::get<1>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.y += static_cast<int>(fmt.lineSpacing * fmt.sizeMult * (float)font.charH);
            } else
            {
                cursorRect.x = std::get<0>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.y = std::get<1>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.x += static_cast<int>((float)font.charW * fmt.sizeMult);
            }
        } else
        {
            cursorRect.x = relativeDestRect.x;
            cursorRect.y = relativeDestRect.y;
        }

        cursorRect.w = static_cast<int>(2.0 * fmt.sizeMult);
        cursorRect.h = static_cast<int>((float)font.charH * fmt.sizeMult);

        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 a;
        SDL_GetRenderDrawColor(guiSDLRenderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(guiSDLRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(guiSDLRenderer, &cursorRect);
        SDL_SetRenderDrawColor(guiSDLRenderer, r, g, b, a);
    }
}

void TextField::handleEvent(GUIEvent& event)
{
    SDL_Cursor *sdlCursor = NULL;
    switch(event.type)
    {
        case mouse_hovered_onto:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
            if(sdlCursor != NULL)
            {
                SDL_SetCursor(sdlCursor);
            } else
            {
                std::cout << "TextField::handleEvent(): Error: SDL_CreateSystemCursor(): " << SDL_GetError() << std::endl;
            }

            break;
        case mouse_hovered_off:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            if(sdlCursor != NULL)
            {
                SDL_SetCursor(sdlCursor);
            } else
            {
                std::cout << "TextField::handleEvent(): Error: SDL_CreateSystemCursor(): " << SDL_GetError() << std::endl;
            }

            break;
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
        case textinput_guievent:
            this->textInput(event.textInputEvent->text);
            break;
        default:
            break;
    }
}

void TextField::mouseClicked(int x, int y, Uint8)
{
    if(updatePositionalValues)
    {
        generateGUITextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues);
        updatePositionalValues = false;
    }

    unsigned pos = getPositionUnderMouse(x, y);
    if(pos == TEXT_POSITION_NONE)
    {
        return;
    }

    if(SDL_GetModState() & KMOD_SHIFT && pos != cursor)
    {
        selectionStart = cursor;
        selectionEnd = pos;
        cursor = pos;
    } else
    {
        cursor = pos;
        selectionStart = selectionEnd = cursor;
    }
}

void TextField::mouseDragged(int x, int y, Uint8)
{
    if(!hasKeyboardFocus)
    {
        return;
    }

    if(updatePositionalValues)
    {
        generateGUITextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues);
        updatePositionalValues = false;
    }

    unsigned pos = getPositionUnderMouse(x, y);
    if(pos == TEXT_POSITION_NONE)
    {
        return;
    }

    lastEventTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cursor = selectionEnd = pos;
}

void TextField::mouseReleased(int, int, Uint8)
{

}

void TextField::keyPressed(SDL_Keycode kc)
{
    switch(kc)
    {
        case SDLK_a:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                shiftCursor((int)(value.size() - cursor));
                selectionStart = 0;
                selectionEnd = cursor;
            }

            break;

        case SDLK_c:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                textCopy();
            }

            break;

        case SDLK_x:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                textCut();
            }

            break;

        case SDLK_v:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                std::string s = {SDL_GetClipboardText()};
                textInsert(s);
            }

            break;

        case SDLK_HOME:
            shiftCursor(0 - (int)cursor);
            break;

        case SDLK_END:
            shiftCursor((int)(value.size() - cursor));
            break;

        case SDLK_BACKSPACE:
            if(selectionStart != selectionEnd)
            {
                textDelete();
            } else if(cursor > 0)
            {
                textDelete(cursor - 1, cursor);
            }

            break;

        case SDLK_DELETE:
            if(selectionStart != selectionEnd)
            {
                textDelete();
            } else if(cursor < value.size())
            {
                textDelete(cursor, cursor + 1);
            }

            break;

        case SDLK_RETURN:
            textInsert( {"\n"} );
            break;

        case SDLK_TAB:
            textInsert( {"    "} );
            break;

        case SDLK_LEFT:
            shiftCursor(-1);

            break;

        case SDLK_RIGHT:
            shiftCursor(1);

            break;

        default:
            break;
    }
}

void TextField::textInput(std::string s)
{
    textInsert(s);
}

unsigned int TextField::getPositionUnderMouse(int x, int y)
{
    if(textPositionalValues.empty())
    {
        return 0;
    }

    std::size_t i = 0;
    int lineY;
    for(auto p : textPositionalValues)
    {
        lineY = std::get<1>(p);
        if(fmt.lineSpacing > 1.0)
        {
            lineY -= (int)(0.5 * (fmt.lineSpacing - 1.0) * fmt.sizeMult * (float)font.charH);
        }

        if(y >= lineY && y < lineY + (int)(fmt.lineSpacing * fmt.sizeMult * (float)font.charH))
        {
            break;
        }

        i++;

        if(i == textPositionalValues.size())
        {
            return static_cast<unsigned>(i);
        }
    }

    lineY = std::get<1>(textPositionalValues[i]);

    for(; i < textPositionalValues.size(); i++)
    {
        int currentX = std::get<0>(textPositionalValues[i]);
        currentX -= static_cast<int>((float)font.charW * fmt.sizeMult * 0.5);
        if(x >= currentX && x < currentX + (int)(fmt.sizeMult * font.charW))
        {
            break;
        }

        if(i < textPositionalValues.size() - 1 && std::get<1>(textPositionalValues[i + 1]) != lineY)
        {
            break;
        }

        if(value[i] == '\n')
        {
            break;
        }
    }

    return static_cast<unsigned>(i);
}

unsigned int TextField::shiftCursor(int offset)
{
    if((unsigned)abs(offset) > cursor && offset < 0)
    {
        cursor = 0;
    } else
    {
        cursor += offset;
        if(cursor > value.size())
        {
            cursor = (unsigned)value.size();
        }
    }

    if(SDL_GetModState() & KMOD_SHIFT)
    {
        selectionEnd = cursor;
    } else
    {
        selectionStart = selectionEnd = cursor;
    }

    return cursor;
}

bool TextField::textInsert(std::string s)
{
    if(selectionStart != selectionEnd)
    {
        textDelete();
    }

    value.insert(cursor, s);
    if(var)
    {
        var->set(value);
    }

    cursor += (unsigned)s.size();
    selectionStart = selectionEnd = cursor;

    lastEventTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    typing = true;
    updatePositionalValues = true;
    return true;
}

void TextField::textDelete()
{
    if(selectionStart != selectionEnd)
    {
        textDelete(selectionStart, selectionEnd);
    }
}

void TextField::textDelete(unsigned int start, unsigned int end)
{
    if(start > end)
    {
        unsigned int p = end;
        end = start;
        start = p;
    }

    if(end > value.size())
    {
        end = (unsigned)value.size();
    }

    value.erase(start, end - start);
    if(var)
    {
        var->set(value);
    }

    if(cursor > start)
    {
        cursor -= end - start;
        if(cursor < start)
        {
            cursor = start;
        }

        if(cursor > value.size())
        {
            cursor = (unsigned)value.size();
        }
    }

    if(selectionStart != selectionEnd)
    {
        selectionStart = selectionEnd = cursor;
    }

    lastEventTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    typing = true;
    updatePositionalValues = true;
}

std::string TextField::textCut()
{
    std::string s = value.substr(selectionStart, abs((int)selectionEnd - (int)selectionStart));
    textDelete();
    SDL_SetClipboardText(s.c_str());
    return s;
}

std::string TextField::textCopy()
{
    std::string s = value.substr(selectionStart, abs((int)selectionEnd - (int)selectionStart));
    SDL_SetClipboardText(s.c_str());
    return s;
}