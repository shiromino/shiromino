#include "CoreState.h"
#include "input/KeyFlags.h"
#include "game_qs.h"
#include "video/Render.h"
#include "gfx_old.h"
#include "gfx_structures.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "QRS0.h"
#include "stringtools.h"
#include "Timer.h"
#include "types.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>

bool img_load(gfx_image *img, std::filesystem::path&& pathWithoutExtension, CoreState *cs) {
    img->tex = NULL;

    SDL_Surface *s = NULL;

    std::filesystem::path& imagePath = pathWithoutExtension.concat(".png");
    s = IMG_Load(imagePath.string().c_str());

    if(!s) {
        s = IMG_Load(imagePath.replace_extension(".jpg").string().c_str());
    }

    if(s) {
        img->tex = SDL_CreateTextureFromSurface(cs->screen.renderer, s);
        SDL_FreeSurface(s);
    }

    return img->tex != NULL;
}

void img_destroy(gfx_image *img) {
    if (img->tex) {
        SDL_DestroyTexture(img->tex);
        img->tex = nullptr;
    }
}

png_monofont *monofont_tiny = NULL;
png_monofont *monofont_small = NULL;
png_monofont *monofont_thin = NULL;
png_monofont *monofont_square = NULL;
png_monofont *monofont_fixedsys = NULL;

struct text_formatting text_fmt_create(unsigned int flags, Shiro::u32 rgba, Shiro::u32 outline_rgba)
{
    struct text_formatting fmt;

    fmt.rgba = rgba;
    fmt.outline_rgba = outline_rgba;

    fmt.outlined = !(flags & DRAWTEXT_NO_OUTLINE);
    fmt.shadow = flags & DRAWTEXT_SHADOW;

    fmt.size_multiplier = 1.0;
    fmt.line_spacing = 1.0;
    fmt.align = ALIGN_LEFT;
    fmt.wrap_length = 0;

    if(flags & DRAWTEXT_CENTERED)
        fmt.align = ALIGN_CENTER;
    if(flags & DRAWTEXT_ALIGN_RIGHT)
        fmt.align = ALIGN_RIGHT;

    return fmt;
}

int gfx_init(CoreState *cs)
{
    monofont_tiny = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_small = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_thin = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_square = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_fixedsys = (png_monofont *)malloc(sizeof(png_monofont));
    assert(
        monofont_tiny != nullptr &&
        monofont_small != nullptr &&
        monofont_thin != nullptr &&
        monofont_square != nullptr &&
        monofont_fixedsys != nullptr
    );

    monofont_tiny->sheet = cs->assets->font_tiny.tex;
    monofont_tiny->outline_sheet = NULL;
    monofont_tiny->char_w = 6;
    monofont_tiny->char_h = 5;

    monofont_small->sheet = cs->assets->font_small.tex;
    monofont_small->outline_sheet = NULL;
    monofont_small->char_w = 12;
    monofont_small->char_h = 10;

    monofont_thin->sheet = cs->assets->font_thin_no_outline.tex;
    monofont_thin->outline_sheet = cs->assets->font_thin_outline_only.tex;
    monofont_thin->char_w = 13;
    monofont_thin->char_h = 18;

    monofont_square->sheet = cs->assets->font_square_no_outline.tex;
    monofont_square->outline_sheet = cs->assets->font_square_outline_only.tex;
    monofont_square->char_w = 15;
    monofont_square->char_h = 16;

    monofont_fixedsys->sheet = cs->assets->font_fixedsys_excelsior.tex;
    monofont_fixedsys->outline_sheet = NULL;
    monofont_fixedsys->char_w = 8;
    monofont_fixedsys->char_h = 16;

    return 0;
}

void gfx_quit(CoreState *cs)
{
    cs->gfx_buttons.clear();

    free(monofont_tiny);
    free(monofont_small);
    free(monofont_thin);
    free(monofont_square);
    free(monofont_fixedsys);
}

