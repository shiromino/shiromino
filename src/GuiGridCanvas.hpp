#ifndef _guigridcanvas_hpp
#define _guigridcanvas_hpp

#include <string>
#include <SDL2/SDL.h>
#include "SGUIL/SGUIL.hpp"

#include "grid.h"

struct paletteMapEntry
{
    paletteMapEntry(int mappedVal, bool isFlag) : mappedVal(mappedVal), isFlag(isFlag) {}

    int mappedVal;
    bool isFlag;
};

class GuiGridCanvas : public GuiInteractable
{
public:
    GuiGridCanvas(grid_t *grid, BindableInt& paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect);

    GuiGridCanvas(grid_t *grid, BindableInt& paletteVar, SDL_Texture *paletteTex, std::vector<paletteMapEntry>& paletteValMap,
        unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect)
        : GuiGridCanvas(grid, paletteVar, paletteTex, cellW, cellH, relativeDestRect)
    {
        this->paletteValMap = paletteValMap;
    }

    ~GuiGridCanvas() { grid_destroy(grid); }

    void draw();

    void handleEvent(GuiEvent& event);
    void mouseMoved(int x, int y);
    void mouseClicked(int x, int y);
    void mouseDragged(int x, int y);
    void mouseReleased(int x, int y);
    void keyPressed(SDL_Keycode kc);

    void readPaletteSelection(BindableVariable *var)
    {
        int s = (int)std::stoll(var->get());
        if(s < 0 || s >= paletteSize)
        {
            s = 0;
        }

        paletteSelection = (unsigned int)s;
    }

    GuiVirtualPoint xyToCell(int x, int y)
    {
        x -= relativeDestRect.x;
        y -= relativeDestRect.y;
        x /= cellW;
        y /= cellH;

        return {x, y};
    }

    void makeBackup();
    void undo();
    void redo();
    void clearUndo();

    void copySelection();
    void cutSelection();
    void pasteSelection();

    void erase(GuiVirtualPoint& point);
    void set(GuiVirtualPoint& point);
    int get(GuiVirtualPoint& point);

    //virtual void erase(unsigned int pos) { gridsetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos), 0); }
    //virtual void set(unsigned int pos) { gridsetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos), paletteSelection + 1); }
    //virtual int get(unsigned int pos) { return gridgetcell(grid, gridpostox(grid, pos), gridpostoy(grid, pos)) - 1; }

protected:
    // grid_t *translatedGrid; // the true underlying grid being edited, with all its coded values

    grid_t *grid;
    grid_t *clipboard;
    SDL_Texture *paletteTex;
    unsigned int cellW;
    unsigned int cellH;

    std::vector<grid_t *> undoBuffer;
    std::vector<grid_t *> redoBuffer;

    BindableInt& paletteVar;
    std::vector<paletteMapEntry> paletteValMap;
    unsigned int paletteSelection;
    unsigned int paletteSize;

    GuiVirtualPoint cellUnderMouse;
    bool editInProgress;
    bool selection;
    GuiVirtualPoint selectionVertex1;
    GuiVirtualPoint selectionVertex2;

    bool clipboardMoveMode;
    bool grid_lines_shown;
};

#endif
