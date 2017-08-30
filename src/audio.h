#ifndef _audio_h
#define _audio_h

#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// class Music
struct music
{
    Mix_Music *data;
    int volume;
};

bool music_load(struct music *m, const char *path_without_ext);
void music_play(struct music *m, coreState *cs);
void music_destroy(struct music *m);

// class Sfx
struct sfx
{
    Mix_Chunk *data;
    int volume;
};

bool sfx_load(struct sfx *s, const char *path_without_ext);
void sfx_play(struct sfx *s);
void sfx_destroy(struct sfx *s);

#endif
