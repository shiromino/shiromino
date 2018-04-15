#include "core.h"

#include "zed_dbg.h"

#include "gfx.h"
#include "gfx_structures.h"
#include "file_io.h"

#include "replay.h"
#include "game_menu.h" // questionable dependencies - TODO look into these

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#if (defined(_WIN64) || defined(_WIN32)) && !defined(__CYGWIN__) && !defined(__CYGWIN32__) && !defined(__MINGW32__) && !defined(__MINGW64__)
#include <direct.h>
#define chdir _chdir

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int nanosleep(struct timespec *t, void* unused)
{
    LARGE_INTEGER ft = {
        .QuadPart = -(t->tv_nsec / 100)
    };

    const HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);

    return 0;
}
#else
#include <unistd.h> // For chdir
#include <errno.h>
#include <sys/stat.h>
#endif

static long framedelay(Uint64 ticks_elap, double fps)
{
    if (fps < 1 || fps > 240)
        return FRAMEDELAY_ERR;

    struct timespec t = { 0, 0 };
    double sec_elap = (double)(ticks_elap) / SDL_GetPerformanceFrequency();
    double spf = (1 / fps);

    if (sec_elap < spf)
    {
        t.tv_nsec = (long)((spf - sec_elap) * 1000000000);

        if (nanosleep(&t, NULL)) {
            // this can happen when the user presses Ctrl+C
            printf("Error: nanosleep() returned failure during frame length calculation\n");
            return FRAMEDELAY_ERR;
        }
    }
    else {
        return (spf - sec_elap)*1000000000.0;
    }

    if (t.tv_nsec)
        return t.tv_nsec;
    else
        return 1;
}

/* <constants> */

struct bindings defaultkeybinds[2] = {
   {
      .left = SDLK_LEFT,
      .right = SDLK_RIGHT,
      .up = SDLK_UP,
      .down = SDLK_DOWN,
      .start = SDLK_RETURN,
      .a = SDLK_f,
      .b = SDLK_d,
      .c = SDLK_s,
      .d = SDLK_a,
      .escape = SDLK_ESCAPE
   },

   {
      .left = SDLK_j,
      .right = SDLK_l,
      .up = SDLK_i,
      .down = SDLK_k,
      .start = SDLK_TAB,
      .a = SDLK_r,
      .b = SDLK_e,
      .c = SDLK_w,
      .d = SDLK_q,
      .escape = SDLK_F11
   }
};

struct settings defaultsettings = {
   .keybinds = &defaultkeybinds[0],
   .video_scale = 1,
   .fullscreen = 0,
   .sfx_volume = 100,
   .mus_volume = 100,
   .master_volume = 50,
   .home_path = NULL,
   .player_name = "ARK"
};


/* </constants> */

int is_left_input_repeat(coreState *cs, int delay)
{
    return cs->keys.left && cs->hold_dir == DAS_LEFT && cs->hold_time >= delay;
}

int is_right_input_repeat(coreState *cs, int delay)
{
    return cs->keys.right && cs->hold_dir == DAS_RIGHT && cs->hold_time >= delay;
}

int is_up_input_repeat(coreState *cs, int delay)
{
    return cs->keys.up && cs->hold_dir == DAS_UP && cs->hold_time >= delay;
}

int is_down_input_repeat(coreState *cs, int delay)
{
    return cs->keys.down && cs->hold_dir == DAS_DOWN && cs->hold_time >= delay;
}

struct bindings *bindings_copy(struct bindings *src)
{
    if (!src)
        return NULL;

    struct bindings *b = malloc(sizeof(struct bindings));
    b->left = src->left;
    b->right = src->right;
    b->up = src->up;
    b->down = src->down;
    b->start = src->start;
    b->a = src->a;
    b->b = src->b;
    b->c = src->c;
    b->d = src->d;
    b->escape = src->escape;

    return b;
}

static bstring make_path(const char *base, const char *subdir, const char *name, const char *ext)
{
    bstring path = base ? bfromcstr(base) : bfromcstr(".");
    bconchar(path, '/');
    bcatcstr(path, subdir);
    bconchar(path, '/');
    bcatcstr(path, name);
    bcatcstr(path, ext);
    //   printf("asset: %s\n", path->data);
    return path;
}

gfx_animation *load_anim_bg(coreState *cs, const char *directory, int frame_multiplier)
{
    if (!directory)
        return NULL;

    gfx_animation *a = malloc(sizeof(gfx_animation));
    a->frame_multiplier = frame_multiplier;
    a->x = 0;
    a->y = 0;
    a->flags = 0;
    a->counter = 0;
    a->rgba_mod = RGBA_DEFAULT;

    //struct stat s;
    //chdir("gfx");
    //int err = stat(directory, &s);
    //chdir("..");

    //if(-1 == err) {
    //   if(ENOENT == errno) {
    //      return NULL;
    //   }
    //} else {
    //   if(!S_ISDIR(s.st_mode)) {
    //      return NULL;
    //   }
    //}

    // bstring full_path = NULL;

    // TODO: convert to new asset system - create an animation asset
    /*
    for(int i = 0; i < 1000; i++) {
       full_path = bformat("%s/%05d", directory, i);
       if(load_asset(cs, ASSET_IMG, (char *)(full_path->data)) < 0) {
          a->num_frames = i;
          break;
       } else {
          //printf("Loaded frame #%d of animated bg: %s\n", i, full_path->data);
       }

       bdestroy(full_path);
    }
    */

    return a;
}

