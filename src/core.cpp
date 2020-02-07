#include "core.h"

#include "Config.hpp"
#include "Debug.hpp"
#include "file_io.h"
#include "gfx.h"
#include "gfx_structures.h"

#include "game_menu.h"
#include "game_qs.h"
#include "replay.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <cinttypes>

#include <iostream>
#include <vector>
#include <array>
#include <string>

#include "SDL.h"
#include "SDL_image.h"
#include "SGUIL/SGUIL.hpp"
#include "GuiGridCanvas.hpp"
#include "GuiScreenManager.hpp"

#include "SPM_Spec.hpp"
#include "QRS.hpp"
#include "ShiroPhysoMino.hpp"

#define PENTOMINO_C_REVISION_STRING "rev 1.2"

using namespace Shiro;
using namespace std;
using namespace PDINI;

BindableVariables bindables;

#if(defined(_WIN64) || defined(_WIN32)) && !defined(__CYGWIN__) && !defined(__CYGWIN32__) && !defined(__MINGW32__) && \
    !defined(__MINGW64__)
#define _WIN32_WINNT 0x0400
#include <direct.h>
#define chdir _chdir

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// TODO: Check Windows docs, and see if setting errno to EFAULT might make
// sense somewhere here. The EINTR case might be wrong, too.
// TODO: Add more granular waits for the seconds portion, then precise waiting
// for the rest.
int nanosleep(const struct timespec* req, struct timespec* rem) {
    LARGE_INTEGER startTime;
    if (req->tv_nsec < 0 || req->tv_nsec > 999999999ll || req->tv_sec < 0) {
        errno = EINVAL;
        return -1;
    }
    QueryPerformanceCounter(&startTime);
    LARGE_INTEGER waitTime = { .QuadPart = -((LONGLONG)req->tv_sec * 10000000ll + (LONGLONG)req->tv_nsec / 100ll) };
    const HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer) {
        SetWaitableTimer(timer, &waitTime, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
        return 0;
    }
    else {
        errno = EINTR;
        LARGE_INTEGER endTime;
        LARGE_INTEGER frequency;
        // Microsoft's docs guarantee these don't fail on XP or newer, and
        // that's all we're supporting. -nightmareci
        QueryPerformanceCounter(&endTime);
        QueryPerformanceFrequency(&frequency);
        LONGLONG nsWait = ((endTime.QuadPart - startTime.QuadPart) * 1000000000ll) / frequency.QuadPart;
        LONGLONG nsRem = ((LONGLONG)req->tv_sec * 1000000000ll + (LONGLONG)req->tv_nsec) - nsWait;
        rem->tv_sec = nsRem / 1000000000ll;
        rem->tv_nsec = nsRem % 1000000000ll;
        return -1;
    }
}

#define S_ISDIR(flags) ((flags) & S_IFDIR)
#else
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h> // For chdir
#endif

// TODO: Analyze frame timing, and see whether it's already accurate enough, or
// needs tweaks to be more accurate.
static long framedelay(Uint64 ticks_elap, double fps)
{
    if(fps < 1 || fps > 240)
        return FRAMEDELAY_ERR;

    struct timespec t = {0, 0};
    double sec_elap = (double)(ticks_elap) / SDL_GetPerformanceFrequency();
    double spf = (1 / fps);

    if(sec_elap < spf)
    {
        t.tv_nsec = (long)((spf - sec_elap) * 1000000000ll);

        struct timespec rem;
        if(nanosleep(&t, &rem))
        {
            // this can happen when the user presses Ctrl+C
            log_err("nanosleep() returned failure during frame length calculation");
            return FRAMEDELAY_ERR;
        }
    }
    else
    {
        return (spf - sec_elap) * 1000000000.0;
    }

    if(t.tv_nsec)
        return t.tv_nsec;
    else
        return 1;
}

/* <constants> */

struct bindings defaultkeybinds[2] = {
    {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_RETURN, SDLK_f, SDLK_d, SDLK_s, SDLK_a, SDLK_ESCAPE},

    {SDLK_j, SDLK_l, SDLK_i, SDLK_k, SDLK_TAB, SDLK_r, SDLK_e, SDLK_w, SDLK_q, SDLK_F11}};

struct settings defaultsettings = {&defaultkeybinds[0], 1, true, false, 50, 100, 100, NULL};

/* </constants> */

Keybinds::Keybinds() : Keybinds(0) {}

Keybinds::Keybinds(int playerNum) {
    switch (playerNum) {
    default:
    case 0:
        left = SDLK_LEFT;
        right = SDLK_RIGHT;
        up = SDLK_UP;
        down = SDLK_DOWN;
        start = SDLK_RETURN;
        a = SDLK_f;
        b = SDLK_d;
        c = SDLK_s;
        d = SDLK_a;
        escape = SDLK_ESCAPE;
        break;

    case 1:    
        left = SDLK_j;
        right = SDLK_l;
        up = SDLK_i;
        down = SDLK_k;
        start = SDLK_TAB;
        a = SDLK_r;
        b = SDLK_e;
        c = SDLK_w;
        d = SDLK_q;
        escape = SDLK_F11;
        break;
    }
}

static array<string, 10> keybindNames = {
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "START",
    "A",
    "B",
    "C",
    "D",
    "ESCAPE"
};

bool Keybinds::read(INI& ini, const string sectionName) {
    bool defaultUsed = false;
    SDL_Keycode* const keycodes[] = {&left, &right, &up, &down, &start, &a, &b, &c, &d, &escape};
    SDL_Keycode* const* keycode = keycodes;
    for (const auto keybindName : keybindNames) {
        string keyName;
        if (!ini.get(sectionName, keybindName, keyName) || SDL_GetKeyFromName(keyName.c_str()) == SDLK_UNKNOWN) {
            log_warn("Binding for %s is invalid", keybindName.c_str());
            defaultUsed = true;
        }
        else {
            **keycode = SDL_GetKeyFromName(keyName.c_str());
        }
        keycode++;
    }
    return defaultUsed;
}

Settings::Settings() :
    videoScale(1.0f),
    videoStretch(1),
    fullscreen(0),
    masterVolume(80),
    sfxVolume(100),
    musicVolume(90),
    basePath("."),
    playerName("ARK") {}

