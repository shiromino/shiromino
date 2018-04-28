#ifndef _spm_player_hpp
#define _spm_player_hpp

#include <vector>

#include "SPM_Spec.hpp"
#include "SPM_Randomizer.hpp"

struct SPM_Player
{
    SPM_Player()
    {
        randomizer = new SPM_NonRandomizer{7};
        mino = NULL;
        hold = NULL;
        playPhase = spm_player_control;
        timings.gravity = 20 * SPM_SUBUNIT_SCALE;
        timings.lockDelay = 30;
        timings.das = 12;
        timings.dasInterval = 0;
        timings.are = 25;
        timings.lineAre = 25;
        timings.lineClear = 40;
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

#endif // _spm_player_hpp
