#ifndef _audio_h
#define _audio_h

#include <SDL2/SDL_mixer.h>
#include "core.h"

void play_track(coreState *cs, Mix_Music *m, int volume);
void play_sfx(Mix_Chunk *c, int volume);

#endif