coreState *coreState_create()
{
    coreState *cs = malloc(sizeof(coreState));

    int i = 0;

    cs->fps = FPS;
    //cs->keyquit = SDLK_F11;
    cs->text_editing = 0;
    cs->text_insert = NULL;
    cs->text_backspace = NULL;
    cs->text_delete = NULL;
    cs->text_seek_left = NULL;
    cs->text_seek_right = NULL;
    cs->text_seek_home = NULL;
    cs->text_seek_end = NULL;
    cs->text_select_all = NULL;
    cs->text_copy = NULL;
    cs->text_cut = NULL;
    cs->left_arrow_das = 0;
    cs->right_arrow_das = 0;
    cs->backspace_das = 0;
    cs->delete_das = 0;
    cs->select_all = 0;
    cs->undo = 0;
    cs->redo = 0;

    cs->zero_pressed = 0;
    cs->one_pressed = 0;
    cs->two_pressed = 0;
    cs->three_pressed = 0;
    cs->four_pressed = 0;
    cs->five_pressed = 0;
    cs->six_pressed = 0;
    cs->seven_pressed = 0;
    cs->nine_pressed = 0;

    cs->assets = malloc(sizeof(struct assetdb));

    cs->joystick = NULL;
    cs->keys_raw = (struct keyflags) { 0 };
    cs->prev_keys = (struct keyflags) { 0 };
    cs->keys = (struct keyflags) { 0 };
    cs->pressed = (struct keyflags) { 0 };
    cs->hold_dir = DAS_NONE;
    cs->hold_time = 0;

    cs->mouse_x = 0;
    cs->mouse_y = 0;
    cs->mouse_left_down = 0;
    cs->mouse_right_down = 0;

    cs->screen.name = "Shiromino v.beta2";
    cs->screen.w = 640;
    cs->screen.h = 480;
    cs->screen.window = NULL;
    cs->screen.renderer = NULL;
    //cs->screen.target_tex = NULL;

    cs->bg = NULL;
    cs->bg_old = NULL;
    //cs->anim_bg = NULL;
    //cs->anim_bg_old = NULL;
    cs->gfx_messages = NULL;
    cs->gfx_animations = NULL;
    cs->gfx_buttons = NULL;
    cs->gfx_messages_max = 0;
    cs->gfx_animations_max = 0;
    cs->gfx_buttons_max = 0;

    cs->settings = NULL;
    cs->menu_input_override = 0;
    cs->button_emergency_override = 0;
    cs->p1game = NULL;
    cs->menu = NULL;

    cs->pracdata_mirror = NULL;

    cs->sfx_volume = 32;
    cs->mus_volume = 32;

    cs->avg_sleep_ms = 0;
    cs->avg_sleep_ms_recent = 0;
    cs->frames = 0;

    for (i = 0; i < RECENT_FRAMES; i++) {
        cs->avg_sleep_ms_recent_array[i] = 0;
    }

    cs->recent_frame_overload = -1;

    return cs;
}

void coreState_destroy(coreState *cs)
{
    if (!cs)
        return;

    if (cs->settings != &defaultsettings && cs->settings) {
        if (cs->settings->keybinds)
            free(cs->settings->keybinds);

        if (cs->settings->home_path)
            free(cs->settings->home_path);

        free(cs->settings);
    }

    if (cs->pracdata_mirror)
        pracdata_destroy(cs->pracdata_mirror);

    free(cs);
}

static void load_image(coreState *cs, gfx_image *img, const char *filename)
{
    bstring path = make_path(cs->settings->home_path, "gfx", filename, "");
    if (!img_load(img, (const char*)path->data, cs))
        log_warn("Failed to load image '%s'", filename);
    bdestroy(path);
}

static int load_asset_volume(coreState *cs, const char *filename)
{
    bstring path = make_path(cs->settings->home_path, "audio", "volume", ".cfg");
    struct bstrList *lines = split_file((char*)path->data);
    bdestroy(path);
    bstring bfilename = bfromcstr(filename);
    int volume = get_asset_volume(lines, bfilename);
    bdestroy(bfilename);
    if (lines)
        bstrListDestroy(lines);
    return volume;
}

