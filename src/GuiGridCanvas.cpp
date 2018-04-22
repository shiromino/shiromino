#include "GuiGridCanvas.hpp"

#include <utility>
#include <SDL2/SDL.h>
#include "SGUIL/SGUIL.hpp"

#include "grid.h"

using namespace std;

GuiGridCanvas::GuiGridCanvas(grid_t *grid, BindableInt& paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect)
    : grid(grid), paletteVar(paletteVar), paletteTex(paletteTex), cellW(cellW), cellH(cellH)
{
    this->relativeDestRect = relativeDestRect;

    readPaletteSelection(&paletteVar);
    paletteSize = std::get<1>(paletteVar.getRange());

    clipboard = NULL;
    clipboardMoveMode = false;

    enabled = true;
    selected = false;
    canHoldKeyboardFocus = true;
    hasDefaultKeyboardFocus = true;
    hasKeyboardFocus = false;
}

void GuiGridCanvas::draw()
{
    
}

void GuiGridCanvas::handleEvent(GuiEvent& event)
{
    switch(event.type)
    {
        case mouse_hovered_onto:
            grid_lines_shown = true;
            break;

        case mouse_hovered_off:
            grid_lines_shown = false;
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

        case mouse_moved:
            this->mouseMoved(event.mouseMovedEvent->x, event.mouseMovedEvent->y);
            break;

        case key_pressed:
            this->keyPressed(event.keyPressedEvent->key);
            break;

        default:
            break;
    }
}

void GuiGridCanvas::mouseMoved(int x, int y)
{

}

void GuiGridCanvas::mouseClicked(int x, int y)
{
    GuiVirtualPoint point = {x, y};
    set(point);

    editInProgress = true;
}

void GuiGridCanvas::mouseDragged(int x, int y)
{

}

void GuiGridCanvas::mouseReleased(int x, int y)
{

}

void GuiGridCanvas::keyPressed(SDL_Keycode kc)
{
    int num = 0;
    bool pressedNumber = false;

    switch(kc)
    {
        case SDLK_a:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                selectionVertex1.x = 0;
                selectionVertex1.y = 0;
                selectionVertex2.x = grid->w - 1;
                selectionVertex2.y = grid->h - 1;
                selection = true;
            }

            break;

        case SDLK_x:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                cutSelection();
                selection = false;
            }

            break;

        case SDLK_c:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                copySelection();
            }

            break;

        case SDLK_v:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                if(clipboard != NULL)
                {
                    clipboardMoveMode = true;
                }
            }

            break;

        case SDLK_1:
            num = 0;
            pressedNumber = true;
            break;

        case SDLK_2:
            num = 1;
            pressedNumber = true;
            break;

        case SDLK_3:
            num = 2;
            pressedNumber = true;
            break;

        case SDLK_4:
            num = 3;
            pressedNumber = true;
            break;

        case SDLK_5:
            num = 4;
            pressedNumber = true;
            break;

        case SDLK_6:
            num = 5;
            pressedNumber = true;
            break;

        case SDLK_7:
            num = 6;
            pressedNumber = true;
            break;

        case SDLK_8:
            num = 7;
            pressedNumber = true;
            break;

        case SDLK_9:
            num = 8;
            pressedNumber = true;
            break;

        case SDLK_0:
            num = 9;
            pressedNumber = true;
            break;

        default:
            break;
    }

    if(pressedNumber && num < paletteSize)
    {
        if(selection)
        {
            int lesserX = selectionVertex1.x;
            int lesserY = selectionVertex1.y;
            int greaterX = selectionVertex2.x;
            int greaterY = selectionVertex2.y;

            if(lesserX > greaterX)
            {
                int swp = lesserX;
                lesserX = greaterX;
                greaterX = swp;
            }

            if(lesserY > greaterY)
            {
                int swp = lesserY;
                lesserY = greaterY;
                greaterY = swp;
            }

            grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX), (greaterY - lesserY)};

            if(paletteValMap.size() <= num + 1)
            {
                makeBackup();

                gridfillrect(grid, &selectionRect, num + 1);
                selection = false;
            }
            else if(!paletteValMap[num + 1].isFlag)
            {
                makeBackup();

                gridfillrect(grid, &selectionRect, paletteValMap[num + 1].mappedVal);
                selection = false;
            }
        }
        else
        {
            paletteVar.setInt(num);
        }
    }
}

