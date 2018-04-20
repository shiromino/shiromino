#ifndef _guigridcanvas_hpp
#define _guigridcanvas_hpp

#include <string>
#include <SDL2/SDL.h>
#include "SGUIL/SGUIL.hpp"

#include "grid.h"

class GuiGridCanvas : public GuiInteractable
// grid of cells which you can edit the values of
// clicking sets the cell to the current palette selection
//
{
public:
    GuiGridCanvas(grid_t *grid, unsigned int cellW, unsigned int cellH, unsigned int paletteSize)
        : grid(grid), cellW(cellW), cellH(cellH), paletteSize(paletteSize), paletteSelection(0) {}

    ~GuiGridCanvas() { grid_destroy(grid); }

    void setPaletteSelection(BindableVariable *var)
    {
        int s = (int)std::stoll(var->get());
        if(s < 0 || s >= paletteSize)
        {
            s = 0;
        }

        paletteSelection = (unsigned int)s;
    }

    unsigned int xyToCell(int x, int y)
    {
        x -= relativeDestRect.x;
        y -= relativeDestRect.y;
        x /= cellW;
        y /= cellH;

        return gridxytopos(grid, x, y);
    }

    virtual void erase(int x, int y) { gridsetcell(grid, x, y, 0); }
    virtual void set(int x, int y) { gridsetcell(grid, x, y, paletteSelection + 1); }
    virtual int get(int x, int y) { return gridgetcell(grid, x, y) - 1; }

    virtual void erase(unsigned int pos) { gridsetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos), 0); }
    virtual void set(unsigned int pos) { gridsetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos), paletteSelection + 1); }
    virtual int get(unsigned int pos) { return gridgetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos)) - 1; }

protected:
    grid_t *grid;
    unsigned int cellW;
    unsigned int cellH;

    unsigned int paletteSize;
    unsigned int paletteSelection;

    bool editInProgress;
    bool selection;
    GuiVirtualPoint selectionVertex1;
    GuiVirtualPoint selectionVertex2;

    bool grid_lines_shown;
};

/*
class GuiCanvasMultiEditor : public GuiGridCanvas
{
public:
    GuiCanvasMultiEditor();
    ~GuiCanvasMultiEditor();

    void draw();

    void mouseClicked(int, int);
    void mouseDragged(int, int);
    void mouseReleased(int, int);
    void keyPressed(SDL_Keycode);

    void handleEvent(GuiEvent&);
};*/

#endif
