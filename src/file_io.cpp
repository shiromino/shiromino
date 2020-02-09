#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "Debug.hpp"
#include "SDL.h"
#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include "stringtools.hpp"

#include "core.h"

using namespace std;

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

            return (MIX_MAX_VOLUME * volume) / 100;
        }
    }

    return MIX_MAX_VOLUME;
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
