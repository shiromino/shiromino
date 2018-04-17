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

GuiWindow::GuiWindow(string title, BitFont& titleFont, function<void(GuiInteractable&)> interactionEventCallback, SDL_Rect& destRect)
    : title(title), titleFont(titleFont)
{
    /*
    if(destRect.w == 0)
    {
        SDL_
        destRect.w =
    }
    */

    this->destRect = destRect;

    canvas = SDL_CreateTexture(
        Gui_SDL_Renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        destRect.w, destRect.h);

    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);

    keyboardFocus = -1;
    controlSelection = -1;
    selectingByMouse = true;

    rgbaBackground = 0x101010AF;
    rgbaTitleBar = 0x202030FF;

    titleBarHeight = 28;
    showTitleBar = true;
    updateTitlePositionalValues = true;
    moveable = true;

    if(interactionEventCallback)
    {
        this->interactionEventCallback = interactionEventCallback;
    }
}

GuiWindow::~GuiWindow()
{
    SDL_DestroyTexture(canvas);
    for(auto e : elements)
    {
        delete e;
    }
}

void GuiWindow::draw()
{
    SDL_SetRenderTarget(Gui_SDL_Renderer, canvas);

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_GetRenderDrawColor(Gui_SDL_Renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(Gui_SDL_Renderer, rgba_R(rgbaBackground), rgba_G(rgbaBackground), rgba_B(rgbaBackground), rgba_A(rgbaBackground));
    SDL_RenderClear(Gui_SDL_Renderer);

    if(showTitleBar)
    {
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, rgba_R(rgbaTitleBar), rgba_G(rgbaTitleBar), rgba_B(rgbaTitleBar), rgba_A(rgbaTitleBar));
        SDL_Rect titleBarRect = {0, 0, destRect.w, titleBarHeight};
        SDL_RenderFillRect(Gui_SDL_Renderer, &titleBarRect);

        TextFormat fmt {};
        fmt.alignment = enumAlignment::center;

        SDL_Rect titleRect = {0, 6, destRect.w, 16};
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, r, g, b, a);

        if(updateTitlePositionalValues)
        {
            Gui_GenerateTextPositionalValues(title, &fmt, titleFont, titleRect, titlePositionalValues, false, false);
            updateTitlePositionalValues = false;
        }

        Gui_DrawText_PV(title, &fmt, titleFont, titlePositionalValues, 0, 0);
    }

    SDL_SetRenderDrawColor(Gui_SDL_Renderer, r, g, b, a);

    for(auto e : elements)
    {
        e->draw();
    }

    SDL_SetRenderTarget(Gui_SDL_Renderer, NULL);
    SDL_RenderCopy(Gui_SDL_Renderer, canvas, NULL, &destRect);
}

void GuiWindow::addEventHook(function<void(GuiWindow&, GuiEvent&)> callback, enumGuiEventType type)
{
    eventHooks.push_back( GuiEventHook<GuiWindow>{callback, type} );
}

void GuiWindow::addRenderHook(function<void(GuiWindow&)> callback)
{
    renderHooks.push_back( GuiRenderHook<GuiWindow>{callback} );
}

/*
void GuiWindow::addText(GuiText& element)
{
    element.setWindow(*this);
    GuiText *newElement = new GuiText{element};

    elements.push_back(newElement);
}

void GuiWindow::addTextField(GuiTextField& element)
{
    element.setWindow(*this);
    GuiTextField *newElement = new GuiTextField{element};

    elements.push_back(newElement);
    controlList.push_back(newElement);
}

void GuiWindow::addButton(GuiButton& element)
{
    element.setWindow(*this);
    GuiButton *newElement = new GuiButton{element};

    elements.push_back(newElement);
    controlList.push_back(newElement);
}
*/

void GuiWindow::addElement(GuiElement *element)
{
    if(element != NULL)
    {
        element->setWindow(*this);
        elements.push_back(element);
    }
}

void GuiWindow::addControlElement(GuiInteractable *element)
{
    if(element != NULL)
    {
        element->setWindow(*this);
        elements.push_back(element);
        controlList.push_back(element);
    }
}

