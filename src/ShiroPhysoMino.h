#pragma once
#include "Game.h"
#include "replay.h"
#include "gui/GUI.h"
#include "Timer.h"
#include "SPM_Spec.h"
#include "SPM_Randomizer.h"
#include <memory>
#include <vector>

class ShiroPhysoMino : public Game {
public:
    ShiroPhysoMino(CoreState& cs) : Game(cs),
        spec(nullptr),
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
    SPM_Spec *spec;

    Shiro::Grid *field;
    GUIPoint fieldPos;

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
        timings.gravity = 20 * 256 * 256;
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

/*
class TestSPM : public ShiroPhysoMino {
public:
    TestSPM(CoreState& cs, SPM_Spec *spec)
        : ShiroPhysoMino(cs) {
        this->spec = spec;

        field = new Shiro::Grid(spec->fieldW, spec->fieldH);
        fieldPos = {48, 60};
        timer = new Shiro::Timer(60.0);
        rep = NULL;
        playback = false;
        playbackIndex = 0;
        gamePhase = spm_gameplay;
    }

    ~TestSPM();

    int init();
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
*/