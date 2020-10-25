#pragma once
#include <filesystem>
#include "SDL.h"
#include <string>
#define EMERGENCY_OVERRIDE 1
#define MESSAGE_EMERGENCY 0x1000000
#define ANIMATION_EMERGENCY 0x1000000
#define BUTTON_EMERGENCY 0x1000000

// class Image
typedef struct
{
    SDL_Texture *tex;
} gfx_image;

struct CoreState;

bool img_load(gfx_image *img, std::filesystem::path&& pathWithoutExtension, CoreState *cs);
void img_destroy(gfx_image *img);

enum text_alignment {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER
};

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

struct gfx_button {
    gfx_button() :
        x(0),
        y(0),
        w(0u),
        h(0u),
        flags(0u),
        highlighted(0),
        clicked(0),
        action(nullptr),
        delete_check(nullptr),
        data(nullptr),
        text_rgba_mod(0x00000000u) {}

    std::string text;
    int x;
    int y;
    std::size_t w;
    std::size_t h;
    unsigned int flags;
    int highlighted;
    int clicked;
    int (*action)(CoreState *, void *);
    int (*delete_check)(CoreState *);
    void *data;
    Uint32 text_rgba_mod;
};
