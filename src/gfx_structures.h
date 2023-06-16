#pragma once
#include "types.h"
#include "SDL.h"
#include <filesystem>
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

enum gfx_button_type {
    BUTTON_TYPE_ACTION,
    BUTTON_TYPE_TOGGLE
};

struct png_monofont {
    SDL_Texture *sheet;
    SDL_Texture *outline_sheet;
    unsigned int char_w;
    unsigned int char_h;
};

struct text_formatting {
    Shiro::u32 rgba;
    Shiro::u32 outline_rgba;
    // Shiro::u32 background_rgba;

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
        toggleOffText(""),
        toggleOnText(""),
        toggleValue(false),
        boolPtr(nullptr),
        type(BUTTON_TYPE_ACTION),
        visible(true),
        active(false),
        action(nullptr),
        activate_check(nullptr),
        deactivate_check(nullptr),
        data(nullptr),
        text_rgba_mod(0x00000000u) {}

    ~gfx_button() {}

    std::string text;

    std::string toggleOffText;
    std::string toggleOnText;

    bool toggleValue;
    bool *boolPtr;

    enum gfx_button_type type;

    bool visible;
    bool active;
    int x;
    int y;
    std::size_t w;
    std::size_t h;
    unsigned int flags;
    int highlighted;
    int clicked;
    int (*action)(CoreState *, void *);
    int (*activate_check)(CoreState *);
    int (*deactivate_check)(CoreState *);
    void *data;
    Shiro::u32 text_rgba_mod;
};