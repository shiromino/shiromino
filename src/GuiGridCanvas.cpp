#include "GuiGridCanvas.hpp"

#include <utility>
#include <memory>
#include <functional>
#include <SDL2/SDL.h>
#include "SGUIL/SGUIL.hpp"

#include "grid.h"

using namespace std;

GuiGridCanvas::GuiGridCanvas(int ID, grid_t *grid, BindableInt& paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect)
    : grid(grid), paletteVar(paletteVar), paletteTex(paletteTex), cellW(cellW), cellH(cellH)
{
    this->relativeDestRect = relativeDestRect;
    this->ID = ID;

    readPaletteSelection(&paletteVar);
    paletteSize = get<1>(paletteVar.getRange());

    function<void(BindableVariable *)> membFunc = [=](BindableVariable *bv) { this->readPaletteSelection(bv); };

    unique_ptr<VariableObserver> vob {
        (VariableObserver *)( new MemberVariableObserver{membFunc} )
    };

    paletteVar.addObserver(vob);

    clipboard = NULL;
    selection = false;
    clipboardMoveMode = false;
    gridLinesShown = false;
    cursorShown = false;

    enabled = true;
    selected = false;
    canHoldKeyboardFocus = true;
    hasDefaultKeyboardFocus = true;
    hasKeyboardFocus = false;

    updateDisplayStringPVs = false;
}

void GuiGridCanvas::draw()
{
    this->prepareRenderTarget(false);

    Gui_DrawBorder(relativeDestRect, 1, GUI_RGBA_DEFAULT);

    for(int i = 0; i < grid->w; i++)
    {
        for(int j = 0; j < grid->h; j++)
        {
            vector<unsigned int> paletteList;
            GuiVirtualPoint point = {i, j};

            int val = getCell(point);

            if(clipboardMoveMode && clipboard)
            {
                if(i >= cellUnderMouse.x && j >= cellUnderMouse.y && i < cellUnderMouse.x + clipboard->w && j < cellUnderMouse.y + clipboard->h)
                {
                    int x = i - cellUnderMouse.x;
                    int y = j - cellUnderMouse.y;
                    val = gridgetcell(clipboard, x, y);
                }
            }

            int destX = relativeDestRect.x + (i * cellW);
            int destY = relativeDestRect.y + (j * cellH);

            SDL_Rect src = {0, 0, cellW, cellH};
            SDL_Rect dest = {destX, destY, cellW, cellH};

            if(!paletteValMap.empty())
            {
                fillCellPaletteListFromMappedVal(val, paletteList);

                for(auto m : paletteList)
                {
                    src.x = m * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderCopy(Gui_SDL_Renderer, paletteTex, &src, &dest);
                    }
                }
            }
            else
            {
                val--;
                if(val >= 0)
                {
                    src.x = val * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderCopy(Gui_SDL_Renderer, paletteTex, &src, &dest);
                    }
                }
            }
        }
    }

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

        grid_rect rect = {lesserX, lesserY, (greaterX - lesserX) + 1, (greaterY - lesserY) + 1};

        SDL_Rect selectionRect = {relativeDestRect.x + (rect.x * cellW), relativeDestRect.y + (rect.y * cellH), rect.w * cellW, rect.h * cellH};

        rgba_t v = 0x9090FF9F;

        SDL_SetRenderDrawColor(Gui_SDL_Renderer, rgba_R(v), rgba_G(v), rgba_B(v), rgba_A(v));
        SDL_RenderFillRect(Gui_SDL_Renderer, &selectionRect);
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }

    if(cursorShown && !editInProgress)
    {
        int cursorX = relativeDestRect.x + (cellUnderMouse.x * cellW);
        int cursorY = relativeDestRect.y + (cellUnderMouse.y * cellH);

        SDL_Rect cursorRect = {cursorX, cursorY, cellW, cellH};

        rgba_t v = 0xEFEFEF9F;

        SDL_SetRenderDrawColor(Gui_SDL_Renderer, rgba_R(v), rgba_G(v), rgba_B(v), rgba_A(v));
        SDL_RenderFillRect(Gui_SDL_Renderer, &cursorRect);
        SDL_SetRenderDrawColor(Gui_SDL_Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }
}

