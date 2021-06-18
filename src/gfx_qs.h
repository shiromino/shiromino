#pragma once
#include "Game.h"
#include "CoreState.h"
#include <fstream>

SDL_Texture *gfx_create_credits_tex(CoreState *cs, int num_lines);
int gfx_drawqs(game_t *g);
int gfx_qs_lineclear(game_t *g, int row);
int gfx_drawqsmedals(game_t *g);
int gfx_drawfield_selection(game_t *g, struct pracdata *d);