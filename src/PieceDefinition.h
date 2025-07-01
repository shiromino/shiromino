#pragma once
#include "Grid.h"
#include <array>
#include <cstdint>

namespace Shiro {
    enum PieceDefinitionFlag {
        PDNONE,
        PDNOWKICK           = 0x00000001,
        PDNOFKICK           = 0x00000002,
        PDNOCEILKICKS       = 0x00000004,

        PDBRACKETS          = 0x00000008,
        PDFLATFLOORKICKS    = 0x00000010,
        PDONECELLFLOORKICKS = 0x00000020,
        PDPREFERWKICK       = 0x00000040,
        PDAIRBORNEFKICKS    = 0x00000080,
        PDFLIPFLOORKICKS    = 0x00000100
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