bool Settings::read(string filename) {
    INI ini;
    auto readStatus = ini.read(filename);
    if (readStatus.second > 0) {
        log_warn("Error reading configuation INI \"%s\" on line %" PRIu64, filename.c_str(), (uint64_t)readStatus.second);
    }
    if (!readStatus.first) {
        log_warn("Failed opening configuration INI \"%s\"", filename.c_str());
        return true;
    }

    bool defaultUsed = keybinds.read(ini, "P1CONTROLS");

    // [PATHS]
    string basePath;
    if (!ini.get("PATHS", "BASE_PATH", basePath)) {
        char *basePath = SDL_GetBasePath();
        this->basePath = basePath;
        SDL_free(basePath);
        defaultUsed = true;
    }
    else {
        this->basePath = basePath;
    }

    // TODO: Add support for player 2.
    // [P1CONTROLS]
    if (this->keybinds.read(ini, "P1CONTROLS")) {
        defaultUsed = true;
    }

    // [AUDIO]
    //value = ini.get("AUDIO", "MASTERVOLUME");
    int volume;
    if (!ini.get("AUDIO", "MASTERVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->masterVolume = volume;
    }
    if (!ini.get("AUDIO", "SFXVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->sfxVolume = volume;
    }
    if (!ini.get("AUDIO", "MUSICVOLUME", volume) || (volume < 0 && volume > 100)) {
        defaultUsed = true;
    }
    else {
        this->musicVolume = volume;
    }

    // [SCREEN]
    float videoScale;
    if (!ini.get("SCREEN", "VIDEOSCALE", videoScale) || videoScale <= 0.0f) {
        defaultUsed = true;
    }
    else {
        this->videoScale = videoScale;
    }
    
    int videoStretch;
    if (!ini.get("SCREEN", "VIDEOSTRETCH", videoStretch)) {
        defaultUsed = true;
    }
    else {
        this->videoStretch = videoStretch;
    }
    
    int fullscreen;
    if (!ini.get("SCREEN", "FULLSCREEN", fullscreen)) {
        defaultUsed = true;
    }
    else {
        this->fullscreen = fullscreen;
    }

    // [ACCOUNT]
    string playerName;
    if (ini.get("ACCOUNT", "PLAYERNAME", playerName)) {
        this->playerName = playerName;
    }
    else {
        defaultUsed = true;
    }

    return defaultUsed;
}

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
    if(!src)
        return NULL;

    struct bindings *b = (struct bindings *)malloc(sizeof(struct bindings));
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

static string make_path(const char *base, const char *subdir, const char *name, const char *ext)
{
    string path = base ? string{base} : string{"."};
    path.append("/");
    path.append(subdir);
    path.append("/");
    path.append(name);
    path.append(ext);

    //printf("asset: %s\n", path.c_str());
    return path;
}

gfx_animation *load_anim_bg(coreState *cs, const char *directory, int frame_multiplier)
{
    if(!directory)
        return NULL;

    gfx_animation *a = (gfx_animation *)malloc(sizeof(gfx_animation));
    a->frame_multiplier = frame_multiplier;
    a->x = 0;
    a->y = 0;
    a->flags = 0;
    a->counter = 0;
    a->rgba_mod = RGBA_DEFAULT;

    struct stat s;
    chdir("gfx");
    int err = stat(directory, &s);
    chdir("..");

    if(-1 == err)
    {
        if(ENOENT == errno)
        {
            return NULL;
        }
    }
    else
    {
        if(!S_ISDIR(s.st_mode))
        {
            return NULL;
        }
    }

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

void coreState_initialize(coreState *cs)
{
    int i = 0;

    cs->fps = FPS;
    // cs->keyquit = SDLK_F11;
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

    cs->assets = new assetdb;

    cs->joystick = NULL;
    cs->prev_keys_raw = {0};
    cs->keys_raw = {0};
    cs->prev_keys = {0};
    cs->keys = {0};
    cs->pressed = {0};
    cs->hold_dir = DAS_NONE;
    cs->hold_time = 0;

    cs->mouse_x = 0;
    cs->mouse_y = 0;
    cs->logical_mouse_x = 0;
    cs->logical_mouse_y = 0;
    cs->mouse_left_down = 0;
    cs->mouse_right_down = 0;

    cs->screen.name = "Shiromino " SHIROMINO_VERSION_STRING;
    cs->screen.w = 640;
    cs->screen.h = 480;
    cs->screen.window = NULL;
    cs->screen.renderer = NULL;
    // cs->screen.target_tex = NULL;

    cs->bg = NULL;
    cs->bg_old = NULL;
    // cs->anim_bg = NULL;
    // cs->anim_bg_old = NULL;
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

    cs->screenManager = new GuiScreenManager {};

    cs->displayMode = game_display_default;
    cs->motionBlur = false;
    cs->pracdata_mirror = NULL;

    cs->sfx_volume = 32;
    cs->mus_volume = 32;

    cs->avg_sleep_ms = 0;
    cs->avg_sleep_ms_recent = 0;
    cs->frames = 0;

    for(i = 0; i < RECENT_FRAMES; i++)
    {
        cs->avg_sleep_ms_recent_array[i] = 0;
    }

    cs->recent_frame_overload = -1;
}

void coreState_destroy(coreState *cs)
{
    if(!cs)
    {
        return;
    }

    delete cs->settings;

    if(cs->pracdata_mirror)
        pracdata_destroy(cs->pracdata_mirror);
}

static void load_image(coreState *cs, gfx_image *img, const char *filename)
{
    string path = make_path(cs->settings->basePath.c_str(), "gfx", filename, "");
    if(!img_load(img, (const char *)path.c_str(), cs))
    {
        log_warn("Failed to load image '%s'", filename);
    }
}

static void load_bitfont(BitFont *font, gfx_image *sheetImg, gfx_image *outlineSheetImg, unsigned int charW, unsigned int charH)
{
    font->sheet = sheetImg->tex;
    font->outlineSheet = outlineSheetImg->tex;
    font->charW = charW;
    font->charH = charH;
    font->isValid = true;
}

static int load_asset_volume(coreState *cs, const char *filename)
{
    string path = make_path(cs->settings->basePath.c_str(), "audio", "volume", ".cfg");
    vector<string> lines = split_file(path.c_str());

    return get_asset_volume(lines, string{filename});
}

static void load_sfx(coreState* cs, Sfx** s, const char* filename)
{
    string path = make_path(cs->settings->basePath.c_str(), "audio", filename, "");
    *s = new Sfx();
    if (!(*s)->load(path)) {
        log_warn("Failed to load sfx '%s'", filename);
    }

    (*s)->volume = load_asset_volume(cs, filename);
}

static void load_music(coreState* cs, Music** m, const char* filename)
{
    string path = make_path(cs->settings->basePath.c_str(), "audio", filename, "");
    *m = new Music();
    if (!(*m)->load(path)) {
        log_warn("Failed to load music '%s'", filename);
    }

    (*m)->volume = load_asset_volume(cs, filename);
}

int load_files(coreState *cs)
{
    if(!cs)
        return -1;

        // image assets

#define IMG(name, filename) load_image(cs, &cs->assets->name, filename);
#include "images.h"
#undef IMG

#define FONT(name, sheetName, outlineSheetName, charW, charH) \
    load_bitfont(&cs->assets->name, &cs->assets->sheetName, &cs->assets->outlineSheetName, charW, charH);
#include "fonts.h"
#undef FONT

        // audio assets

#define MUSIC(name, i) load_music(cs, &cs->assets->name[i], #name #i);
#include "music.h"
#undef MUSIC

#define SFX(name) load_sfx(cs, &cs->assets->name, #name);
#define SFX_ARRAY(name, i) load_sfx(cs, &cs->assets->name[i], #name #i);
#include "sfx.h"
#undef SFX_ARRAY
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

int init(coreState *cs, Settings* settings)
{
    try {
        if(!cs)
            return -1;

        const char *name = NULL;
        // SDL_Texture *blank = NULL;

        // copy settings into main game structure

        if(settings) {
            cs->settings = settings;
            printf("Base path is: %s\n", cs->settings->basePath.c_str());
            if(chdir(cs->settings->basePath.c_str()) < 0) { // chdir so relative paths later on make sense
                log_err("chdir() returned failure");
            }
        }
        else {
            cs->settings = new Settings();
        }

        check(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0,
              "SDL_Init: Error: %s\n",
              SDL_GetError());
        check(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0, "SDL_InitSubSystem: Error: %s\n", SDL_GetError());
        check(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG,
              "IMG_Init: Failed to initialize PNG support: %s\n",
              IMG_GetError());  // IMG_GetError() not necessarily reliable here
        Mix_Init(MIX_INIT_OGG); // check(Mix_Init(MIX_INIT_OGG) == MIX_INIT_OGG, "Mix_Init: Failed to initialize OGG
                                // support: %s\n", Mix_GetError());
        check(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) != -1, "Mix_OpenAudio: Error\n");

        Mix_AllocateChannels(32);
        Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);

        if(SDL_NumJoysticks() > 0)
        {
            cs->joystick = SDL_JoystickOpen(0);

            if(cs->joystick)
            {
                printf("Opened Joystick 0\n");
                printf("Name: %s\n", SDL_JoystickNameForIndex(0));
                printf("Number of Axes: %d\n", SDL_JoystickNumAxes(cs->joystick));
                printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(cs->joystick));
                printf("Number of Balls: %d\n", SDL_JoystickNumBalls(cs->joystick));
            }
            else
            {
                printf("Couldn't open Joystick 0\n");
            }
        }

        cs->screen.w = cs->settings->videoScale * 640;
        cs->screen.h = cs->settings->videoScale * 480;
        unsigned int w = cs->screen.w;
        unsigned int h = cs->screen.h;
        name = cs->screen.name;

        int windowFlags = SDL_WINDOW_RESIZABLE;

        cs->screen.window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, windowFlags);
        check(cs->screen.window != NULL, "SDL_CreateWindow: Error: %s\n", SDL_GetError());
        cs->screen.renderer =
            SDL_CreateRenderer(cs->screen.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        check(cs->screen.renderer != NULL, "SDL_CreateRenderer: Error: %s\n", SDL_GetError());

        SDL_SetRenderDrawBlendMode(cs->screen.renderer, SDL_BLENDMODE_BLEND);

        SDL_SetWindowMinimumSize(cs->screen.window, 640, 480);
        if(cs->settings->fullscreen)
        {
            SDL_SetWindowSize(cs->screen.window, 640, 480);
            SDL_SetWindowFullscreen(cs->screen.window, SDL_WINDOW_FULLSCREEN);
        }

        SDL_RenderSetLogicalSize(cs->screen.renderer, 640, 480);
        if(!cs->settings->videoStretch)
        {
            SDL_RenderSetIntegerScale(cs->screen.renderer, SDL_TRUE);
        }

        check(load_files(cs) == 0, "load_files() returned failure\n");

        check(Gui_Init(cs->screen.renderer, NULL), "Gui_Init() returned failure\n");
        check(gfx_init(cs) == 0, "gfx_init returned failure\n");

        cs->bg = cs->assets->bg_temp.tex;
        cs->bg_old = cs->bg;
        // blank = cs->assets->blank.tex;

        // check(SDL_RenderCopy(cs->screen.renderer, blank, NULL, NULL) > -1, "SDL_RenderCopy: Error: %s\n", SDL_GetError());

        cs->menu = menu_create(cs);
        check(cs->menu != NULL, "menu_create returned failure\n");

        cs->menu->init(cs->menu);

        cs->screenManager->addScreen("main", mainMenu_create(cs, cs->screenManager, cs->assets->fixedsys));
        //SDL_Rect gameScreenRect = {0, 0, 640, 480};
        //cs->screenManager->addScreen("game", new GuiScreen {cs, "game", NULL, gameScreenRect});
        cs->screenManager->loadScreen("main");

        // check(SDL_RenderCopy(cs->screen.renderer, blank, NULL, NULL) > -1, "SDL_RenderCopy: Error: %s\n", SDL_GetError());

        // TODO: Configurable scores.db path
        static const char scoredb_file[] = "scores.db";
        scoredb_init(&cs->scores, scoredb_file);
        scoredb_create_player(&cs->scores, &cs->player, cs->settings->playerName.c_str());

        /*
        static const char archive_file[] = "archive.db";
        scoredb_init(&cs->archive, archive_file);
        scoredb_create_player(&cs->archive, &cs->player, cs->settings->player_name);
        */

        cs->menu = menu_create(cs);
        check(cs->menu != NULL, "menu_create returned failure\n");

        cs->menu->init(cs->menu);

        printf("\nPENTOMINO C: %s\n\n", PENTOMINO_C_REVISION_STRING);

        return 0;
    }
    catch (const logic_error& error) {
        return 1;
    }
}

void quit(coreState *cs)
{
    scoredb_terminate(&cs->scores);
    // scoredb_terminate(&cs->archive);

    if(cs->assets)
    {

        // Already destroyed in the BitFont destructor previously; this prevents a double-free.
        cs->assets->font_fixedsys_excelsior.tex = nullptr;

#define IMG(name, filename) img_destroy(&cs->assets->name);
#include "images.h"
#undef IMG

        // All the textures have been destroyed, so prevent them being freed by
        // the GuiWindow destructor.
#define FONT(name, sheetName, outlineSheetName, charW, charH) \
        cs->assets->name.isValid = false; \
        cs->assets->name.sheet = nullptr; \
        cs->assets->name.outlineSheet = nullptr;
#include "fonts.h"
#undef FONT

#define MUSIC(name, i) delete cs->assets->name[i];
#include "music.h"
#undef MUSIC

#define SFX(name) delete cs->assets->name;
#define SFX_ARRAY(name, i) delete cs->assets->name[i];
#include "sfx.h"
#undef SFX_ARRAY
#undef SFX

        delete cs->assets;
    }

    if(cs->screen.renderer)
        SDL_DestroyRenderer(cs->screen.renderer);

    if(cs->screen.window)
        SDL_DestroyWindow(cs->screen.window);

    // SDL_DestroyTexture(cs->screen.target_tex);

    cs->screen.window = NULL;
    cs->screen.renderer = NULL;

    if(cs->joystick && SDL_JoystickGetAttached(cs->joystick))
    {
        SDL_JoystickClose(cs->joystick);
    }

    if(cs->p1game)
    {
        printf("quit(): Found leftover game struct, attempting ->quit\n");
        cs->p1game->quit(cs->p1game);
        free(cs->p1game);
        cs->p1game = NULL;
    }

    if(cs->menu)
    {
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
    if(!cs)
        return -1;

    bool running = true;

    int windW = 620;
    int windH = 460;
    SDL_Rect gridRect = {16, 44, windW - 32, windH - 60};

    grid_t *g = grid_create(gridRect.w / 16, gridRect.h / 16);
    SDL_Texture *paletteTex = cs->assets->tets_bright_qs.tex;

    BindableInt paletteVar {"paletteVar", 0, 25};

    GuiGridCanvas *gridCanvas = new GuiGridCanvas{
        0,
        g,
        paletteVar,
        paletteTex,
        32, 32,
        gridRect
    };

    SDL_Rect windowRect = {10, 10, windW, windH};
    // TODO: Fix how fixedsys is loaded/unloaded; currently, both SGUIL and
    // Shiromio try to free it, with SGUIL freeing it first in the GuiWindow
    // destructor.
    GuiWindow wind {cs, "Shiromino", &cs->assets->fixedsys, GUI_WINDOW_CALLBACK_NONE, windowRect};

    wind.addControlElement(gridCanvas);

    // SPM_Spec spec;
    QRS spec {qrs_variant_P, false};
    TestSPM SPMgame {*cs, &spec};
    SPMgame.init();

    //cs->p1game = qs_game_create(cs, 0, MODE_PENTOMINO, NO_REPLAY);
    //cs->p1game->init(cs->p1game);

    while(running)
    {
        Uint64 timestamp = SDL_GetPerformanceCounter();

        // TODO: Rearrange the input->draw loop
        cs->prev_keys_raw = cs->keys_raw;
        cs->prev_keys = cs->keys;

        if(procevents(cs, wind))
        {
            return 1;
        }

        handle_replay_input(cs);

        update_input_repeat(cs);
        update_pressed(cs);

        gfx_buttons_input(cs);

        // SDL_SetRenderTarget(cs->screen.renderer, NULL);
        // SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 255);
        SDL_RenderClear(cs->screen.renderer);
        gfx_drawbg(cs);

        /*
        SPMgame.input();
        SPMgame.frame();
        SPMgame.draw();
        */


        if(cs->p1game)
        {
            if(procgame(cs->p1game, !cs->button_emergency_override))
            {
                cs->p1game->quit(cs->p1game);
                free(cs->p1game);
                cs->p1game = NULL;

                cs->bg = cs->assets->bg_temp.tex;
            }
        }
        else
        {
            //cs->screenManager->drawScreen();
        }

        //wind.draw();

        // process menus, graphics, and framerate

        // menu is processed if: either there's no game, or menu overrides existing game
        if(cs->menu && ((!cs->p1game || cs->menu_input_override) ? 1 : 0))
        {
            if(procgame(cs->menu, !cs->button_emergency_override))
            {
                cs->menu->quit(cs->menu);
                free(cs->menu);

                cs->menu = NULL;

                if(!cs->p1game)
                    running = 0;
            }

            // if(!((!cs->button_emergency_override && ((!cs->p1game || cs->menu_input_override) ? 1 : 0)))) printf("Not
            // processing menu input\n");
        }

        if(!cs->menu && !cs->p1game)
        {
            running = false;
        }


        // SDL_SetRenderTarget(cs->screen.renderer, NULL);

        gfx_drawbuttons(cs, 0);
        gfx_drawmessages(cs, 0);
        gfx_drawanimations(cs, 0);

        if(cs->button_emergency_override)
            gfx_draw_emergency_bg_darken(cs);

        gfx_drawbuttons(cs, EMERGENCY_OVERRIDE);
        gfx_drawmessages(cs, EMERGENCY_OVERRIDE);
        gfx_drawanimations(cs, EMERGENCY_OVERRIDE);

        SDL_RenderPresent(cs->screen.renderer);

        if(cs->sfx_volume != cs->settings->sfxVolume)
        {
            cs->sfx_volume = cs->settings->sfxVolume;
            Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);
        }

        if(cs->mus_volume != cs->settings->musicVolume)
            cs->mus_volume = cs->settings->musicVolume;

        if(cs->master_volume != cs->settings->masterVolume)
        {
            cs->master_volume = cs->settings->masterVolume;
            Mix_Volume(-1, (cs->sfx_volume * cs->master_volume) / 100);
        }

        timestamp = SDL_GetPerformanceCounter() - timestamp;
        long sleep_ns = framedelay(timestamp, cs->fps);

        if(sleep_ns == FRAMEDELAY_ERR)
            return 1;

        cs->avg_sleep_ms = ((cs->avg_sleep_ms * (long double)(cs->frames)) + ((long double)(sleep_ns) / 1000000.0L)) /
                           (long double)(cs->frames + 1);
        cs->frames++;

        if(cs->frames <= RECENT_FRAMES)
        {
            cs->avg_sleep_ms_recent_array[cs->frames - 1] = ((long double)(sleep_ns) / 1000000.0L);
            if(sleep_ns < 0)
            {
                // printf("Lag from last frame in microseconds: %ld (%f frames)\n", abs(sleep_ns/1000), fabs((long
                // double)(sleep_ns)/(1000000000.0L/(long double)(cs->fps))));
                cs->recent_frame_overload = cs->frames - 1;
            }
        }
        else
        {
            cs->avg_sleep_ms_recent_array[(cs->frames - 1) % RECENT_FRAMES] = ((long double)(sleep_ns) / 1000000.0L);
            if(sleep_ns < 0)
            {
                // printf("Lag from last frame in microseconds: %ld (%f frames)\n", abs(sleep_ns/1000), fabs((long
                // double)(sleep_ns)/(1000000000.0L/(long double)(cs->fps))));
                cs->recent_frame_overload = (cs->frames - 1) % RECENT_FRAMES;
            }
            else if(cs->recent_frame_overload == (cs->frames - 1) % RECENT_FRAMES)
                cs->recent_frame_overload = -1;
        }

        cs->avg_sleep_ms_recent = 0;
        for(int i = 0; i < RECENT_FRAMES; i++)
        {
            cs->avg_sleep_ms_recent += cs->avg_sleep_ms_recent_array[i];
        }

        cs->avg_sleep_ms_recent /= (cs->frames < RECENT_FRAMES ? cs->frames : RECENT_FRAMES);

        // printf("Frame elapsed.\n");
    }

    return 0;
}

int procevents(coreState *cs, GuiWindow& wind)
{
    if(!cs)
        return -1;

    struct keyflags *k = NULL;
    //struct keyflags joyflags{0};
    SDL_Joystick *joy = cs->joystick;

    SDL_Event event;
    SDL_Keycode kc;
    Keybinds& kb = cs->settings->keybinds;

    Uint8 rc = 0;

    if(cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
    {
        cs->mouse_left_down = 1;
    }
    if(cs->mouse_right_down == BUTTON_PRESSED_THIS_FRAME)
    {
        cs->mouse_right_down = 1;
    }

    if(cs->select_all)
    {
        cs->select_all = 0;
    }
    if(cs->undo)
    {
        cs->undo = 0;
    }
    if(cs->redo)
    {
        cs->redo = 0;
    }

    SDL_GetMouseState(&cs->mouse_x, &cs->mouse_y);

    int windowW;
    int windowH;
    SDL_GetWindowSize(cs->screen.window, &windowW, &windowH);

    if(windowW == (windowH * 4) / 3)
    {
        float scale_ = (float)windowW / 640.0;
        cs->logical_mouse_x = (int)((float)cs->mouse_x / scale_);
        cs->logical_mouse_y = (int)((float)cs->mouse_y / scale_);
    }
    else if(windowW < (windowH * 4) / 3) // squished horizontally (results in horizontal bars on the top and bottom of window)
    {
        float scale_ = (float)windowW / 640.0;
        int yOffset = (windowH - ((windowW * 3) / 4)) / 2;
        if(cs->mouse_y < yOffset || cs->mouse_y >= windowH - yOffset)
        {
            cs->logical_mouse_y = -1;
        }
        else
        {
            cs->logical_mouse_y = (int)((float)(cs->mouse_y - yOffset) / scale_);
        }

        cs->logical_mouse_x = (int)((float)cs->mouse_x / scale_);
    }
    else
    {
        float scale_ = (float)windowH / 480.0;
        int xOffset = (windowW - ((windowH * 4) / 3)) / 2;
        if(cs->mouse_x < xOffset || cs->mouse_x >= windowW - xOffset)
        {
            cs->logical_mouse_x = -1;
        }
        else
        {
            cs->logical_mouse_x = (int)((float)(cs->mouse_x - xOffset) / scale_);
        }

        cs->logical_mouse_y = (int)((float)cs->mouse_y / scale_);
    }

    while(SDL_PollEvent(&event))
    {
        // wind.handleSDLEvent(event, {cs->logical_mouse_x, cs->logical_mouse_y} );
        //printf("Handling SDL event\n");
        //cs->screenManager->handleSDLEvent(event, {cs->logical_mouse_x, cs->logical_mouse_y});

        switch(event.type)
        {
            case SDL_QUIT:
                return 1;

            case SDL_JOYAXISMOTION:
                k = &cs->keys_raw;

                if(event.jaxis.which == 0)
                {
                    if(event.jaxis.axis == 0) // x axis
                    {
                        if(event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            k->left = 1;
                            k->right = 0;
                        }
                        else if(event.jaxis.value > JOYSTICK_DEAD_ZONE)
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
                    else if(event.jaxis.axis == 1) // y axis
                    {
                        if(event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            k->up = 1;
                            k->down = 0;
                        }
                        else if(event.jaxis.value > JOYSTICK_DEAD_ZONE)
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
                k = &cs->keys_raw;

                if(event.jhat.which == 0)
                {
                    if(event.jhat.hat == 0)
                    {
                        if(event.jhat.value == SDL_HAT_LEFT)
                        {
                            k->left = 1;
                            k->right = 0;
                        }
                        else if(event.jhat.value == SDL_HAT_RIGHT)
                        {
                            k->right = 1;
                            k->left = 0;
                        }
                        else if(event.jhat.value == SDL_HAT_UP)
                        {
                            k->up = 1;
                            k->down = 0;
                        }
                        else if(event.jhat.value == SDL_HAT_DOWN)
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
                k = &cs->keys_raw;
                if(joy)
                {
                    rc = SDL_JoystickGetButton(joy, 0);
                    if(!rc)
                        k->a = 0;
                    if(rc && k->a == 0)
                        k->a = 1;

                    rc = SDL_JoystickGetButton(joy, 3);
                    if(!rc)
                        k->b = 0;
                    if(rc && k->b == 0)
                        k->b = 1;

                    rc = SDL_JoystickGetButton(joy, 5);
                    if(!rc)
                        k->c = 0;
                    if(rc && k->c == 0)
                        k->c = 1;

                    rc = SDL_JoystickGetButton(joy, 4);
                    if(!rc)
                        k->d = 0;
                    if(rc && k->d == 0)
                        k->d = 1;
                    // TODO: Add all buttons here.
                }

                break;

            case SDL_KEYDOWN:
                if(event.key.repeat)
                    break;

                kc = event.key.keysym.sym;

                k = &cs->keys_raw;

                if(kc == kb.left)
                    k->left = 1;

                if(kc == kb.right)
                    k->right = 1;

                if(kc == kb.up)
                    k->up = 1;

                if(kc == kb.down)
                    k->down = 1;

                if(kc == kb.start)
                    k->start = 1;

                if(kc == kb.a)
                    k->a = 1;

                if(kc == kb.b)
                    k->b = 1;

                if(kc == kb.c)
                    k->c = 1;

                if(kc == kb.d)
                    k->d = 1;

                if(kc == kb.escape)
                    k->escape = 1;

                if(kc == SDLK_v && SDL_GetModState() & KMOD_CTRL)
                {
                    if(cs->text_editing && cs->text_insert)
                        cs->text_insert(cs, SDL_GetClipboardText());

                    break;
                }

                if(kc == SDLK_c && SDL_GetModState() & KMOD_CTRL)
                {
                    if(cs->text_editing && cs->text_copy)
                        cs->text_copy(cs);

                    break;
                }

                if(kc == SDLK_x && SDL_GetModState() & KMOD_CTRL)
                {
                    if(cs->text_editing && cs->text_cut)
                        cs->text_cut(cs);

                    break;
                }

                if(kc == SDLK_a && SDL_GetModState() & KMOD_CTRL)
                {
                    if(cs->text_editing && cs->text_select_all)
                        cs->text_select_all(cs);
                    cs->select_all = 1;
                    break;
                }

                if(kc == SDLK_z && SDL_GetModState() & KMOD_CTRL)
                {
                    cs->undo = 1;
                    break;
                }

                if(kc == SDLK_y && SDL_GetModState() & KMOD_CTRL)
                {
                    cs->redo = 1;
                    break;
                }

                if(kc == SDLK_BACKSPACE)
                {
                    cs->backspace_das = 1;
                }

                if(kc == SDLK_DELETE)
                {
                    cs->delete_das = 1;
                }

                if(kc == SDLK_HOME)
                {
                    if(cs->text_editing && cs->text_seek_home)
                        cs->text_seek_home(cs);
                }

                if(kc == SDLK_END)
                {
                    if(cs->text_editing && cs->text_seek_end)
                        cs->text_seek_end(cs);
                }

                if(kc == SDLK_RETURN)
                {
                    if(cs->text_toggle)
                        cs->text_toggle(cs);
                }

                if(kc == SDLK_LEFT)
                {
                    cs->left_arrow_das = 1;
                }

                if(kc == SDLK_RIGHT)
                {
                    cs->right_arrow_das = 1;
                }

                if(kc == SDLK_F8)
                {
                    switch(cs->displayMode)
                    {
                        case game_display_default:
                            cs->displayMode = game_display_detailed;
                            break;
                        case game_display_detailed:
                            cs->displayMode = game_display_centered;
                            break;
                        default:
                            cs->displayMode = game_display_default;
                            break;
                    }
                }

                if(kc == SDLK_F9)
                {
                    cs->motionBlur = !cs->motionBlur;
                }

                if(kc == SDLK_F11)
                {
                    if(cs->settings->fullscreen)
                    {
                        cs->settings->fullscreen = false;
                        SDL_SetWindowFullscreen(cs->screen.window, 0);
                        SDL_SetWindowSize(cs->screen.window, 640.0*cs->settings->videoScale, 480.0*cs->settings->videoScale);
                    }
                    else
                    {
                        cs->settings->fullscreen = true;
                        SDL_SetWindowSize(cs->screen.window, 640, 480);
                        int flags = (SDL_GetModState() & KMOD_SHIFT) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
                        if(SDL_SetWindowFullscreen(cs->screen.window, flags) < 0)
                        {
                            cout << "SDL_SetWindowFullscreen(): Error: " << SDL_GetError() << endl;
                        }
                    }
                }

                if(kc == SDLK_F4 && SDL_GetModState() & KMOD_ALT)
                {
                    return 1;
                }

                if(kc == SDLK_0)
                {
                    cs->zero_pressed = 1;
                }

                if(kc == SDLK_1)
                {
                    if(SDL_GetModState() & KMOD_ALT)
                    {
                        cs->settings->videoScale = 1;
                        SDL_SetWindowSize(cs->screen.window, 640, 480);
                    }

                    cs->one_pressed = 1;
                }

                if(kc == SDLK_2)
                {
                    if(SDL_GetModState() & KMOD_ALT)
                    {
                        cs->settings->videoScale = 2;
                        SDL_SetWindowSize(cs->screen.window, 2*640, 2*480);
                    }

                    cs->two_pressed = 1;
                }

                if(kc == SDLK_3)
                {
                    if(SDL_GetModState() & KMOD_ALT)
                    {
                        cs->settings->videoScale = 3;
                        SDL_SetWindowSize(cs->screen.window, 3*640, 3*480);
                    }

                    cs->three_pressed = 1;
                }

                if(kc == SDLK_4)
                {
                    if(SDL_GetModState() & KMOD_ALT)
                    {
                        cs->settings->videoScale = 4;
                        SDL_SetWindowSize(cs->screen.window, 4*640, 4*480);
                    }

                    cs->four_pressed = 1;
                }

                if(kc == SDLK_5)
                {
                    if(SDL_GetModState() & KMOD_ALT)
                    {
                        cs->settings->videoScale = 5;
                        SDL_SetWindowSize(cs->screen.window, 5*640, 5*480);
                    }

                    cs->five_pressed = 1;
                }

                if(kc == SDLK_6)
                {
                    cs->six_pressed = 1;
                }

                if(kc == SDLK_7)
                {
                    cs->seven_pressed = 1;
                }

                if(kc == SDLK_9)
                {
                    cs->nine_pressed = 1;
                }

                break;

            case SDL_KEYUP:
                kc = event.key.keysym.sym;

                k = &cs->keys_raw;

                if(kc == kb.left)
                    k->left = 0;

                if(kc == kb.right)
                    k->right = 0;

                if(kc == kb.up)
                    k->up = 0;

                if(kc == kb.down)
                    k->down = 0;

                if(kc == kb.start)
                    k->start = 0;

                if(kc == kb.a)
                    k->a = 0;

                if(kc == kb.b)
                    k->b = 0;

                if(kc == kb.c)
                    k->c = 0;

                if(kc == kb.d)
                    k->d = 0;

                if(kc == kb.escape)
                    k->escape = 0;

                if(kc == SDLK_LEFT)
                    cs->left_arrow_das = 0;

                if(kc == SDLK_RIGHT)
                    cs->right_arrow_das = 0;

                if(kc == SDLK_BACKSPACE)
                    cs->backspace_das = 0;

                if(kc == SDLK_DELETE)
                    cs->delete_das = 0;

                if(kc == SDLK_0)
                {
                    cs->zero_pressed = 0;
                }

                if(kc == SDLK_1)
                {
                    cs->one_pressed = 0;
                }

                if(kc == SDLK_2)
                {
                    cs->two_pressed = 0;
                }

                if(kc == SDLK_3)
                {
                    cs->three_pressed = 0;
                }

                if(kc == SDLK_4)
                {
                    cs->four_pressed = 0;
                }

                if(kc == SDLK_5)
                {
                    cs->five_pressed = 0;
                }

                if(kc == SDLK_6)
                {
                    cs->six_pressed = 0;
                }

                if(kc == SDLK_7)
                {
                    cs->seven_pressed = 0;
                }

                if(kc == SDLK_9)
                {
                    cs->nine_pressed = 0;
                }

                break;

            case SDL_TEXTINPUT:
                if(cs->text_editing)
                {
                    if(!((event.text.text[0] == 'c' || event.text.text[0] == 'C') &&
                         (event.text.text[0] == 'v' || event.text.text[0] == 'V') &&
                         (event.text.text[0] == 'x' || event.text.text[0] == 'X') &&
                         (event.text.text[0] == 'a' || event.text.text[0] == 'A') && SDL_GetModState() & KMOD_CTRL))
                    {
                        if(cs->text_insert)
                            cs->text_insert(cs, event.text.text);
                    }
                }

                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    cs->mouse_left_down = BUTTON_PRESSED_THIS_FRAME;
                }
                if(event.button.button == SDL_BUTTON_RIGHT)
                    cs->mouse_right_down = BUTTON_PRESSED_THIS_FRAME;
                break;

            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT)
                    cs->mouse_left_down = 0;
                if(event.button.button == SDL_BUTTON_RIGHT)
                    cs->mouse_right_down = 0;
                break;

            default:
                break;
        }
    }

    if(cs->left_arrow_das)
    {
        if(cs->left_arrow_das == 1 || cs->left_arrow_das == 30)
        {
            if(cs->text_editing && cs->text_seek_left)
                cs->text_seek_left(cs);
            if(cs->left_arrow_das == 1)
                cs->left_arrow_das = 2;
        }
        else
        {
            cs->left_arrow_das++;
        }
    }

    if(cs->right_arrow_das)
    {
        if(cs->right_arrow_das == 1 || cs->right_arrow_das == 30)
        {
            if(cs->text_editing && cs->text_seek_right)
                cs->text_seek_right(cs);
            if(cs->right_arrow_das == 1)
                cs->right_arrow_das = 2;
        }
        else
        {
            cs->right_arrow_das++;
        }
    }

    if(cs->backspace_das)
    {
        if(cs->backspace_das == 1 || cs->backspace_das == 30)
        {
            if(cs->text_editing && cs->text_backspace)
                cs->text_backspace(cs);
            if(cs->backspace_das == 1)
                cs->backspace_das = 2;
        }
        else
        {
            cs->backspace_das++;
        }
    }

    if(cs->delete_das)
    {
        if(cs->delete_das == 1 || cs->delete_das == 30)
        {
            if(cs->text_editing && cs->text_delete)
                cs->text_delete(cs);
            if(cs->delete_das == 1)
                cs->delete_das = 2;
        }
        else
        {
            cs->delete_das++;
        }
    }

    /*
        const uint8_t *keystates = SDL_GetKeyboardState(NULL);

        if (cs->settings->keybinds) {
            k = &cs->keys_raw;
            kb = cs->settings->keybinds;

            *k = (struct keyflags) { 0 };

            if (joy) {
                Uint8 hat = SDL_JoystickGetHat(joy, 0);
                if(hat == SDL_HAT_LEFT)
                    k->left = 1;

                if(hat == SDL_HAT_RIGHT)
                    k->right = 1;

                if(hat == SDL_HAT_UP)
                    k->up = 1;

                if(hat == SDL_HAT_DOWN)
                    k->down = 1;

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
    */

    cs->keys = cs->keys_raw;



    return 0;
}

int procgame(game_t *g, int input_enabled)
{
    if(!g)
        return -1;

    if(g->preframe)
    {
        if(g->preframe(g))
            return 1;
    }

    if(g->input && input_enabled)
    {
        if(g->input(g))
            return 1;
    }

    Uint64 benchmark = SDL_GetPerformanceCounter();

    if(g->frame)
    {
        if(g->frame(g))
            return 1;
    }

    benchmark = SDL_GetPerformanceCounter() - benchmark;
    //   printf("%fms\n", (double)(benchmark) * 1000 / (double)SDL_GetPerformanceFrequency());

    if(g->draw)
    {
        if(g->draw(g))
            return 1;
    }

    g->frame_counter++;

    return 0;
}

void handle_replay_input(coreState *cs)
{
    game_t *g = cs->p1game;
    if(g != NULL)
    {
        qrsdata *q = (qrsdata *)g->data;

        if(q == NULL)
        {
            return;
        }

        if(q->playback)
        {
            if((unsigned int)(q->playback_index) == q->replay->len)
                qrs_end_playback(g);
            else
            {
                unpack_input(q->replay->pinputs[q->playback_index], &cs->keys);

                q->playback_index++;
            }
        }
        else if(q->recording)
        {
            q->replay->pinputs[q->replay->len] = pack_input(&cs->keys_raw);

            q->replay->len++;
        }
    }
}

void update_input_repeat(coreState *cs)
{
    struct keyflags *k = &cs->keys;

    if(cs->hold_dir == DAS_LEFT && k->right)
    {
        cs->hold_time = 0;
        cs->hold_dir = DAS_RIGHT;
    }
    else if(cs->hold_dir == DAS_RIGHT && k->left)
    {
        cs->hold_time = 0;
        cs->hold_dir = DAS_LEFT;
    }
    else if(cs->hold_dir == DAS_UP && k->down)
    {
        cs->hold_time = 0;
        cs->hold_dir = DAS_DOWN;
    }
    else if(cs->hold_dir == DAS_DOWN && k->up)
    {
        cs->hold_time = 0;
        cs->hold_dir = DAS_UP;
    }

    if(cs->hold_dir == DAS_LEFT && k->left)
        cs->hold_time++;
    else if(cs->hold_dir == DAS_RIGHT && k->right)
        cs->hold_time++;
    else if(cs->hold_dir == DAS_UP && k->up)
        cs->hold_time++;
    else if(cs->hold_dir == DAS_DOWN && k->down)
        cs->hold_time++;
    else
    {
        if(k->left)
            cs->hold_dir = DAS_LEFT;
        else if(k->right)
            cs->hold_dir = DAS_RIGHT;
        else if(k->up)
            cs->hold_dir = DAS_UP;
        else if(k->down)
            cs->hold_dir = DAS_DOWN;
        else
            cs->hold_dir = DAS_NONE;

        cs->hold_time = 0;
    }
}

void update_pressed(coreState *cs)
{
    cs->pressed.left = (cs->keys.left == 1 && cs->prev_keys.left == 0) ? 1 : 0;
    cs->pressed.right = (cs->keys.right == 1 && cs->prev_keys.right == 0) ? 1 : 0;
    cs->pressed.up = (cs->keys.up == 1 && cs->prev_keys.up == 0) ? 1 : 0;
    cs->pressed.down = (cs->keys.down == 1 && cs->prev_keys.down == 0) ? 1 : 0;
    cs->pressed.start = (cs->keys.start == 1 && cs->prev_keys.start == 0) ? 1 : 0;
    cs->pressed.a = (cs->keys.a == 1 && cs->prev_keys.a == 0) ? 1 : 0;
    cs->pressed.b = (cs->keys.b == 1 && cs->prev_keys.b == 0) ? 1 : 0;
    cs->pressed.c = (cs->keys.c == 1 && cs->prev_keys.c == 0) ? 1 : 0;
    cs->pressed.d = (cs->keys.d == 1 && cs->prev_keys.d == 0) ? 1 : 0;
    cs->pressed.escape = (cs->keys.escape == 1 && cs->prev_keys.escape == 0) ? 1 : 0;
}

int button_emergency_inactive(coreState *cs)
{
    if(cs->button_emergency_override)
        return 0;
    else
        return 1;

    return 1;
}

int gfx_buttons_input(coreState *cs)
{
    if(!cs)
        return -1;

    if(!cs->gfx_buttons)
        return 1;

    int i = 0;
    gfx_button *b = NULL;

    int scaled_x = 0;
    int scaled_y = 0;
    int scaled_w = 0;
    int scaled_h = 0;

    int scale = 1;
    if(cs->settings)
    {
        scale = cs->settings->videoScale;
    }

    for(i = 0; i < cs->gfx_buttons_max; i++)
    {
        if(!cs->gfx_buttons[i])
            continue;

        b = cs->gfx_buttons[i];
        scaled_x = scale * b->x;
        scaled_y = scale * b->y;
        scaled_w = scale * b->w;
        scaled_h = scale * b->h;

        if(cs->button_emergency_override && !(cs->gfx_buttons[i]->flags & BUTTON_EMERGENCY))
        {
            cs->gfx_buttons[i]->highlighted = 0;
            if(b->delete_check)
            {
                if(b->delete_check(cs))
                {
                    gfx_button_destroy(b);
                    cs->gfx_buttons[i] = NULL;
                }
            }

            continue;
        }

        if(cs->mouse_x < scaled_x + scaled_w && cs->mouse_x >= scaled_x && cs->mouse_y < scaled_y + scaled_h &&
           cs->mouse_y >= scaled_y)
            b->highlighted = 1;
        else
            b->highlighted = 0;

        if(b->highlighted && cs->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
        {
            if(b->action)
            {
                b->action(cs, b->data);
            }

            b->clicked = 3;
        }

        if(b->delete_check && (!b->clicked || b->flags & BUTTON_EMERGENCY))
        {
            if(b->delete_check(cs))
            {
                gfx_button_destroy(b);
                cs->gfx_buttons[i] = NULL;
            }
        }
    }

    for(i = 0; i < cs->gfx_buttons_max; i++)
    {
        if(!cs->gfx_buttons[i])
            continue;

        b = cs->gfx_buttons[i];

        if(cs->button_emergency_override && !(cs->gfx_buttons[i]->flags & BUTTON_EMERGENCY))
        {
            if(b->delete_check)
            {
                if(b->delete_check(cs))
                {
                    gfx_button_destroy(b);
                    cs->gfx_buttons[i] = NULL;
                }
            }

            continue;
        }

        if(b->delete_check && (!b->clicked || b->flags & BUTTON_EMERGENCY))
        {
            if(b->delete_check(cs))
            {
                gfx_button_destroy(b);
                cs->gfx_buttons[i] = NULL;
            }
        }
    }

    return 0;
}

int request_fps(coreState *cs, double fps)
{
    if(!cs)
        return -1;
    if(fps != FPS && fps != G2_FPS)
        return 1;

    cs->fps = fps;
    return 0;
}
