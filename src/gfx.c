#include <stdio.h>
#include <math.h>
#include "bstrlib.h"
#include <SDL2/SDL.h>

#include "core.h"
#include "piecedef.h"
#include "qrs.h"
#include "game_qs.h"
#include "grid.h"
#include "timer.h"
#include "gfx.h"

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

void gfx_message_destroy(gfx_message *m)
{
	if(!m)
		return;

	if(m->text)
		bdestroy(m->text);

	free(m);
}

void gfx_animation_destroy(gfx_animation *a)
{
	if(!a)
		return;

	if(a->name)
		bdestroy(a->name);

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
	return 0;
}

void gfx_quit(coreState *cs)
{
	int i = 0;

	if(cs->gfx_messages) {
		for(i = 0; i < cs->gfx_messages_max; i++) {
			if(cs->gfx_messages[i])
				gfx_message_destroy(cs->gfx_messages[i]);
		}

		free(cs->gfx_messages);
	}

	if(cs->gfx_animations) {
		for(i = 0; i < cs->gfx_animations_max; i++) {
			if(cs->gfx_animations[i])
				gfx_animation_destroy(cs->gfx_animations[i]);
		}

		free(cs->gfx_animations);
	}

    if(cs->gfx_buttons) {
		for(i = 0; i < cs->gfx_buttons_max; i++) {
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
}

int gfx_rendercopy(coreState *cs, SDL_Texture *t, SDL_Rect *src, SDL_Rect *dest_)
{
	SDL_Rect dest;

	//SDL_SetRenderTarget(cs->screen.renderer, cs->screen.target_tex);

	if(dest_) {
		dest.x = dest_->x * cs->settings->video_scale;
		dest.y = dest_->y * cs->settings->video_scale;
		dest.w = dest_->w * cs->settings->video_scale;
		dest.h = dest_->h * cs->settings->video_scale;

		return SDL_RenderCopy(cs->screen.renderer, t, src, &dest);
	} else
		return SDL_RenderCopy(cs->screen.renderer, t, src, dest_);
}

int gfx_start_bg_fade_in(coreState *cs)
{
	if(!cs)
		return -1;

	if(cs->bg)
		SDL_SetTextureColorMod(cs->bg, 0, 0, 0);
	else return 1;

	return 0;
}

int gfx_drawbg(coreState *cs)
{
    //SDL_Texture *bg_darken = (asset_by_name(cs, "bg_darken"))->data;
    //SDL_Texture *anim_bg_frame = NULL;
    //bstring asset_name = NULL;
	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 0;
    //Uint8 a;

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

	if(cs->bg != cs->bg_old) {
		if(cs->bg_old)
			SDL_GetTextureColorMod(cs->bg_old, &r, &g, &b);

		if(r && cs->bg_old) {
			if(r > BG_FADE_RATE) {
				r -= BG_FADE_RATE;
				g -= BG_FADE_RATE;
				b -= BG_FADE_RATE;
			} else {
				r = 0;
				g = 0;
				b = 0;

				SDL_SetTextureColorMod(cs->bg, 0, 0, 0);
			}

			SDL_SetTextureColorMod(cs->bg_old, r, g, b);
			gfx_rendercopy(cs, cs->bg_old, NULL, NULL);
		} else {
            if(!cs->bg)
                return 0;

			SDL_GetTextureColorMod(cs->bg, &r, &g, &b);
			if(r < 255) {
				if(r < 255 - BG_FADE_RATE) {
					r += BG_FADE_RATE;
					g += BG_FADE_RATE;
					b += BG_FADE_RATE;
				} else {
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
	} else {
        if(!cs->bg)
            return 0;

		gfx_rendercopy(cs, cs->bg, NULL, NULL);
    }

	return 0;
}

int gfx_draw_emergency_bg_darken(coreState *cs)
{
    SDL_Texture *bg_darken = (asset_by_name(cs, "bg_darken"))->data;
    SDL_SetTextureColorMod(bg_darken, 0, 0, 0);
    SDL_SetTextureAlphaMod(bg_darken, 210);
    gfx_rendercopy(cs, bg_darken, NULL, NULL);
    SDL_SetTextureColorMod(bg_darken, 255, 255, 255);
    SDL_SetTextureAlphaMod(bg_darken, 255);

    return 0;
}

int gfx_pushmessage(coreState *cs, const char *text, int x, int y, unsigned int flags, unsigned int counter, int (*delete_check)(coreState *), Uint32 rgba)
{
	if(!text)
		return -1;

	gfx_message *m = malloc(sizeof(gfx_message));
	m->text = bfromcstr(text);
	m->x = x;
	m->y = y;
    m->flags = flags;
	m->counter = counter;
    m->delete_check = delete_check;
	m->rgba_mod = rgba;

	cs->gfx_messages_max++;
	cs->gfx_messages = realloc(cs->gfx_messages, cs->gfx_messages_max * sizeof(gfx_message *));

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

	for(i = 0; i < cs->gfx_messages_max; i++) {
		if(!cs->gfx_messages[i]) {
			n++;
			continue;
		}

        m = cs->gfx_messages[i];

        if(type == EMERGENCY_OVERRIDE && !(m->flags & MESSAGE_EMERGENCY))
            continue;
        if(type == 0 && (m->flags & MESSAGE_EMERGENCY))
            continue;

		if(!cs->gfx_messages[i]->counter) {
			gfx_message_destroy(cs->gfx_messages[i]);
			cs->gfx_messages[i] = NULL;
			continue;
		}

        if(m->delete_check) {
            if(m->delete_check(cs)) {
                gfx_message_destroy(cs->gfx_messages[i]);
    			cs->gfx_messages[i] = NULL;
    			continue;
            }
        }

		gfx_drawtext(cs, m->text, m->x, m->y, m->flags, m->rgba_mod, RGBA_OUTLINE_DEFAULT);
        if(m->counter > 0)
            m->counter--;
	}

	if(n == cs->gfx_messages_max) {
		free(cs->gfx_messages);
		cs->gfx_messages = NULL;
		cs->gfx_messages_max = 0;
	}

	return 0;
}

int gfx_pushanimation(coreState *cs, bstring name, int x, int y, int num_frames, int frame_multiplier, Uint32 rgba)
{
	gfx_animation *a = malloc(sizeof(gfx_animation));
	a->name = bstrcpy(name);
	a->x = x;
	a->y = y;
    a->flags = 0;
	a->num_frames = num_frames;
	a->frame_multiplier = frame_multiplier;
	a->rgba_mod = rgba;
	a->counter = 0;

	int i = 0;
	for(i = 0; i < cs->gfx_animations_max; i++) {
		if(cs->gfx_animations[i] == NULL) {
			cs->gfx_animations[i] = a;
			return 0;
		}
	}

	cs->gfx_animations_max++;
	cs->gfx_animations = realloc(cs->gfx_animations, cs->gfx_animations_max * sizeof(gfx_animation *));

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
	bstring bnum = NULL;
	bstring asset_name = NULL;

	for(i = 0; i < cs->gfx_animations_max; i++) {
		if(!cs->gfx_animations[i]) {
			n++;
			continue;
		}

		a = cs->gfx_animations[i];

        if(type == EMERGENCY_OVERRIDE && !(a->flags & ANIMATION_EMERGENCY))
            continue;
        if(type == 0 && (a->flags & ANIMATION_EMERGENCY))
            continue;

		if(a->counter == (unsigned int)(a->frame_multiplier * a->num_frames)) {
			gfx_animation_destroy(a);
			cs->gfx_animations[i] = NULL;
			continue;
		}

		asset_name = bstrcpy(a->name);
		bnum = bformat("%d", a->counter / a->frame_multiplier);
		bconcat(asset_name, bnum);
		t = (asset_by_name(cs, (char *)(asset_name->data)))->data;
		if(!t)
			printf("NULL texture on frame %d (name string: %s)\n", a->counter / a->frame_multiplier, asset_name->data);

		dest.x = a->x;
		dest.y = a->y;
		SDL_QueryTexture(t, NULL, NULL, &dest.w, &dest.h);

		SDL_SetTextureColorMod(t, R(a->rgba_mod), G(a->rgba_mod), B(a->rgba_mod));
		SDL_SetTextureAlphaMod(t, A(a->rgba_mod));
		gfx_rendercopy(cs, t, NULL, &dest);
		SDL_SetTextureAlphaMod(t, 255);
		SDL_SetTextureColorMod(t, 255, 255, 255);

		bdestroy(asset_name);
		bdestroy(bnum);
		a->counter++;
	}

	if(n == cs->gfx_animations_max) {
		free(cs->gfx_animations);
		cs->gfx_animations = NULL;
		cs->gfx_animations_max = 0;
	}

	return 0;
}

int gfx_draw_anim_bg()
{
	return 0;
}

int gfx_createbutton(coreState *cs, const char *text, int x, int y,
                     unsigned int flags, int (*action)(coreState *, void *), int (*delete_check)(coreState *),
                     void *data, Uint32 rgba)
{
	if(!text)
		return -1;

	gfx_button *b = malloc(sizeof(gfx_button));
	b->text = bfromcstr(text);
	b->x = x;
	b->y = y;
    b->w = 2*6 + 16*(b->text->slen);
    b->h = 28;
    b->flags = flags;
    b->highlighted = 0;
    b->clicked = 0;
	b->action = action;
    b->delete_check = delete_check;
    b->data = data;
	b->text_rgba_mod = rgba;

	cs->gfx_buttons_max++;
	cs->gfx_buttons = realloc(cs->gfx_buttons, cs->gfx_buttons_max * sizeof(gfx_button *));

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
    SDL_Texture *font = (asset_by_name(cs, "font"))->data;
    //SDL_Texture *font_no_outline = (asset_by_name(cs, "font_no_outline"))->data;
    SDL_Rect src = {.x = 0, .y = 0, .w = 6, .h = 28};
    SDL_Rect dest = {.x = 0, .y = 0, .w = 6, .h = 28};

	for(i = 0; i < cs->gfx_buttons_max; i++) {
		if(!cs->gfx_buttons[i]) {
			n++;
			continue;
		}

        b = cs->gfx_buttons[i];

        if(type == EMERGENCY_OVERRIDE && !(b->flags & BUTTON_EMERGENCY))
            continue;
        if(type == 0 && (b->flags & BUTTON_EMERGENCY))
            continue;

        if(b->highlighted) {
            if(b->clicked) {
                src.x = 362;
                b->clicked--;
            } else
                src.x = 298;
        } else if(b->clicked) {
            src.x = 362;
            b->clicked--;
        } else
            src.x = 330;

        src.w = 6;
        dest.w = 6;
        src.y = 26;
        dest.x = b->x;
        dest.y = b->y;

        if(b->highlighted) {
            SDL_SetTextureColorMod(font, R(b->text_rgba_mod), G(b->text_rgba_mod), B(b->text_rgba_mod));
            SDL_SetTextureAlphaMod(font, A(b->text_rgba_mod));
        }

        gfx_rendercopy(cs, font, &src, &dest);

        src.x += 6;
        dest.x += 6;
        src.w = 16;
        dest.w = 16;

        for(j = 0; j < b->text->slen; j++) {
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
            gfx_drawtext(cs, b->text, b->x + 6, b->y + 6, DRAWTEXT_NO_OUTLINE|DRAWTEXT_SHADOW|DRAWTEXT_LINEFEED, 0x000000FF, RGBA_OUTLINE_DEFAULT);
        else
            gfx_drawtext(cs, b->text, b->x + 6, b->y + 6, DRAWTEXT_LINEFEED, b->text_rgba_mod, RGBA_OUTLINE_DEFAULT);
	}

	if(n == cs->gfx_buttons_max) {
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

	SDL_Texture *tetrion_qs = (asset_by_name(cs, "tetrion_qs_white"))->data;
	SDL_Texture *playfield_grid = (asset_by_name(cs, "playfield_grid_alt"))->data;
	SDL_Texture *tets = (asset_by_name(cs, "tets_dark_qs"))->data;
	SDL_Texture *misc = (asset_by_name(cs, "misc"))->data;

	SDL_Rect tdest = {.x = x, .y = y - 48, .w = 274, .h = 416};
	SDL_Rect src = {.x = 0, .y = 0, .w = 16, .h = 16};
	SDL_Rect dest = {.x = 0, .y = 0, .w = 16, .h = 16};

	//SDL_Rect field_dest = {.x = x+16, .y = y+32, .w = 16*12, .h = 16*20};

	//qrsdata *q = cs->p1game->data;
	int use_deltas = 0;

	int i = 0;
	int j = 0;
    //int k = 0;
	int c = 0;
	//int outline = 0;

	bstring piece_bstr = bfromcstr("A");

    int z = cs->p1game->frame_counter;

    int r = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)(z % 3000) / 3000.0) ));
    int g = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 1000) % 3000) / 3000.0) ));
    int b = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 2000) % 3000) / 3000.0) ));

	//SDL_SetTextureAlphaMod(misc, 180);

	/*if(flags & TETRION_DEATH) {
		tetrion_qs = (asset_by_name(cs, "tetrion/tetrion_death"))->data;
		gfx_rendercopy(cs, tetrion_qs, NULL, &tdest);
	} else {
		SDL_SetTextureColorMod(tetrion_qs, (Uint8)r, (Uint8)g, (Uint8)b);
		gfx_rendercopy(cs, tetrion_qs, NULL, &tdest);
	}*/

	if(flags & GFX_G2) {
		tets = (asset_by_name(cs, "g2/tets_dark_g2"))->data;
	}

	switch(mode) {
		case MODE_G1_MASTER:
		case MODE_G1_20G:
			tetrion_qs = (asset_by_name(cs, "g1/tetrion_g1"))->data;
			break;
		case MODE_G2_DEATH:
			tetrion_qs = (asset_by_name(cs, "g2/tetrion_g2_death"))->data;
			break;

		case MODE_G3_TERROR:
			tetrion_qs = (asset_by_name(cs, "g3/tetrion_g3_terror"))->data;
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

	for(i = 0; i < QRS_FIELD_W; i++) {     // test feature: last 31 frames of every 91 frames make the stack shine
		for(j = 2; j < QRS_FIELD_H; j++) {
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

			if(c != -2 && c) {
                if(c == -5) {
                    src.x = 25 * 16;
                } else if(c == QRS_FIELD_W_LIMITER) {
                    if(!(IS_INBOUNDS(gridgetcell(field, i - 1, j))) && !(IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 27 * 16;
                    else if((IS_INBOUNDS(gridgetcell(field, i - 1, j))) && !(IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 28 * 16;
                    else if(!(IS_INBOUNDS(gridgetcell(field, i - 1, j))) && (IS_INBOUNDS(gridgetcell(field, i + 1, j))))
                        src.x = 29 * 16;
                } else if(c & QRS_PIECE_RAINBOW) {
                    src.x = 26 * 16;
                    c &= ~QRS_PIECE_RAINBOW;
                    r = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z + 53*c) % 300) / 300.0) ));
                    g = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 100 + 53*c) % 300) / 300.0) ));
                    b = 127 + (int)(127.0 * sin(2.0 * 3.14159265358979 * ((double)((z - 200 + 53*c) % 300) / 300.0) ));
                    SDL_SetTextureColorMod(tets, (Uint8)g, (Uint8)b, (Uint8)r);
                } else if(c & QRS_PIECE_BRACKETS) {
                    src.x = 30*16;
                } else {
                    src.x = (c - 1) * 16;
                    /*if(z % 91 > 59) {
                        for(k = 0; k < 20; k++) {
                            if(i == ((z % 91) - 79 + k) && j == (2 + k))
                                src.x = 26*16;
                        }
                    }*/
                }

				src.y = 0;

				if(!use_deltas) {
					dest.x = x + 16 + (i * 16);
					dest.y = y + (j * 16);
				}/* else {
					dest.x = i*16;
					dest.y = j*16;
				}*/

				//piece_bstr->data[0] = c + 'A' - 1;

				//gfx_drawtext(cs, piece_bstr, dest.x, dest.y, (gfx_piece_colors[c-1] * 0x100) + 0xFF); //gfx_rendercopy(cs, tets, &src, &dest);
                if(!(flags & DRAWFIELD_INVISIBLE) || (c == QRS_FIELD_W_LIMITER)) {
					if(!use_deltas)
                    	gfx_rendercopy(cs, tets, &src, &dest);
					/*else {
						SDL_RenderCopy(cs->screen.renderer, tets, &src, &dest);
					}*/

					if((!(c & QRS_PIECE_BRACKETS) || c < 0) && !(flags & DRAWFIELD_NO_OUTLINE)) {
						c = gridgetcell(field, i, j - 1);	//above, left, right, below
						if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
							src.x = 0;
							src.y = 48;

							if(!use_deltas)
		                    	gfx_rendercopy(cs, misc, &src, &dest);
							/*else {
								SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
							}*/
						}

						c = gridgetcell(field, i - 1, j);	//above, left, right, below
						if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
							src.x = 16;
							src.y = 48;

							if(!use_deltas)
		                    	gfx_rendercopy(cs, misc, &src, &dest);
							/*else {
								SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
							}*/
						}

						c = gridgetcell(field, i + 1, j);	//above, left, right, below
						if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
							src.x = 32;
							src.y = 48;

							if(!use_deltas)
		                    	gfx_rendercopy(cs, misc, &src, &dest);
							/*else {
								SDL_RenderCopy(cs->screen.renderer, misc, &src, &dest);
							}*/
						}

						c = gridgetcell(field, i, j + 1);	//above, left, right, below
						if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
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
			} /*else if(!c && !(flags & DRAWFIELD_NO_OUTLINE) && !(flags & DRAWFIELD_INVISIBLE) && !(flags & DRAWFIELD_BRACKETS)) {        // i corresponds to x, j to y
				outline = 0;         // 8 above, 4 left, 2 right, 1 below
                                     // second pass: 8 left and above, 4 right and above, 2 left and below, 1 right and below
                // none = 0, below = 1, right = 2, right & below = 3, left = 4, left & below = 5, left & right = 6, left & right & below = 7,
                // above = 8, above & below = 9, above & right = 10, above & right & below = 11, above & left = 12, above & left & below = 13, above & left & right = 14, all = 15

                // none = 0, BR = 1, BL = 2, BL & BR = 3, TR = 4, TR & BR = 5, TR & BL = 6, TR & BL & BR = 7,
                // TL = 8, TL & BR = 9, TL & BL = 10, TL & BL & BR = 11, TL & TR = 12, TL & TR & BR = 13, TL & TR & BL = 14, TL & TR & BL & BR = 15

				c = gridgetcell(field, i, j - 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 8;

				c = gridgetcell(field, i - 1, j);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 4;

				c = gridgetcell(field, i + 1, j);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 2;

				c = gridgetcell(field, i, j + 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 1;

				src.x = outline * 16;
				src.y = 0;

				dest.x = x + 16 + (i * 16);
				dest.y = 64 + (j * 16);

				gfx_rendercopy(cs, misc, &src, &dest);

				outline = 0;

				c = gridgetcell(field, i - 1, j - 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 8;

				c = gridgetcell(field, i + 1, j - 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 4;

				c = gridgetcell(field, i - 1, j + 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 2;

				c = gridgetcell(field, i + 1, j + 1);
				if(IS_STACK(c) && !(c & QRS_PIECE_BRACKETS))
					outline |= 1;

				src.x = outline * 16;
				src.y = 16;

				gfx_rendercopy(cs, misc, &src, &dest);
			}/*/
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

	SDL_Texture *font = (asset_by_name(cs, "font"))->data;
	SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
	SDL_SetTextureAlphaMod(font, A(rgba));

	SDL_Rect src = {0, 80, 16, 16};
	SDL_Rect dest = {0, y, 16, 16};

	bstring text_a = bfromcstr("A");
	bstring text_b = bfromcstr("B");
	bstring text_c = bfromcstr("C");
	bstring text_d = bfromcstr("D");

	if(k->left) {
		SDL_SetTextureColorMod(font, 255, 255, 255);
	} else {
		SDL_SetTextureColorMod(font, 40, 40, 40);
	}

	src.x = 0;
	dest.x = x;
	gfx_rendercopy(cs, font, &src, &dest);

	if(k->right) {
		SDL_SetTextureColorMod(font, 255, 255, 255);
	} else {
		SDL_SetTextureColorMod(font, 40, 40, 40);
	}

	src.x = 16;
	dest.x = x + 16;
	gfx_rendercopy(cs, font, &src, &dest);

	if(k->up) {
		SDL_SetTextureColorMod(font, 255, 255, 255);
	} else {
		SDL_SetTextureColorMod(font, 40, 40, 40);
	}

	src.x = 32;
	dest.x = x + 32;
	gfx_rendercopy(cs, font, &src, &dest);

	if(k->down) {
		SDL_SetTextureColorMod(font, 255, 255, 255);
	} else {
		SDL_SetTextureColorMod(font, 40, 40, 40);
	}

	src.x = 48;
	dest.x = x + 48;
	gfx_rendercopy(cs, font, &src, &dest);

	SDL_SetTextureColorMod(font, 255, 255, 255);

	if(k->a)
		gfx_drawtext(cs, text_a, x + 64, y, 0, rgba, RGBA_OUTLINE_DEFAULT);
	else
		gfx_drawtext(cs, text_a, x + 64, y, 0, 0x282828FF, RGBA_OUTLINE_DEFAULT);

	if(k->b)
		gfx_drawtext(cs, text_b, x + 80, y, 0, rgba, RGBA_OUTLINE_DEFAULT);
	else
		gfx_drawtext(cs, text_b, x + 80, y, 0, 0x282828FF, RGBA_OUTLINE_DEFAULT);

	if(k->c)
		gfx_drawtext(cs, text_c, x + 96, y, 0, rgba, RGBA_OUTLINE_DEFAULT);
	else
		gfx_drawtext(cs, text_c, x + 96, y, 0, 0x282828FF, RGBA_OUTLINE_DEFAULT);

	if(k->d)
		gfx_drawtext(cs, text_d, x + 112, y, 0, rgba, RGBA_OUTLINE_DEFAULT);
	else
		gfx_drawtext(cs, text_d, x + 112, y, 0, 0x282828FF, RGBA_OUTLINE_DEFAULT);

	bdestroy(text_a);
	bdestroy(text_b);
	bdestroy(text_c);
	bdestroy(text_d);

	SDL_SetTextureColorMod(font, 255, 255, 255);
	SDL_SetTextureAlphaMod(font, 255);

	return 0;
}