static void load_sfx(coreState *cs, struct sfx *s, const char *filename)
{
    bstring path = make_path(cs->settings->home_path, "audio", filename, "");
    if (!sfx_load(s, (const char*)path->data))
        log_warn("Failed to load sfx '%s'", filename);
    bdestroy(path);
    s->volume = load_asset_volume(cs, filename);
}

static void load_music(coreState *cs, struct music *m, const char *filename)
{
    bstring path = make_path(cs->settings->home_path, "audio", filename, "");
    if (!music_load(m, (const char*)path->data))
        log_warn("Failed to load music '%s'", filename);
    bdestroy(path);
    m->volume = load_asset_volume(cs, filename);
}

int load_files(coreState *cs)
{
    if (!cs)
        return -1;

#define IMG(name, filename) load_image(cs, &cs->assets->name, filename);
#include "images.h"
#undef IMG

    // audio assets

#define MUS(name, filename) load_music(cs, &cs->assets->name, filename);
#include "music.h"
#undef MUS

#define SFX(name) load_sfx(cs, &cs->assets->name, #name);
#include "sfx.h"
#undef SFX

/*
#ifdef ENABLE_ANIM_BG
   bstring filename;

   for(i = 0; i < 10; i++) {
      filename = bformat("g2_bg/bg%d", i);
      cs->g2_bgs[i] = load_anim_bg(cs, filename->data, 2);
      //if(cs->g2_bgs[i]) printf("Successfully loaded G2 bg #%d\n", i);
      bdestroy(filename);
   }
#endif
*/
    return 0;
}

int init(coreState *cs, struct settings *s)
{
    if (!cs)
        return -1;

    int flags = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    int n = 0;
    char *name = NULL;
    //SDL_Texture *blank = NULL;

    // copy settings into main game structure

    if (s) {
        cs->settings = malloc(sizeof(struct settings));
        //cs->settings->keybinds[1] = bindings_create(malloc(sizeof(struct bindings));
        if (s->keybinds)
            cs->settings->keybinds = bindings_copy(s->keybinds);
        else
            cs->settings->keybinds = bindings_copy(&defaultkeybinds[0]);
        //bindings_copy(s->keybinds[1], &defaultkeybinds[1]);
        cs->settings->video_scale = s->video_scale;
        cs->settings->fullscreen = s->fullscreen;
        cs->settings->sfx_volume = s->sfx_volume;
        cs->settings->mus_volume = s->mus_volume;
        cs->settings->master_volume = s->master_volume;
        cs->settings->player_name = s->player_name;

        cs->sfx_volume = s->sfx_volume;
        cs->mus_volume = s->mus_volume;
        cs->master_volume = s->master_volume;
        if (s->home_path) {
            n = strlen(s->home_path);
            if (s->home_path[n - 1] == '/' || s->home_path[n - 1] == '\\') {
                cs->settings->home_path = malloc(strlen(s->home_path));
                strncpy(cs->settings->home_path, s->home_path, n - 1);
                cs->settings->home_path[n - 1] = '\0';
            }
            else {
                cs->settings->home_path = malloc(strlen(s->home_path) + 1);
                strcpy(cs->settings->home_path, s->home_path);
            }

            printf("Home path is: %s\n", cs->settings->home_path);
            if (chdir(cs->settings->home_path) < 0)      // chdir so relative paths later on make sense
                log_err("chdir() returned failure");
        }
        else
            cs->settings->home_path = NULL;
    }
    else
        cs->settings = &defaultsettings;

    check(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0, "SDL_Init: Error: %s\n", SDL_GetError());
    check(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0, "SDL_InitSubSystem: Error: %s\n", SDL_GetError());
    check(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG, "IMG_Init: Failed to initialize PNG support: %s\n", IMG_GetError());      // IMG_GetError() not necessarily reliable here
    Mix_Init(MIX_INIT_OGG); // check(Mix_Init(MIX_INIT_OGG) == MIX_INIT_OGG, "Mix_Init: Failed to initialize OGG support: %s\n", Mix_GetError());
    check(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) != -1, "Mix_OpenAudio: Error\n");

    Mix_AllocateChannels(32);
    Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);

    if (SDL_NumJoysticks() > 0) {
        cs->joystick = SDL_JoystickOpen(0);

        if (cs->joystick) {
            printf("Opened Joystick 0\n");
            printf("Name: %s\n", SDL_JoystickNameForIndex(0));
            printf("Number of Axes: %d\n", SDL_JoystickNumAxes(cs->joystick));
            printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(cs->joystick));
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(cs->joystick));
        }
        else {
            printf("Couldn't open Joystick 0\n");
        }
    }

    cs->screen.w = cs->settings->video_scale * 640;
    cs->screen.h = cs->settings->video_scale * 480;
    w = cs->screen.w;
    h = cs->screen.h;
    name = cs->screen.name;

    cs->screen.window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
    check(cs->screen.window != NULL, "SDL_CreateWindow: Error: %s\n", SDL_GetError());
    cs->screen.renderer = SDL_CreateRenderer(cs->screen.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    check(cs->screen.renderer != NULL, "SDL_CreateRenderer: Error: %s\n", SDL_GetError());
    //cs->screen.target_tex = SDL_CreateTexture(cs->screen.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    //check(cs->screen.target_tex != NULL, "SDL_CreateTexture: Error: %s\n", SDL_GetError());

    check(load_files(cs) == 0, "load_files() returned failure\n");

    check(gfx_init(cs) == 0, "gfx_init returned failure\n");

    cs->bg = cs->assets->bg_temp.tex;
    cs->bg_old = cs->bg;
    //blank = cs->assets->blank.tex;

    //check(gfx_rendercopy(cs, blank, NULL, NULL) > -1, "SDL_RenderCopy: Error: %s\n", SDL_GetError());

    // TODO: Configurable scores.db path
    static const char scoredb_file[] = "scores.db";
    scoredb_init(&cs->scores, scoredb_file);
    scoredb_create_player(&cs->scores, &cs->player, cs->settings->player_name);

    cs->menu = menu_create(cs);
    check(cs->menu != NULL, "menu_create returned failure\n");

    cs->menu->init(cs->menu);

    return 0;

error:
    return 1;
}

