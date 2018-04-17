#include "bstrlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "core.h"
#include "game_qs.h"
#include "gfx.h"
#include "gfx_structures.h"
#include "grid.h"
#include "piecedef.h"
#include "qrs.h"
#include "timer.h"

/*
int gfx_piece_colors[25] =
{
    0xD00000,
    0x0000FF,
    0xFF6000,
    0x101010,
    0xBB3CBB,
    0x00CD00,
    0x70ECEE,   // ice
    0xFF658B,
    0xD22D04,
    0x1200BF,   // dark blue
    0xC70694,
    0x1200BF,   // dark blue
    0xEFEF00,
    0xC70694,
    0xD22D04,
    0x70ECEE,   // ice
    0xFF658B,
    0x00A3A3,   // teal
    0xD00000,
    0x00A3A3,   // teal
    0x0000FF,
    0xFF6000,
    0xEFEF00,
    0xBB3CBB,
    0x00CD00,
    0x808080
};
*/

bool img_load(gfx_image *img, const char *path_without_ext, coreState *cs)
{
    img->tex = NULL;

    SDL_Surface *s = NULL;

    bstring path = bfromcstr(path_without_ext);
    bcatcstr(path, ".png");
    s = IMG_Load((const char *)(path->data));
    bdestroy(path);

    if(!s)
    {
        path = bfromcstr(path_without_ext);
        bcatcstr(path, ".jpg");
        s = IMG_Load((const char *)(path->data));
        bdestroy(path);
    }

    if(s)
    {
        img->tex = SDL_CreateTextureFromSurface(cs->screen.renderer, s);
        SDL_FreeSurface(s);
    }

    return img->tex != NULL;
}

void img_destroy(gfx_image *img)
{
    if(img->tex)
        SDL_DestroyTexture(img->tex);
}

png_monofont *monofont_tiny = NULL;
png_monofont *monofont_small = NULL;
png_monofont *monofont_thin = NULL;
png_monofont *monofont_square = NULL;
png_monofont *monofont_fixedsys = NULL;

struct text_formatting *text_fmt_create(unsigned int flags, Uint32 rgba, Uint32 outline_rgba)
{
    struct text_formatting *fmt = (struct text_formatting *)malloc(sizeof(struct text_formatting));

    fmt->rgba = rgba;
    fmt->outline_rgba = outline_rgba;

    fmt->outlined = !(flags & DRAWTEXT_NO_OUTLINE);
    fmt->shadow = flags & DRAWTEXT_SHADOW;

    fmt->size_multiplier = 1.0;
    fmt->line_spacing = 1.0;
    fmt->align = ALIGN_LEFT;
    fmt->wrap_length = 0;

    if(flags & DRAWTEXT_CENTERED)
        fmt->align = ALIGN_CENTER;
    if(flags & DRAWTEXT_ALIGN_RIGHT)
        fmt->align = ALIGN_RIGHT;

    return fmt;
}

void gfx_message_destroy(gfx_message *m)
{
    if(!m)
        return;

    if(m->text)
        bdestroy(m->text);

    if(m->fmt)
        free(m->fmt);

    free(m);
}

void gfx_animation_destroy(gfx_animation *a)
{
    if(!a)
        return;

    free(a);
}

void gfx_button_destroy(gfx_button *b)
{
    if(!b)
        return;

    if(b->text)
        bdestroy(b->text);

    free(b);
}

