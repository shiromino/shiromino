#pragma once

#include "SDL.h"
#include <string>

#include "core.h"

#define EMERGENCY_OVERRIDE 1
#define MESSAGE_EMERGENCY 0x1000000
#define ANIMATION_EMERGENCY 0x1000000
#define BUTTON_EMERGENCY 0x1000000

// class Image
typedef struct
{
    SDL_Texture *tex;
} gfx_image;

bool img_load(gfx_image *img, std::string path_without_ext, coreState *cs);
void img_destroy(gfx_image *img);

enum text_alignment { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER };

struct png_monofont {
    SDL_Texture *sheet;
    SDL_Texture *outline_sheet;
    unsigned int char_w;
    unsigned int char_h;
};

struct text_formatting {
    Uint32 rgba;
    Uint32 outline_rgba;
    // Uint32 background_rgba;

    bool outlined;
    bool shadow;

    float size_multiplier;
    float line_spacing;
    enum text_alignment align;
    std::size_t wrap_length;
};

struct gfx_message {
    std::string text;
    int x;
    int y;
    unsigned int flags;
    png_monofont *font;
    struct text_formatting *fmt;
    unsigned int counter;
    int (*delete_check)(coreState *);
};

struct gfx_animation {
    gfx_image *first_frame;
    int x;
    int y;
    unsigned int flags;
    int num_frames;
    int frame_multiplier;
    unsigned int counter;
    Uint32 rgba_mod;
};

struct gfx_button {
    std::string text;
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
};
