#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <vector>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "SGUIL.hpp"

using namespace std;


GuiTextField::GuiTextField(int ID, BindableString *var, BitFont& font, SDL_Rect relativeDestRect)
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

GuiTextField::GuiTextField(int ID, BindableString *var, string valueDefault, BitFont& font, SDL_Rect relativeDestRect)
    : GuiTextField(ID, var, font, relativeDestRect)
{
    if(!valueDefault.empty())
    {
        this->value = valueDefault;
    }
}
/*
GuiTextField::GuiTextField(int ID, string valueDefault, BitFont& font,
    SDL_Rect relativeDestRect, function<int(optionID_t, string)> valueUpdateCallback, string *accessPtr, bool accessCopy)
    : GuiTextField(ID, optionID, font, relativeDestRect)
{
    if(!valueDefault.empty())
    {
        this->value = valueDefault;
    }

    if(valueUpdateCallback)
    {
        this->valueUpdateCallback = valueUpdateCallback;
        this->accessType |= use_callback;
    }

    if(accessPtr)
    {
        this->accessPtr = accessPtr;
        if(accessCopy)
        {
            this->accessType |= random_access_copy;
        } else
        {
            this->accessType |= random_access;
        }
    }
}*/

/*
GuiTextField::GuiTextField(const GuiTextField& gtf)
    : font(gtf.font)
{
    containingWindow = gtf.containingWindow;
    relativeDestRect = gtf.relativeDestRect;

    enabled = gtf.enabled;
    canHoldKeyboardFocus = gtf.canHoldKeyboardFocus;
    selected = gtf.selected;
    hasKeyboardFocus = gtf.hasKeyboardFocus;
    ID = gtf.ID;
    displayString = gtf.displayString;
    displayStringPositionalValues = gtf.displayStringPositionalValues;
    updateDisplayStringPVs = gtf.updateDisplayStringPVs;
    displayTexture = gtf.displayTexture;
    // yes, this does just copy the pointer... copying the texture itself would
    // be costly and probably unnecessary in almost all cases

    valueUpdateCallback = gtf.valueUpdateCallback;
    optionID = gtf.optionID;
    value = gtf.value;

    eventHooks = gtf.eventHooks;
    renderHooks = gtf.renderHooks;
    textPositionalValues = gtf.textPositionalValues;
    fmt = gtf.fmt;

    cursor = gtf.cursor;
    scrollPosX = gtf.scrollPosX;
    scrollPosY = gtf.scrollPosY;
    lastEventTime = gtf.lastEventTime;
    typing = gtf.typing;
    selectionStart = gtf.selectionStart;
    selectionEnd = gtf.selectionEnd;
    updatePositionalValues = gtf.updatePositionalValues;
}
*/
GuiTextField::~GuiTextField()
{

}

void GuiTextField::setTextFormat(TextFormat& fmt)
{
    this->fmt = fmt;
}

void GuiTextField::draw()
{
    this->prepareRenderTarget(false);

    if(updatePositionalValues)
    {
        Gui_GenerateTextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues, horizontalScroll, verticalScroll);
        updatePositionalValues = false;
    }

    rgba_t rgbaBorder = this->hasKeyboardFocus ? 0xFF2020FF : 0x8080FFFF;
    Gui_DrawBorder(relativeDestRect, 1, rgbaBorder);

    if(selectionStart == selectionEnd)
    {
        Gui_DrawText_PV(value, &fmt, font, textPositionalValues, scrollPosX, scrollPosY);
    } else
    {
        TextFormat selectionFmt = fmt;
        selectionFmt.rgba = 0x000000FF;
        selectionFmt.highlight = true;

        unsigned int start = selectionStart < selectionEnd ? selectionStart : selectionEnd;
        unsigned int end = selectionStart < selectionEnd ? selectionEnd : selectionStart;

        Gui_DrawTextPartial_PV(value, 0, start, &fmt, font, textPositionalValues, scrollPosX, scrollPosY);
        Gui_DrawTextPartial_PV(value, start, end - start, &selectionFmt, font, textPositionalValues, scrollPosX, scrollPosY);
        Gui_DrawTextPartial_PV(value, end, value.length() - end, &fmt, font, textPositionalValues, scrollPosX, scrollPosY);
    }

    bool cursorBlinkOn = false;
    uint64_t ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

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
            cursorRect.x = get<0>(textPositionalValues[cursor]);
            cursorRect.y = get<1>(textPositionalValues[cursor]);
        } else if(textPositionalValues.size() != 0)
        {
            if(value[textPositionalValues.size() - 1] == '\n')
            {
                cursorRect.x = relativeDestRect.x;
                cursorRect.y = get<1>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.y += fmt.lineSpacing * fmt.sizeMult * (float)font.charH;
            } else
            {
                cursorRect.x = get<0>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.y = get<1>(textPositionalValues[textPositionalValues.size() - 1]);
                cursorRect.x += (float)font.charW * fmt.sizeMult;
            }
        } else
        {
            cursorRect.x = relativeDestRect.x;
            cursorRect.y = relativeDestRect.y;
        }

        cursorRect.w = 2.0 * fmt.sizeMult;
        cursorRect.h = (float)font.charH * fmt.sizeMult;

        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 a;
        SDL_GetRenderDrawColor(Gui_SDL_Renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(Gui_SDL_Renderer, &cursorRect);
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, r, g, b, a);
    }
}

