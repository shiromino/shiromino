#pragma once

#include "Grid.h"
#include <bitset>
#include <array>
#include <cstdint>

namespace Shiro {
    enum PieceDefinitionFlag {
        PDNONE,
        PDNOWKICK           = 0x00000001,
        PDNOFKICK           = 0x00000002,

        PDBRACKETS          = 0x00000004,
        PDFLATFLOORKICKS    = 0x00000008,
        PDONECELLFLOORKICKS = 0x00000010,
        PDPREFERWKICK       = 0x00000020,
        PDAIRBORNEFKICKS    = 0x00000040,
        PDFLIPFLOORKICKS    = 0x00000080
    };

    enum Orientation {
        FLAT,
        CW,
        FLIP,
        CCW
    };

    class PieceDefinition {
    public:
        PieceDefinition();
        uint8_t qrsID; // minor cross-contamination (old: int color)
        PieceDefinitionFlag flags;
        int anchorX;
        int anchorY;
        std::array<Grid, 4> rotationTable;
    };
}