void quit(coreState *cs)
{
    scoredb_terminate(&cs->scores);

    if (cs->assets) {

#define IMG(name, filename) img_destroy(&cs->assets->name);
#include "images.h"
#undef IMG

#define MUS(name, filename) music_destroy(&cs->assets->name);
#include "music.h"
#undef MUS

#define SFX(name) sfx_destroy(&cs->assets->name);
#include "sfx.h"
#undef SFX

        free(cs->assets);
    }

    if (cs->screen.renderer)
        SDL_DestroyRenderer(cs->screen.renderer);

    if (cs->screen.window)
        SDL_DestroyWindow(cs->screen.window);

    //SDL_DestroyTexture(cs->screen.target_tex);

    cs->screen.window = NULL;
    cs->screen.renderer = NULL;

    if (cs->joystick && SDL_JoystickGetAttached(cs->joystick)) {
        SDL_JoystickClose(cs->joystick);
    }

    if (cs->p1game) {
        printf("quit(): Found leftover game struct, attempting ->quit\n");
        cs->p1game->quit(cs->p1game);
        free(cs->p1game);
        cs->p1game = NULL;
    }

    if (cs->menu) {
        cs->menu->quit(cs->menu);
        free(cs->menu);
        cs->menu = NULL;
    }

    /*for(int i = 0; i < 10; i++) {
       if(cs->g2_bgs[i])
          gfx_animation_destroy(cs->g2_bgs[i]);
    }*/

    gfx_quit(cs);

    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

int run(coreState *cs)
{
    if (!cs)
        return -1;

    bool running = true;
    while (running)
    {
        Uint64 timestamp = SDL_GetPerformanceCounter();

        // TODO: Rearrange the input->draw loop
        cs->prev_keys = cs->keys;

        if (procevents(cs))
        {
            return 1;
        }

        handle_replay_input(cs);

        update_input_repeat(cs);
        update_pressed(cs);

        gfx_buttons_input(cs);

        //SDL_SetRenderTarget(cs->screen.renderer, NULL);
        //SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 255);
        SDL_RenderClear(cs->screen.renderer);
        gfx_drawbg(cs);

        if (cs->p1game)
        {
            if (procgame(cs->p1game, !cs->button_emergency_override))
            {
                cs->p1game->quit(cs->p1game);
                free(cs->p1game);
                cs->p1game = NULL;

                cs->bg = cs->assets->bg_temp.tex;
            }
        }

        // process menus, graphics, and framerate

        if (cs->menu && ((!cs->p1game || cs->menu_input_override) ? 1 : 0)) {
            if (procgame(cs->menu, !cs->button_emergency_override)) {
                cs->menu->quit(cs->menu);
                free(cs->menu);

                cs->menu = NULL;

                if (!cs->p1game)
                    running = 0;
            }

            //if(!((!cs->button_emergency_override && ((!cs->p1game || cs->menu_input_override) ? 1 : 0))))   printf("Not processing menu input\n");
        }

        if (!cs->menu && !cs->p1game)
            running = false;

        //SDL_SetRenderTarget(cs->screen.renderer, NULL);

        gfx_drawbuttons(cs, 0);
        gfx_drawmessages(cs, 0);
        gfx_drawanimations(cs, 0);

        if (cs->button_emergency_override)
            gfx_draw_emergency_bg_darken(cs);

        gfx_drawbuttons(cs, EMERGENCY_OVERRIDE);
        gfx_drawmessages(cs, EMERGENCY_OVERRIDE);
        gfx_drawanimations(cs, EMERGENCY_OVERRIDE);

        SDL_RenderPresent(cs->screen.renderer);

        if (cs->sfx_volume != cs->settings->sfx_volume) {
            cs->sfx_volume = cs->settings->sfx_volume;
            Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);
        }

        if (cs->mus_volume != cs->settings->mus_volume)
            cs->mus_volume = cs->settings->mus_volume;

        if (cs->master_volume != cs->settings->master_volume) {
            cs->master_volume = cs->settings->master_volume;
            Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);
        }

        timestamp = SDL_GetPerformanceCounter() - timestamp;
        long sleep_ns = framedelay(timestamp, cs->fps);

        if (sleep_ns == FRAMEDELAY_ERR)
            return 1;

        cs->avg_sleep_ms = ((cs->avg_sleep_ms * (long double)(cs->frames)) + ((long double)(sleep_ns) / 1000000.0L)) / (long double)(cs->frames + 1);
        cs->frames++;

        if (cs->frames <= RECENT_FRAMES) {
            cs->avg_sleep_ms_recent_array[cs->frames - 1] = ((long double)(sleep_ns) / 1000000.0L);
            if (sleep_ns < 0) {
                //printf("Lag from last frame in microseconds: %ld (%f frames)\n", abs(sleep_ns/1000), fabs((long double)(sleep_ns)/(1000000000.0L/(long double)(cs->fps))));
                cs->recent_frame_overload = cs->frames - 1;
            }
        }
        else {
            cs->avg_sleep_ms_recent_array[(cs->frames - 1) % RECENT_FRAMES] = ((long double)(sleep_ns) / 1000000.0L);
            if (sleep_ns < 0) {
                //printf("Lag from last frame in microseconds: %ld (%f frames)\n", abs(sleep_ns/1000), fabs((long double)(sleep_ns)/(1000000000.0L/(long double)(cs->fps))));
                cs->recent_frame_overload = (cs->frames - 1) % RECENT_FRAMES;
            }
            else if (cs->recent_frame_overload == (cs->frames - 1) % RECENT_FRAMES)
                cs->recent_frame_overload = -1;
        }

        cs->avg_sleep_ms_recent = 0;
        for (int i = 0; i < RECENT_FRAMES; i++) {
            cs->avg_sleep_ms_recent += cs->avg_sleep_ms_recent_array[i];
        }

        cs->avg_sleep_ms_recent /= (cs->frames < RECENT_FRAMES ? cs->frames : RECENT_FRAMES);

        //printf("Frame elapsed.\n");
    }

    return 0;
}