int gfx_createbutton(CoreState *cs, const char *text, int x, int y, unsigned int flags, int (*action)(CoreState *, void *), int (*deactivate_check)(CoreState *),
                     void *data, Shiro::u32 rgba)
{
    if(!text)
        return -1;

    gfx_button b;
    b.text = text;
    b.x = x;
    b.y = y;
    b.w = 2 * 6 + 16 * (b.text.size());
    b.h = 28;
    b.flags = flags;
    b.highlighted = 0;
    b.clicked = 0;
    b.action = action;
    b.deactivate_check = deactivate_check;
    b.data = data;
    b.text_rgba_mod = rgba;
    cs->gfx_buttons.push_back(b);

    return 0;
}

int gfx_drawbuttons(CoreState *cs, int type)
{
    if(!cs)
        return -1;

    if(!cs->gfx_buttons.size())
        return 0;

    SDL_Texture *font = cs->assets->font.tex;
    SDL_Rect src = { 0, 0, 6, 28 };
    SDL_Rect dest = { 0, 0, 6, 28 };

    struct text_formatting fmt = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    for (auto it = cs->gfx_buttons.begin(); it != cs->gfx_buttons.end(); it++) {
        gfx_button& b = *it;

        if(type == EMERGENCY_OVERRIDE && !(b.flags & BUTTON_EMERGENCY))
            continue;
        if(type == 0 && (b.flags & BUTTON_EMERGENCY))
            continue;

        if(b.highlighted)
        {
            if(b.clicked)
            {
                src.x = 362;
            }
            else
                src.x = 298;
        }
        else if(b.clicked)
        {
            src.x = 362;
        }
        else
            src.x = 330;

        src.w = 6;
        dest.w = 6;
        src.y = 26;
        dest.x = b.x;
        dest.y = b.y;

        if(b.highlighted)
        {
            SDL_SetTextureColorMod(font, R(b.text_rgba_mod), G(b.text_rgba_mod), B(b.text_rgba_mod));
            SDL_SetTextureAlphaMod(font, A(b.text_rgba_mod));
        }

        Shiro::RenderCopy(cs->screen, font, &src, &dest);

        src.x += 6;
        dest.x += 6;
        src.w = 16;
        dest.w = 16;

        for (decltype(b.text)::size_type j = 0; j < b.text.size(); j++)
        {
            if(j)
                dest.x += 16;

            Shiro::RenderCopy(cs->screen, font, &src, &dest);
        }

        src.x += 16;
        src.w = 6;
        dest.w = 6;
        dest.x += 16;

        Shiro::RenderCopy(cs->screen, font, &src, &dest);

        SDL_SetTextureColorMod(font, 255, 255, 255);
        SDL_SetTextureAlphaMod(font, 255);

        if(b.highlighted || b.clicked)
        {
            fmt.outlined = false;
            fmt.shadow = true;
            fmt.rgba = 0x000000FF;
        }
        else
        {
            fmt.outlined = true;
            fmt.shadow = false;
            fmt.rgba = b.text_rgba_mod;
        }

        gfx_drawtext(cs, b.text, b.x + 6, b.y + 6, monofont_square, &fmt);
    }

    //cs->gfx_buttons.clear();

    return 0;
}

