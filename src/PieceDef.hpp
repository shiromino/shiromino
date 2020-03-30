#pragma once

#include "Grid.hpp"
#include <bitset>
#include <array>
#include <cstdint>

namespace Shiro {
    enum PieceDefFlag {
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

    class PieceDef {
    public:
        PieceDef();

        bool setWidth(int width);
        bool setHeight(int height);
        bool setCell(Orientation orientation, int x, int y);
        bool checkFlags(PieceDefFlag flags);

        uint8_t qrsID; // minor cross-contamination (old: int color)
        PieceDefFlag flags;
        int anchorX;
        int anchorY;
        std::array<Grid, 4> rotationTable;
    };
}

#if 0
#define PDNOWKICK           0x00000001
#define PDNOFKICK           0x00000002

#define PDBRACKETS          0x00000004
#define PDFLATFLOORKICKS    0x00000008
#define PDONECELLFLOORKICKS 0x00000010
#define PDPREFERWKICK       0x00000020
#define PDAIRBORNEFKICKS    0x00000040
#define PDFLIPFLOORKICKS    0x00000080

enum { FLAT = 0, CW = 1, FLIP = 2, CCW = 3 };

typedef struct
{
    uint8_t qrs_id; // minor cross-contamination (old: int color)
    unsigned int flags;
    int anchorx;
    int anchory;
    std::array<Shiro::Grid, 4> rotation_tables; // these grids technically don't have to be the same size
} piecedef;

piecedef *piecedef_create();
void piecedef_destroy(piecedef *pd);

piecedef *piecedef_cpy(piecedef *pd);

int pdsetw(piecedef *pd, int w);
int pdseth(piecedef *pd, int h);
int pdsetcell(piecedef *pd, int orientation, int x, int y);
// int pdchkflags(piecedef *pd, unsigned int tflags);
#endif