void GuiGridCanvas::makeBackup()
{
    undoBuffer.push_back(gridcpy(grid, NULL));

    for(auto r : redoBuffer)
    {
        grid_destroy(r);
    }

    redoBuffer.clear();
}

void GuiGridCanvas::undo()
{
    if(undoBuffer.empty())
    {
        return;
    }

    redoBuffer.push_back(gridcpy(grid, NULL));
    grid = undoBuffer.back();
    undoBuffer.pop_back();
}

void GuiGridCanvas::redo()
{
    if(redoBuffer.empty())
    {
        return;
    }

    undoBuffer.push_back(gridcpy(grid, NULL));
    grid = redoBuffer.back();
    redoBuffer.pop_back();
}

void GuiGridCanvas::clearUndo()
{
    for(auto u : undoBuffer)
    {
        grid_destroy(u);
    }

    for(auto r : redoBuffer)
    {
        grid_destroy(r);
    }

    undoBuffer.clear();
    redoBuffer.clear();
}

void GuiGridCanvas::copySelection()
{
    if(clipboard)
    {
        grid_destroy(clipboard);
    }

    int lesserX = selectionVertex1.x;
    int lesserY = selectionVertex1.y;
    int greaterX = selectionVertex2.x;
    int greaterY = selectionVertex2.y;

    if(lesserX > greaterX)
    {
        int swp = lesserX;
        lesserX = greaterX;
        greaterX = swp;
    }

    if(lesserY > greaterY)
    {
        int swp = lesserY;
        lesserY = greaterY;
        greaterY = swp;
    }

    grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX), (greaterY - lesserY)};
    clipboard = gridfromsrcrect(grid, selectionRect);
}

void GuiGridCanvas::cutSelection()
{
    if(clipboard)
    {
        grid_destroy(clipboard);
    }

    int lesserX = selectionVertex1.x;
    int lesserY = selectionVertex1.y;
    int greaterX = selectionVertex2.x;
    int greaterY = selectionVertex2.y;

    if(lesserX > greaterX)
    {
        int swp = lesserX;
        lesserX = greaterX;
        greaterX = swp;
    }

    if(lesserY > greaterY)
    {
        int swp = lesserY;
        lesserY = greaterY;
        greaterY = swp;
    }

    grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX), (greaterY - lesserY)};
    clipboard = gridfromsrcrect(grid, selectionRect);

    if(paletteValMap.size() == 0)
    {
        gridfillrect(grid, &selectionRect, 0);
    }
    else
    {
        gridfillrect(grid, &selectionRect, paletteValMap[0].mappedVal);
    }
}

void GuiGridCanvas::pasteSelection()
{

}

void GuiGridCanvas::erase(GuiVirtualPoint& point)
{
    if(paletteValMap.size() == 0)
    {
        gridsetcell(grid, point.x, point.y, 0);
    }
    else
    {
        gridsetcell(grid, point.x, point.y, paletteValMap[0].mappedVal);
    }
}

void GuiGridCanvas::set(GuiVirtualPoint& point)
{
    if(paletteValMap.size() <= paletteSelection)
    {
        gridsetcell(grid, point.x, point.y, paletteSelection + 1);
    }
    else
    {
        if(paletteValMap[paletteSelection + 1].isFlag && (get(point) != paletteValMap[0].mappedVal))
        // only allow xor if the cell isn't empty
        {
            gridxorcell(grid, point.x, point.y, paletteValMap[paletteSelection + 1].mappedVal);
        }
        else
        {
            gridsetcell(grid, point.x, point.y, paletteValMap[paletteSelection + 1].mappedVal);
        }
    }
}

int GuiGridCanvas::get(GuiVirtualPoint& point)
{
    return gridgetcell(grid, point.x, point.y);
}
