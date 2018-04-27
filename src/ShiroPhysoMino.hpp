#ifndef _shirophysomino_hpp
#define _shirophysomino_hpp

#include <memory>
#include <vector>
#include "SGUIL/SGUIL.hpp"

#include "core.h"
#include "replay.h"
#include "SPM_Spec.hpp"

class ShiroPhysoMino : public Game
{
public:
    ShiroPhysoMino(coreState& cs, SPM_Spec *spec);
    ~ShiroPhysoMino();

    int init();
    int quit();
    int input();
    int frame();
    int draw();

protected:
    SPM_Spec *spec;

    grid_t *field;
    GuiPoint fieldPos;

    nz_timer *timer;

    replay *rep;
    bool playback;
    int playbackIndex;

    SPM_gamePhase gamePhase;

private:
    bool spawnDelayExpired(SPM_Player&);
    bool lockDelayExpired(SPM_Player&);
    bool lineClearExpired(SPM_Player&);
    bool initNextMino(SPM_Player&);

    SPM_Player player;
};

#endif // _shirophysomino_hpp
