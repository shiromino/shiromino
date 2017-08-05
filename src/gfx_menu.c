#include <stdio.h>
#include <stdlib.h>
#include "bstrlib.h"
#include <SDL2/SDL.h>

#include "core.h"
#include "game_menu.h"

#include "gfx.h"
#include "gfx_menu.h"

int gfx_drawmenu(game_t *g)
{
	if(!g)
		return -1;
	if(!g->origin)
		return -1;

	coreState *cs = g->origin;

	SDL_Texture *font = (asset_by_name(cs, "font"))->data;
	SDL_Texture *font_thin = (asset_by_name(cs, "font_thin"))->data;
	SDL_Rect src = {.x = 0, .y = 80, .w = 16, .h = 16};
	SDL_Rect dest = {.x = 0, .y = 0, .w = 16, .h = 16};
	SDL_Rect barsrc = {.x = 12*16, .y = 17, .w = 2, .h = 14};
	SDL_Rect bardest = {.x = 0, .y = 0, .w = 2, .h = 14};
	SDL_Rect baroutlinesrc = {.x = 256, .y = 0, .w = 102, .h = 16};
	SDL_Rect baroutlinedest = {.x = 0, .y = 0, .w = 102, .h = 16};

	menudata *d = (menudata *)(g->data);
	struct menu_opt *m = NULL;
	struct multi_opt_data *d2 = NULL;
	struct game_multiopt_data *d3 = NULL;
	struct text_opt_data *d7 = NULL;
	struct toggle_opt_data *d8 = NULL;

	bstring textinput_display = NULL;
	bstring page_bstr = NULL;

	//double mspf = 1000.0 * (1.0/cs->fps);
	//int cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent) / mspf));
	//bstring avg_sleep_ms = bformat("%LF", cs->avg_sleep_ms_recent);
    //bstring ctp_bstr = bformat("%d%%", cpu_time_percentage);
    //bstring ctp_overload_bstr = NULL;

	int i = 0;
	int j = 0;
	int k = 0;
	int mod = 0;

	int initial_opt = 0;
	int final_opt = d->numopts - 1;

	unsigned int text_flags = d->use_target_tex ? GFX_TARGET_TEXTURE_OVERWRITE : 0;

	//gfx_rendercopy(cs, d->target_tex, NULL, NULL);
	//return 0;

	if(d->is_paged) {
		page_bstr = bformat("PAGE %d/%d", d->page + 1, ((d->numopts - 1) / d->page_length) + 1);
		gfx_drawtext(cs, page_bstr, d->page_text_x, d->page_text_y, DRAWTEXT_ALIGN_RIGHT, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);
		bdestroy(page_bstr);

		initial_opt = d->page * d->page_length;
		final_opt = d->page * d->page_length + d->page_length - 1;
		if(final_opt > d->numopts - 1)
			final_opt = d->numopts - 1;
	}

	if(!d->menu)
		return 0;

	if(d->title)
		gfx_drawtext(cs, d->title, d->x, d->y, 0, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);

	if(d->use_target_tex) {
		SDL_SetRenderTarget(cs->screen.renderer, d->target_tex);
	}

	//printf("initial_opt = %d, d->selection = %d (label = %s), d->numopts = %d\n", initial_opt, d->selection, d->menu[d->selection]->label->data, d->numopts);

	for(i = initial_opt; i <= final_opt; i++) {
		if(d->menu[i]) {
			m = d->menu[i];
			if(d->use_target_tex && !m->render_update)
				continue;
			else if(d->use_target_tex && i == initial_opt) {
				SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
				SDL_RenderClear(g->origin->screen.renderer);
			}
			//printf("Drawing menu opt %d with label %s\n", i, m->label->data);
			if(i == d->selection) {
				/*if(cs->obnoxious_text)
					gfx_drawtext(cs, m->label, m->x, m->y, m->label_text_flags|DRAWTEXT_RAINBOW|DRAWTEXT_NO_OUTLINE, m->label_text_rgba, RGBA_OUTLINE_DEFAULT);
				else*/
					gfx_drawtext(cs, m->label, m->x, m->y, text_flags|m->label_text_flags, RGBA_DEFAULT, RGBA_DEFAULT);
			} else
				gfx_drawtext(cs, m->label, m->x, m->y, text_flags|m->label_text_flags, m->label_text_rgba, RGBA_OUTLINE_DEFAULT);

			if(m->type == MENU_MULTIOPT) {
				d2 = m->data;
				gfx_drawtext(cs, d2->labels[d2->selection], m->value_x, m->value_y, text_flags|m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
				if(m->value_text_flags & DRAWTEXT_VALUE_BAR) {
					barsrc.x = 14*16;
					bardest.x = m->value_x;
					baroutlinedest.x = bardest.x;
					bardest.y = m->value_y + 1;
					baroutlinedest.y = m->value_y;
					gfx_rendercopy(cs, font, &baroutlinesrc, &baroutlinedest);

					if(d2->selection > 0) {
						barsrc.x += 1;
						bardest.x += 1;
						barsrc.w = 1;
						bardest.w = 1;
						for(j = 0; j < d2->selection; j++) {
							mod = (200 * (85 - j)) / 100;
							if(mod < 0)
								mod = 0;

							if((i % 3) == 1)
								SDL_SetTextureColorMod(font, 255, mod, mod);
							else if((i % 3) == 2)
								SDL_SetTextureColorMod(font, mod, 255, mod);
							else if((i % 3) == 0)
								SDL_SetTextureColorMod(font, mod, mod, 255);

							gfx_rendercopy(cs, font, &barsrc, &bardest);
							bardest.x += 1;
						}

						SDL_SetTextureColorMod(font, 255, 255, 255);
					}
				}
			}

			if(m->type == MENU_GAME_MULTIOPT) {
				d3 = m->data;
				if(d3->labels) {
					if(d3->labels[d3->selection])
						gfx_drawtext(cs, d3->labels[d3->selection], m->value_x, m->value_y, m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
				}
			}

			if(m->type == MENU_TEXTINPUT) {
				d7 = m->data;

				if(d7->text) {
					if(d7->text->slen) {
						textinput_display = blk2bstr(&d7->text->data[d7->leftmost_position], d7->text->slen - d7->leftmost_position);
						if(textinput_display->slen > d7->visible_chars)
							btrunc(textinput_display, d7->visible_chars);

						if(d7->selection) {
							SDL_SetTextureColorMod(font, 255, 255, 255);
							SDL_SetTextureAlphaMod(font, 255);
							src.x = 17*16 - 1;
							src.y = 32 - 1;
							src.h = 18;
							src.w = (m->value_text_flags & DRAWTEXT_THIN_FONT ? 15 : 18);
							dest.w = src.w;
							dest.h = 18;
							dest.y = m->value_y + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 1 : 0);
							for(k = 0; k < (d7->text->slen - d7->leftmost_position) && k < d7->visible_chars; k++) {
								dest.x = m->value_x + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16)*(k) - 1;

								if(gfx_rendercopy(cs, font, &src, &dest))
									printf("%s\n", SDL_GetError());
							}

							src.h = 16;
							src.w = 16;
							dest.w = 16;
							dest.h = 16;
						}

						if(d7->leftmost_position > 0) {
							src.x = 64;
							src.y = 80;
							if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT) {
								dest.x = m->value_x - ((m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16)*d7->visible_chars) - 16;
							} else {
								dest.x = m->value_x - 16;
							}
							dest.y = m->value_y + 1;

							gfx_rendercopy(cs, font, &src, &dest);
						}

						if(d7->leftmost_position < d7->text->slen - d7->visible_chars) {
							src.x = 80;
							src.y = 80;
							if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT) {
								dest.x = m->value_x;
							} else {
								dest.x = m->value_x + ((m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16)*d7->visible_chars);
							}
							dest.y = m->value_y + 1;

							gfx_rendercopy(cs, font, &src, &dest);
						}

						gfx_drawtext(cs, textinput_display, m->value_x, m->value_y + 1, m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
					}

					if(d7->active) {
						if(m->value_text_flags & DRAWTEXT_THIN_FONT) {
							src.x = 15*13;
							src.y = 2*18;
							if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
								dest.x = m->value_x - 13*((d7->text->slen > d7->visible_chars ? d7->visible_chars : d7->text->slen)) + 13*(d7->position - d7->leftmost_position);
							else
								dest.x = m->value_x + 13*(d7->position - d7->leftmost_position);
							dest.y = m->value_y + 18;

							src.w = 13;
							dest.w = 13;
							src.h = 18;
							dest.h = 18;

							gfx_rendercopy(cs, font_thin, &src, &dest);

							src.w = 16;
							dest.w = 16;
							src.h = 16;
							dest.h = 16;
						} else {
							src.x = 15*16;
							src.y = 32;
							if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
								dest.x = m->value_x - 16*((d7->text->slen > d7->visible_chars ? d7->visible_chars : d7->text->slen)) + 16*(d7->position - d7->leftmost_position);
							else
								dest.x = m->value_x + 16*(d7->position - d7->leftmost_position);
							dest.y = m->value_y + 16;

							gfx_rendercopy(cs, font, &src, &dest);
						}
					}
				}
			}

			if(m->type == MENU_TOGGLE) {
				d8 = m->data;
				if(*(d8->param))
					gfx_drawtext(cs, d8->labels[1], m->value_x, m->value_y, m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
				else
					gfx_drawtext(cs, d8->labels[0], m->value_x, m->value_y, m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
			}
/*
			if(i == d->selection) {
				src.x = 16;
				src.y = 80;
				dest.x = m->x - 16;
				dest.y = m->y;

				gfx_rendercopy(cs, font, &src, &dest);

				src.x = 0;
				dest.x = m->x + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16)*(m->label->slen);
				dest.y = m->y + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 1 : 0);

				if(m->type != MENU_MULTIOPT && m->type != MENU_GAME_MULTIOPT)
					gfx_rendercopy(cs, font, &src, &dest);
			}
*/
		}
	}

	/*gfx_drawtext(cs, ctp_bstr, 640 - 16 + 16 * (1 - ctp_bstr->slen), 2 + 32, DRAWTEXT_SHADOW|DRAWTEXT_NO_OUTLINE, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);

	if(cs->recent_frame_overload >= 0) {
		cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent_array[cs->recent_frame_overload]) / mspf));
		ctp_overload_bstr = bformat("%d%%", cpu_time_percentage);

		gfx_drawtext(cs, ctp_overload_bstr, 640 - 16 + 16 * (1 - ctp_overload_bstr->slen), 18 + 32, DRAWTEXT_SHADOW|DRAWTEXT_NO_OUTLINE, 0xB00000FF, RGBA_OUTLINE_DEFAULT);

		bdestroy(ctp_overload_bstr);
	}*/

	SDL_SetRenderTarget(cs->screen.renderer, NULL);
	SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 255);

	if(d->use_target_tex) {
		for(i = 0; i < d->numopts; i++) {
			d->menu[i]->render_update = 0;
		}
		gfx_rendercopy(cs, d->target_tex, NULL, NULL);
	}

	return 0;
}