void GuiTextField::addEventHook(function<void(GuiTextField&, GuiEvent&)> callback, enumGuiEventType type)
{
    eventHooks.push_back( GuiEventHook<GuiTextField>{callback, type} );
}

void GuiTextField::addRenderHook(function<void(GuiTextField&)> callback)
{
    renderHooks.push_back( GuiRenderHook<GuiTextField>{callback} );
}

void GuiTextField::handleEvent(GuiEvent& event)
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
                cout << "GuiTextField::handleEvent(): Error: SDL_CreateSystemCursor(): " << SDL_GetError() << endl;
            }

            break;
        case mouse_hovered_off:
            sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
            if(sdlCursor != NULL)
            {
                SDL_SetCursor(sdlCursor);
            } else
            {
                cout << "GuiTextField::handleEvent(): Error: SDL_CreateSystemCursor(): " << SDL_GetError() << endl;
            }

            break;
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
        case textinput_event:
            this->textInput(event.textInputEvent->text);
            break;
        default:
            break;
    }
}

void GuiTextField::mouseClicked(int x, int y)
{
    if(updatePositionalValues)
    {
        Gui_GenerateTextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues, horizontalScroll, verticalScroll);
        updatePositionalValues = false;
    }

    int pos = getPositionUnderMouse(x, y);

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

void GuiTextField::mouseDragged(int x, int y)
{
    if(!hasKeyboardFocus)
    {
        return;
    }

    if(updatePositionalValues)
    {
        Gui_GenerateTextPositionalValues(this->value, &fmt, font, relativeDestRect, textPositionalValues, horizontalScroll, verticalScroll);
        updatePositionalValues = false;
    }

    int pos = getPositionUnderMouse(x, y);
    lastEventTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    cursor = selectionEnd = pos;
}

void GuiTextField::mouseReleased(int x, int y)
{

}

void GuiTextField::keyPressed(SDL_Keycode kc)
{
    switch(kc)
    {
        case SDLK_a:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                shiftCursor((int)(value.length() - cursor));
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
                string s = {SDL_GetClipboardText()};
                textInsert(s);
            }

            break;

        case SDLK_HOME:
            shiftCursor(0 - (int)cursor);
            break;

        case SDLK_END:
            shiftCursor((int)(value.length() - cursor));
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
            } else if(cursor < value.length())
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

void GuiTextField::textInput(string s)
{
    textInsert(s);
}

unsigned int GuiTextField::getPositionUnderMouse(int x, int y)
{
    int i = 0;
    int lineY;
    for(auto p : textPositionalValues)
    {
        lineY = get<1>(p);
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
            return i;
        }
    }

    lineY = get<1>(textPositionalValues[i]);

    for(; i < textPositionalValues.size(); i++)
    {
        int currentX = get<0>(textPositionalValues[i]);
        currentX -= (float)font.charW * fmt.sizeMult * 0.5;
        if(x >= currentX && x < currentX + (int)(fmt.sizeMult * font.charW))
        {
            break;
        }

        if(i < textPositionalValues.size() - 1 && get<1>(textPositionalValues[i + 1]) != lineY)
        {
            break;
        }

        if(value[i] == '\n')
        {
            break;
        }
    }

    return i;
}

unsigned int GuiTextField::shiftCursor(int offset)
{
    unsigned int oldcursor = cursor;

    if(abs(offset) > cursor && offset < 0)
    {
        cursor = 0;
    } else
    {
        cursor += offset;
        if(cursor > value.length())
        {
            cursor = value.length();
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

bool GuiTextField::textInsert(string s)
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

    cursor += s.length();
    selectionStart = selectionEnd = cursor;

    lastEventTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    typing = true;
    updatePositionalValues = true;
    return true;
}

void GuiTextField::textDelete()
{
    if(selectionStart != selectionEnd)
    {
        textDelete(selectionStart, selectionEnd);
    }
}

void GuiTextField::textDelete(unsigned int start, unsigned int end)
{
    if(start > end)
    {
        unsigned int p = end;
        end = start;
        start = p;
    }

    if(start < 0)
    {
        start = 0;
    }

    if(end > value.length())
    {
        end = value.length();
    }

    value.erase((string::iterator)&value[start], (string::iterator)&value[end]);
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

        if(cursor > value.length())
        {
            cursor = value.length();
        }
    }

    if(selectionStart != selectionEnd)
    {
        selectionStart = selectionEnd = cursor;
    }

    lastEventTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    typing = true;
    updatePositionalValues = true;
}

string GuiTextField::textCut()
{
    string s = value.substr(selectionStart, abs((int)selectionEnd - (int)selectionStart));
    textDelete();
    SDL_SetClipboardText(s.c_str());
    return s;
}

string GuiTextField::textCopy()
{
    string s = value.substr(selectionStart, abs((int)selectionEnd - (int)selectionStart));
    SDL_SetClipboardText(s.c_str());
    return s;
}