int gfx_drawtext(coreState *cs, bstring text, int x, int y, unsigned int flags, Uint32 rgba, Uint32 outline_rgba)
{
    if(!cs || !text)
        return -1;

    return gfx_drawtext_partial(cs, text, 0, text->slen, x, y, flags, rgba, outline_rgba);
}

int gfx_drawtext_partial(coreState *cs, bstring text, int pos, int len, int x, int y, unsigned int flags, Uint32 rgba, Uint32 outline_rgba)
{
	if(!cs || !text)
		return -1;

	SDL_Texture *font = (asset_by_name(cs, "font_no_outline"))->data;
    SDL_Texture *font_outline_only = (asset_by_name(cs, "font_outline_only"))->data;
    int font_h = 16;
    int font_w = 16;

    int z = cs->frames;

    int r = RAINBOW(z, 210, 0);
    int g = RAINBOW(z, 210, 70);
    int b = RAINBOW(z, 210, 140);

    Uint32 rgba_negative = RGBA_NEGATIVE(rgba);

    int x_delta = ((cs->frames-20)/33) % 8;
    if(x_delta == 0) {
        x_delta = -2;
    } else if(x_delta == 1) {
        x_delta = -1;
    } else if(x_delta == 2) {
        x_delta = 0;
    } else if(x_delta == 3) {
        x_delta = 1;
    } else if(x_delta == 4) {
        x_delta = 2;
    } else if(x_delta == 5) {
        x_delta = 1;
    } else if(x_delta == 6) {
        x_delta = 0;
    } else if(x_delta == 7) {
        x_delta = -1;
    }

    int y_delta = ((cs->frames-20)/33) % 8;
    if(y_delta == 0) {
        y_delta = 0;
    } else if(y_delta == 1) {
        y_delta = -1;
    } else if(y_delta == 2) {
        y_delta = -2;
    } else if(y_delta == 3) {
        y_delta = -1;
    } else if(y_delta == 4) {
        y_delta = 0;
    } else if(y_delta == 5) {
        y_delta = 1;
    } else if(y_delta == 6) {
        y_delta = 2;
    } else if(y_delta == 7) {
        y_delta = 1;
    }

    int num_shadows = (cs->frames % 33); //8 frames for 4, 6 for 3, 4 for 2, 2 for 1, 1 for 0. 8 + 6 + 4 + 2 + 1 + 2 + 4 + 6 = 33
    if(num_shadows < 8) {
        num_shadows = 4;
    } else if(num_shadows < 14) {
        num_shadows = 3;
    } else if(num_shadows < 18) {
        num_shadows = 2;
    } else if(num_shadows < 20) {
        num_shadows = 1;
    } else if(num_shadows == 20) {
        num_shadows = 0;
    } else if(num_shadows < 23) {
        num_shadows = 1;
    } else if(num_shadows < 27) {
        num_shadows = 2;
    } else if(num_shadows < 33) {
        num_shadows = 3;
    } else {
        num_shadows = 0;
    }

    if(!cs->obnoxious_text)
        num_shadows = 0;

    if(flags & DRAWTEXT_THIN_FONT) {
        font = (asset_by_name(cs, "font_thin_no_outline"))->data;
        font_outline_only = (asset_by_name(cs, "font_thin_outline_only"))->data;
        font_w = 13;
        font_h = 18;
    } else if(flags & DRAWTEXT_TINY_FONT) {
		font = (asset_by_name(cs, "font_tiny"))->data;
		font_outline_only = NULL;
		font_w = 6;
		font_h = 5;
	} else if(flags & DRAWTEXT_SMALL_FONT) {
		font = (asset_by_name(cs, "font_small"))->data;
		font_outline_only = NULL;
		font_w = 12;
		font_h = 10;
	}

    if(flags & DRAWTEXT_RAINBOW) {
        SDL_SetTextureColorMod(font, r, g, b);
    	SDL_SetTextureAlphaMod(font, 255);

        rgba_negative = (RGBA_NEGATIVE((r*0x1000000) + (g*0x10000) + (b*0x100))) + A(rgba);
    } else {
        SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
    	SDL_SetTextureAlphaMod(font, A(rgba));
        num_shadows = 0;
    }

    if(flags & DRAWTEXT_NEGATIVE_OUTLINE) {
        SDL_SetTextureColorMod(font_outline_only, R(rgba_negative), G(rgba_negative), B(rgba_negative));
        SDL_SetTextureAlphaMod(font_outline_only, A(rgba_negative));
    } else {
        SDL_SetTextureColorMod(font_outline_only, R(outline_rgba), G(outline_rgba), B(outline_rgba));
        SDL_SetTextureAlphaMod(font_outline_only, A(outline_rgba));
    }

	SDL_Rect src = {.x = 0, .y = 0, .w = font_w, .h = font_h};
	SDL_Rect dest = {.x = x, .y = y, .w = font_w, .h = font_h};

	int i = 0;
    int j = 0;
    int k = 0;
    int x_orig = x;
    int y_orig = y;
    int linefeeds = 0;
    int message_char_width = 0;

    struct bstrList *lines = bsplit(text, '\n');

    if(flags & DRAWTEXT_ALIGN_RIGHT) {
        dest.x = x - (font_w)*(lines->entry[0]->slen);
    }

    for(k = 0; k < (cs->obnoxious_text ? 2 : 1)*num_shadows + 1; k++) {
        if(flags & DRAWTEXT_RAINBOW) {
            x = x_orig - num_shadows * (cs->obnoxious_text ? x_delta : 4);
            y = y_orig + num_shadows * (cs->obnoxious_text ? y_delta : 4);
            x += (cs->obnoxious_text ? x_delta : 4) * k;
            y -= (cs->obnoxious_text ? y_delta : 4) * k;
            dest.x = x;
            dest.y = y;
            SDL_SetTextureAlphaMod(font, 255 - 50*(abs(num_shadows - k)));
        }

        if(flags & DRAWTEXT_RAINBOW && !cs->obnoxious_text)
            SDL_SetTextureAlphaMod(font, 255 - 160*(abs(num_shadows - k)));

        for(i = pos; i < text->slen && i < len; i++) {
    		if(text->data[i] >= 'A' && text->data[i] <= 'P') {		// const char is really just a number, so this is legal but awkward
    			src.y = 0;
    			src.x = (text->data[i] - 'A') * font_w;
    		} else if(text->data[i] >= 'Q' && text->data[i] <= 'Z') {
    			src.y = 1*font_h;
    			src.x = (text->data[i] - 'Q') * font_w;
    		} else if(text->data[i] >= '0' && text->data[i] <= '9') {
    			src.y = 2*font_h;
    			src.x = (text->data[i] - '0') * font_w;
    		} else if(text->data[i] >= ' ' && text->data[i] <= '/') {
    			src.y = 3*font_h;
    			src.x = (text->data[i] - ' ') * font_w;
    		} else if(text->data[i] >= ':' && text->data[i] <= '?') {
    			src.y = 4*font_h;
    			src.x = (text->data[i] - ':') * font_w;
    		} else if(text->data[i] == 'a') {
    			src.y = 1*font_h;
    			src.x = 10*font_w;
    		} else if(text->data[i] == 'b') {
    			src.y = 1*font_h;
    			src.x = 11*font_w;
    		} else if(text->data[i] == 'm') {
    			src.y = 1*font_h;
    			src.x = 12*font_w;
    		} else if(text->data[i] == '\n' && flags & DRAWTEXT_LINEFEED) {
                linefeeds++;
                dest.y = y + font_h;

                if(flags & DRAWTEXT_CENTERED) {
                    for(j = 0; j < lines->qty; j++) {
                        if(lines->entry[j]->slen > message_char_width)
                            message_char_width = lines->entry[j]->slen;
                    }

                    dest.x = x + (font_w/2)*(message_char_width - lines->entry[linefeeds]->slen);
                } else if(flags & DRAWTEXT_ALIGN_RIGHT) {
                    dest.x = x - (font_w)*(lines->entry[linefeeds]->slen);
                } else
                    dest.x = x;

                continue;
            } else {
                src.y = 1*font_h;
                src.x = 15*font_w;
            }

            if(flags & DRAWTEXT_SHADOW && !(flags & GFX_TARGET_TEXTURE_OVERWRITE)) {
                dest.x -= 2;
                dest.y += 2;

                SDL_SetTextureAlphaMod(font, A(rgba) / 4);
                SDL_SetTextureAlphaMod(font_outline_only, A(rgba) / 4);

				gfx_rendercopy(cs, font, &src, &dest);

                if(!(flags & DRAWTEXT_NO_OUTLINE)) {
                    gfx_rendercopy(cs, font_outline_only, &src, &dest);
				}

                dest.x += 2;
                dest.y -= 2;

                SDL_SetTextureAlphaMod(font, A(rgba));
                SDL_SetTextureAlphaMod(font_outline_only, A(rgba));
            }

			if(flags & GFX_TARGET_TEXTURE_OVERWRITE) {
				SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0);
				SDL_RenderFillRect(cs->screen.renderer, &dest);
				SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
			} else
				gfx_rendercopy(cs, font, &src, &dest);

            if(!(flags & DRAWTEXT_NO_OUTLINE)) {
				if(flags & GFX_TARGET_TEXTURE_OVERWRITE) {
					SDL_RenderCopy(cs->screen.renderer, font_outline_only, &src, &dest);
				} else
                	gfx_rendercopy(cs, font_outline_only, &src, &dest);
			}

            dest.x += font_w;
    	}
    }

	if(lines)
		bstrListDestroy(lines);

	// printf("Successfully drew text\n");

	SDL_SetTextureColorMod(font, 255, 255, 255);
	SDL_SetTextureAlphaMod(font, 255);

    SDL_SetTextureColorMod(font_outline_only, 255, 255, 255);
	SDL_SetTextureAlphaMod(font_outline_only, 255);

	return 0;
}

