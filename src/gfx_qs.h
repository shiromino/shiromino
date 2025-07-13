#pragma once
#include "SDL_render.h"
#include "Game.h"

struct CoreState;

SDL_Texture *gfx_create_credits_tex(CoreState *cs, int num_lines);
int gfx_drawqs(game_t *g);
int gfx_qs_lineclear(game_t *g, int row);
int gfx_drawqsmedals(game_t *g);
int gfx_drawfield_selection(game_t *g, struct pracdata *d);