void GuiGridCanvas::handleEvent(GuiEvent& event)
{
    switch(event.type)
    {
        case mouse_hovered_onto:
            gridLinesShown = true;
            cursorShown = true;
            break;

        case mouse_hovered_off:
            gridLinesShown = false;
            cursorShown = false;
            editInProgress = false;
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
    cellUnderMouse = xyToCell(x, y);
}

void GuiGridCanvas::mouseClicked(int x, int y, Uint8 button)
{
    if(clipboardMoveMode)
    {
        if(button == SDL_BUTTON_LEFT)
        {
            makeBackup();
            pasteSelection();
            clipboardMoveMode = false;
        }

        return;
    }

    makeBackup();

    if(button == SDL_BUTTON_LEFT)
    {
        if(SDL_GetModState() & KMOD_SHIFT)
        {
            selectionVertex1 = selectionVertex2 = cellUnderMouse;
            selection = true;
        }
        else
        {
            GuiVirtualPoint point = xyToCell(x, y);
            fillCell(point);
            selection = false;
            editInProgress = true;
        }
    }
    else if(button == SDL_BUTTON_RIGHT)
    {
        GuiVirtualPoint point = xyToCell(x, y);
        eraseCell(point);
        selection = false;
        editInProgress = true;
    }
}

void GuiGridCanvas::mouseDragged(int x, int y, Uint8 button)
{
    if(!editInProgress)
    {
        makeBackup();
        editInProgress = true;
    }

    cellUnderMouse = xyToCell(x, y);

    if(button == SDL_BUTTON_LEFT)
    {
        if(SDL_GetModState() & KMOD_SHIFT)
        {
            selectionVertex2 = cellUnderMouse;
        }
        else
        {
            GuiVirtualPoint point = xyToCell(x, y);
            fillCell(point);
        }
    }
    else if(button == SDL_BUTTON_RIGHT)
    {
        GuiVirtualPoint point = xyToCell(x, y);
        eraseCell(point);
    }
}

void GuiGridCanvas::mouseReleased(int x, int y, Uint8 button)
{
    editInProgress = false;
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
                    selection = false;
                }
            }

            break;

        case SDLK_z:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                undo();
            }

            break;

        case SDLK_y:
            if(SDL_GetModState() & KMOD_CTRL)
            {
                redo();
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

            grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX) + 1, (greaterY - lesserY) + 1};

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

    grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX) + 1, (greaterY - lesserY) + 1};
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

    grid_rect selectionRect = {lesserX, lesserY, (greaterX - lesserX) + 1, (greaterY - lesserY) + 1};
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
    if(!clipboard)
    {
        return;
    }

    grid_rect dest = {cellUnderMouse.x, cellUnderMouse.y, clipboard->w, clipboard->h};
    gridcpyrect(clipboard, grid, NULL, &dest);
}

void GuiGridCanvas::eraseCell(GuiVirtualPoint& point)
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

void GuiGridCanvas::fillCell(GuiVirtualPoint& point)
{
    if(paletteValMap.size() <= paletteSelection)
    {
        gridsetcell(grid, point.x, point.y, paletteSelection + 1);
    }
    else
    {
        if(paletteValMap[paletteSelection + 1].isFlag && (getCell(point) != paletteValMap[0].mappedVal))
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

int GuiGridCanvas::getCell(GuiVirtualPoint& point)
{
    return gridgetcell(grid, point.x, point.y);
}

void GuiGridCanvas::fillCellPaletteListFromMappedVal(int mappedVal, std::vector<unsigned int>& paletteList)
{
    paletteList.clear();

    int baseVal = -1;

    for(unsigned int i = 1; i < paletteValMap.size(); i++)
    {
        paletteMapEntry m = paletteValMap[i];
        if(m.isFlag)
        {
            if(mappedVal & m.mappedVal)
            {
                paletteList.push_back(i - 1);
            }
        }
        else
        {
            if(mappedVal & m.mappedVal)
            {
                baseVal = (int)(i);
            }
        }
    }

    if(baseVal >= 0)
    {
        paletteList.insert(paletteList.begin(), baseVal);
    }
}
