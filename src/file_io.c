#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>
#include "zed_dbg.h"

#include "core.h"

struct settings *parse_cfg(const char *filename)
{
    if(!filename)
        return NULL;

    //if(access(filename, F_OK) || access(filename, R_OK)) {
    //    perror("Configuration file could not be accessed");
    //    return NULL;
    //}

    struct settings *s = malloc(sizeof(struct settings));
    struct bstrList *cfg_file_lines = split_file(filename);
    if(!cfg_file_lines) {
        printf("Error splitting config file\n");
    }

    struct stat info;

    bstring sfxvolume = bfromcstr("SFXVOLUME");
    bstring musicvolume = bfromcstr("MUSICVOLUME");
    bstring mastervolume = bfromcstr("MASTERVOLUME");
    bstring home_path = bfromcstr("HOME_PATH");
    bstring videoscale = bfromcstr("VIDEOSCALE");
    bstring player_name = bfromcstr("PLAYERNAME");

    s->keybinds = get_cfg_bindings(cfg_file_lines);

    s->fullscreen = defaultsettings.fullscreen;        // unused option for the time being

    s->sfx_volume = get_cfg_option(cfg_file_lines, sfxvolume);
    s->mus_volume = get_cfg_option(cfg_file_lines, musicvolume);
    s->master_volume = get_cfg_option(cfg_file_lines, mastervolume);
    s->video_scale = get_cfg_option(cfg_file_lines, videoscale);
    s->home_path = get_cfg_string(cfg_file_lines, home_path);

    s->player_name = get_cfg_string(cfg_file_lines, player_name);
    if (s->player_name == NULL)
    {
        char *player_name_config_key = bstr2cstr(player_name, '\0');
        log_info("Could not find %s setting in config file. Using default player name \"%s\"",
                 player_name_config_key,
                 defaultsettings.player_name);
        bcstrfree(player_name_config_key); 

        s->player_name = defaultsettings.player_name;
    }

    if(s->sfx_volume == OPTION_INVALID || s->sfx_volume < 0 || s->sfx_volume > 100)
        s->sfx_volume = 100;
    if(s->mus_volume == OPTION_INVALID || s->mus_volume < 0 || s->mus_volume > 100)
        s->mus_volume = 100;
    if(s->master_volume == OPTION_INVALID || s->master_volume < 0 || s->master_volume > 100)
        s->master_volume = 100;
    if(s->video_scale == OPTION_INVALID || s->video_scale < 1 || s->video_scale > 4)
        s->video_scale = 1;

    //if(s->home_path) {
    //    if(stat(s->home_path, &info) != 0 || !S_ISDIR(info.st_mode)) {
    //        s->home_path = NULL;
    //        log_err("Invalid HOME_PATH setting");
    //    }
    //}

    return s;
}

long get_cfg_option(struct bstrList *lines, bstring label)
{
    int value = 0;

    if(!lines)
        return OPTION_INVALID;

    bstring wspace = bfromcstr(" \t");
    struct bstrList *opt = NULL;
    int i = 0;

    for(i = 0; i < lines->qty; i++) {
        if(lines->entry[i]->data[0] == '#')
            continue;

        if(binstr(lines->entry[i], 0, label) == 0) {
            break;
        }

        if(i == lines->qty - 1) {
            bdestroy(wspace);
            return OPTION_INVALID;
        }
    }

    opt = bsplits(lines->entry[i], wspace);
    if(opt) {
        value = parse_long((char *)(opt->entry[opt->qty - 1]->data));    // returns OPTION_INVALID on error
        bstrListDestroy(opt);
    }

    bdestroy(wspace);
    return value;
}

char *get_cfg_string(struct bstrList *lines, bstring label)
{
    if(!lines)
        return NULL;

    char *str = NULL;
    bstring wspace = bfromcstr(" \t");
    struct bstrList *opt = NULL;
    int i = 0;

    for(i = 0; i < lines->qty; i++) {
        if(lines->entry[i]->data[0] == '#')
            continue;

        if(binstr(lines->entry[i], 0, label) == 0) {
            printf("Found %s\n", label->data);
            break;
        }

        if(i == lines->qty - 1) {
            str = NULL;
            bdestroy(wspace);
            return str;
        }
    }

    opt = bsplits(lines->entry[i], wspace);
    if(opt) {
        str = malloc(opt->entry[opt->qty - 1]->slen + 1);
        strcpy(str, (char *)(opt->entry[opt->qty - 1]->data));
        bstrListDestroy(opt);
    }

    bdestroy(wspace);
    return str;
}

