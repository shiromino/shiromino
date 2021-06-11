#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <map>
#include "SDL.h"
#include "SDL_image.h"
#include "GUI.h"

Button::Button(int ID, SDL_Rect relativeDestRect, std::string displayString, BitFont& font)
    : font(font)
{
    this->ID = ID;
    this->enabled = true;
    this->canHoldKeyboardFocus = false;
    this->hasDefaultKeyboardFocus = false;
    this->selected = false;
    this->hasKeyboardFocus = false;
    this->displayString = displayString;
    this->updateDisplayStringPVs = true;

    this->relativeDestRect = relativeDestRect;
}

Button::~Button()
{

}

void Button::draw()
{
    TextFormat fmt {};
    fmt.alignment = enumAlignment::center;

    SDL_Rect textRect = relativeDestRect;
    textRect.y += 4;

    Shiro::GUI::rgba_t rgbaBorder = this->selected
        ? 0xFF2020FF
        : Shiro::GUI::RGBA_DEFAULT;

    if(updateDisplayStringPVs)
    {
        generateGUITextPositionalValues(displayString, &fmt, font, textRect, displayStringPositionalValues, false, false);
        updateDisplayStringPVs = false;
    }

    GUIDrawBorder(relativeDestRect, 2, rgbaBorder);
    drawGUITextPV(displayString, &fmt, font, displayStringPositionalValues, 0, 0);
}

void Button::mouseClicked(int x, int y, Uint8 button)
{

}

void Button::mouseDragged(int x, int y, Uint8 button)
{

}

void Button::mouseReleased(int x, int y, Uint8 button)
{

}

void Button::keyPressed(SDL_Keycode kc)
{

}
