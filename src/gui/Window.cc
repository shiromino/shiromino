#include "SDL_blendmode.h"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include "GUI.h"

struct CoreState;

Window::Window(CoreState *origin, std::string title, BitFont *titleFont, std::function<void(GUIInteractable&, GUIEvent&)> interactionEventCallback, SDL_Rect& destRect)
    : title(title)
{
    if(titleFont)
    {
        this->titleFont = *titleFont;
    }
    this->destRect = destRect;
    this->origin = origin;

    canvas = SDL_CreateTexture(
        guiSDLRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        destRect.w, destRect.h);

    {
        void* pixels = malloc(640 * 480 * sizeof(uint32_t));
        if(!pixels)
        {
            throw std::bad_alloc();
        }
        memset(pixels, 0, 640 * 480 * sizeof(uint32_t));
        SDL_UpdateTexture(canvas, NULL, pixels, 640 * sizeof(uint32_t));
        free(pixels);
    }

    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);

    keyboardFocus = -1;
    controlSelection = 0;
    selectingByMouse = true;

    rgbaBackground = 0x101010AF;
    rgbaTitleBar = 0x202030FF;

    titleBarHeight = 28;
    showBackground = true;
    showTitleBar = true;
    updateTitlePositionalValues = true;
    moveable = true;
    moving = false;

    useExtWindowX = false;
    useExtWindowY = false;
    initialX = 0;
    initialY = 0;
    moveBeginX = 0;
    moveBeginY = 0;

    if(interactionEventCallback)
    {
        this->interactionEventCallback = interactionEventCallback;
    }
}

Window::~Window()
{
    SDL_DestroyTexture(canvas);
    for(auto e : elements)
    {
        delete e;
    }
}

