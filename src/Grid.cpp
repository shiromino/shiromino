#include "Grid.hpp"

using namespace Shiro;
using namespace std;

Grid::Grid() : width(0u), height(0u) {}

Grid::Grid(const size_t width, const size_t height) :
    width(width),
    height(height),
    cells(vector<vector<int>>(height, vector<int>(width, 0))) {}

Grid::Grid(const Grid& srcGrid, const GridRect& srcRect) {
    int startX = srcRect.x;
    int endX = static_cast<int>(srcRect.x + srcRect.width);
    int startY = srcRect.y;
    int endY = static_cast<int>(srcRect.y + srcRect.height);

    if(startX < 0) {
        startX = 0;
    }
    if(endX > srcGrid.width) {
        endX = static_cast<int>(srcGrid.width);
    }
    if(startY < 0) {
        startY = 0;
    }
    if(endY > srcGrid.height) {
        endY = static_cast<int>(srcGrid.height);
    }
    if (startX >= srcGrid.width || startY < 0) {
        this->width = 0;
        this->height = 0;
        return;
    }

    this->width = (long)endX - startX;
    this->height = (long)endY - startY;
    this->cells = vector<vector<int>>(this->height, vector<int>(this->width));
    for (size_t y = 0; y < this->height; y++) {
        for (size_t x = 0; x < this->width; x++) {
            this->cells[y][x] = srcGrid.cells[startY + y][startX + x];
        }
    }
}

void Grid::setWidth(const size_t width) {
    this->width = width;
    for (auto& row : this->cells) {
        row.resize(this->width, 0);
    }
}

void Grid::setHeight(const size_t height) {
    this->height = height;
    this->cells.resize(height, vector<int>(this->width, 0));
}

void Grid::resize(const size_t width, const size_t height) {
    this->width = width;
    this->height = height;
    for (auto& row : cells) {
        row.resize(this->width, 0);
    }
    cells.resize(this->height, vector<int>(this->width, 0));
}

int& Grid::cell(const int x, const int y) {
    return cells[y][x];
}

int Grid::setCell(const int x, const int y, const int value) {
    if (x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    if (value == GRID_OOB) {
        return 1;
    }

    this->cells[y][x] = value;
    return 0;
}

int Grid::xorCell(const int x, const int y, const int value) {
    if (x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    if (value == GRID_OOB) {
        return 1;
    }

    cells[y][x] ^= value;
    return 0;
}

void Grid::fill(const int value) {
    GridRect defaultRect = { 0, 0, this->width, this->height };
    fill(defaultRect, value);
}

void Grid::fill(const GridRect& rect, const int value) {
    int startX = rect.x;
    int endX = static_cast<int>(rect.x + rect.width);
    int startY = rect.y;
    int endY = static_cast<int>(rect.y + rect.height);

    if(startX < 0)
    {
        startX = 0;
    }
    if(endX > this->width)
    {
        endX = static_cast<int>(this->width);
    }
    if(startY < 0)
    {
        startY = 0;
    }
    if(endY > this->height)
    {
        endY = static_cast<int>(this->height);
    }

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            this->cells[y][x] = value;
        }
    }
}

void Grid::copyRect(const Grid& srcGrid, const GridRect& srcRect, const GridRect& dstRect) {
    GridRect defaultSrcRect(srcRect.width, srcRect.height);
    GridRect defaultDstRect(dstRect.width, dstRect.height);

    int srcStartX = srcRect.x;
    int srcEndX = static_cast<int>(srcRect.x + srcRect.width);
    int srcStartY = srcRect.y;
    int srcEndY = static_cast<int>(srcRect.y + srcRect.height);

    if (srcStartX < 0) {
        srcStartX = 0;
    }
    if (srcEndX > srcGrid.width) {
        srcEndX = static_cast<int>(srcGrid.width);
    }
    if (srcStartY < 0) {
        srcStartY = 0;
    }
    if (srcEndY > srcGrid.height) {
        srcEndY = static_cast<int>(srcGrid.height);
    }
    if (srcStartX >= srcGrid.width || srcStartY < 0) {
        return;
    }

    int dstStartX = dstRect.x;
    int dstEndX = static_cast<int>(dstRect.x + dstRect.width);
    int dstStartY = dstRect.y;
    int dstEndY = static_cast<int>(dstRect.y + dstRect.height);

    if (dstStartX < 0) {
        dstStartX = 0;
    }
    if (dstEndX > this->width) {
        dstEndX = static_cast<int>(this->width);
    }
    if (dstStartY < 0) {
        dstStartY = 0;
    }
    if (dstEndY > this->height) {
        dstEndY = static_cast<int>(this->height);
    }
    if (dstStartX >= this->width || dstStartY < 0) {
        return;
    }

    if ((srcEndX - srcStartX) < (dstEndX - dstStartX)) {
        dstEndX = dstStartX + (srcEndX - srcStartX);
    }
    else if ((dstEndX - dstStartX) < (srcEndX - srcStartX)) {
        srcEndX = srcStartX + (dstEndX - dstStartX);
    }

    if ((srcEndY - srcStartY) < (dstEndY - dstStartY)) {
        dstEndY = dstStartY + (srcEndY - srcStartY);
    }
    else if ((dstEndY - dstStartY) < (srcEndY - srcStartY)) {
        srcEndY = srcStartY + (dstEndY - dstStartY);
    }

    int destX = dstStartX;
    int destY = dstStartY;

    for (size_t y = 0; y < srcRect.height; y++) {
        for (size_t x = 0; x < srcRect.width; x++) {
            this->cells[dstStartY + y][dstStartX + x] = srcGrid.cells[srcStartY + y][srcStartX + x];
        }
    }
}

void Grid::copyRow(const size_t srcRow, const size_t dstRow) {
    if (srcRow != dstRow) {
        copyRow(*this, srcRow, dstRow);
    }
}

void Grid::copyRow(const Grid& srcGrid, const size_t srcRow, const size_t dstRow) {
    for (size_t x = 0; x < this->width; x++) {
        if (x < srcGrid.width) {
            this->cells[dstRow][x] = srcGrid.cells[srcRow][x];
        }
        else {
            this->cells[dstRow][x] = 0;
        }
    }
}

size_t Grid::getWidth() const {
    return this->width;
}

size_t Grid::getHeight() const {
    return this->height;
}

int Grid::getCell(const size_t x, const size_t y) {
    if (x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    return this->cells[y][x];
}

size_t Grid::cellsFilled() {
    size_t numCellsFilled = 0;
    for (auto& row : cells) {
        for (auto cell : row) {
            numCellsFilled += cell != 0;
        }
    }
    return numCellsFilled;
}