struct bstrList *split_file(const char *filename)
{
    //printf("splitting file\n");

    FILE *f = fopen(filename, "rb");
    if(!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    char *strbuf = malloc(len + 1);

    fseek(f, 0, SEEK_SET);
    fread(strbuf, 1, len, f);
    strbuf[len] = '\0';
    bstring buf = bfromcstr(strbuf);
    free(strbuf);
    struct bstrList *lines = bsplit(buf, '\n');
    bdestroy(buf);

    //printf("split file\n");

    return lines;
}

struct bindings *get_cfg_bindings(struct bstrList *lines)
{
    if(!lines)
        return NULL;

    struct bindings *bindings = bindings_copy(&defaultkeybinds[0]);
    SDL_Keycode tmp = 0;
    int i = 0;
    int j = 0;
    bstring wspace = bfromcstr(" \t");
    bstring label = bformat("P%dCONTROLS", 1);
    struct bstrList *keyopt = NULL;

    bstring keystrings[9] = {
        bformat("P%dLEFT", 1),
        bformat("P%dRIGHT", 1),
        bformat("P%dUP", 1),
        bformat("P%dDOWN", 1),
        bformat("P%dA", 1),
        bformat("P%dB", 1),
        bformat("P%dC", 1),
        bformat("P%dD", 1),
        bformat("P%dESCAPE", 1)
    };

    SDL_Keycode *keyptrs[9] = {
        &bindings->left,
        &bindings->right,
        &bindings->up,
        &bindings->down,
        &bindings->a,
        &bindings->b,
        &bindings->c,
        &bindings->d,
        &bindings->escape
    };

    for(i = 0; i < lines->qty; i++) {
        if(lines->entry[i]->data[0] == '#')
            continue;

        if(binstr(lines->entry[i], 0, label) == 0) {
            break;
        }
    }

    i++;

    for(; i < lines->qty; i++) {
        if(lines->entry[i]->data[0] == '#')
            continue;

        keyopt = bsplits(lines->entry[i], wspace);
        if(keyopt->entry[0]->slen == 0)
            continue;

        for(j = 0; j < 9; j++) {
            if(bstrcmp(keyopt->entry[0], keystrings[j]) == 0) {
                tmp = bstr_sdlk(keyopt->entry[keyopt->qty - 1]);

                if(tmp) {
                    (*keyptrs[j]) = tmp;
                    break;
                } else {
                    // print message about invalid keycode?
                }
            }
        }

        bstrListDestroy(keyopt);
    }

    bdestroy(wspace);
    bdestroy(label);

    return bindings;
}

int get_asset_volume(struct bstrList *lines, bstring asset_name)
{
    int volume = 100;

    if(!lines)
        return volume;

    bstring wspace = bfromcstr(" \t");
    struct bstrList *opt = NULL;
    int i = 0;

    for(i = 0; i < lines->qty; i++) {
        if(lines->entry[i]->data[0] == '#')
            continue;

        if(binstr(lines->entry[i], 0, asset_name) == 0) {
            break;
        }

        if(i == lines->qty - 1) {
            bdestroy(wspace);
            return 128;
        }
    }

    opt = bsplits(lines->entry[i], wspace);
    volume = parse_long((char *)(opt->entry[opt->qty - 1]->data));

    if(volume == OPTION_INVALID || volume < 0 || volume > 100)
        volume = 100;

    volume = (128 * volume) / 100;

    bstrListDestroy(opt);
    bdestroy(wspace);
    return volume;
}

long parse_long(const char *str)
{
    errno = 0;
    char *temp;
    long val = strtol(str, &temp, 10);

    if( temp == str || (*temp) != '\0' || ((val == LONG_MIN || val == LONG_MAX) && errno == ERANGE) )
        return OPTION_INVALID;

    return val;
}

SDL_Keycode bstr_sdlk(bstring b)
{
    long c = b->data[0];

    if(c == 'K') {
        c = parse_long((char *)(&b->data[1]));
        if(c == OPTION_INVALID)
            c = 0;

        return (SDL_Keycode)(c);
    }

    return (SDL_Keycode)(c);
}
