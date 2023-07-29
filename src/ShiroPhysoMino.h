#pragma once
#include "Game.h"
#include "replay.h"
#include "gui/GUI.h"
#include "video/Gfx.h"
#include "Timer.h"
#include "SPM_Randomizer.h"

class ShiroPhysoMino : public Game {
public:
    ShiroPhysoMino(CoreState& cs) : Game(cs),
        field(nullptr),
        timer(nullptr),
        rep(nullptr),
        playback(false),
        recording(false),
        playbackIndex(0),
        gamePhase(SPM_gamePhase::spm_paused) {}

    virtual ~ShiroPhysoMino() {
        delete field;
        delete timer;
        delete rep;
    }

protected:
    Shiro::Grid *field;
    Shiro::Gfx::Point fieldPos;

    Shiro::Timer *timer;

    replay *rep;
    bool playback;
    bool recording;
    int playbackIndex;

    SPM_gamePhase gamePhase;
};

struct SPM_Player {
    SPM_Player() :
        randomizer(nullptr),
        mino(nullptr),
        hold(nullptr),
        playPhase(spm_spawn_delay),
        minoSeqIndex(0)
    {
        timings.gravity = 32;
        timings.lockDelay = -1;
        timings.das = 8;
        timings.are = 11;
        timings.lineAre = 6;
        timings.lineClear = 6;
    }

    virtual ~SPM_Player() {
        delete randomizer;
        delete mino;
        for (const auto* mino : previews) {
            delete mino;
        }
        delete hold;
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
