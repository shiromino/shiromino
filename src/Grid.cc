#include "Grid.h"

Shiro::Grid::Grid() : width(0), height(0) {}

Shiro::Grid::Grid(std::size_t width, std::size_t height) :
    width(width),
    height(height),
    cells(std::vector<std::vector<int>>(height, std::vector<int>(width, 0))) {}

Shiro::Grid::Grid(const Grid& srcGrid, const GridRect& srcRect) {
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
    this->cells = std::vector<std::vector<int>>(this->height, std::vector<int>(this->width));
    for (std::size_t y = 0; y < this->height; y++) {
        for (std::size_t x = 0; x < this->width; x++) {
            this->cells[y][x] = srcGrid.cells[startY + y][startX + x];
        }
    }
}

void Shiro::Grid::setWidth(std::size_t width) {
    this->width = width;
    for (auto& row : this->cells) {
        row.resize(this->width, 0);
    }
}

void Shiro::Grid::setHeight(std::size_t height) {
    this->height = height;
    this->cells.resize(height, std::vector<int>(this->width, 0));
}

void Shiro::Grid::resize(std::size_t width, std::size_t height) {
    this->width = width;
    this->height = height;
    for (auto& row : cells) {
        row.resize(this->width, 0);
    }
    cells.resize(this->height, std::vector<int>(this->width, 0));
}

int& Shiro::Grid::cell(int x, int y) {
    return cells[y][x];
}

int Shiro::Grid::setCell(int x, int y, int value) {
    if (x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    if (value == GRID_OOB) {
        return 1;
    }

    this->cells[y][x] = value;
    return 0;
}

int Shiro::Grid::xorCell(int x, int y, int value) {
    if (x < 0 || y < 0 || x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    if (value == GRID_OOB) {
        return 1;
    }

    cells[y][x] ^= value;
    return 0;
}

void Shiro::Grid::fill(int value) {
    GridRect defaultRect = { 0, 0, this->width, this->height };
    fill(defaultRect, value);
}

void Shiro::Grid::fill(const GridRect& rect, int value) {
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

void Shiro::Grid::copyRect(const Grid& srcGrid, const GridRect& srcRect, const GridRect& dstRect) {
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

    for (std::size_t y = 0; y < srcRect.height; y++) {
        for (std::size_t x = 0; x < srcRect.width; x++) {
            this->cells[dstStartY + y][dstStartX + x] = srcGrid.cells[srcStartY + y][srcStartX + x];
        }
    }
}

void Shiro::Grid::copyRow(std::size_t srcRow, std::size_t dstRow) {
    if (srcRow != dstRow) {
        copyRow(*this, srcRow, dstRow);
    }
}

void Shiro::Grid::copyRow(const Grid& srcGrid, std::size_t srcRow, std::size_t dstRow) {
    for (std::size_t x = 0; x < this->width; x++) {
        if (x < srcGrid.width) {
            this->cells[dstRow][x] = srcGrid.cells[srcRow][x];
        }
        else {
            this->cells[dstRow][x] = 0;
        }
    }
}

std::size_t Shiro::Grid::getWidth() const {
    return this->width;
}

std::size_t Shiro::Grid::getHeight() const {
    return this->height;
}

int Shiro::Grid::getCell(std::size_t x, std::size_t y) const {
    if (x >= this->width || y >= this->height) {
        return GRID_OOB;
    }
    return this->cells[y][x];
}

std::size_t Shiro::Grid::cellsFilled() const {
    std::size_t numCellsFilled = 0;
    for (auto&& row : cells) {
        for (auto&& cell : row) {
            numCellsFilled += cell != 0;
        }
    }
    return numCellsFilled;
}