int procevents(coreState *cs)
{
    if (!cs)
        return -1;

    struct bindings *kb = NULL;
    struct keyflags *k = NULL;
    SDL_Joystick *joy = cs->joystick;

    SDL_Event event;
    SDL_Keycode kc;

    Uint8 rc = 0;

    if (cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
        cs->mouse_left_down = 1;
    if (cs->mouse_right_down == BUTTON_PRESSED_THIS_FRAME)
        cs->mouse_right_down = 1;

    if (cs->select_all)
        cs->select_all = 0;
    if (cs->undo)
        cs->undo = 0;
    if (cs->redo)
        cs->redo = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return 1;

        case SDL_JOYAXISMOTION:
            k = &cs->keys;

            if (event.jaxis.which == 0)
            {
                if (event.jaxis.axis == 0) // x axis
                {
                    if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                    {
                        k->left = 1;
                        k->right = 0;
                    }
                    else if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                    {
                        k->right = 1;
                        k->left = 0;
                    }
                    else
                    {
                        k->right = 0;
                        k->left = 0;
                    }
                }
                else if (event.jaxis.axis == 1) // y axis
                {
                    if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                    {
                        k->up = 1;
                        k->down = 0;
                    }
                    else if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                    {
                        k->down = 1;
                        k->up = 0;
                    }
                    else
                    {
                        k->up = 0;
                        k->down = 0;
                    }
                }

            }

            break;

        case SDL_JOYHATMOTION:
            k = &cs->keys;

            if (event.jhat.which == 0)
            {
                if (event.jhat.hat == 0)
                {
                    if (event.jhat.value == SDL_HAT_LEFT)
                    {
                        k->left = 1;
                        k->right = 0;
                    }
                    else if (event.jhat.value == SDL_HAT_RIGHT)
                    {
                        k->right = 1;
                        k->left = 0;
                    }
                    else if (event.jhat.value == SDL_HAT_UP)
                    {
                        k->up = 1;
                        k->down = 0;
                    }
                    else if (event.jhat.value == SDL_HAT_DOWN)
                    {
                        k->down = 1;
                        k->up = 0;
                    }
                    else
                    {
                        k->right = 0;
                        k->left = 0;
                        k->up = 0;
                        k->down = 0;
                    }
                }
            }

            break;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            k = &cs->keys;
            if (joy) {
                rc = SDL_JoystickGetButton(joy, 0);
                if (!rc)
                    k->a = 0;
                if (rc && k->a == 0)
                    k->a = 1;

                rc = SDL_JoystickGetButton(joy, 3);
                if (!rc)
                    k->b = 0;
                if (rc && k->b == 0)
                    k->b = 1;

                rc = SDL_JoystickGetButton(joy, 5);
                if (!rc)
                    k->c = 0;
                if (rc && k->c == 0)
                    k->c = 1;

                rc = SDL_JoystickGetButton(joy, 1);
                if (!rc)
                    k->d = 0;
                if (rc && k->d == 0)
                    k->d = 1;
            }

            break;

        case SDL_KEYDOWN:
            if (event.key.repeat)
                break;

            kc = event.key.keysym.sym;

            if (kc == SDLK_v && SDL_GetModState() & KMOD_CTRL) {
                if (cs->text_editing && cs->text_insert)
                    cs->text_insert(cs, SDL_GetClipboardText());

                break;
            }

            if (kc == SDLK_c && SDL_GetModState() & KMOD_CTRL) {
                if (cs->text_editing && cs->text_copy)
                    cs->text_copy(cs);

                break;
            }

            if (kc == SDLK_x && SDL_GetModState() & KMOD_CTRL) {
                if (cs->text_editing && cs->text_cut)
                    cs->text_cut(cs);

                break;
            }

            if (kc == SDLK_a && SDL_GetModState() & KMOD_CTRL) {
                if (cs->text_editing && cs->text_select_all)
                    cs->text_select_all(cs);
                cs->select_all = 1;
                break;
            }

            if (kc == SDLK_z && SDL_GetModState() & KMOD_CTRL) {
                cs->undo = 1;
                break;
            }

            if (kc == SDLK_y && SDL_GetModState() & KMOD_CTRL) {
                cs->redo = 1;
                break;
            }

            if (kc == SDLK_BACKSPACE) {
                cs->backspace_das = 1;
            }

            if (kc == SDLK_DELETE) {
                cs->delete_das = 1;
            }

            if (kc == SDLK_HOME) {
                if (cs->text_editing && cs->text_seek_home)
                    cs->text_seek_home(cs);
            }

            if (kc == SDLK_END) {
                if (cs->text_editing && cs->text_seek_end)
                    cs->text_seek_end(cs);
            }

            if (kc == SDLK_RETURN) {
                if (cs->text_toggle)
                    cs->text_toggle(cs);
            }

            if (kc == SDLK_LEFT) {
                cs->left_arrow_das = 1;
            }

            if (kc == SDLK_RIGHT) {
                cs->right_arrow_das = 1;
            }

            if (kc == SDLK_0) {
                cs->zero_pressed = 1;
            }

            if (kc == SDLK_1) {
                cs->one_pressed = 1;
            }

            if (kc == SDLK_2) {
                cs->two_pressed = 1;
            }

            if (kc == SDLK_3) {
                cs->three_pressed = 1;
            }

            if (kc == SDLK_4) {
                cs->four_pressed = 1;
            }

            if (kc == SDLK_5) {
                cs->five_pressed = 1;
            }

            if (kc == SDLK_6) {
                cs->six_pressed = 1;
            }

            if (kc == SDLK_7) {
                cs->seven_pressed = 1;
            }

            if (kc == SDLK_9) {
                cs->nine_pressed = 1;
            }

            break;

        case SDL_KEYUP:
            kc = event.key.keysym.sym;

            if (cs->settings->keybinds) {
                k = &cs->keys;
                kb = cs->settings->keybinds;

                if (kc == kb->left)
                    k->left = 0;

                if (kc == kb->right)
                    k->right = 0;

                if (kc == kb->up)
                    k->up = 0;

                if (kc == kb->down)
                    k->down = 0;

                if (kc == kb->start)
                    k->start = 0;

                if (kc == kb->a)
                    k->a = 0;

                if (kc == kb->b)
                    k->b = 0;

                if (kc == kb->c)
                    k->c = 0;

                if (kc == kb->d)
                    k->d = 0;

                if (kc == kb->escape)
                    k->escape = 0;
            }

            if (kc == SDLK_LEFT)
                cs->left_arrow_das = 0;

            if (kc == SDLK_RIGHT)
                cs->right_arrow_das = 0;

            if (kc == SDLK_BACKSPACE)
                cs->backspace_das = 0;

            if (kc == SDLK_DELETE)
                cs->delete_das = 0;

            if (kc == SDLK_0) {
                cs->zero_pressed = 0;
            }

            if (kc == SDLK_1) {
                cs->one_pressed = 0;
            }

            if (kc == SDLK_2) {
                cs->two_pressed = 0;
            }

            if (kc == SDLK_3) {
                cs->three_pressed = 0;
            }

            if (kc == SDLK_4) {
                cs->four_pressed = 0;
            }

            if (kc == SDLK_5) {
                cs->five_pressed = 0;
            }

            if (kc == SDLK_6) {
                cs->six_pressed = 0;
            }

            if (kc == SDLK_7) {
                cs->seven_pressed = 0;
            }

            if (kc == SDLK_9) {
                cs->nine_pressed = 0;
            }

            break;

        case SDL_TEXTINPUT:
            if (cs->text_editing) {
                if (!(
                    (event.text.text[0] == 'c' || event.text.text[0] == 'C') &&
                    (event.text.text[0] == 'v' || event.text.text[0] == 'V') &&
                    (event.text.text[0] == 'x' || event.text.text[0] == 'X') &&
                    (event.text.text[0] == 'a' || event.text.text[0] == 'A') &&
                    SDL_GetModState() & KMOD_CTRL))
                {
                    if (cs->text_insert)
                        cs->text_insert(cs, event.text.text);
                }
            }

            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                cs->mouse_left_down = BUTTON_PRESSED_THIS_FRAME;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
                cs->mouse_right_down = BUTTON_PRESSED_THIS_FRAME;
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
                cs->mouse_left_down = 0;
            if (event.button.button == SDL_BUTTON_RIGHT)
                cs->mouse_right_down = 0;
            break;

        default:
            break;
        }
    }

    if (cs->left_arrow_das) {
        if (cs->left_arrow_das == 1 || cs->left_arrow_das == 30) {
            if (cs->text_editing && cs->text_seek_left)
                cs->text_seek_left(cs);
            if (cs->left_arrow_das == 1)
                cs->left_arrow_das = 2;
        }
        else {
            cs->left_arrow_das++;
        }
    }

    if (cs->right_arrow_das) {
        if (cs->right_arrow_das == 1 || cs->right_arrow_das == 30) {
            if (cs->text_editing && cs->text_seek_right)
                cs->text_seek_right(cs);
            if (cs->right_arrow_das == 1)
                cs->right_arrow_das = 2;
        }
        else {
            cs->right_arrow_das++;
        }
    }

    if (cs->backspace_das) {
        if (cs->backspace_das == 1 || cs->backspace_das == 30) {
            if (cs->text_editing && cs->text_backspace)
                cs->text_backspace(cs);
            if (cs->backspace_das == 1)
                cs->backspace_das = 2;
        }
        else {
            cs->backspace_das++;
        }
    }

    if (cs->delete_das) {
        if (cs->delete_das == 1 || cs->delete_das == 30) {
            if (cs->text_editing && cs->text_delete)
                cs->text_delete(cs);
            if (cs->delete_das == 1)
                cs->delete_das = 2;
        }
        else {
            cs->delete_das++;
        }
    }

    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    if (cs->settings->keybinds) {
        k = &cs->keys_raw;
        kb = cs->settings->keybinds;

        *k = (struct keyflags) { 0 };

        if (keystates[SDL_GetScancodeFromKey(kb->left)])
            k->left = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->right)])
            k->right = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->up)])
            k->up = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->down)])
            k->down = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->start)])
            k->start = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->a)])
            k->a = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->b)])
            k->b = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->c)])
            k->c = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->d)])
            k->d = 1;

        if (keystates[SDL_GetScancodeFromKey(kb->escape)])
            k->escape = 1;

        cs->keys = cs->keys_raw;
    }


    SDL_GetMouseState(&cs->mouse_x, &cs->mouse_y);

    return 0;
}

