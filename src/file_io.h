#ifndef _file_io_h
#define _file_io_h

#include <SDL2/SDL.h>
#include "bstrlib.h"
#include "core.h"

#define OPTION_INVALID -255

#define MINIMUM_REPLAY_SIZE (long)(REPLAY_HEADER_SIZE + sizeof(struct keyflags))
#define REPLAY_HEADER_SIZE (6*sizeof(int) + 3*sizeof(long))

struct settings *parse_cfg(const char *filename);
long get_cfg_option(struct bstrList *lines, bstring label);
char *get_cfg_string(struct bstrList *lines, bstring label);
struct bstrList *split_file(const char *filename);

struct bindings *get_cfg_bindings(struct bstrList *lines);
int get_asset_volume(struct bstrList *lines, bstring asset_name);
long parse_long(const char *str);
SDL_Keycode bstr_sdlk(bstring b);

#endif