int gfx_init(coreState *cs)
{
    monofont_tiny = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_small = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_thin = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_square = (png_monofont *)malloc(sizeof(png_monofont));
    monofont_fixedsys = (png_monofont *)malloc(sizeof(png_monofont));

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

void gfx_quit(coreState *cs)
{
    int i = 0;

    if(cs->gfx_messages)
    {
        for(i = 0; i < cs->gfx_messages_max; i++)
        {
            if(cs->gfx_messages[i])
                gfx_message_destroy(cs->gfx_messages[i]);
        }

        free(cs->gfx_messages);
    }

    if(cs->gfx_animations)
    {
        for(i = 0; i < cs->gfx_animations_max; i++)
        {
            if(cs->gfx_animations[i])
                gfx_animation_destroy(cs->gfx_animations[i]);
        }

        free(cs->gfx_animations);
    }

    if(cs->gfx_buttons)
    {
        for(i = 0; i < cs->gfx_buttons_max; i++)
        {
            if(cs->gfx_buttons[i])
                gfx_button_destroy(cs->gfx_buttons[i]);
        }

        free(cs->gfx_buttons);
    }

    cs->gfx_messages = NULL;
    cs->gfx_animations = NULL;
    cs->gfx_buttons = NULL;
    cs->gfx_messages_max = 0;
    cs->gfx_animations_max = 0;
    cs->gfx_buttons_max = 0;

    free(monofont_tiny);
    free(monofont_small);
    free(monofont_thin);
    free(monofont_square);
    free(monofont_fixedsys);
}

int gfx_rendercopy(coreState *cs, SDL_Texture *t, SDL_Rect *src, SDL_Rect *dest_)
{
    SDL_Rect dest;

    // SDL_SetRenderTarget(cs->screen.renderer, cs->screen.target_tex);

    if(dest_)
    {
        dest.x = dest_->x * cs->settings->video_scale;
        dest.y = dest_->y * cs->settings->video_scale;
        dest.w = dest_->w * cs->settings->video_scale;
        dest.h = dest_->h * cs->settings->video_scale;

        return SDL_RenderCopy(cs->screen.renderer, t, src, &dest);
    }
    else
        return SDL_RenderCopy(cs->screen.renderer, t, src, dest_);
}

int gfx_start_bg_fade_in(coreState *cs)
{
    if(!cs)
        return -1;

    if(cs->bg)
        SDL_SetTextureColorMod(cs->bg, 0, 0, 0);
    else
        return 1;

    return 0;
}

int gfx_drawbg(coreState *cs)
{
    // SDL_Texture *bg_darken = (asset_by_name(cs, "bg_darken"))->data;
    // SDL_Texture *anim_bg_frame = NULL;
    // bstring asset_name = NULL;
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    // Uint8 a;

    /*if(cs->anim_bg || cs->anim_bg_old) {
        SDL_SetTextureColorMod(bg_darken, 0, 0, 0);
        if(cs->anim_bg != cs->anim_bg_old) {
            SDL_GetTextureAlphaMod(bg_darken, &a);
            if(a < 255) {
                if(a < 255 - BG_FADE_RATE)
                    a += BG_FADE_RATE;
                else
                    a = 255;
            } else {

            }
        } else {
            asset_name = bformat("%s/%05d", cs->anim_bg->name->data, cs->anim_bg->counter/cs->anim_bg->frame_multiplier);
            anim_bg_frame = (asset_by_name(cs, asset_name->data))->data;
            //printf("Drawing %s\n", asset_name->data);

            gfx_rendercopy(cs, anim_bg_frame, NULL, NULL);
        //}

        if(cs->anim_bg) {
            cs->anim_bg->counter++;
            if(cs->anim_bg->counter == cs->anim_bg->num_frames*cs->anim_bg->frame_multiplier)
                cs->anim_bg->counter = 0;
        }

      if(cs->anim_bg_old && cs->anim_bg != cs->anim_bg_old) {
            cs->anim_bg_old->counter++;
            if(cs->anim_bg_old->counter == cs->anim_bg->num_frames*cs->anim_bg->frame_multiplier)
                cs->anim_bg_old->counter = 0;
        }

        return 0;
    }*/

    if(cs->bg != cs->bg_old)
    {
        if(cs->bg_old)
            SDL_GetTextureColorMod(cs->bg_old, &r, &g, &b);

        if(r && cs->bg_old)
        {
            if(r > BG_FADE_RATE)
            {
                r -= BG_FADE_RATE;
                g -= BG_FADE_RATE;
                b -= BG_FADE_RATE;
            }
            else
            {
                r = 0;
                g = 0;
                b = 0;

                SDL_SetTextureColorMod(cs->bg, 0, 0, 0);
            }

            SDL_SetTextureColorMod(cs->bg_old, r, g, b);
            gfx_rendercopy(cs, cs->bg_old, NULL, NULL);
        }
        else
        {
            if(!cs->bg)
                return 0;

            SDL_GetTextureColorMod(cs->bg, &r, &g, &b);
            if(r < 255)
            {
                if(r < 255 - BG_FADE_RATE)
                {
                    r += BG_FADE_RATE;
                    g += BG_FADE_RATE;
                    b += BG_FADE_RATE;
                }
                else
                {
                    r = 255;
                    g = 255;
                    b = 255;

                    SDL_SetTextureColorMod(cs->bg_old, 255, 255, 255);
                    cs->bg_old = cs->bg;
                }
            }

            SDL_SetTextureColorMod(cs->bg, r, g, b);
            gfx_rendercopy(cs, cs->bg, NULL, NULL);
        }
    }
    else
    {
        if(!cs->bg)
            return 0;

        gfx_rendercopy(cs, cs->bg, NULL, NULL);
    }

    return 0;
}

int gfx_draw_emergency_bg_darken(coreState *cs)
{
    SDL_Texture *bg_darken = cs->assets->bg_darken.tex;
    SDL_SetTextureColorMod(bg_darken, 0, 0, 0);
    SDL_SetTextureAlphaMod(bg_darken, 210);
    gfx_rendercopy(cs, bg_darken, NULL, NULL);
    SDL_SetTextureColorMod(bg_darken, 255, 255, 255);
    SDL_SetTextureAlphaMod(bg_darken, 255);

    return 0;
}

/*
int gfx_brighten_texture(SDL_Texture *tex, Uint8 amt)
{

}

int gfx_darken_texture(SDL_Texture *tex, Uint8 amt)
{

}
*/

int gfx_pushmessage(coreState *cs, const char *text, int x, int y, unsigned int flags, png_monofont *font, struct text_formatting *fmt, unsigned int counter,
                    int (*delete_check)(coreState *))
{
    if(!text)
        return -1;

    gfx_message *m = (gfx_message *)malloc(sizeof(gfx_message));
    m->text = bfromcstr(text);
    m->x = x;
    m->y = y;
    m->flags = flags;
    m->font = font;
    m->fmt = fmt;
    m->counter = counter;
    m->delete_check = delete_check;

    cs->gfx_messages_max++;
    cs->gfx_messages = (gfx_message **)realloc(cs->gfx_messages, cs->gfx_messages_max * sizeof(gfx_message *));

    cs->gfx_messages[cs->gfx_messages_max - 1] = m;

    return 0;
}

int gfx_drawmessages(coreState *cs, int type)
{
    if(!cs)
        return -1;

    if(!cs->gfx_messages)
        return 0;

    int i = 0;
    int n = 0;
    gfx_message *m = NULL;

    for(i = 0; i < cs->gfx_messages_max; i++)
    {
        if(!cs->gfx_messages[i])
        {
            n++;
            continue;
        }

        m = cs->gfx_messages[i];

        if(type == EMERGENCY_OVERRIDE && !(m->flags & MESSAGE_EMERGENCY))
            continue;
        if(type == 0 && (m->flags & MESSAGE_EMERGENCY))
            continue;

        if(!m->counter || (m->delete_check && m->delete_check(cs)))
        {
            gfx_message_destroy(m);
            cs->gfx_messages[i] = NULL;
            continue;
        }

        gfx_drawtext(cs, m->text, m->x, m->y, m->font, m->fmt);
        m->counter--;
    }

    if(n == cs->gfx_messages_max)
    {
        free(cs->gfx_messages);
        cs->gfx_messages = NULL;
        cs->gfx_messages_max = 0;
    }

    return 0;
}

int gfx_pushanimation(coreState *cs, gfx_image *first_frame, int x, int y, int num_frames, int frame_multiplier, Uint32 rgba)
{
    gfx_animation *a = (gfx_animation *)malloc(sizeof(gfx_animation));
    a->first_frame = first_frame;
    a->x = x;
    a->y = y;
    a->flags = 0;
    a->num_frames = num_frames;
    a->frame_multiplier = frame_multiplier;
    a->rgba_mod = rgba;
    a->counter = 0;

    int i = 0;
    for(i = 0; i < cs->gfx_animations_max; i++)
    {
        if(cs->gfx_animations[i] == NULL)
        {
            cs->gfx_animations[i] = a;
            return 0;
        }
    }

    cs->gfx_animations_max++;
    cs->gfx_animations = (gfx_animation **)realloc(cs->gfx_animations, cs->gfx_animations_max * sizeof(gfx_animation *));

    cs->gfx_animations[cs->gfx_animations_max - 1] = a;

    return 0;
}

int gfx_drawanimations(coreState *cs, int type)
{
    if(!cs)
        return -1;

    if(!cs->gfx_animations)
        return 0;

    int i = 0;
    int n = 0;
    gfx_animation *a = NULL;
    SDL_Rect dest = {.x = 0, .y = 0, .w = 0, .h = 0};
    SDL_Texture *t = NULL;

    for(i = 0; i < cs->gfx_animations_max; i++)
    {
        if(!cs->gfx_animations[i])
        {
            n++;
            continue;
        }

        a = cs->gfx_animations[i];

        if(type == EMERGENCY_OVERRIDE && !(a->flags & ANIMATION_EMERGENCY))
            continue;
        if(type == 0 && (a->flags & ANIMATION_EMERGENCY))
            continue;

        if(a->counter == (unsigned int)(a->frame_multiplier * a->num_frames))
        {
            gfx_animation_destroy(a);
            cs->gfx_animations[i] = NULL;
            continue;
        }

        int framenum = a->counter / a->frame_multiplier;
        t = (a->first_frame + framenum)->tex;
        if(!t)
            printf("NULL texture on frame %d\n", framenum);

        dest.x = a->x;
        dest.y = a->y;
        SDL_QueryTexture(t, NULL, NULL, &dest.w, &dest.h);

        SDL_SetTextureColorMod(t, R(a->rgba_mod), G(a->rgba_mod), B(a->rgba_mod));
        SDL_SetTextureAlphaMod(t, A(a->rgba_mod));
        gfx_rendercopy(cs, t, NULL, &dest);
        SDL_SetTextureAlphaMod(t, 255);
        SDL_SetTextureColorMod(t, 255, 255, 255);

        a->counter++;
    }

    if(n == cs->gfx_animations_max)
    {
        free(cs->gfx_animations);
        cs->gfx_animations = NULL;
        cs->gfx_animations_max = 0;
    }

    return 0;
}

int gfx_draw_anim_bg() { return 0; }

int gfx_createbutton(coreState *cs, const char *text, int x, int y, unsigned int flags, int (*action)(coreState *, void *), int (*delete_check)(coreState *),
                     void *data, Uint32 rgba)
{
    if(!text)
        return -1;

    gfx_button *b = (gfx_button *)malloc(sizeof(gfx_button));
    b->text = bfromcstr(text);
    b->x = x;
    b->y = y;
    b->w = 2 * 6 + 16 * (b->text->slen);
    b->h = 28;
    b->flags = flags;
    b->highlighted = 0;
    b->clicked = 0;
    b->action = action;
    b->delete_check = delete_check;
    b->data = data;
    b->text_rgba_mod = rgba;

    cs->gfx_buttons_max++;
    cs->gfx_buttons = (gfx_button **)realloc(cs->gfx_buttons, cs->gfx_buttons_max * sizeof(gfx_button *));

    cs->gfx_buttons[cs->gfx_buttons_max - 1] = b;

    return 0;
}

int gfx_drawbuttons(coreState *cs, int type)
{
    if(!cs)
        return -1;

    if(!cs->gfx_buttons)
        return 0;

    int i = 0;
    int j = 0;
    int n = 0;
    gfx_button *b = NULL;
    SDL_Texture *font = cs->assets->font.tex;
    // SDL_Texture *font_no_outline = cs->assets->font_no_outline.tex;
    SDL_Rect src = {.x = 0, .y = 0, .w = 6, .h = 28};
    SDL_Rect dest = {.x = 0, .y = 0, .w = 6, .h = 28};

    struct text_formatting fmt = {.rgba = RGBA_DEFAULT,
                                  .outline_rgba = RGBA_OUTLINE_DEFAULT,
                                  .outlined = true,
                                  .shadow = false,
                                  .size_multiplier = 1.0,
                                  .line_spacing = 1.0,
                                  .align = ALIGN_LEFT,
                                  .wrap_length = 0};

    for(i = 0; i < cs->gfx_buttons_max; i++)
    {
        if(!cs->gfx_buttons[i])
        {
            n++;
            continue;
        }

        b = cs->gfx_buttons[i];

        if(type == EMERGENCY_OVERRIDE && !(b->flags & BUTTON_EMERGENCY))
            continue;
        if(type == 0 && (b->flags & BUTTON_EMERGENCY))
            continue;

        if(b->highlighted)
        {
            if(b->clicked)
            {
                src.x = 362;
                b->clicked--;
            }
            else
                src.x = 298;
        }
        else if(b->clicked)
        {
            src.x = 362;
            b->clicked--;
        }
        else
            src.x = 330;

        src.w = 6;
        dest.w = 6;
        src.y = 26;
        dest.x = b->x;
        dest.y = b->y;

        if(b->highlighted)
        {
            SDL_SetTextureColorMod(font, R(b->text_rgba_mod), G(b->text_rgba_mod), B(b->text_rgba_mod));
            SDL_SetTextureAlphaMod(font, A(b->text_rgba_mod));
        }

        gfx_rendercopy(cs, font, &src, &dest);

        src.x += 6;
        dest.x += 6;
        src.w = 16;
        dest.w = 16;

        for(j = 0; j < b->text->slen; j++)
        {
            if(j)
                dest.x += 16;

            gfx_rendercopy(cs, font, &src, &dest);
        }

        src.x += 16;
        src.w = 6;
        dest.w = 6;
        dest.x += 16;

        gfx_rendercopy(cs, font, &src, &dest);

        SDL_SetTextureColorMod(font, 255, 255, 255);
        SDL_SetTextureAlphaMod(font, 255);

        if(b->highlighted || b->clicked)
        {
            fmt.outlined = false;
            fmt.shadow = true;
            fmt.rgba = 0x000000FF;
        }
        else
        {
            fmt.outlined = true;
            fmt.shadow = false;
            fmt.rgba = b->text_rgba_mod;
        }

        gfx_drawtext(cs, b->text, b->x + 6, b->y + 6, monofont_square, &fmt);
    }

    if(n == cs->gfx_buttons_max)
    {
        free(cs->gfx_buttons);
        cs->gfx_buttons = NULL;
        cs->gfx_buttons_max = 0;
    }

    return 0;
}

int gfx_drawqrsfield(coreState *cs, grid_t *field, unsigned int mode, unsigned int flags, int x, int y)
{
    if(!cs || !field)
        return -1;

    SDL_Texture *tetrion_qs = cs->assets->tetrion_qs_white.tex;
    SDL_Texture *playfield_grid = cs->assets->playfield_grid_alt.tex;
    SDL_Texture *tets = cs->assets->tets_dark_qs.tex;
    SDL_Texture *misc = cs->assets->misc.tex;

    SDL_Rect tdest = {.x = x, .y = y - 48, .w = 274, .h = 416};
    SDL_Rect src = {.x = 0, .y = 0, .w = 16, .h = 16};
    SDL_Rect dest = {.x = 0, .y = 0, .w = 16, .h = 16};

    // SDL_Rect field_dest = {.x = x+16, .y = y+32, .w = 16*12, .h = 16*20};

    qrsdata *q = (qrsdata *)cs->p1game->data;
    int use_deltas = 0;

    int i = 0;
    int j = 0;
    // int k = 0;
    int c = 0;
    // int outline = 0;

    bstring piece_bstr = bfromcstr("A");

    int z = cs->p1game->frame_counter;

    int r = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)(z % 3000) / 3000.0)));
    int g = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 1000) % 3000) / 3000.0)));
    int b = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 2000) % 3000) / 3000.0)));

    // SDL_SetTextureAlphaMod(misc, 180);

    /*if(flags & TETRION_DEATH) {
        tetrion_qs = (asset_by_name(cs, "tetrion/tetrion_death"))->data;
        gfx_rendercopy(cs, tetrion_qs, NULL, &tdest);
    } else {
        SDL_SetTextureColorMod(tetrion_qs, (Uint8)r, (Uint8)g, (Uint8)b);
        gfx_rendercopy(cs, tetrion_qs, NULL, &tdest);
    }*/

    //   if(flags & GFX_G2) {
    //      tets = cs->assets->g2_tets_dark_g2.tex;
    //   }

    switch(mode)
    {
        case MODE_G1_MASTER:
        case MODE_G1_20G:
            tetrion_qs = cs->assets->g1_tetrion_g1.tex;
            break;

        case MODE_G2_MASTER:
            tetrion_qs = cs->assets->g2_tetrion_g2_master.tex;
            break;

        case MODE_G2_DEATH:
            tetrion_qs = cs->assets->g2_tetrion_g2_death.tex;
            break;

        case MODE_G3_TERROR:
            tetrion_qs = cs->assets->g3_tetrion_g3_terror.tex;
            break;

        default:
            break;
    }

    gfx_rendercopy(cs, tetrion_qs, NULL, &tdest);

    if(flags & DRAWFIELD_GRID)
        gfx_rendercopy(cs, playfield_grid, NULL, &tdest);

    /*if(field == cs->p1game->field) {
          use_deltas = 1;
          if(!grid_cells_filled(q->field_deltas)) {
             SDL_SetRenderTarget(cs->screen.renderer, NULL);
             gfx_rendercopy(cs, q->field_tex, NULL, &field_dest);
             return 0;
          }

       SDL_SetRenderTarget(cs->screen.renderer, q->field_tex);
       SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
       SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_NONE);
       }*/

    if(flags & GFX_G2)
        SDL_SetTextureColorMod(misc, 124, 124, 116);
    else
        SDL_SetTextureAlphaMod(misc, 140);

    for(i = 0; i < QRS_FIELD_W; i++)
    { // test feature: last 31 frames of every 91 frames make the stack shine
        for(j = 2; j < QRS_FIELD_H; j++)
        {
            if(flags & TEN_W_TETRION && (i == 0 || i == 11))
                continue;
            /*if(use_deltas && !gridgetcell(q->field_deltas, i, j))
                continue;*/

            c = gridgetcell(field, i, j);
            if(c == GRID_OOB)
                return 1;

            /*if((!c || c == -2) && use_deltas) {
                dest.x = i*16;
                dest.y = (j-2)*16;
                SDL_RenderFillRect(cs->screen.renderer, &dest);
            }*/

            if(c != -2 && c)
            {
                if(c == -5)
                {
                    src.x = 25 * 16;
                }
                else if(c == QRS_FIELD_W_LIMITER)
                {
                    if(!(IS_INBOUNDS(gridgetcell(field, i - 1, j))) && !(IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 27 * 16;
                    else if((IS_INBOUNDS(gridgetcell(field, i - 1, j))) && !(IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 28 * 16;
                    else if(!(IS_INBOUNDS(gridgetcell(field, i - 1, j))) && (IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 29 * 16;
                }
                else if(c & QRS_PIECE_BRACKETS)
                {
                    src.x = 30 * 16;
                }
                else if(c & QRS_PIECE_GEM)
                {
                    src.x = ((c & 0xff) - 1) * 16;
                    src.y = 0;
                    dest.x = x + 16 + (i * 16);
                    dest.y = y + (j * 16);

                    gfx_rendercopy(cs, tets, &src, &dest);
                    src.x = 32 * 16;
                }
                else
                {
                    src.x = ((c & 0xff) - 1) * 16;
                }

                src.y = 0;
                dest.x = x + 16 + (i * 16);
                dest.y = y + (j * 16);

                // piece_bstr->data[0] = c + 'A' - 1;

                // gfx_drawtext(cs, piece_bstr, dest.x, dest.y, (gfx_piece_colors[c-1] * 0x100) + 0xFF); //gfx_rendercopy(cs, tets, &src, &dest);
                if(!(flags & DRAWFIELD_INVISIBLE) || (c == QRS_FIELD_W_LIMITER))
                {
                    // this stuff should be handled more elegantly, without needing access to the qrsdata
                    if(q->state_flags & GAMESTATE_FADING)
                    {
                        if(GET_PIECE_FADE_COUNTER(c) > 10)
                            gfx_rendercopy(cs, tets, &src, &dest);
                        else if(GET_PIECE_FADE_COUNTER(c) > 0)
                        {
                            SDL_SetTextureAlphaMod(tets, GET_PIECE_FADE_COUNTER(c) * 25);
                            gfx_rendercopy(cs, tets, &src, &dest);
                            SDL_SetTextureAlphaMod(tets, 255);
                        }
                    }
                    else
                        gfx_rendercopy(cs, tets, &src, &dest);

                    if((!(c & QRS_PIECE_BRACKETS) || c < 0) && !(flags & DRAWFIELD_NO_OUTLINE))
                    {
                        c = gridgetcell(field, i, j - 1); // above, left, right, below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            src.x = 0;
                            src.y = 48;

                            if(!use_deltas)
                                gfx_rendercopy(cs, misc, &src, &dest);
                            /*else {
                               SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }*/
                        }

                        c = gridgetcell(field, i - 1, j); // above, left, right, below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            src.x = 16;
                            src.y = 48;

                            if(!use_deltas)
                                gfx_rendercopy(cs, misc, &src, &dest);
                            /*else {
                               SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }*/
                        }

                        c = gridgetcell(field, i + 1, j); // above, left, right, below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            src.x = 32;
                            src.y = 48;

                            if(!use_deltas)
                                gfx_rendercopy(cs, misc, &src, &dest);
                            /*else {
                               SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }*/
                        }

                        c = gridgetcell(field, i, j + 1); // above, left, right, below
                        if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                        {
                            src.x = 48;
                            src.y = 48;

                            if(!use_deltas)
                                gfx_rendercopy(cs, misc, &src, &dest);
                            /*else {
                               SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
                            }*/
                        }
                    }
                }

                SDL_SetTextureColorMod(tets, 255, 255, 255);
            }
        }
    }

    /*if(use_deltas) {
       SDL_SetRenderTarget(cs->screen.renderer, NULL);
       SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_BLEND);
       gfx_rendercopy(cs, q->field_tex, NULL, &field_dest);
    }*/

    bdestroy(piece_bstr);

    SDL_SetTextureColorMod(misc, 255, 255, 255);
    SDL_SetTextureAlphaMod(misc, 255);

    return 0;
}

int gfx_drawkeys(coreState *cs, struct keyflags *k, int x, int y, Uint32 rgba)
{
    if(!cs)
        return -1;

    SDL_Texture *font = cs->assets->font.tex;
    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    SDL_Rect src = {0, 80, 16, 16};
    SDL_Rect dest = {0, y, 16, 16};

    bstring text_a = bfromcstr("A");
    bstring text_b = bfromcstr("B");
    bstring text_c = bfromcstr("C");
    bstring text_d = bfromcstr("D");

    struct text_formatting fmt = {.rgba = RGBA_DEFAULT,
                                  .outline_rgba = RGBA_OUTLINE_DEFAULT,
                                  .outlined = true,
                                  .shadow = false,
                                  .size_multiplier = 1.0,
                                  .line_spacing = 1.0,
                                  .align = ALIGN_LEFT,
                                  .wrap_length = 0};

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
    gfx_rendercopy(cs, font, &src, &dest);

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
    gfx_rendercopy(cs, font, &src, &dest);

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
    gfx_rendercopy(cs, font, &src, &dest);

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
    gfx_rendercopy(cs, font, &src, &dest);

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

    bdestroy(text_a);
    bdestroy(text_b);
    bdestroy(text_c);
    bdestroy(text_d);

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}

int gfx_drawtext(coreState *cs, std::string text, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    bstring b = bfromcstr(text.c_str());
    int rc = gfx_drawtext(cs, b, x, y, font, fmt);
    bdestroy(b);
    return rc;
}

int gfx_drawtext(coreState *cs, bstring text, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    if(!text)
        return -1;

    return gfx_drawtext_partial(cs, text, 0, text->slen, x, y, font, fmt);
}

int gfx_drawtext_partial(coreState *cs, bstring text, int pos, int len, int x, int y, png_monofont *font, struct text_formatting *fmt)
{
    if(!cs || !text)
        return -1;

    if(!font)
        font = monofont_fixedsys;

    struct text_formatting fmt_ = {.rgba = RGBA_DEFAULT,
                                   .outline_rgba = RGBA_OUTLINE_DEFAULT,
                                   .outlined = true,
                                   .shadow = false,
                                   .size_multiplier = 1.0,
                                   .line_spacing = 1.0,
                                   .align = ALIGN_LEFT,
                                   .wrap_length = 0};

    if(!fmt)
        fmt = &fmt_;

    SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
    SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, R(fmt->outline_rgba), G(fmt->outline_rgba), B(fmt->outline_rgba));
        SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->outline_rgba));
    }

    SDL_Rect src = {.x = 0, .y = 0, .w = font->char_w, .h = font->char_h};
    SDL_Rect dest = {.x = x, .y = y, .w = fmt->size_multiplier * (float)font->char_w, .h = fmt->size_multiplier * (float)font->char_h};

    int i = 0;

    int linefeeds = 0;
    int last_wrap_line_pos = 0;
    int last_wrap_pos = 0;

    struct bstrList *lines = bsplit(text, '\n');

    bool using_target_tex = false;

    if(SDL_GetRenderTarget(cs->screen.renderer) != NULL)
        using_target_tex = true;

    for(i = pos; i < text->slen && i < len; i++)
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
                    dest.x = x - (fmt->size_multiplier * (float)font->char_w) * (lines->entry[0]->slen);
                    break;

                case ALIGN_CENTER:
                    if(fmt->wrap_length < lines->entry[0]->slen - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = x + (fmt->size_multiplier * (float)font->char_w / 2.0) * (fmt->wrap_length - (lines->entry[0]->slen - last_wrap_line_pos));

                    break;
            }
        }

        if((fmt->wrap_length && i != 0 && (i - last_wrap_pos) % fmt->wrap_length == 0) || text->data[i] == '\n')
        {
            if(text->data[i] == '\n')
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

            dest.y += fmt->line_spacing * fmt->size_multiplier * (float)font->char_h;

            switch(fmt->align)
            {
                default:
                case ALIGN_LEFT:
                    dest.x = x;
                    break;

                case ALIGN_RIGHT:
                    dest.x = x - (font->char_w) * (lines->entry[linefeeds]->slen);
                    break;

                case ALIGN_CENTER:
                    if(fmt->wrap_length < lines->entry[linefeeds]->slen - last_wrap_line_pos)
                        dest.x = x;
                    else
                        dest.x = x + (font->char_w / 2) * (fmt->wrap_length - (lines->entry[linefeeds]->slen - last_wrap_line_pos));

                    break;
            }

            if(text->data[i] == '\n')
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
                SDL_RenderFillRect(cs->screen.renderer, &dest);
                SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);
            }
            else
            {
                gfx_rendercopy(cs, font->sheet, &src, &dest);
            }

            SDL_SetTextureColorMod(font->sheet, R(fmt->rgba), G(fmt->rgba), B(fmt->rgba));
            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
        }

        src.x = font->char_w * (text->data[i] % 32);
        src.y = font->char_h * ((int)(text->data[i] / 32) - 1);
        if(src.y < 0)
        {
            src.x = 31 * font->char_w;
            src.y = 2 * font->char_h;
        }

        if(fmt->shadow && !using_target_tex)
        {
            dest.x -= 2.0 * fmt->size_multiplier;
            dest.y += 2.0 * fmt->size_multiplier;

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba) / 4);
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba) / 4);

            gfx_rendercopy(cs, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
            {
                gfx_rendercopy(cs, font->outline_sheet, &src, &dest);
            }

            dest.x += 2.0 * fmt->size_multiplier;
            dest.y -= 2.0 * fmt->size_multiplier;

            SDL_SetTextureAlphaMod(font->sheet, A(fmt->rgba));
            if(font->outline_sheet)
                SDL_SetTextureAlphaMod(font->outline_sheet, A(fmt->rgba));
        }

        if(using_target_tex)
        {
            SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
            SDL_RenderFillRect(cs->screen.renderer, &dest);
            SDL_RenderCopy(cs->screen.renderer, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                SDL_RenderCopy(cs->screen.renderer, font->outline_sheet, &src, &dest);
        }
        else
        {
            gfx_rendercopy(cs, font->sheet, &src, &dest);

            if(fmt->outlined && font->outline_sheet)
                gfx_rendercopy(cs, font->outline_sheet, &src, &dest);
        }

        dest.x += fmt->size_multiplier * (float)font->char_w;
    }

    if(lines)
        bstrListDestroy(lines);

    SDL_SetTextureColorMod(font->sheet, 255, 255, 255);
    SDL_SetTextureAlphaMod(font->sheet, 255);

    if(font->outline_sheet)
    {
        SDL_SetTextureColorMod(font->outline_sheet, 255, 255, 255);
        SDL_SetTextureAlphaMod(font->outline_sheet, 255);
    }

    return 0;
}