void GuiWindow::handleSDLEvent(SDL_Event& sdlEvent)
{
    switch(sdlEvent.type)
    {
        case SDL_KEYDOWN:
            if(sdlEvent.key.keysym.sym == SDLK_ESCAPE)
            {
                if(keyboardFocus > -1 && keyboardFocus < controlList.size())
                {
                    controlList[keyboardFocus]->hasKeyboardFocus = false;
                    keyboardFocus = -1;
                }
            }

            if(keyboardFocus > -1 && keyboardFocus < controlList.size())
            {
                GuiEvent keyDownEvent {key_pressed, sdlEvent.key.keysym.sym};
                controlList[keyboardFocus]->handleEvent(keyDownEvent);
            }

            break;

        case SDL_KEYUP:
            if(keyboardFocus > -1 && keyboardFocus < controlList.size())
            {
                GuiEvent keyUpEvent {key_released, sdlEvent.key.keysym.sym};
                controlList[keyboardFocus]->handleEvent(keyUpEvent);
            }

            break;

        case SDL_TEXTINPUT:
            if(keyboardFocus > -1 && keyboardFocus < controlList.size())
            {
                GuiEvent textInputEvent {textinput_event, {sdlEvent.text.text} };
                controlList[keyboardFocus]->handleEvent(textInputEvent);
            }

            break;

        case SDL_MOUSEBUTTONDOWN:
            if(sdlEvent.button.button == SDL_BUTTON_LEFT)
            {
                int x;
                int y;
                SDL_GetMouseState(&x, &y);
                GuiEvent mouseButtonDownEvent {mouse_clicked, x - destRect.x, y - destRect.y};
                GuiInteractable *e = getControlElementAt(x, y);
                if(e)
                {
                    if(keyboardFocus > -1 && e != controlList[keyboardFocus])
                    {
                        controlList[keyboardFocus]->hasKeyboardFocus = false;
                        keyboardFocus = -1;
                    }

                    if(e->canHoldKeyboardFocus)
                    {
                        for(unsigned int c = 0; c < controlList.size(); c++)
                        {
                            if(controlList[c] == e)
                            {
                                keyboardFocus = c;
                            }
                        }

                        e->hasKeyboardFocus = true;
                    }

                    e->handleEvent(mouseButtonDownEvent);
                } else
                {
                    if(keyboardFocus > -1)
                    {
                        controlList[keyboardFocus]->hasKeyboardFocus = false;
                        keyboardFocus = -1;
                    }

                    mouseClicked(x, y);
                }
            }

            break;

        case SDL_MOUSEBUTTONUP:
            if(sdlEvent.button.button == SDL_BUTTON_LEFT)
            {
                int x;
                int y;
                SDL_GetMouseState(&x, &y);
                GuiEvent mouseButtonUpEvent {mouse_released, x - destRect.x, y - destRect.y};
                GuiInteractable *e = getControlElementAt(x, y);
                if(e)
                {
                    e->handleEvent(mouseButtonUpEvent);
                } else
                {
                    mouseReleased(x, y);
                }
            }

            break;

        case SDL_MOUSEMOTION:
            if(moving && (sdlEvent.motion.state & SDL_BUTTON_LMASK))
            {
                int x;
                int y;
                SDL_GetMouseState(&x, &y);
                mouseDragged(x, y);
            } else if(sdlEvent.motion.state & SDL_BUTTON_LMASK)
            {
                int x;
                int y;
                SDL_GetMouseState(&x, &y);
                GuiEvent mouseDraggedEvent {mouse_dragged, x - destRect.x, y - destRect.y};
                GuiInteractable *e = getControlElementAt(x, y);
                if(e)
                {
                    e->handleEvent(mouseDraggedEvent);
                } else
                {
                    mouseDragged(x, y);
                }
            } else
            {
                int x;
                int y;
                SDL_GetMouseState(&x, &y);
                GuiInteractable *e = getControlElementAt(x, y);
                GuiInteractable *selectedElement = controlSelection > -1 ? controlList[controlSelection] : NULL;
                if(e)
                {
                    if(e != selectedElement)
                    {
                        selectingByMouse = true;

                        if(selectedElement != NULL)
                        {
                            GuiEvent mouseHoveredOffEvent {mouse_hovered_off, x - destRect.x, y - destRect.y};
                            selectedElement->handleEvent(mouseHoveredOffEvent);
                            selectedElement->selected = false;
                        }

                        for(unsigned int c = 0; c < controlList.size(); c++)
                        {
                            if(controlList[c] == e)
                            {
                                controlSelection = c;
                            }
                        }

                        e->selected = true;
                        GuiEvent mouseHoveredOntoEvent {mouse_hovered_onto, x - destRect.x, y - destRect.y};
                        e->handleEvent(mouseHoveredOntoEvent);
                    }

                    GuiEvent mouseMovedEvent {mouse_moved, x - destRect.x, y - destRect.y};
                    e->handleEvent(mouseMovedEvent);
                } else
                {
                    if(selectingByMouse && selectedElement != NULL)
                    {
                        GuiEvent mouseHoveredOffEvent {mouse_hovered_off, x - destRect.x, y - destRect.y};
                        selectedElement->handleEvent(mouseHoveredOffEvent);
                        selectedElement->selected = false;

                        controlSelection = -1;
                    }

                    mouseMoved(x, y);
                }
            }

            break;

        default:
            break;
    }
}

GuiInteractable *GuiWindow::getControlElementAt(int x, int y)
{
    x -= destRect.x;
    y -= destRect.y;

    for(auto e : controlList)
    {
        if(e->canInteractAt(x, y))
        {
            return e;
        }
    }

    return NULL;
}

void GuiWindow::mouseMoved(int x, int y)
{

}

void GuiWindow::mouseClicked(int x, int y)
{
    if(showTitleBar && moveable)
    {
        if(x > destRect.x && x <= destRect.x + destRect.w && y > destRect.y && y <= destRect.y + titleBarHeight)
        {
            initialX = destRect.x;
            initialY = destRect.y;
            moveBeginX = x;
            moveBeginY = y;
            moving = true;
        }
    }
}

void GuiWindow::mouseDragged(int x, int y)
{
    if(moving)
    {
        destRect.x = initialX + x - moveBeginX;
        destRect.y = initialY + y - moveBeginY;
    }
}

void GuiWindow::mouseReleased(int x, int y)
{
    moving = false;
}

void GuiWindow::keyPressed(SDL_Keycode kc)
{

}

void GuiWindow::textInput(string s)
{

}
