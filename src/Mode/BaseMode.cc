/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "BaseMode.h"
#include <cstring>
namespace Shiro::Mode {
    BaseMode::BaseMode(const int fieldWidth, const int fieldHeight) :
        activePiece(PolyominoType::Empty),
        activePos{ -1, -1 },
        fieldWidth(fieldWidth),
        fieldHeight(fieldHeight),
        field(fieldHeight, std::vector<Block>(fieldWidth, Block(PolyominoType::Empty))),
        numLines(0u),
        numPieces(0u) {}

    int BaseMode::eraseRow(const int row) {
        int numBlocks = 0;
        for (Block& block : field[row]) {
            block.type = PolyominoType::Empty;
            numBlocks++;
        }
        return numBlocks;
    }

    bool BaseMode::clearLine(const int row) {
        int numBlocks = field[row].size();
        if (numBlocks == fieldWidth) {
            numLines++;
            return true;
        }
        else {
            return false;
        }
    }

    int BaseMode::naiveCollapse() {
        // This is about as optimized as naive gravity can be; each row that isn't
        // a line clear is copied into its final position, once. -Brandon McGriff
        int numNaiveCollapseLines = 0;
        for (int writeRowNum = 0, copyRowNum = 0; writeRowNum < fieldHeight; writeRowNum++, copyRowNum++) {
            while (clearLine(copyRowNum)) {
                copyRowNum++;
                numNaiveCollapseLines++;
                if (copyRowNum >= fieldHeight) {
                    break;
                }
            }
            if (copyRowNum >= fieldHeight) {
                for (int finalEraseRowNum = writeRowNum; finalEraseRowNum < fieldHeight; finalEraseRowNum++) {
                    eraseRow(finalEraseRowNum);
                }
                break;
            }

            if (copyRowNum > writeRowNum) {
                field[writeRowNum] = field[copyRowNum];
            }
        }

        return numNaiveCollapseLines;
    }
}