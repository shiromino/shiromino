#pragma once
#include "RotationTables.h"
#include "SPM_Structures.h"
#include "SPM_Spec.h"
#include "Grid.h"
#include <cstdint>
#include <vector>

class ARS : public SPM_Spec
{
public:
    ARS(bool isG2);
    ARS() : ARS(false) {}

    bool wallkick(Shiro::Grid *field, ActivatedPolyomino& mino);

    bool checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir) override;
};

class ARS3 : public SPM_Spec
{
public:
    ARS3();

    bool wallkick(Shiro::Grid *field, ActivatedPolyomino& mino);
    bool floorkick(Shiro::Grid *field, ActivatedPolyomino& mino);

    bool checkedRotate(Shiro::Grid *field, ActivatedPolyomino& mino, SPM_orientation dir) override;
};