int procgame(game_t *g, int input_enabled)
{
    if (!g)
        return -1;

    if (g->preframe)
    {
        if (g->preframe(g))
            return 1;
    }

    if (g->input && input_enabled)
    {
        if (g->input(g))
            return 1;
    }

    Uint64 benchmark = SDL_GetPerformanceCounter();

    if (g->frame)
    {
        if (g->frame(g))
            return 1;
    }

    benchmark = SDL_GetPerformanceCounter() - benchmark;
    //   printf("%fms\n", (double)(benchmark) * 1000 / (double)SDL_GetPerformanceFrequency());

    if (g->draw)
    {
        if (g->draw(g))
            return 1;
    }

    g->frame_counter++;

    return 0;
}

void handle_replay_input(coreState* cs)
{
    game_t *g = cs->p1game;
    if (g != NULL) {
        qrsdata *q = g->data;

        if (q == NULL) {
            return;
        }

        if (q->playback) {
            if ((unsigned int)(q->playback_index) == q->replay->len)
                qrs_end_playback(g);
            else {
                // keyflags a few extra fields that aren't in the packed inputs, carry those over
                cs->keys_raw = cs->keys;
                unpack_input(q->replay->pinputs[q->playback_index], &cs->keys_raw);
                cs->keys = cs->keys_raw;

                q->playback_index++;
            }
        }
        else if (q->recording) {
            q->replay->pinputs[q->replay->len] = pack_input(&cs->keys_raw);

            q->replay->len++;
        }
    }
}