int gfx_drawpiece(coreState *cs, grid_t *field, int field_x, int field_y, piecedef *pd, unsigned int flags, int orient, int x, int y, Uint32 rgba)
{
	if(!cs || !pd)
		return -1;

	if(flags & DRAWPIECE_BRACKETS && flags & DRAWPIECE_LOCKFLASH)
		return 0;

	SDL_Texture *tets;
	SDL_Texture *misc = (asset_by_name(cs, "misc"))->data;

	if(flags & GFX_G2) {
		if(flags & DRAWPIECE_SMALL)
			tets = (asset_by_name(cs, "g2/tets_bright_g2_small"))->data;
		else
			tets = (asset_by_name(cs, "g2/tets_bright_g2"))->data;
	} else {
		if(flags & DRAWPIECE_SMALL)
			tets = (asset_by_name(cs, "tets_bright_qs_small"))->data;
		else
			tets = (asset_by_name(cs, "tets_bright_qs"))->data;
	}

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

	for(i = 0; i < w; i++) {
		for(j = 0; j < h; j++) {
			if(gridgetcell(g, i, j) && (y + (j*16) > (field_y + 16) || flags & DRAWPIECE_PREVIEW)) {
				dest.x = x + (i * size);

				if(w == 4 && flags & DRAWPIECE_PREVIEW)
					dest.y = y + ((j + 1) * size);
				else
					dest.y = y + (j * size);

                if(flags & DRAWPIECE_BRACKETS || pd->flags & PDBRACKETS)
                    src.x = 30*size;

				if(flags & DRAWPIECE_LOCKFLASH && !(flags & DRAWPIECE_BRACKETS) && !(pd->flags & PDBRACKETS)) {
					src.x = 26*size;
					cell_x = (x - field_x)/16 + i - 1;
					cell_y = (y - field_y)/16 + j;
					if(gridgetcell(field, cell_x, cell_y) > 0 || flags & DRAWPIECE_PREVIEW) {
						gfx_rendercopy(cs, tets, &src, &dest);

						if(!(flags & DRAWPIECE_PREVIEW)) {
							c = gridgetcell(field, cell_x, cell_y - 1);	//above, left, right, below
							if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
								src.x = 0;
								src.y = 48;

								gfx_rendercopy(cs, misc, &src, &dest);
							}

							c = gridgetcell(field, cell_x - 1, cell_y);	//above, left, right, below
							if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
								src.x = 16;
								src.y = 48;

								gfx_rendercopy(cs, misc, &src, &dest);
							}

							c = gridgetcell(field, cell_x + 1, cell_y);	//above, left, right, below
							if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
								src.x = 32;
								src.y = 48;

								gfx_rendercopy(cs, misc, &src, &dest);
							}

							c = gridgetcell(field, cell_x, cell_y + 1);	//above, left, right, below
							if(!IS_STACK(c) && c != QRS_FIELD_W_LIMITER && c != GRID_OOB) {
								src.x = 48;
								src.y = 48;

								gfx_rendercopy(cs, misc, &src, &dest);
							}

							src.y = 0;
						}
					}
				} else
					//gfx_drawtext(cs, piece_bstr, dest.x, dest.y, (gfx_piece_colors[pd->qrs_id] * 0x100) + A(rgba)); //gfx_rendercopy(cs, tets, &src, &dest);
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
	SDL_Texture *font = (asset_by_name(cs, "font"))->data;
	qrsdata *q = cs->p1game->data;
	int y = q->field_y;

	SDL_Rect src = {.x = 0, .y = 96, .w = 20, .h = 32};
	SDL_Rect dest = {.x = x, .y = 27 * 16 + 4 - QRS_FIELD_Y + y, .w = 20, .h = 32};

	int min = timegetmin(t);
	int sec = timegetsec(t) % 60;
	int csec = (timegetmsec(t) / 10) % 100;		// centiseconds

	int i = 0;
	//int n = 0;

	int digits[6];

	digits[0] = min / 10;
	digits[1] = min % 10;
	digits[2] = sec / 10;
	digits[3] = sec % 10;
	digits[4] = csec / 10;
	digits[5] = csec % 10;

	SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
	SDL_SetTextureAlphaMod(font, A(rgba));

	for(i = 0; i < 6; i++) {
		src.x = digits[i] * 20;
		gfx_rendercopy(cs, font, &src, &dest);
		dest.x += 20;

		if(i == 1 || i == 3) {
			src.x = 200;		// colon character offset
			gfx_rendercopy(cs, font, &src, &dest);
			dest.x += 20;
		}
	}

	SDL_SetTextureColorMod(font, 255, 255, 255);
	SDL_SetTextureAlphaMod(font, 255);

	return 0;
}