int gfx_drawpiece(coreState *cs, grid_t *field, int field_x, int field_y, piecedef *pd, unsigned int flags, int orient, int x, int y, Uint32 rgba)
{
    if(!cs || !pd)
        return -1;

    if(flags & DRAWPIECE_BRACKETS && flags & DRAWPIECE_LOCKFLASH)
        return 0;

    SDL_Texture *tets;
    SDL_Texture *misc = cs->assets->misc.tex;

    //   if(flags & GFX_G2) {
    //      if(flags & DRAWPIECE_SMALL)
    //         tets = cs->assets->g2_tets_bright_g2_small.tex;
    //      else
    //         tets = cs->assets->g2_tets_bright_g2.tex;
    //   } else {
    if(flags & DRAWPIECE_SMALL)
        tets = cs->assets->tets_bright_qs_small.tex;
    else
        tets = cs->assets->tets_bright_qs.tex;
    //   }

    int size = (flags & DRAWPIECE_SMALL) ? 8 : 16;
    SDL_Rect src = {.x = 0, .y = 0, .w = size, .h = size};
    SDL_Rect dest = {.x = 0, .y = 0, .w = size, .h = size};

    bstring piece_bstr = bfromcstr("A");
    piece_bstr->data[0] = pd->qrs_id + 'A';

    grid_t *g = NULL;

    int i = 0;
    int j = 0;
    int w = pd->rotation_tables[0]->w;
    int h = pd->rotation_tables[0]->h;
    int c = 0;

    int cell_x = 0;
    int cell_y = 0;

    g = pd->rotation_tables[orient & 3];
    src.x = pd->qrs_id * size;

    /*if(flags & DRAWPIECE_IPREVIEW && !(flags & GFX_G2)) {
       y += 8;
    }*/

    SDL_SetTextureColorMod(tets, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(tets, A(rgba));

    for(i = 0; i < w; i++)
    {
        for(j = 0; j < h; j++)
        {
            if(gridgetcell(g, i, j) && (y + (j * 16) > (field_y + 16) || flags & DRAWPIECE_PREVIEW))
            {
                dest.x = x + (i * size);

                if(w == 4 && flags & DRAWPIECE_PREVIEW)
                    dest.y = y + ((j + 1) * size);
                else
                    dest.y = y + (j * size);

                if(flags & DRAWPIECE_BRACKETS || pd->flags & PDBRACKETS)
                    src.x = 30 * size;

                if(flags & DRAWPIECE_LOCKFLASH && !(flags & DRAWPIECE_BRACKETS) && !(pd->flags & PDBRACKETS))
                {
                    src.x = 26 * size;
                    cell_x = (x - field_x) / 16 + i - 1;
                    cell_y = (y - field_y) / 16 + j;
                    if(gridgetcell(field, cell_x, cell_y) > 0 || flags & DRAWPIECE_PREVIEW)
                    {
                        gfx_rendercopy(cs, tets, &src, &dest);

                        if(!(flags & DRAWPIECE_PREVIEW))
                        {
                            c = gridgetcell(field, cell_x, cell_y - 1); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 0;
                                src.y = 48;

                                gfx_rendercopy(cs, misc, &src, &dest);
                            }

                            c = gridgetcell(field, cell_x - 1, cell_y); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 16;
                                src.y = 48;

                                gfx_rendercopy(cs, misc, &src, &dest);
                            }

                            c = gridgetcell(field, cell_x + 1, cell_y); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 32;
                                src.y = 48;

                                gfx_rendercopy(cs, misc, &src, &dest);
                            }

                            c = gridgetcell(field, cell_x, cell_y + 1); // above, left, right, below
                            if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB)
                            {
                                src.x = 48;
                                src.y = 48;

                                gfx_rendercopy(cs, misc, &src, &dest);
                            }

                            src.y = 0;
                        }
                    }
                }
                else
                    // gfx_drawtext(cs, piece_bstr, dest.x, dest.y, (gfx_piece_colors[pd->qrs_id] * 0x100) + A(rgba)); //gfx_rendercopy(cs, tets, &src, &dest);
                    gfx_rendercopy(cs, tets, &src, &dest);
            }
        }
    }

    SDL_SetTextureColorMod(tets, 255, 255, 255);
    SDL_SetTextureAlphaMod(tets, 255);

    bdestroy(piece_bstr);

    return 0;
}