int gfx_drawqrsfield(CoreState *cs, Shiro::Grid *field, unsigned int mode, unsigned int flags, int x, int y)
{
    if(!cs || !field)
        return -1;

    SDL_Texture *tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "tetrion_qs_white").getTexture();
    SDL_Texture *blocks = Shiro::ImageAsset::get(cs->assetMgr, "pieces-256x256").getTexture();

    SDL_SetTextureColorMod(blocks, 220, 220, 220);

    SDL_Rect tdest = { x, y - 48, 274, 416 };
    SDL_Rect src = { 0, 0, 256, 256 };
    SDL_Rect dest = { 0, 0, 16, 16 };

    qrsdata *q = (qrsdata *)cs->p1game->data;
    int use_deltas = 0;

    int i = 0;
    int j = 0;
    int c = 0;

    std::string piece_str = "A";

    switch(mode)
    {
        case MODE_G1_MASTER:
        case MODE_G1_20G:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g1_tetrion").getTexture();
            break;

        case MODE_G2_MASTER:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g2_tetrion_master").getTexture();
            break;

        case MODE_G2_DEATH:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g2_tetrion_death").getTexture();
            break;

        case MODE_G3_TERROR:
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "g3_tetrion_terror").getTexture();
            break;

        default:
            break;
    }

    if(q->pracdata)
    {
        if(q->pracdata->field_w <= 10)
        {
            tetrion_qs = Shiro::ImageAsset::get(cs->assetMgr, "tetrion_qs_white_10x10").getTexture();
            flags |= TEN_W_TETRION;
        }
    }

    Shiro::RenderCopy(cs->screen, tetrion_qs, NULL, &tdest);

    int logicalW = QRS_FIELD_W;
    int logicalH = QRS_FIELD_H;

    if((flags & DRAWFIELD_GRID) && !(flags & DRAWFIELD_BIG))
    {
        SDL_Rect gridSrc = { 33 * 256, 0, 256, 256 };
        SDL_Rect gridDest = { 0, 0, 16, 16 };

        for(i = 0; i < logicalW; i++)
        {
            for(j = QRS_FIELD_H - 20; j < logicalH; j++)
            {
                c = field->getCell(i, j);

                if(c == GRID_OOB)
                {
                    return 1;
                }

                if(c == QRS_FIELD_W_LIMITER)
                {
                    continue;
                }

                gridDest.x = x + 16 + (i * 16);
                gridDest.y = y + 32 + ((j - QRS_FIELD_H + 20) * 16);

                Shiro::RenderCopy(cs->screen, blocks, &gridSrc, &gridDest);
            }
        }
    }
    int cellSize = 16;

    if(flags & DRAWFIELD_BIG)
    {
        logicalW = q->field_w;
        logicalH = QRS_FIELD_H - 10;

        cellSize = 32;
    }

    dest.w = cellSize;
    dest.h = cellSize;

    for(i = 0; i < logicalW; i++)
    { // test feature: last 31 frames of every 91 frames make the stack shine
        for(j = QRS_FIELD_H - 20; j < logicalH; j++)
        {
            if(flags & TEN_W_TETRION)
            {
                if(!(flags & DRAWFIELD_BIG) && (i == 0 || i == 11))
                {
                    continue;
                }
            }
            c = field->getCell(i, j);
            if(c == GRID_OOB) {
                return 1;
            }

            if(c != -2 && c)
            {
                if(c == -5)
                {
                    src.x = 25 * 256;
                }
                else if(c == QRS_FIELD_W_LIMITER)
                {
                    if(!(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && !(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 27 * 256;
                    else if((IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && !(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 28 * 256;
                    else if(!(IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) - 1, j))) && (IS_INBOUNDS(field->getCell(static_cast<std::size_t>(i) + 1, j))))
                        src.x = 29 * 256;
                }
                else if(c & QRS_PIECE_BRACKETS)
                {
                    src.x = 30 * 256;
                }
                else if(c & QRS_PIECE_GEM)
                {
                    if(flags & DRAWFIELD_JEWELED)
                    {
                        src.x = ((c & 0xff) - 19) * 256;
                    }
                    else
                    {
                        src.x = ((c & 0xff) - 1) * 256;
                    }

                    src.y = 0;
                    dest.x = x + 16 + (i * cellSize);
                    dest.y = y + 32 + ((j - QRS_FIELD_H + 20) * cellSize);

                    if((flags & DRAWFIELD_BIG) && (flags & TEN_W_TETRION))
                    {
                        dest.x += 16;
                    }
                    Shiro::RenderCopy(cs->screen, blocks, &src, &dest);

                    src.x = 32 * 256;
                }
                else
                {
                    src.x = ((c & 0xff) - 1) * 256;
                }

                src.y = 0;
                dest.x = x + 16 + (i * cellSize);
                dest.y = y + 32 + ((j - QRS_FIELD_H + 20) * cellSize);

                if((flags & DRAWFIELD_BIG) && (flags & TEN_W_TETRION))
                {
                    dest.x += 16;
                }
                if(!(flags & DRAWFIELD_INVISIBLE) || (c == QRS_FIELD_W_LIMITER))
                {
                    // this stuff should be handled more elegantly, without needing access to the qrsdata
                    if(q->state_flags & GAMESTATE_FADING)
                    {
                        if(GET_PIECE_FADE_COUNTER(c) > 10)
                        {
                            Shiro::RenderCopy(cs->screen, blocks, &src, &dest);
                        }
                        else if(GET_PIECE_FADE_COUNTER(c) > 0)
                        {
                            SDL_SetTextureAlphaMod(blocks, GET_PIECE_FADE_COUNTER(c) * 25);
                            Shiro::RenderCopy(cs->screen, blocks, &src, &dest);
                            SDL_SetTextureAlphaMod(blocks, 255);
                        }
                    }
                    else {
                        Shiro::RenderCopy(cs->screen, blocks, &src, &dest);
                    }

                    if((!(c & QRS_PIECE_BRACKETS) || c < 0) && !(flags & DRAWFIELD_NO_OUTLINE))
                    {
                        Uint8 r_;
                        Uint8 g_;
                        Uint8 b_;
                        Uint8 a_;
                        SDL_GetRenderDrawColor(cs->screen.renderer, &r_, &g_, &b_, &a_);
                        SDL_SetRenderDrawColor(cs->screen.renderer, 0xFF, 0xFF, 0xFF, 0x8C);

                        SDL_Rect outlineRect = { dest.x, dest.y, cellSize, 2 };

                        c = field->getCell(i, static_cast<std::size_t>(j) - 1); // above
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y;
                            outlineRect.w = cellSize;
                            outlineRect.h = 2;

                            if(!use_deltas)
                            {
                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }
                        }

                        c = field->getCell(static_cast<std::size_t>(i) - 1, j); // left
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y;
                            outlineRect.w = 2;
                            outlineRect.h = cellSize;

                            if(!use_deltas)
                            {
                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }
                        }

                        c = field->getCell(static_cast<std::size_t>(i) + 1, j); // right
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x + cellSize - 2;
                            outlineRect.y = dest.y;
                            outlineRect.w = 2;
                            outlineRect.h = cellSize;

                            if(!use_deltas)
                            {
                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }
                        }

                        c = field->getCell(i, static_cast<std::size_t>(j) + 1); // below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            outlineRect.x = dest.x;
                            outlineRect.y = dest.y + cellSize - 2;
                            outlineRect.w = cellSize;
                            outlineRect.h = 2;

                            if(!use_deltas)
                            {
                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }
                        }

                        SDL_SetRenderDrawColor(cs->screen.renderer, r_, g_, b_, a_);
                    }
                }

                SDL_SetTextureColorMod(blocks, 220, 220, 220);
            }
        }
    }

    /*if(use_deltas) {
       SDL_SetRenderTarget(cs->screen.renderer, NULL);
       SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_BLEND);
       Shiro::RenderCopy(cs->screen, q->field_tex, NULL, &field_dest);
    }*/

    SDL_SetTextureColorMod(blocks, 255, 255, 255);

    return 0;
}

