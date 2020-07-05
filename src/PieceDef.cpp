#include "PieceDef.hpp"

using namespace Shiro;
using namespace std;

PieceDef::PieceDef() :
    qrsID(0),
    flags(PDNONE),
    anchorX(0),
    anchorY(0) {}

bool PieceDef::setWidth(std::size_t width) {
    if (width < 1) {
        return true;
    }

    if (rotationTable[0].getWidth() == width) {
        return false;
    }

    for (auto& rotation : rotationTable) {
        rotation.setWidth(width);
    }
    return false;
}

bool Shiro::PieceDef::setHeight(std::size_t height)
{
    if (height < 1) {
        return true;
    }
    if (rotationTable[0].getHeight() == height) {
        return false;
    }

    for (auto& rotation : rotationTable) {
        rotation.setHeight(height);
    }
    return false;
}

bool PieceDef::setCell(Orientation orientation, int x, int y) {
    if (x < 0 || y < 0 || x >= rotationTable[0].getWidth() || y >= rotationTable[0].getHeight()) {
        return true;
    }

    return (rotationTable[orientation].cell(x, y) ^= 1) != 0;
}

bool PieceDef::checkFlags(PieceDefFlag testFlags) {
    return (flags & testFlags) == testFlags;
}
