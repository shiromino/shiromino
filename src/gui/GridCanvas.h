#pragma once
#include "SDL_keycode.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include <iostream>
#include <string>
#include <vector>
#include "Grid.h"
#include "gui/GUI.h"

struct paletteMapEntry
{
    paletteMapEntry(int mappedVal, bool isFlag) : mappedVal(mappedVal), isFlag(isFlag) {}

    int mappedVal;
    bool isFlag;
};

class GridCanvas : public GUIInteractable
{
public:
    GridCanvas(int ID, Shiro::Grid *cells, BindableInt& paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect);

    GridCanvas(int ID, Shiro::Grid *cells, BindableInt& paletteVar, SDL_Texture *paletteTex, std::vector<paletteMapEntry>& paletteValMap,
        unsigned int cellW, unsigned int cellH, SDL_Rect relativeDestRect)
        : GridCanvas(ID, cells, paletteVar, paletteTex, cellW, cellH, relativeDestRect)
    {
        this->paletteValMap = paletteValMap;
    }

    ~GridCanvas()
    {
        delete cells;
        cells = nullptr;

        for (const auto* g : undoBuffer) {
            delete g;
        }
        undoBuffer.clear();

        for (const auto* g : redoBuffer) {
            delete g;
        }
        redoBuffer.clear();

        delete clipboard;
        clipboard = nullptr;
    }

    void draw();

    void handleEvent(GUIEvent& event);
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

    GUIVirtualPoint xyToCell(int x, int y)
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

    void eraseCell(GUIVirtualPoint& point);
    void fillCell(GUIVirtualPoint& point);
    int getCell(GUIVirtualPoint& point);

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

    GUIVirtualPoint cellUnderMouse;
    bool editInProgress;
    bool selection;
    GUIVirtualPoint selectionVertex1;
    GUIVirtualPoint selectionVertex2;

    bool clipboardMoveMode;
    bool gridLinesShown;
    bool cursorShown;
};

static inline void GridCanvas_callPaletteObserver(GridCanvas& obj, BindableVariable *bv)
{
    obj.readPaletteSelection(bv);
}