int gfx_drawkeys(CoreState *cs, Shiro::KeyFlags *k, int x, int y, Shiro::u32 rgba)
{
    if(!cs)
        return -1;

    SDL_Texture *font = cs->assets->font.tex;
    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    SDL_Rect src = { 0, 80, 16, 16 };
    SDL_Rect dest = { 0, y, 16, 16 };

    std::string text_a = "A";
    std::string text_b = "B";
    std::string text_c = "C";
    std::string text_d = "D";

    struct text_formatting fmt = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    if(k->left)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 0;
    dest.x = x;
    Shiro::RenderCopy(cs->screen, font, &src, &dest);

    if(k->right)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 16;
    dest.x = x + 16;
    Shiro::RenderCopy(cs->screen, font, &src, &dest);

    if(k->up)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 32;
    dest.x = x + 32;
    Shiro::RenderCopy(cs->screen, font, &src, &dest);

    if(k->down)
    {
        SDL_SetTextureColorMod(font, 255, 255, 255);
    }
    else
    {
        SDL_SetTextureColorMod(font, 40, 40, 40);
    }

    src.x = 48;
    dest.x = x + 48;
    Shiro::RenderCopy(cs->screen, font, &src, &dest);

    SDL_SetTextureColorMod(font, 255, 255, 255);

    if(k->a)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_a, x + 64, y, monofont_square, &fmt);

    if(k->b)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_b, x + 80, y, monofont_square, &fmt);

    if(k->c)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_c, x + 96, y, monofont_square, &fmt);

    if(k->d)
        fmt.rgba = rgba;
    else
        fmt.rgba = 0x282828FF;

    gfx_drawtext(cs, text_d, x + 112, y, monofont_square, &fmt);

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}

