#include "SDL_keycode.h"
#include "SDL_rect.h"
#include "SDL_stdinc.h"
#include <string>
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
        generateGUITextPositionalValues(displayString, &fmt, font, textRect, displayStringPositionalValues);
        updateDisplayStringPVs = false;
    }

    GUIDrawBorder(relativeDestRect, 2, rgbaBorder);
    drawGUITextPV(displayString, &fmt, font, displayStringPositionalValues);
}

void Button::mouseClicked(int, int, Uint8)
{

}

void Button::mouseDragged(int, int, Uint8)
{

}

void Button::mouseReleased(int, int, Uint8)
{

}

void Button::keyPressed(SDL_Keycode)
{

}
