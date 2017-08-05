#ifndef gfx_qs_h_
#define gfx_qs_h_

#include "core.h"

int gfx_drawqs(game_t *g);
int gfx_qs_lineclear(game_t *g, int row);
int gfx_drawqsmedals(game_t *g);
int gfx_drawfield_selection(game_t *g, struct pracdata *d);

#endif
