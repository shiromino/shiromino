#include "Game.h"

bool game::update(bool inputEnabled) {
    if (preframe && preframe(this)) {
        return false;
    }

    if (input && inputEnabled && input(this)) {
        return false;
    }

    //Uint64 benchmark = SDL_GetPerformanceCounter();

    if (frame && frame(this)) {
        return false;
    }

    //benchmark = SDL_GetPerformanceCounter() - benchmark;
    //std::cerr << (double) (benchmark) * 1000 / (double) SDL_GetPerformanceFrequency() << " ms" << std::endl;

    return true;
}
