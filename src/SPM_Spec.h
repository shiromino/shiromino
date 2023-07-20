#pragma once
#include <cstdint>
#include <vector>
#include "Grid.h"
#include "SPM_Structures.h"
#include "SPM_Randomizer.h"

class SPM_Spec {
public:
    SPM_Spec() :
        randomizer(nullptr),
        fieldW(10),
        fieldH(23),
        visualFieldH(20),
        rotateA(spm_counter_clockwise),
        rotateB(spm_clockwise),
        rotateC(spm_counter_clockwise),
        buttonDAction(spm_do_nothing),
        useLockDelay(false),
        initialRotate(false),
        initialHold(false),
        numPreviews(0u),
        allowHardDrop(false),
        softDropLock(false),
        hardDropLock(false) {}
    
    virtual ~SPM_Spec() { delete randomizer; }

    virtual bool checkCollision(Shiro::Grid *, ActivatedPolyomino&);
    virtual bool checkCollision(Shiro::Grid *, ActivatedPolyomino&, std::pair<int, int>&);
    virtual bool isGrounded(Shiro::Grid *, ActivatedPolyomino&);

    virtual bool checkedShift(Shiro::Grid *, ActivatedPolyomino&, SPM_offset);
    virtual bool checkedRotate(Shiro::Grid *, ActivatedPolyomino&, SPM_orientation);
    virtual int checkedFall(Shiro::Grid *, ActivatedPolyomino&, int);

    virtual void imprintMino(Shiro::Grid *, ActivatedPolyomino&);
    virtual int checkAndClearLines(Shiro::Grid *, int);
    virtual void dropField(Shiro::Grid *);

    SPM_Randomizer *randomizer;

    int fieldW;
    int fieldH;
    int visualFieldH;

    std::vector<SPM_point> spawnPositions;
    std::vector<Polyomino> polyominoes;

    // TODO: no reset vs. step reset vs. move reset, as well as possibility for other options?

    SPM_orientation rotateA;
    SPM_orientation rotateB;
    SPM_orientation rotateC;
    SPM_buttonAction buttonDAction;

    bool useLockDelay;
    bool initialRotate;
    bool initialHold;

    unsigned int numPreviews;

    bool allowHardDrop;
    bool softDropLock;
    bool hardDropLock;
};

class SPM_TestSpec : public SPM_Spec
{
public:
    SPM_TestSpec();
};