int gfx_drawtimer(coreState *cs, nz_timer *t, int x, Uint32 rgba)
{
    SDL_Texture *font = cs->assets->font.tex;
    qrsdata *q = (qrsdata *)cs->p1game->data;
    int y = q->field_y;

    SDL_Rect src = {.x = 0, .y = 96, .w = 20, .h = 32};
    SDL_Rect dest = {.x = x, .y = 26 * 16 + 8 - QRS_FIELD_Y + y, .w = 20, .h = 32};

    int min = timegetmin(t);
    int sec = timegetsec(t) % 60;
    int csec = (timegetmsec(t) / 10) % 100; // centiseconds

    int i = 0;
    // int n = 0;

    int digits[6];

    digits[0] = min / 10;
    digits[1] = min % 10;
    digits[2] = sec / 10;
    digits[3] = sec % 10;
    digits[4] = csec / 10;
    digits[5] = csec % 10;

    SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    SDL_SetTextureAlphaMod(font, A(rgba));

    for(i = 0; i < 6; i++)
    {
        src.x = digits[i] * 20;
        gfx_rendercopy(cs, font, &src, &dest);
        dest.x += 20;

        if(i == 1 || i == 3)
        {
            src.x = 200; // colon character offset
            gfx_rendercopy(cs, font, &src, &dest);
            dest.x += 20;
        }
    }

    SDL_SetTextureColorMod(font, 255, 255, 255);
    SDL_SetTextureAlphaMod(font, 255);

    return 0;
}
