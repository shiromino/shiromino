#ifndef _file_io_h
#define _file_io_h

#include <SDL2/SDL.h>
#include "core.h"

#include <string>
#include <vector>

#define OPTION_INVALID -255

#define MINIMUM_REPLAY_SIZE (long)(REPLAY_HEADER_SIZE + sizeof(struct keyflags))
#define REPLAY_HEADER_SIZE (6*sizeof(int) + 3*sizeof(long))

struct settings *parse_cfg(const char *filename);
long get_cfg_option(std::vector<std::string>& lines, std::string label);
char *get_cfg_string(std::vector<std::string>& lines, std::string label);
std::vector<std::string> split_file(const char *filename);

struct bindings *get_cfg_bindings(std::vector<std::string>& lines);
int get_asset_volume(std::vector<std::string>& lines, std::string asset_name);
long parse_long(const char *str);
SDL_Keycode str_sdlk(std::string str);

#endif
