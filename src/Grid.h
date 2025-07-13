#pragma once
#include <array>
#include <cstddef>
#include <tuple>
#include <vector>

#define GRID_OOB 8128

namespace Shiro {
    struct GridRect {
        const int x;
        const int y;
        const std::size_t width;
        const std::size_t height;
    };

    class Grid {
    public:
        Grid();
        Grid(std::size_t width, std::size_t height);
        Grid(const Grid& srcGrid, const GridRect& srcRect);

        template<std::size_t polyominoWidth, std::size_t polyominoHeight>
        Grid(const std::array<std::array<bool, polyominoWidth>, polyominoHeight>& polyomino) : width(polyominoWidth), height(polyominoHeight) {
            cells.resize(polyominoHeight);
            for (auto y = 0u; y < polyominoHeight; y++) {
                cells[y].resize(polyominoWidth);
                for (auto x = 0u; x < polyominoWidth; x++) {
                    cells[y][x] = polyomino[y][x];
                }
            }
        }

        void setWidth(std::size_t width);
        void setHeight(std::size_t height);
        void resize(std::size_t width, std::size_t height);
        int& cell(int x, int y);
        int setCell(int x, int y, int value);
        int xorCell(int x, int y, int value);

        void fill(int value);
        void fill(const GridRect& rect, int value);

        void copyRect(const Grid& srcGrid, const GridRect& srcRect, const GridRect& dstRect);
        void copyRow(std::size_t srcRow, std::size_t dstRow);
        void copyRow(const Grid& srcGrid, std::size_t srcRow, std::size_t dstRow);

        std::size_t getWidth() const;
        std::size_t getHeight() const;

        int getCell(std::size_t x, std::size_t y) const;

        std::size_t cellsFilled() const;

    private:
        std::size_t width;
        std::size_t height;
        std::vector<std::vector<int>> cells; // cells[row #][column #]
    };
}
