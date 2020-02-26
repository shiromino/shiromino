#pragma once

#include <vector>
#include <array>

#define GRID_OOB 8128

namespace Shiro {
    struct GridRect {
        GridRect() : x(0), y(0), width(0), height(0) {}
        GridRect(const std::size_t width, const std::size_t height) : x(0), y(0), width(width), height(height) {}
        GridRect(const int x, const int y, const std::size_t width, const std::size_t height) : x(x), y(y), width(width), height(height) {}

        int x;
        int y;
        std::size_t width;
        std::size_t height;
    };

    class Grid {
    public:
        Grid();
        Grid(const std::size_t width, const std::size_t height);
        Grid(const Grid& srcGrid, const GridRect& srcRect);

        template<const std::size_t polyominoWidth, const std::size_t polyominoHeight>
        Grid(const std::array<std::array<bool, polyominoWidth>, polyominoHeight>& polyomino) : width(polyominoWidth), height(polyominoHeight) {
            cells.resize(polyominoHeight);
            for (std::size_t y = 0; y < polyominoHeight; y++) {
                cells[y].resize(polyominoWidth);
                for (std::size_t x = 0; x < polyominoWidth; x++) {
                    cells[y][x] = polyomino[y][x];
                }
            }
        }

        void setWidth(const std::size_t width);
        void setHeight(const std::size_t height);
        void resize(const std::size_t width, const std::size_t height);
        int& cell(const int x, const int y);
        int setCell(const int x, const int y, const int value);
        int xorCell(const int x, const int y, const int value);

        void fill(const int value);
        void fill(const GridRect& rect, const int value);

        void copyRect(const Grid& srcGrid, const GridRect& srcRect, const GridRect& dstRect);
        void copyRow(const std::size_t srcRow, const std::size_t dstRow);
        void copyRow(const Grid& srcGrid, const std::size_t srcRow, const std::size_t dstRow);

        std::size_t getWidth() const;
        std::size_t getHeight() const;

        int getCell(const std::size_t x, const std::size_t y);

        std::size_t cellsFilled();

    private:
        std::size_t width;
        std::size_t height;
        std::vector<std::vector<int>> cells; // cells[row #][column #]
    };
}