void update_input_repeat(coreState *cs)
{
    struct keyflags *k = &cs->keys;

    if (cs->hold_dir == DAS_LEFT && k->right) { cs->hold_time = 0; cs->hold_dir = DAS_RIGHT; }
    else if (cs->hold_dir == DAS_RIGHT && k->left) { cs->hold_time = 0; cs->hold_dir = DAS_LEFT; }
    else if (cs->hold_dir == DAS_UP && k->down) { cs->hold_time = 0; cs->hold_dir = DAS_DOWN; }
    else if (cs->hold_dir == DAS_DOWN && k->up) { cs->hold_time = 0; cs->hold_dir = DAS_UP; }

    if (cs->hold_dir == DAS_LEFT && k->left)        cs->hold_time++;
    else if (cs->hold_dir == DAS_RIGHT && k->right) cs->hold_time++;
    else if (cs->hold_dir == DAS_UP && k->up)       cs->hold_time++;
    else if (cs->hold_dir == DAS_DOWN && k->down)   cs->hold_time++;
    else
    {
        if (k->left)       cs->hold_dir = DAS_LEFT;
        else if (k->right) cs->hold_dir = DAS_RIGHT;
        else if (k->up)    cs->hold_dir = DAS_UP;
        else if (k->down)  cs->hold_dir = DAS_DOWN;
        else cs->hold_dir = DAS_NONE;

        cs->hold_time = 0;
    }
}

