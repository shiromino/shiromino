#pragma once
#include "Game.h"
#include "ShiroPhysoMino.h"
#include "SPM_SType.h"
#include "replay.h"
#include "SPM_Spec.h"
#include "SPM_Randomizer.h"
#include <memory>
#include <vector>

class G2_Death : public SPM_SType
{
public:
    G2_Death(CoreState&, SPM_Spec *);
    ~G2_Death();

    int init() override;
    int input() override;
    int frame() override;
    int draw() override;
};