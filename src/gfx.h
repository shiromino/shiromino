#pragma once

#include <cstdlib>
#include "SDL.h"
#include <string>

#include "core.h"
#include "Grid.hpp"
#include "Timer.hpp"
#include "PieceDef.hpp"
#include "gfx_structures.h"

#define QRS_FIELD_X 4
#define QRS_FIELD_Y 50

#define FIELD_EDITOR_PALETTE_X (16*14 + 4 + QRS_FIELD_X)
#define FIELD_EDITOR_PALETTE_Y 96

#define GFX_G2 0x10000000
#define GFX_TARGET_TEXTURE_OVERWRITE 0x80000000

#define DRAWPIECE_LOCKFLASH     0x0001
#define DRAWPIECE_PREVIEW       0x0002
#define DRAWPIECE_IPREVIEW      0x0004
#define DRAWPIECE_SMALL         0x0008
#define DRAWPIECE_BRACKETS      0x0010
#define DRAWPIECE_BIG           0x0020
#define DRAWPIECE_JEWELED       0x0040

#define DRAWFIELD_BRACKETS      0x0001
#define DRAWFIELD_INVISIBLE     0x0002
#define DRAWFIELD_NO_OUTLINE    0x0004
#define DRAWFIELD_GRID          0x0008
#define DRAWFIELD_CURSES        0x0010
#define DRAWFIELD_BIG           0x0020
#define DRAWFIELD_JEWELED       0x0040
#define TEN_W_TETRION           0x8000
#define TETRION_DEATH       0x00010000

//#define DRAWTEXT_LINEFEED       0x0001
#define DRAWTEXT_CENTERED       0x0002
#define DRAWTEXT_NO_OUTLINE     0x0004
#define DRAWTEXT_SHADOW         0x0008
#define DRAWTEXT_THIN_FONT      0x0010
#define DRAWTEXT_ALIGN_RIGHT    0x0020
#define DRAWTEXT_VALUE_BAR      0x0040
//#define DRAWTEXT_RAINBOW        0x0080
//#define DRAWTEXT_NEGATIVE_OUTLINE   0x0100
#define DRAWTEXT_TINY_FONT      0x0200
#define DRAWTEXT_SMALL_FONT     0x0400
#define DRAWTEXT_FIXEDSYS_FONT  0x0800

#define RGBA_DEFAULT 0xFFFFFFFF
#define RGBA_OUTLINE_DEFAULT 0x000000FF
#define R(N) ((N & 0xFF000000) / 0x1000000)
#define G(N) ((N & 0x00FF0000) / 0x0010000)
#define B(N) ((N & 0x0000FF00) / 0x0000100)
#define A(N) (N & 0x000000FF)

#define RGBA_NEGATIVE(N) (~((N) & 0xFFFFFF00))

#define RAINBOW(Z, A, B) (127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((Z - B) % A) / (double)(A)) )))

#define BG_FADE_RATE 25

extern png_monofont *monofont_tiny;
extern png_monofont *monofont_small;
extern png_monofont *monofont_thin;
extern png_monofont *monofont_square;
extern png_monofont *monofont_fixedsys;

struct text_formatting *text_fmt_create(unsigned int flags, Uint32 rgba, Uint32 outline_rgba);

int gfx_init(CoreState *cs);
void gfx_quit(CoreState *cs);

int gfx_start_bg_fade_in(CoreState *cs, SDL_Texture* bg_new);
void gfx_updatebg(CoreState* cs);
void gfx_drawbg(CoreState *cs);
int gfx_draw_emergency_bg_darken(CoreState *cs);

// these are a little bit hacky... just add to each RGB value of the pixels
// mostly would use these for animations

//int gfx_brighten_texture(SDL_Texture *tex, Uint8 amt);
// int gfx_darken_texture(SDL_Texture *tex, Uint8 amt);

int gfx_pushmessage(CoreState *cs, const char *text, int x, int y, unsigned int flags, png_monofont *font, struct text_formatting *fmt, unsigned int counter, int (*delete_check)(CoreState *));
int gfx_drawmessages(CoreState *cs, int type);

int gfx_pushanimation(CoreState *cs, gfx_image *first_frame, int x, int y, int num_frames, int frame_multiplier, Uint32 rgba);
int gfx_drawanimations(CoreState *cs, int type);

int gfx_createbutton(CoreState *cs, const char *text, int x, int y, unsigned int flags, int (*action)(CoreState *, void *), int (*delete_check)(CoreState *), void *data, Uint32 rgba);
int gfx_drawbuttons(CoreState *cs, int type);

int gfx_drawqrsfield(CoreState *cs, Shiro::Grid *field, unsigned int mode, unsigned int flags, int x, int y);
int gfx_drawkeys(CoreState *cs, struct keyflags *k, int x, int y, Uint32 rgba);

int gfx_drawtext(CoreState *cs, std::string text, int x, int y, png_monofont *font, struct text_formatting *fmt);
int gfx_drawtext_partial(CoreState *cs, std::string text, int pos, int len, int x, int y, png_monofont *font, struct text_formatting *fmt);
int gfx_drawpiece(CoreState *cs, Shiro::Grid *field, int field_x, int field_y, Shiro::PieceDef& pd, unsigned int flags, int orient, int x, int y, Uint32 rgba);
int gfx_drawtimer(CoreState *cs, Shiro::Timer *t, int x, Uint32 rgba);