int gfx_drawtext(CoreState *cs, std::string text, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    return gfx_drawtext_partial(cs, text, 0, text.size(), x, y, font, fmt);
}

int gfx_drawtext_partial(CoreState *cs, std::string text, int pos, std::size_t len, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    if(!cs || text == "")
        return -1;

    if(!font)
        font = monofont_fixedsys;

    struct text_formatting fmt_ = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    if(!fmt)
        fmt = &fmt_;

    SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
    SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, R(fmt->outline_rgba), G(fmt->outline_rgba), B(fmt->outline_rgba));
        SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->outline_rgba));
    }

    SDL_Rect src = { 0, 0, (int) font->char_w, (int) font->char_h };
    SDL_Rect dest = { x, y, (int) (fmt->size_multiplier * (float) font->char_w), (int) (fmt->size_multiplier * (float) font->char_h) };

    std::size_t i = 0;

    std::size_t linefeeds = 0;

    std::size_t last_wrap_line_pos = 0;
    std::size_t last_wrap_pos = 0;

    std::vector<std::string> lines = strtools::split(text, '\n');

    bool using_target_tex = false;

    if(SDL_GetRenderTarget(cs->screen.renderer) != NULL)
        using_target_tex = true;

    for(i = pos; i < text.size() && i < len; i++)
    {
        if(i == 0)
        {
            switch(fmt->align)
            {
                default:
                case ALIGN_LEFT:
                    dest.x = x;
                    break;

                case ALIGN_RIGHT:
                    dest.x = static_cast<int>(x - (fmt->size_multiplier * (float)font->char_w) * lines[0].size());
                    break;

                case ALIGN_CENTER:
                    if (fmt->wrap_length < lines[0].size() - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = x + static_cast<int>((fmt->size_multiplier * (float)font->char_w / 2.0f) * (fmt->wrap_length - (lines[0].size() - last_wrap_line_pos)));

                    break;
            }
        }

        if((fmt->wrap_length && i != 0 && (i - last_wrap_pos) % fmt->wrap_length == 0) || text[i] == '\n')
        {
            if(text[i] == '\n')
            {
                linefeeds++;
                last_wrap_line_pos = i - last_wrap_pos + last_wrap_line_pos;
                last_wrap_pos = i;
            }
            else if(i != 0 && i % fmt->wrap_length == 0)
            {
                last_wrap_line_pos = i - last_wrap_pos + last_wrap_line_pos;
                last_wrap_pos = i;
            }

            dest.y += static_cast<int>(fmt->line_spacing * fmt->size_multiplier * (float)font->char_h);

            switch(fmt->align)
            {
                default:
                case ALIGN_LEFT:
                    dest.x = x;
                    break;

                case ALIGN_RIGHT:
                    dest.x = static_cast<int>(x - (font->char_w) * lines[linefeeds].size());
                    break;

                case ALIGN_CENTER:
                    if(fmt->wrap_length < lines[linefeeds].size() - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = (int)(x + (font->char_w / 2) * (fmt->wrap_length - (lines[linefeeds].size() - last_wrap_line_pos)));

                    break;
            }

            if(text[i] == '\n')
                continue;
        }

        // we draw a square behind each character if we have no outlines to use
        if(fmt->outlined && !font->outline_sheet)
        {
            src.x = 31 * font->char_w;
            src.y = 3 * font->char_h;
            SDL_SetTextureColorMod(font->sheet, R(fmt->outline_rgba), G(fmt->outline_rgba), B(fmt->outline_rgba));
            SDL_SetTextureAlphaMod(font->sheet, A(fmt->outline_rgba));

            if(using_target_tex)
            {
                SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
                Shiro::RenderFillRect(cs->screen, &dest);
                Shiro::RenderCopy(cs->screen, font->sheet, &src, &dest);
            }
            else
            {
                Shiro::RenderCopy(cs->screen, font->sheet, &src, &dest);
            }

            SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
        }

        src.x = font->char_w * (text[i] % 32);
        src.y = font->char_h * ((int)(text[i] / 32) - 1);
        if(src.y < 0)
        {
            src.x = 31 * font->char_w;
            src.y = 2 * font->char_h;
        }

        if(fmt->shadow)
        {
            dest.x -= static_cast<int>(2.0f * fmt->size_multiplier);
            dest.y += static_cast<int>(2.0f * fmt->size_multiplier);

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba) / 4);
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba) / 4);

            Shiro::RenderCopy(cs->screen, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
            {
                Shiro::RenderCopy(cs->screen, font->outline_sheet, &src, &dest);
            }

            dest.x += static_cast<int>(2.0f * fmt->size_multiplier);
            dest.y -= static_cast<int>(2.0f * fmt->size_multiplier);

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba));
        }

        if(using_target_tex)
        {
            SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
            Shiro::RenderFillRect(cs->screen, &dest);
            Shiro::RenderCopy(cs->screen, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                Shiro::RenderCopy(cs->screen, font->outline_sheet, &src, &dest);
        }
        else
        {
            Shiro::RenderCopy(cs->screen, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                Shiro::RenderCopy(cs->screen, font->outline_sheet, &src, &dest);
        }

        dest.x += static_cast<int>(fmt->size_multiplier * (float)font->char_w);
    }

    SDL_SetTextureColorMod(font->sheet, 255, 255, 255);
    SDL_SetTextureAlphaMod(font->sheet, 255);

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, 255, 255, 255);
        SDL_SetTextureAlphaMod(font->outline_sheet, 255);
    }

    return 0;
}

