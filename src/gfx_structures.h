#ifndef _gfx_structures_h
#define _gfx_structures_h

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "bstrlib.h"

#define EMERGENCY_OVERRIDE  1
#define MESSAGE_EMERGENCY       0x1000000
#define ANIMATION_EMERGENCY     0x1000000
#define BUTTON_EMERGENCY        0x1000000

enum text_alignment {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER
};

typedef struct {
    SDL_Texture *sheet;
    SDL_Texture *outline_sheet;
    unsigned int char_w;
    unsigned int char_h;
} png_monofont;

struct text_formatting {
    Uint32 rgba;
    Uint32 outline_rgba;
    //Uint32 background_rgba;

    bool outlined;
    bool shadow;

    float size_multiplier;
    float line_spacing;
    enum text_alignment align;
    unsigned int wrap_length;
};

typedef struct {
   bstring text;
   int x;
   int y;
    unsigned int flags;
   png_monofont *font;
   struct text_formatting *fmt;
   unsigned int counter;
   int (*delete_check)(coreState *);
} gfx_message;

typedef struct {
   bstring name;
   int x;
   int y;
   unsigned int flags;
   int num_frames;
   int frame_multiplier;
   unsigned int counter;
   Uint32 rgba_mod;
} gfx_animation;

typedef struct {
   bstring text;
   int x;
   int y;
   int w;
   int h;
   unsigned int flags;
   int highlighted;
   int clicked;
   int (*action)(coreState *, void *);
   int (*delete_check)(coreState *);
   void *data;
   Uint32 text_rgba_mod;
} gfx_button;

#endif
