/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once

#include <vector>
#include <utility>
#include <cstdint>

namespace Shiro {
    namespace Mode {
        /**
         * The constants for each of the polyomino types are arranged so that,
         * when they're cast to an integral type, they'll be in sequence from 1
         * on up, so they can be used for array indices.
         */
        enum class PolyominoType {
            Empty,

            // Pentominoes
            I = Empty + 1,
            J,
            L,
            X,
            S,
            Z,
            N,
            G,
            U,
            T,
            Fa,
            Fb,
            P,
            Q,
            W,
            Ya,
            Yb,
            V,
            NumPentominoTypes = V - Empty,

            // Tetrominoes
            I4 = V + 1,
            T4,
            J4,
            L4,
            O,
            S4,
            Z4,
            NumTetrominoTypes = Z4 - NumPentominoTypes
        };

        struct Block {
            Block() = delete;
            Block(const PolyominoType type) : type(type) {}
            PolyominoType type;
        };

        /**
         * Base class of all game modes.
         */
        class BaseMode {
        public:
            BaseMode() = delete;
            BaseMode(const int fieldWidth, const int fieldHeight);

        private:
            int eraseRow(const int row);
            bool clearLine(const int row);
            int naiveCollapse();

            std::vector<PolyominoType> nextPieces;
            PolyominoType activePiece;
            int activePos[2];
            const int fieldWidth;
            const int fieldHeight;
            std::vector<std::vector<Block>> field;
            unsigned numLines;
            unsigned numPieces;
        };
    }
}