#ifndef _shirophysomino_hpp
#define _shirophysomino_hpp

#include <memory>
#include <vector>
#include "SGUIL/SGUIL.hpp"

#include "core.h"
#include "replay.h"
#include "SPM_Spec.hpp"
#include "SPM_Randomizer.hpp"

class ShiroPhysoMino : public Game
{
public:
    ShiroPhysoMino(coreState& cs) : Game(cs) {}
    virtual ~ShiroPhysoMino();

protected:
    SPM_Spec *spec;

    grid_t *field;
    GuiPoint fieldPos;

    nz_timer *timer;

    replay *rep;
    bool playback;
    int playbackIndex;

    SPM_gamePhase gamePhase;
};

struct SPM_Player
{
    SPM_Player()
    {
        randomizer = new G3_Randomizer;
        mino = NULL;
        hold = NULL;
        playPhase = spm_player_control;
        timings.gravity = 20 * 256 * 256;
        timings.lockDelay = -1;
        timings.das = 8;
        timings.dasInterval = 0;
        timings.are = 11;
        timings.lineAre = 6;
        timings.lineClear = 6;
        minoSeqIndex = 0;
    }

    SPM_Randomizer *randomizer;

    ActivatedPolyomino *mino;
    std::vector<ActivatedPolyomino *> previews;
    ActivatedPolyomino *hold;

    SPM_playPhase playPhase;

    SPM_frameTimings timings;
    SPM_frameCounters counters;

    std::vector<SPM_minoID> minoSequence;
    unsigned int minoSeqIndex;
};

class TestSPM : public ShiroPhysoMino
{
public:
    TestSPM(coreState& cs, SPM_Spec *spec)
        : ShiroPhysoMino(cs)
    {
        this->spec = spec;

        player.randomizer = new G3_Randomizer;

        field = grid_create(spec->fieldW, spec->fieldH);
        fieldPos = {48, 60};
        timer = nz_timer_create(60);
        rep = NULL;
        playback = false;
        playbackIndex = 0;
        gamePhase = spm_gameplay;
    }

    int init();
    int quit();
    int input();
    int frame();
    int draw();

private:
    bool spawnDelayExpired(SPM_Player&);
    bool lockDelayExpired(SPM_Player&);
    bool lineClearExpired(SPM_Player&);
    bool initNextMino(SPM_Player&);
    ActivatedPolyomino *activateMino(SPM_minoID ID);

    SPM_Player player;
};

#endif // _shirophysomino_hpp
