#ifndef _shirophysomino_hpp
#define _shirophysomino_hpp

#include "SPM_spec.hpp"

class ShiroPhysoMino : public Game
{
public:
    ShiroPhysoMino();
    virtual ~ShiroPhysoMino();

    int init();
    int quit();
    int input();
    int draw();

protected:
    SPM_FrameTimings&& timingsNow;
    SPM_FrameTimings&& timingsNext;
};

#endif // _shirophysomino_hpp
