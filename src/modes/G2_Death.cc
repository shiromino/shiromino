#include "G2_Death.h"
#include "CoreState.h"
#include "gui/GUI.h"
#include "video/Draw.h"
#include "SPM_SType.h"
#include "SPM_Spec.h"
#include "SPM_Structures.h"
#include "SPM_Randomizer.h"
#include "ARS.h"
#include <iostream>
#include <memory>
#include <vector>

static std::vector<std::pair<int, SPM_frameTimings>> G2_Death_speedcurve =
{
    {   {0}, {20 * SPM_SUBUNIT_SCALE, 30, 10, 16, 12, 12} },
    { {101}, {20 * SPM_SUBUNIT_SCALE, 26, 10, 12,  6,  6} },
    { {200}, {20 * SPM_SUBUNIT_SCALE, 26,  9, 12,  6,  6} },
    { {201}, {20 * SPM_SUBUNIT_SCALE, 22,  9, 12,  6,  6} },
    { {300}, {20 * SPM_SUBUNIT_SCALE, 22,  8, 12,  6,  6} },
    { {301}, {20 * SPM_SUBUNIT_SCALE, 18,  8,  6,  6,  6} },
    { {400}, {20 * SPM_SUBUNIT_SCALE, 18,  6,  6,  6,  6} },
    { {401}, {20 * SPM_SUBUNIT_SCALE, 15,  6,  5,  5,  5} },
    { {500}, {20 * SPM_SUBUNIT_SCALE, 15,  6,  4,  4,  4} }
};

G2_Death::G2_Death(CoreState& cs, SPM_Spec *spec) : SPM_SType(cs, spec)
{
    
}

G2_Death::~G2_Death()
{
    
}

int G2_Death::init()
{
    int rc = SPM_SType::init();
    return rc;
}

int G2_Death::input()
{
    int rc = SPM_SType::input();
    return rc;
}

int G2_Death::frame()
{
    int rc = SPM_SType::frame();
    return rc;
}

int G2_Death::draw()
{
    int rc = SPM_SType::draw();
    return rc;
}