int gfx_drawpiece(CoreState *cs, Shiro::Grid *field, int field_x, int field_y, Shiro::PieceDefinition& pieceDefinition, unsigned int flags, int orient, int x, int y, Shiro::u32 rgba)
{
    if(!cs)
        return -1;

    if(flags & DRAWPIECE_BRACKETS && flags & DRAWPIECE_LOCKFLASH)
    {
        return 0;
    }

    if((flags & DRAWPIECE_PREVIEW) && (flags & DRAWPIECE_BIG))
    {
        flags &= ~DRAWPIECE_BIG;
    }

    if((flags & DRAWPIECE_JEWELED) && (flags & DRAWPIECE_SMALL))
    {
        flags &= ~DRAWPIECE_JEWELED;
    }

    SDL_Texture *blocks;
    blocks = Shiro::ImageAsset::get(cs->assetMgr, "pieces-256x256").getTexture();
    int size = (flags & DRAWPIECE_SMALL) ? 8 : (flags & DRAWPIECE_BIG ? 32 : 16);
    SDL_Rect src = { 0, 0, 256, 256 };
    SDL_Rect dest = { 0, 0, size, size };

    std::string piece_str = "A";
    piece_str[0] = pieceDefinition.qrsID + 'A';

    Shiro::Grid *g = NULL;

    int i = 0;
    int j = 0;
    int w = int(pieceDefinition.rotationTable[0].getWidth());
    int h = int(pieceDefinition.rotationTable[0].getWidth());
    int c = 0;

    int cell_x = 0;
    int cell_y = 0;

    g = &pieceDefinition.rotationTable[orient & 3];
    src.x = pieceDefinition.qrsID * 256;
    if(flags & DRAWPIECE_JEWELED) {
        src.x -= 18 * 256;
    }
    SDL_SetTextureColorMod(blocks, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(blocks, A(rgba));

    for(i = 0; i < w; i++)
    {
        for(j = 0; j < h; j++)
        {
            if(g->getCell(i, j) && (y + (j * size) > (field_y + 16) || flags & DRAWPIECE_PREVIEW))
            {
                dest.x = x + 16 + ((i - 1) * size);

                if(flags & DRAWPIECE_PREVIEW)
                {
                    if(w == 4)
                    {
                        dest.y = y + ((j + 1) * size);
                    }
                    else
                    {
                        dest.y = y + (j * size);
                    }
                }
                else
                {
                    dest.y = y + 16 + ((j - 1) * size);
                }

                if(flags & DRAWPIECE_BRACKETS || pieceDefinition.flags & Shiro::PDBRACKETS)
                    src.x = 30 * 256;

                if(flags & DRAWPIECE_LOCKFLASH && !(flags & DRAWPIECE_BRACKETS) && !(pieceDefinition.flags & Shiro::PDBRACKETS))
                {
                    src.x = 26 * 256;
                    cell_x = (x - field_x - 16) / size + i;
                    cell_y = (y - field_y - 32) / size + j + QRS_FIELD_H - 20;

                    if(field->getCell(cell_x, cell_y) > 0 || flags & DRAWPIECE_PREVIEW)
                    {
                        Shiro::RenderCopy(cs->screen, blocks, &src, &dest);

                        if(!(flags & DRAWPIECE_PREVIEW))
                        {
                            Uint8 r_;
                            Uint8 g_;
                            Uint8 b_;
                            Uint8 a_;
                            SDL_GetRenderDrawColor(cs->screen.renderer, &r_, &g_, &b_, &a_);
                            SDL_SetRenderDrawColor(cs->screen.renderer, 0xFF, 0xFF, 0xFF, 0x8C);

                            SDL_Rect outlineRect = { dest.x, dest.y, size, 2 };

                            c = field->getCell(i, static_cast<std::size_t>(j) - 1); // above
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                outlineRect.x = dest.x;
                                outlineRect.y = dest.y;
                                outlineRect.w = size;
                                outlineRect.h = 2;

                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }

                            c = field->getCell(static_cast<std::size_t>(i) - 1, j); // left
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                outlineRect.x = dest.x;
                                outlineRect.y = dest.y;
                                outlineRect.w = 2;
                                outlineRect.h = size;

                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }

                            c = field->getCell(static_cast<std::size_t>(i) + 1, j); // right
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                outlineRect.x = dest.x + size - 2;
                                outlineRect.y = dest.y;
                                outlineRect.w = 2;
                                outlineRect.h = size;

                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }

                            c = field->getCell(i, static_cast<std::size_t>(j) + 1); // below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                outlineRect.x = dest.x;
                                outlineRect.y = dest.y + size - 2;
                                outlineRect.w = size;
                                outlineRect.h = 2;

                                Shiro::RenderFillRect(cs->screen, &outlineRect);
                            }

                            SDL_SetRenderDrawColor(cs->screen.renderer, r_, g_, b_, a_);
                        }
                    }
                }
                else {
                    Shiro::RenderCopy(cs->screen, blocks, &src, &dest);
                }
            }
        }
    }

    SDL_SetTextureColorMod(blocks, 255, 255, 255);
    SDL_SetTextureAlphaMod(blocks, 255);

    return 0;
}

int gfx_drawtimer(CoreState *cs, Shiro::Timer *t, int x, Shiro::u32 rgba)
{
    SDL_Texture *font = cs->assets->font.tex;
    qrsdata *q = (qrsdata *)cs->p1game->data;
    int y = q->field_y;

    SDL_Rect src = { 0, 96, 20, 32 };
    SDL_Rect dest = { x, 26 * 16 + 8 - QRS_FIELD_Y + y, 20, 32 };

    uint64_t min = t->min();
    uint64_t sec = t->sec() % 60;
    uint64_t csec = t->csec() % 100;

    int i = 0;

    int digits[6];

    digits[0] = int(min / 10);
    digits[1] = int(min % 10);
    digits[2] = int(sec / 10);
    digits[3] = int(sec % 10);
    digits[4] = int(csec / 10);
    digits[5] = int(csec % 10);

    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    for(i = 0; i < 6; i++)
    {
        src.x = digits[i] * 20;
        Shiro::RenderCopy(cs->screen, font, &src, &dest);
        dest.x += 20;

        if(i == 1 || i == 3)
        {
            src.x = 200; // colon character offset
            Shiro::RenderCopy(cs->screen, font, &src, &dest);
            dest.x += 20;
        }
    }

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}