int gfx_drawtime(coreState *cs, long time, int x, int y, Uint32 rgba)
{
	if(!cs)
		return -1;

	SDL_Texture *font = (asset_by_name(cs, "font"))->data;
	SDL_SetTextureColorMod(font, R(rgba), G(rgba), B(rgba));
	SDL_SetTextureAlphaMod(font, A(rgba));

	SDL_Rect src = {.x = 0, .y = 478, .w = 13, .h = 18};
	SDL_Rect dest = {.x = x, .y = y, .w = 13, .h = 18};

	nz_timer *t = nz_timer_create(60);
	t->time = time;

	int min = timegetmin(t);
	int sec = timegetsec(t) % 60;
	int csec = (timegetmsec(t) / 10) % 100;		// centiseconds

	int digits[6];

	digits[0] = min / 10;
	digits[1] = min % 10;
	digits[2] = sec / 10;
	digits[3] = sec % 10;
	digits[4] = csec / 10;
	digits[5] = csec % 10;

	int i = 0;

	for(i = 0; i < 6; i++) {
		src.x = digits[i] * 13;
		gfx_rendercopy(cs, font, &src, &dest);
		dest.x += 13;

		if(i == 1 || i == 3) {
			src.x = 13*10;		// colon character offset
			src.w = 7;
			dest.w = 7;
			gfx_rendercopy(cs, font, &src, &dest);
			dest.x += 7;
			dest.w = 13;
			src.w = 13;
		}
	}

	SDL_SetTextureColorMod(font, 255, 255, 255);
	SDL_SetTextureAlphaMod(font, 255);

	nz_timer_destroy(t);

	return 0;
}
