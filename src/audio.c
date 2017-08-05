/*
    audio.c - wrapper functions for playing audio
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>

#include "core.h"
#include "audio.h"

void play_track(coreState *cs, Mix_Music *m, int volume)
{
    if(!m)
        return;

    Mix_VolumeMusic((volume * ((cs->mus_volume * cs->master_volume) / 100)) / 128);
    Mix_PlayMusic(m, -1);
}

void play_sfx(Mix_Chunk *c, int volume)
{
    if(!c)
        return;

    // Mix_VolumeChunk(c, lrintf(((float)(nz->settings->sfx_volume) / 100.0) * 128.0));
    Mix_VolumeChunk(c, volume);
    if(Mix_PlayChannel(-1, c, 0) < 0)
        printf("Mix_PlayChannel() error: %s\n", Mix_GetError());
}
