#pragma once
#include "RotationTables.h"
#include "SPM_Structures.h"
#include "SPM_Spec.h"
#include "Grid.h"
#include <cstdint>
#include <vector>

#define ARS_I 0
#define ARS_T 1
#define ARS_J 2
#define ARS_L 3
#define ARS_O 4
#define ARS_S 5
#define ARS_Z 6

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