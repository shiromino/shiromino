#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "zed_dbg.h"
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include "stringtools.hpp"

#include "core.h"

using namespace std;

struct settings *parse_cfg(const char *filename)
{
    if(!filename)
    {
        return NULL;
    }

    struct settings *s = (struct settings *)malloc(sizeof(struct settings));
    vector<string> cfg_file_lines = split_file(filename);

    if(cfg_file_lines.empty())
    {
        printf("Error splitting config file\n");
    }

    string sfxvolume = "SFXVOLUME";
    string musicvolume = "MUSICVOLUME";
    string mastervolume = "MASTERVOLUME";
    string home_path = "HOME_PATH";
    string videoscale = "VIDEOSCALE";
    string player_name = "PLAYERNAME";
    string fscreen = "FULLSCREEN";
    string videostretch = "VIDEOSTRETCH";

    s->keybinds = get_cfg_bindings(cfg_file_lines);

    s->sfx_volume = get_cfg_option(cfg_file_lines, sfxvolume);
    s->mus_volume = get_cfg_option(cfg_file_lines, musicvolume);
    s->master_volume = get_cfg_option(cfg_file_lines, mastervolume);
    s->video_scale = get_cfg_option(cfg_file_lines, videoscale);
    s->home_path = get_cfg_string(cfg_file_lines, home_path);

    int fscreenInt = get_cfg_option(cfg_file_lines, fscreen);
    if(fscreenInt != OPTION_INVALID)
    {
        s->fullscreen = fscreenInt >= 1 ? true : false;
    }
    else
    {
        s->fullscreen = defaultsettings.fullscreen;
    }

    int vstretchInt = get_cfg_option(cfg_file_lines, videostretch);
    if(vstretchInt != OPTION_INVALID)
    {
        s->video_stretch = vstretchInt >= 1 ? true : false;
    }
    else
    {
        s->video_stretch = defaultsettings.video_stretch;
    }

    s->player_name = get_cfg_string(cfg_file_lines, player_name);
    if(s->player_name == NULL)
    {
        log_info("Could not find %s setting in config file. Using default player name \"%s\"", player_name.c_str(), defaultsettings.player_name);
        s->player_name = defaultsettings.player_name;
    }

    if(s->sfx_volume == OPTION_INVALID || s->sfx_volume < 0 || s->sfx_volume > 100)
    {
        s->sfx_volume = 100;
    }
    if(s->mus_volume == OPTION_INVALID || s->mus_volume < 0 || s->mus_volume > 100)
    {
        s->mus_volume = 100;
    }
    if(s->master_volume == OPTION_INVALID || s->master_volume < 0 || s->master_volume > 100)
    {
        s->master_volume = 100;
    }
    if(s->video_scale == OPTION_INVALID || s->video_scale < 1 || s->video_scale > 5)
    {
        s->video_scale = 1;
    }

    return s;
}

long get_cfg_option(vector<string>& lines, string label)
{
    for(auto str : lines)
    {
        if(str[0] == '#')
        {
            continue;
        }

        if(str.find(label) == 0)
        {
            vector<string> tokens = strtools::words(str);
            if(!tokens.empty())
            {
                return parse_long(tokens.back().c_str());
            }
            else
            {
                return OPTION_INVALID;
            }
        }
    }

    return OPTION_INVALID;
}

char *get_cfg_string(vector<string>& lines, string label)
{
    for(auto str : lines)
    {
        if(str[0] == '#')
        {
            continue;
        }

        if(str.find(label) == 0)
        {
            vector<string> tokens = strtools::words(str);
            if(!tokens.empty())
            {
                char *str = (char *)malloc(tokens.back().length() + 1);
                strcpy(str, tokens.back().c_str());
                return str;
            }
            else
            {
                return NULL;
            }
        }
    }

    return NULL;
}

vector<string> split_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f)
    {
        return {};
    }

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    char *strbuf = (char *)malloc(len + 1);
    fseek(f, 0, SEEK_SET);
    fread(strbuf, 1, len, f);
    strbuf[len] = '\0';

    string buf {strbuf};
    free(strbuf);

    return strtools::split(buf, '\n');
}

struct bindings *get_cfg_bindings(vector<string>& lines)
{
    struct bindings *bindings = bindings_copy(&defaultkeybinds[0]);
    string label = strtools::format("P%dCONTROLS", 1);

    string keystrings[9] = {strtools::format("P%dLEFT", 1),
                            strtools::format("P%dRIGHT", 1),
                            strtools::format("P%dUP", 1),
                            strtools::format("P%dDOWN", 1),
                            strtools::format("P%dA", 1),
                            strtools::format("P%dB", 1),
                            strtools::format("P%dC", 1),
                            strtools::format("P%dD", 1),
                            strtools::format("P%dESCAPE", 1)};

    SDL_Keycode *keyptrs[9] = {
        &bindings->left, &bindings->right, &bindings->up, &bindings->down, &bindings->a, &bindings->b, &bindings->c, &bindings->d, &bindings->escape};

    unsigned int lineIndex = 0;
    for(auto str : lines)
    {
        if(str[0] == '#')
        {
            continue;
        }

        if(str.find(label) == 0)
        {
            break;
        }

        lineIndex++;
    }

    lineIndex++;

    for(; lineIndex < lines.size(); lineIndex++)
    {
        string& str = lines[lineIndex];
        if(str[0] == '#')
        {
            continue;
        }

        vector<string> tokens = strtools::words(str);

        if(tokens.empty())
        {
            continue;
        }

        for(int j = 0; j < 9; j++)
        {
            if(tokens[0] == keystrings[j])
            {
                SDL_Keycode val = str_sdlk(tokens.back());
                if(val > 0)
                {
                    (*keyptrs[j]) = val;
                    break;
                }
                else
                {
                    log_warn("Binding for %s is invalid, using default", keystrings[j].c_str());
                }
            }
        }
    }

    return bindings;
}

int get_asset_volume(vector<string>& lines, string asset_name)
{
    for(auto str : lines)
    {
        if(str[0] == '#')
        {
            continue;
        }

        if(str.find(asset_name) == 0)
        {
            vector<string> tokens = strtools::words(str);
            int volume = parse_long(tokens.back().c_str());

            if(volume == OPTION_INVALID || volume < 0 || volume > 100)
            {
                volume = 100;
            }

            return (128 * volume) / 100;
        }
    }

    return 128;
}

long parse_long(const char *str)
{
    errno = 0;
    char *temp;
    long val = strtol(str, &temp, 10);

    if(temp == str || (*temp) != '\0' || ((val == LONG_MIN || val == LONG_MAX) && errno == ERANGE))
    {
        return OPTION_INVALID;
    }

    return val;
}

SDL_Keycode str_sdlk(string s)
{
    long c = (long)(s[0]);

    if(c == 'K')
    {
        c = parse_long(s.c_str() + 1);
        if(c == OPTION_INVALID)
        {
            c = 0;
        }

        return (SDL_Keycode)(c);
    }

    return (SDL_Keycode)(c);
}
