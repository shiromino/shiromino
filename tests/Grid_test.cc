#include "Grid.h"
#include "QRS0.h"
#include "catch2/catch_test_macros.hpp"
#include <array>
#include <iostream>

SCENARIO("Grid operations")
{
    GIVEN("An empty grid")
    {
        auto grid = Shiro::Grid{QRS_FIELD_W, QRS_FIELD_H};

        REQUIRE(grid.getWidth() == QRS_FIELD_W);
        REQUIRE(grid.getHeight() == QRS_FIELD_H);
        REQUIRE(grid.cellsFilled() == 0);

        WHEN("when setting a cell")
        {
            const int cellValue = 5;
            grid.setCell(1, 1, cellValue);

            THEN("the cell should be filled")
            {
                REQUIRE(grid.getCell(1, 1) == cellValue);
                REQUIRE(grid.cellsFilled() == 1);
            }
        }
    }

    GIVEN("A grid with some cells filled")
    {
        auto grid = Shiro::Grid{QRS_FIELD_W, QRS_FIELD_H};
        grid.setCell(0, 0, 1);
        grid.setCell(1, 1, 2);
        grid.setCell(2, 2, 3);
        REQUIRE(grid.cellsFilled() == 3);

        WHEN("filling the entire grid")
        {
            grid.fill(4);

            THEN("all cells should be filled with the new value")
            {
                for(std::size_t y = 0; y < grid.getHeight(); ++y)
                {
                    for(std::size_t x = 0; x < grid.getWidth(); ++x)
                    {
                        REQUIRE(grid.getCell(x, y) == 4);
                    }
                }
                REQUIRE(grid.cellsFilled() == QRS_FIELD_W * QRS_FIELD_H);
            }
        }

        WHEN("filling a specific rectangle")
        {
            Shiro::GridRect rect{1, 1, 2, 2};
            const int rectValue = 5;
            grid.fill(rect, rectValue);

            THEN("the specified rectangle should be filled with the new value")
            {
                REQUIRE(grid.cellsFilled() == 5); // 1 original (2 overwritten) + 4 new cells filled
                for(std::size_t y = 0; y < grid.getHeight(); ++y)
                {
                    for(std::size_t x = 0; x < grid.getWidth(); ++x)
                    {
                        const bool cellInRect = (x >= static_cast<std::size_t>(rect.x) && x < static_cast<std::size_t>(rect.x + rect.width) &&
                                                 y >= static_cast<std::size_t>(rect.y) && y < static_cast<std::size_t>(rect.y + rect.height));
                        if(cellInRect)
                        {
                            REQUIRE(grid.getCell(x, y) == rectValue);
                        }
                        else
                        {
                            REQUIRE(grid.getCell(x, y) != rectValue);
                        }
                    }
                }
            }
        }
    }
}