void Window::draw()
{
    SDL_SetRenderTarget(guiSDLRenderer, canvas);

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
    SDL_GetRenderDrawColor(guiSDLRenderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(guiSDLRenderer, rgba_R(rgbaBackground), rgba_G(rgbaBackground), rgba_B(rgbaBackground), rgba_A(rgbaBackground));
    SDL_RenderClear(guiSDLRenderer);

    if(showTitleBar)
    {
        SDL_SetRenderDrawColor(guiSDLRenderer, rgba_R(rgbaTitleBar), rgba_G(rgbaTitleBar), rgba_B(rgbaTitleBar), rgba_A(rgbaTitleBar));
        SDL_Rect titleBarRect = {0, 0, destRect.w, titleBarHeight};
        SDL_RenderFillRect(guiSDLRenderer, &titleBarRect);

        TextFormat fmt {};
        fmt.alignment = enumAlignment::center;

        SDL_Rect titleRect = {0, 6, destRect.w, 16};
        SDL_SetRenderDrawColor(guiSDLRenderer, r, g, b, a);

        if(updateTitlePositionalValues)
        {
            generateGUITextPositionalValues(title, &fmt, titleFont, titleRect, titlePositionalValues);
            updateTitlePositionalValues = false;
        }

        drawGUITextPV(title, &fmt, titleFont, titlePositionalValues);
    }

    SDL_SetRenderDrawColor(guiSDLRenderer, r, g, b, a);

    for(auto e : elements)
    {
        e->draw();
    }

    SDL_SetRenderTarget(guiSDLRenderer, NULL);
    SDL_RenderCopy(guiSDLRenderer, canvas, NULL, &destRect);
}

void Window::addElement(GUIElement *element)
{
    if(element != NULL)
    {
        element->setWindow(*this);
        elements.push_back(element);
    }
}

void Window::addControlElement(GUIInteractable *element)
{
    if(element != NULL)
    {
        element->setWindow(*this);
        elements.push_back(element);
        controlList.push_back(element);
    }
}

void Window::handleSDLEvent(SDL_Event& sdlEvent, GUIPoint logicalMousePos)
{
    int x = logicalMousePos.x;
    int y = logicalMousePos.y;

    switch(sdlEvent.type)
    {
        case SDL_KEYDOWN:
            if(sdlEvent.key.keysym.sym == SDLK_ESCAPE)
            {
                if(keyboardFocus > -1 && decltype(controlList)::size_type(keyboardFocus) < controlList.size())
                {
                    controlList[keyboardFocus]->hasKeyboardFocus = false;
                    keyboardFocus = -1;
                }
            }

            if(keyboardFocus > -1 && decltype(controlList)::size_type(keyboardFocus) < controlList.size())
            {
                GUIEvent keyDownEvent {key_pressed, sdlEvent.key.keysym.sym};
                controlList[keyboardFocus]->handleEvent(keyDownEvent);

                if(interactionEventCallback)
                {
                    interactionEventCallback(*controlList[keyboardFocus], keyDownEvent);
                }
            }
            else
            {
                GUIEvent keyDownEvent {key_pressed, sdlEvent.key.keysym.sym};
                for(auto e : controlList)
                {
                    if(e->hasDefaultKeyboardFocus)
                    {
                        e->handleEvent(keyDownEvent);
                    }
                }
            }

            break;

        case SDL_KEYUP:
            if(keyboardFocus > -1 && decltype(controlList)::size_type(keyboardFocus) < controlList.size())
            {
                GUIEvent keyUpEvent {key_released, sdlEvent.key.keysym.sym};
                controlList[keyboardFocus]->handleEvent(keyUpEvent);

                if(interactionEventCallback)
                {
                    interactionEventCallback(*controlList[keyboardFocus], keyUpEvent);
                }
            }
            else
            {
                GUIEvent keyUpEvent {key_released, sdlEvent.key.keysym.sym};
                for(auto e : controlList)
                {
                    if(e->hasDefaultKeyboardFocus)
                    {
                        e->handleEvent(keyUpEvent);
                    }
                }
            }

            break;

        case SDL_TEXTINPUT:
            if(keyboardFocus > -1 && decltype(controlList)::size_type(keyboardFocus) < controlList.size())
            {
                GUIEvent textInputEvent {textinput_guievent, {sdlEvent.text.text} };
                controlList[keyboardFocus]->handleEvent(textInputEvent);

                if(interactionEventCallback)
                {
                    interactionEventCallback(*controlList[keyboardFocus], textInputEvent);
                }
            }
            else
            {
                GUIEvent textInputEvent {textinput_guievent, {sdlEvent.text.text} };
                for(auto e : controlList)
                {
                    if(e->hasDefaultKeyboardFocus)
                    {
                        e->handleEvent(textInputEvent);
                    }
                }
            }

            break;

        case SDL_MOUSEBUTTONDOWN:
            if(sdlEvent.button.button == SDL_BUTTON_LEFT || sdlEvent.button.button == SDL_BUTTON_RIGHT)
            {
                GUIEvent mouseButtonDownEvent {mouse_clicked, x - destRect.x, y - destRect.y, sdlEvent.button.button};
                GUIInteractable *e = getControlElementAt(x, y);
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

                    if(interactionEventCallback)
                    {
                        interactionEventCallback(*e, mouseButtonDownEvent);
                    }
                } else
                {
                    if(keyboardFocus > -1)
                    {
                        controlList[keyboardFocus]->hasKeyboardFocus = false;
                        keyboardFocus = -1;
                    }

                    mouseClicked(x, y, sdlEvent.button.button);
                }
            }

            break;

        case SDL_MOUSEBUTTONUP:
            if(sdlEvent.button.button == SDL_BUTTON_LEFT || sdlEvent.button.button == SDL_BUTTON_RIGHT)
            {
                GUIEvent mouseButtonUpEvent {mouse_released, x - destRect.x, y - destRect.y, sdlEvent.button.button};
                GUIInteractable *e = getControlElementAt(x, y);
                if(e)
                {
                    e->handleEvent(mouseButtonUpEvent);

                    if(interactionEventCallback)
                    {
                        interactionEventCallback(*e, mouseButtonUpEvent);
                    }
                }
                else
                {
                    mouseReleased(x, y, sdlEvent.button.button);
                }
            }

            break;

        case SDL_MOUSEMOTION:
            if(moving && (sdlEvent.motion.state & SDL_BUTTON_LMASK))
            {
                mouseDragged(x, y, SDL_BUTTON_LEFT);
            }

            if(sdlEvent.motion.state & SDL_BUTTON_LMASK || sdlEvent.motion.state & SDL_BUTTON_RMASK)
            {
                Uint8 button = sdlEvent.motion.state & SDL_BUTTON_LMASK ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
                GUIEvent mouseDraggedEvent {mouse_dragged, x - destRect.x, y - destRect.y, button};
                GUIInteractable *e = getControlElementAt(x, y);
                if(e)
                {
                    e->handleEvent(mouseDraggedEvent);

                    if(interactionEventCallback)
                    {
                        interactionEventCallback(*e, mouseDraggedEvent);
                    }
                } else
                {
                    mouseDragged(x, y, button);
                }
            } else
            {
                GUIInteractable *e = getControlElementAt(x, y);
                GUIInteractable *selectedElement = controlSelection > -1 ? controlList[controlSelection] : NULL;
                if(e)
                {
                    if(e != selectedElement)
                    {
                        selectingByMouse = true;

                        if(selectedElement != NULL)
                        {
                            GUIEvent mouseHoveredOffEvent {mouse_hovered_off, x - destRect.x, y - destRect.y, 0};
                            selectedElement->handleEvent(mouseHoveredOffEvent);
                            selectedElement->selected = false;

                            if(interactionEventCallback)
                            {
                                interactionEventCallback(*selectedElement, mouseHoveredOffEvent);
                            }
                        }

                        for(unsigned int c = 0; c < controlList.size(); c++)
                        {
                            if(controlList[c] == e)
                            {
                                controlSelection = c;
                            }
                        }

                        e->selected = true;
                        GUIEvent mouseHoveredOntoEvent {mouse_hovered_onto, x - destRect.x, y - destRect.y, 0};
                        e->handleEvent(mouseHoveredOntoEvent);

                        if(interactionEventCallback)
                        {
                            interactionEventCallback(*e, mouseHoveredOntoEvent);
                        }
                    }

                    GUIEvent mouseMovedEvent {mouse_moved, x - destRect.x, y - destRect.y, 0};
                    e->handleEvent(mouseMovedEvent);

                    if(interactionEventCallback)
                    {
                        interactionEventCallback(*e, mouseMovedEvent);
                    }
                } else
                {
                    if(selectingByMouse && selectedElement != NULL)
                    {
                        GUIEvent mouseHoveredOffEvent {mouse_hovered_off, x - destRect.x, y - destRect.y, 0};
                        selectedElement->handleEvent(mouseHoveredOffEvent);
                        selectedElement->selected = false;

                        if(interactionEventCallback)
                        {
                            interactionEventCallback(*selectedElement, mouseHoveredOffEvent);
                        }

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

GUIInteractable *Window::getControlElementAt(int x, int y)
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

void Window::mouseMoved(int, int)
{

}

void Window::mouseClicked(int x, int y, Uint8)
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

void Window::mouseDragged(int x, int y, Uint8)
{
    if(moving)
    {
        destRect.x = initialX + x - moveBeginX;
        destRect.y = initialY + y - moveBeginY;
    }
}

void Window::mouseReleased(int, int, Uint8)
{
    moving = false;
}

void Window::keyPressed(SDL_Keycode)
{

}

void Window::textInput(std::string)
{

}
