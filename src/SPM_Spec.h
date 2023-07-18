#pragma once
#include <cstdint>
#include <vector>
#include "Grid.h"
#include "SPM_Structures.h"
#include "SPM_Randomizer.h"

class SPM_Spec {
public:
    SPM_Spec() :
        fieldW(0),
        fieldH(0),
        visualFieldH(0),
        numPreviews(0u),
        allowHold(false),
        allowHardDrop(false),
        softDropLock(false),
        hardDropLock(false) {}
    virtual ~SPM_Spec() {}

    virtual bool checkCollision(Shiro::Grid* field, ActivatedPolyomino& mino);
    virtual bool checkCollision(Shiro::Grid* field, ActivatedPolyomino& mino, std::pair<int, int>& pos);
    virtual bool isGrounded(Shiro::Grid *field, ActivatedPolyomino& mino);

    virtual bool checkedShift(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_offset offset);
    virtual bool checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir);
    virtual int checkedFall(Shiro::Grid *field, ActivatedPolyomino& mino, int subY);

    virtual void imprintMino(Shiro::Grid *field, ActivatedPolyomino& mino);
    virtual int checkAndClearLines(Shiro::Grid *field, int bound);
    virtual void dropField(Shiro::Grid *field);

    SPM_Randomizer *randomizer;

    int fieldW;
    int fieldH;
    int visualFieldH;

    std::vector<SPM_point> spawnPositions;
    std::vector<Polyomino> polyominoes;

    // TODO: no reset vs. step reset vs. move reset, as well as possibility for other options?

    unsigned int numPreviews;
    bool allowHold;
    bool allowHardDrop;

    bool softDropLock;
    bool hardDropLock;
};

class SPM_TestSpec : public SPM_Spec {
public:
    SPM_TestSpec() {
        for (int i = 0; i < 7; i++) {
            polyominoes.push_back(Shiro::TetroRotationTables[i]);

            spawnPositions.push_back( {3, 2} );
        }

        fieldW = 10;
        fieldH = 23;
        visualFieldH = 20;

        numPreviews = 1;
        allowHold = false;
        allowHardDrop = true;

        softDropLock = true;
        hardDropLock = false;
    }
};
