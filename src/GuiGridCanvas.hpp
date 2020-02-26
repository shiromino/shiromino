#pragma once

#include <iostream>
#include <string>
#include "SDL.h"
#include "SGUIL/SGUIL.hpp"

#include "Grid.hpp"

struct paletteMapEntry
{
    paletteMapEntry(int mappedVal, bool isFlag) : mappedVal(mappedVal), isFlag(isFlag) {}

    int mappedVal;
    bool isFlag;
};

class GuiGridCanvas : public GuiInteractable
{
public:
    GuiGridCanvas(int ID, Shiro::Grid *cells, BindableInt& paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect);

    GuiGridCanvas(int ID, Shiro::Grid *cells, BindableInt& paletteVar, SDL_Texture *paletteTex, std::vector<paletteMapEntry>& paletteValMap,
        unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect)
        : GuiGridCanvas(ID, cells, paletteVar, paletteTex, cellW, cellH, relativeDestRect)
    {
        this->paletteValMap = paletteValMap;
    }

    ~GuiGridCanvas()
    {
        for(auto g : undoBuffer)
        {
            delete g;
        }

        for(auto g : redoBuffer)
        {
            delete g;
        }

        if(clipboard)
        {
            delete clipboard;
        }
    }

    void draw();

    void handleEvent(GuiEvent& event);
    void mouseMoved(int x, int y);
    void mouseClicked(int x, int y, Uint8 button);
    void mouseDragged(int x, int y, Uint8 button);
    void mouseReleased(int x, int y, Uint8 button);
    void keyPressed(SDL_Keycode kc);

    void readPaletteSelection(BindableVariable *var)
    {
        int s = (int)std::stoll(var->get());
        if(s < 0 || (unsigned)s >= paletteSize)
        {
            s = 0;
        }

        paletteSelection = (unsigned int)s;
        std::cout << "Updated palette selection to " << s << std::endl;
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

    void eraseCell(GuiVirtualPoint& point);
    void fillCell(GuiVirtualPoint& point);
    int getCell(GuiVirtualPoint& point);

    void fillCellPaletteListFromMappedVal(int mappedVal, std::vector<unsigned int>& paletteList);

    //virtual void erase(unsigned int pos) { gridsetcell(cells, gridpostox(cells, pos), gridpostoy(cells, pos), 0); }
    //virtual void set(unsigned int pos) { gridsetcell(cells, gridpostox(cells, pos), gridpostoy(cells, pos), paletteSelection + 1); }
    //virtual int get(unsigned int pos) { return gridgetcell(cells, gridpostox(cells, pos), gridpostoy(cells, pos)) - 1; }

protected:
    // Shiro::Grid *translatedGrid; // the true underlying cells being edited, with all its coded values

    Shiro::Grid *cells;
    Shiro::Grid *clipboard;
    SDL_Texture *paletteTex;
    unsigned int cellW;
    unsigned int cellH;

    std::vector<Shiro::Grid *> undoBuffer;
    std::vector<Shiro::Grid *> redoBuffer;

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
    bool gridLinesShown;
    bool cursorShown;
};

static inline void GuiGridCanvas_callPaletteObserver(GuiGridCanvas& obj, BindableVariable *bv)
{
    obj.readPaletteSelection(bv);
}