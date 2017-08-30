/*
    audio.c - wrapper functions for playing audio
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>

#include "core.h"
#include "audio.h"

static void play_track(coreState *cs, Mix_Music *m, int volume)
{
    if(!m)
        return;

    Mix_VolumeMusic((volume * ((cs->mus_volume * cs->master_volume) / 100)) / MIX_MAX_VOLUME);
    Mix_PlayMusic(m, -1);
}

static void play_sfx(Mix_Chunk *c, int volume)
{
    if(!c)
        return;

    // Mix_VolumeChunk(c, lrintf(((float)(nz->settings->sfx_volume) / 100.0) * 128.0));
    Mix_VolumeChunk(c, volume);
    if(Mix_PlayChannel(-1, c, 0) < 0)
        printf("Mix_PlayChannel() error: %s\n", Mix_GetError());
}

bool music_load(struct music *m, const char *path_without_ext)
{
    m->data = NULL;
    m->volume = MIX_MAX_VOLUME;

    bstring path = bfromcstr(path_without_ext);
    bcatcstr(path, ".ogg");
    m->data = Mix_LoadMUS((const char *)(path->data));
    bdestroy(path);
    if(m->data)
        return true;

    path = bfromcstr(path_without_ext);
    bcatcstr(path, ".wav");
    m->data = Mix_LoadMUS((const char *)(path->data));
    bdestroy(path);

    return m->data != NULL;
}

void music_play(struct music *m, coreState *cs)
{
    play_track(cs, m->data, m->volume);
}

void music_destroy(struct music *m)
{
    if(m->data)
        Mix_FreeMusic(m->data);
}

bool sfx_load(struct sfx *s, const char *path_without_ext)
{
    s->data = NULL;
    s->volume = MIX_MAX_VOLUME;

    bstring path = bfromcstr(path_without_ext);
    bcatcstr(path, ".wav");
    s->data = Mix_LoadWAV((const char *)(path->data));
    bdestroy(path);

    return s->data != NULL;
}

void sfx_play(struct sfx *s)
{
    play_sfx(s->data, s->volume);
}

void sfx_destroy(struct sfx *s)
{
    if(s->data)
        Mix_FreeChunk(s->data);
}
