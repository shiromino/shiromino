#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "SGUIL.hpp"

using namespace std;

GuiButton::GuiButton(int ID, SDL_Rect relativeDestRect, string displayString, BitFont& font)
    : font(font)
{
    this->ID = ID;
    this->enabled = true;
    this->canHoldKeyboardFocus = false;
    this->selected = false;
    this->hasKeyboardFocus = false;
    this->displayString = displayString;
    this->updateDisplayStringPVs = true;

    this->relativeDestRect = relativeDestRect;
}
/*
GuiButton::GuiButton(const GuiButton& gb)
    : font(gb.font)
{
    containingWindow = gb.containingWindow;
    relativeDestRect = gb.relativeDestRect;

    enabled = gb.enabled;
    canHoldKeyboardFocus = gb.canHoldKeyboardFocus;
    selected = gb.selected;
    hasKeyboardFocus = gb.hasKeyboardFocus;
    ID = gb.ID;
    displayString = gb.displayString;
    displayStringPositionalValues = gb.displayStringPositionalValues;
    updateDisplayStringPVs = gb.updateDisplayStringPVs;
    displayTexture = gb.displayTexture;
    // yes, this does just copy the pointer... copying the texture itself would
    // be costly and probably unnecessary in most cases

    eventHooks = gb.eventHooks;
    renderHooks = gb.renderHooks;
}
*/
GuiButton::~GuiButton()
{

}

void GuiButton::draw()
{
    TextFormat fmt {};
    fmt.alignment = enumAlignment::center;

    SDL_Rect textRect = relativeDestRect;
    textRect.y += 4;

    rgba_t rgbaBorder = this->selected ? 0xFF2020FF : GUI_RGBA_DEFAULT;

    if(updateDisplayStringPVs)
    {
        Gui_GenerateTextPositionalValues(displayString, &fmt, font, textRect, displayStringPositionalValues, false, false);
        updateDisplayStringPVs = false;
    }

    Gui_DrawBorder(relativeDestRect, 2, rgbaBorder);
    Gui_DrawText_PV(displayString, &fmt, font, displayStringPositionalValues, 0, 0);
}

void GuiButton::addEventHook(function<void(GuiButton&, GuiEvent&)> callback, enumGuiEventType type)
{
    eventHooks.push_back( GuiEventHook<GuiButton>{callback, type} );
}

void GuiButton::addRenderHook(function<void(GuiButton&)> callback)
{
    renderHooks.push_back( GuiRenderHook<GuiButton>{callback} );
}

void GuiButton::mouseClicked(int x, int y)
{

}

void GuiButton::mouseDragged(int x, int y)
{

}

void GuiButton::mouseReleased(int x, int y)
{

}

void GuiButton::keyPressed(SDL_Keycode kc)
{

}