void update_pressed(coreState *cs)
{
    cs->pressed = (struct keyflags) {
        .left = cs->keys.left - cs->prev_keys.left >= 1,
            .right = cs->keys.right - cs->prev_keys.right >= 1,
            .up = cs->keys.up - cs->prev_keys.up >= 1,
            .down = cs->keys.down - cs->prev_keys.down >= 1,
            .start = cs->keys.start - cs->prev_keys.start >= 1,
            .a = cs->keys.a - cs->prev_keys.a >= 1,
            .b = cs->keys.b - cs->prev_keys.b >= 1,
            .c = cs->keys.c - cs->prev_keys.c >= 1,
            .d = cs->keys.d - cs->prev_keys.d >= 1,
            .escape = cs->keys.escape - cs->prev_keys.escape >= 1,
    };
}

int button_emergency_inactive(coreState *cs)
{
    if (cs->button_emergency_override)
        return 0;
    else
        return 1;

    return 1;
}

int gfx_buttons_input(coreState *cs)
{
    if (!cs)
        return -1;

    if (!cs->gfx_buttons)
        return 1;

    int i = 0;
    gfx_button *b = NULL;

    int scaled_x = 0;
    int scaled_y = 0;
    int scaled_w = 0;
    int scaled_h = 0;

    int scale = 1;
    if (cs->settings) {
        scale = cs->settings->video_scale;
    }

    for (i = 0; i < cs->gfx_buttons_max; i++) {
        if (!cs->gfx_buttons[i])
            continue;

        b = cs->gfx_buttons[i];
        scaled_x = scale * b->x;
        scaled_y = scale * b->y;
        scaled_w = scale * b->w;
        scaled_h = scale * b->h;

        if (cs->button_emergency_override && !(cs->gfx_buttons[i]->flags & BUTTON_EMERGENCY)) {
            cs->gfx_buttons[i]->highlighted = 0;
            if (b->delete_check) {
                if (b->delete_check(cs)) {
                    gfx_button_destroy(b);
                    cs->gfx_buttons[i] = NULL;
                }
            }

            continue;
        }

        if (cs->mouse_x < scaled_x + scaled_w && cs->mouse_x >= scaled_x && cs->mouse_y < scaled_y + scaled_h && cs->mouse_y >= scaled_y)
            b->highlighted = 1;
        else
            b->highlighted = 0;

        if (b->highlighted && cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME) {
            if (b->action) {
                b->action(cs, b->data);
            }

            b->clicked = 3;
        }

        if (b->delete_check && (!b->clicked || b->flags & BUTTON_EMERGENCY)) {
            if (b->delete_check(cs)) {
                gfx_button_destroy(b);
                cs->gfx_buttons[i] = NULL;
            }
        }
    }

    for (i = 0; i < cs->gfx_buttons_max; i++) {
        if (!cs->gfx_buttons[i])
            continue;

        b = cs->gfx_buttons[i];

        if (cs->button_emergency_override && !(cs->gfx_buttons[i]->flags & BUTTON_EMERGENCY)) {
            if (b->delete_check) {
                if (b->delete_check(cs)) {
                    gfx_button_destroy(b);
                    cs->gfx_buttons[i] = NULL;
                }
            }

            continue;
        }

        if (b->delete_check && (!b->clicked || b->flags & BUTTON_EMERGENCY)) {
            if (b->delete_check(cs)) {
                gfx_button_destroy(b);
                cs->gfx_buttons[i] = NULL;
            }
        }
    }

    return 0;
}

int request_fps(coreState *cs, double fps)
{
    if (!cs)
        return -1;
    if (fps != FPS && fps != G2_FPS)
        return 1;

    cs->fps = fps;
    return 0;
}
