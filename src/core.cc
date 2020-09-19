#include "CoreState.h"
#include "Debug.h"
#include "definitions.h"
#include "DisplayMode.h"
#include "gfx_old.h"
#include "gfx_structures.h"
#include "game_menu.h"
#include "game_qs.h"
#include "GuiGridCanvas.h"
#include "GuiScreenManager.h"
#include "Input/KeyFlags.h"
#include "Magic.h"
#include "QRS1.h"
#include "RefreshRates.h"
#include "replay.h"
#include "SGUIL/SGUIL.h"
#include "ShiroPhysoMino.h"
#include "SPM_Spec.h"
#include "Version.h"
#include <array>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>
#ifdef ENABLE_OPENGL_INTERPOLATION
#define GL_GLEXT_PROTOTYPES
#include "glad/glad.h"
#endif

#define PENTOMINO_C_REVISION_STRING "rev 1.3"
#define FRAMEDELAY_ERR 0

#if(defined(_WIN64) || defined(_WIN32)) && !defined(__CYGWIN__) && !defined(__CYGWIN32__) && !defined(__MINGW32__) && \
    !defined(__MINGW64__)
#define _WIN32_WINNT 0x0400
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if 0
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
        // Signal after waiting for the requested time, but if the signal took
        // too long, use millisecond precision with WaitForSingleObject.
        SetWaitableTimer(timer, &waitTime, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, waitTime.QuadPart / 10000);
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
#endif

#define S_ISDIR(flags) ((flags) & S_IFDIR)
#else
#include <errno.h>
#include <sys/stat.h>
#endif

#if 0
static long framedelay(Uint64 ticks_elap, double fps)
{
    const Uint64 freq = SDL_GetPerformanceFrequency();
    const double start = (double)SDL_GetPerformanceCounter() / freq;
    if(fps < 1 || fps > 240)
        return FRAMEDELAY_ERR;

    struct timespec t = {0, 0};
    double sec_elap = (double)(ticks_elap) / freq;
    double spf = (1 / fps);

    if(sec_elap < spf)
    {
        t.tv_nsec = (long)((spf - sec_elap) * 1000000000ll);
#ifndef BUSYLOOP_DELAY

        struct timespec rem;
        if(nanosleep(&t, &rem))
        {
            // this can happen when the user presses Ctrl+C
            log_err("nanosleep() returned failure during frame length calculation");
            return FRAMEDELAY_ERR;
        }
#else
        const double end = start + spf - sec_elap;
        for (double newTime = start; newTime < end; newTime = (double)SDL_GetPerformanceCounter() / freq);
#endif
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
#endif

int is_left_input_repeat(CoreState *cs, int delay)
{
    return cs->keys.left && cs->hold_dir == Shiro::DASDirection::LEFT && cs->hold_time >= delay;
}

int is_right_input_repeat(CoreState *cs, int delay)
{
    return cs->keys.right && cs->hold_dir == Shiro::DASDirection::RIGHT && cs->hold_time >= delay;
}

int is_up_input_repeat(CoreState *cs, int delay)
{
    return cs->keys.up && cs->hold_dir == Shiro::DASDirection::UP && cs->hold_time >= delay;
}

int is_down_input_repeat(CoreState *cs, int delay)
{
    return cs->keys.down && cs->hold_dir == Shiro::DASDirection::DOWN && cs->hold_time >= delay;
}

CoreState::CoreState(Shiro::Settings& settings) :
    screen(Shiro::Version::DESCRIPTOR, settings.videoScale * 640u, settings.videoScale * 480u),
    settings(settings),
    gfx(screen)
{
    fps = Shiro::RefreshRates::menu;
    // keyquit = SDLK_F11;
    text_editing = 0;
    text_insert = NULL;
    text_backspace = NULL;
    text_delete = NULL;
    text_seek_left = NULL;
    text_seek_right = NULL;
    text_seek_home = NULL;
    text_seek_end = NULL;
    text_select_all = NULL;
    text_copy = NULL;
    text_cut = NULL;
    left_arrow_das = 0;
    right_arrow_das = 0;
    backspace_das = 0;
    delete_das = 0;
    select_all = false;
    undo = false;
    redo = false;

    for (auto& pressed : pressedDigits) {
        pressed = false;
    }

    assets = new Shiro::AssetStore();

    joystick = NULL;
    prev_keys_raw = {};
    keys_raw = {};
    prev_keys = {};
    keys = {};
    pressed = {};
    hold_dir = Shiro::DASDirection::NONE;
    hold_time = 0;

    mouse_x = 0;
    mouse_y = 0;
    logical_mouse_x = 0;
    logical_mouse_y = 0;
    mouse_left_down = 0;
    mouse_right_down = 0;

    bg = NULL;
    bg_old = NULL;
    bg_r = 0u;
    bg_g = 0u;
    bg_b = 0u;

    this->settings = settings;
    menu_input_override = false;
    button_emergency_override = false;
    p1game = NULL;
    menu = NULL;

    screenManager = new GuiScreenManager {};

    displayMode = Shiro::DisplayMode::DEFAULT;
    motionBlur = false;
    pracdata_mirror = NULL;

    //avg_sleep_ms = 0;
    //avg_sleep_ms_recent = 0;
    frames = 0;

#if 0
    for(i = 0; i < RECENT_FRAMES; i++)
    {
        avg_sleep_ms_recent_array[i] = 0;
    }

    recent_frame_overload = -1;
#endif
}

CoreState::~CoreState() {
    if (pracdata_mirror) {
        pracdata_destroy(pracdata_mirror);
    }
    //quit(this);

    scoredb_terminate(&records);
    // scoredb_terminate(&archive);

    if(assets)
    {

        // Already destroyed in the BitFont destructor previously; this prevents a double-free.
        assets->font_fixedsys_excelsior.tex = nullptr;

#define IMG(name) img_destroy(&assets->name);
#define IMG_ARRAY(name, i) img_destroy(&assets->name[i]);
#include "images.h"
#undef IMG_ARRAY
#undef IMG

        // All the textures have been destroyed, so prevent them being freed by
        // the GuiWindow destructor.
#define FONT(name, sheetName, outlineSheetName, charW, charH) \
        assets->name.isValid = false; \
        assets->name.sheet = nullptr; \
        assets->name.outlineSheet = nullptr
#include "fonts.h"
#undef FONT

#define MUSIC(name, i) delete assets->name[i];
#include "music.h"
#undef MUSIC

#define SFX(name) delete assets->name;
#define SFX_ARRAY(name, i) delete assets->name[i];
#include "sfx.h"
#undef SFX_ARRAY
#undef SFX

        delete assets;
    }

    if(joystick && SDL_JoystickGetAttached(joystick))
    {
        SDL_JoystickClose(joystick);
    }

    if(p1game)
    {
        std::cerr << "quit(): Found leftover game struct, attempting ->quit" << std::endl;
        p1game->quit(p1game);
        free(p1game);
        p1game = NULL;
    }

    if(menu)
    {
        menu->quit(menu);
        free(menu);
        menu = NULL;
    }

    /*for(int i = 0; i < 10; i++) {
       if(g2_bgs[i])
          gfx_animation_destroy(g2_bgs[i]);
    }*/

    gfx_quit(this);
}

bool CoreState::init() {
    if (!screen.init(settings)) {
        std::cerr << "Failed to init screen, aborting" << std::endl;
        return false;
    }

    try {
        // SDL_Texture *blank = NULL;

        // copy settings into main game structure
        if (SDL_NumJoysticks()) {
            const int numJoysticks = SDL_NumJoysticks();
            for (int i = 0; i < numJoysticks; i++) {
                SDL_Joystick* joystick;
                if ((joystick = SDL_JoystickOpen(i))) {
                    std::cerr << "Attached joystick \"" << SDL_JoystickName(joystick) << "\" at index " << i << std::endl;
                    SDL_JoystickClose(joystick);
                }
                else {
                    std::cerr << "Joystick at index " << i << " not attached" << std::endl;
                }
            }
            joystick = nullptr;
            if (settings.controllerBindings.name != "") {
                const int numJoysticks = SDL_NumJoysticks();
                for (int i = 0; i < numJoysticks; i++) {
                    if ((joystick = SDL_JoystickOpen(i))) {
                        if (SDL_JoystickName(joystick) == settings.controllerBindings.name) {
                            settings.controllerBindings.controllerIndex = i;
                            settings.controllerBindings.controllerID = SDL_JoystickInstanceID(joystick);
                            break;
                        }
                        else {
                            SDL_JoystickClose(joystick);
                            joystick = nullptr;
                        }
                    }
                }
            }
            if (!joystick && settings.controllerBindings.controllerIndex >= 0 && settings.controllerBindings.controllerIndex < SDL_NumJoysticks()) {
                if ((joystick = SDL_JoystickOpen(settings.controllerBindings.controllerIndex))) {
                    settings.controllerBindings.controllerID = SDL_JoystickInstanceID(joystick);
                }
            }
            if (settings.controllerBindings.controllerIndex >= 0 && joystick) {
                std::cerr
                    << "Joysticks are enabled" << std::endl
                    << "Name: " << SDL_JoystickNameForIndex(settings.controllerBindings.controllerIndex) << std::endl
                    << "Index: " << settings.controllerBindings.controllerIndex << std::endl
                    << "Buttons: " << SDL_JoystickNumButtons(joystick) << std::endl
                    << "Axes: " << SDL_JoystickNumAxes(joystick) << std::endl
                    << "Hats: " << SDL_JoystickNumHats(joystick) << std::endl;
            }
            else {
                joystick = nullptr;
                std::cerr << "Joysticks are disabled" << std::endl;
            }
        }
        else {
            std::cerr << "No joysticks are attached" << std::endl;
        }

        check(load_files(this) == 0, "load_files() returned failure\n");

        check(Gui_Init(screen.renderer, NULL), "Gui_Init() returned failure\n");
        check(gfx_init(this) == 0, "gfx_init returned failure\n");

        bg = assets->bg_temp.tex;
        bg_old = bg;
        bg_r = 255;
        bg_g = 255;
        bg_b = 255;
        // blank = assets->blank.tex;

        // check(SDL_RenderCopy(screen.renderer, blank, NULL, NULL) > -1, "SDL_RenderCopy: Error: %s\n", SDL_GetError());

        menu = menu_create(this);
        check(menu != NULL, "menu_create returned failure\n");

        menu->init(menu);

        screenManager->addScreen("main", mainMenu_create(this, screenManager, assets->fixedsys));
        //SDL_Rect gameScreenRect = {0, 0, 640, 480};
        //screenManager->addScreen("game", new GuiScreen {this, "game", NULL, gameScreenRect});
        screenManager->loadScreen("main");

        // check(SDL_RenderCopy(screen.renderer, blank, NULL, NULL) > -1, "SDL_RenderCopy: Error: %s\n", SDL_GetError());

        // TODO: Configurable directory
        static const char scoredb_file[] = "shiromino.sqlite";
        scoredb_init(&records, scoredb_file);
        scoredb_create_player(&records, &player, settings.playerName.c_str());

        /*
        static const char archive_file[] = "archive.db";
        scoredb_init(&archive, archive_file);
        scoredb_create_player(&archive, &player, settings.player_name);
        */

        std::cerr << "PENTOMINO C: " << PENTOMINO_C_REVISION_STRING << std::endl;
    }
    catch (const std::logic_error& error) {
        std::cerr << "Failed to init CoreState, aborting." << std::endl;
        return false;
    }

    return true;
}

void CoreState::run() {
    bool running = true;

    int windowWidth = 620;
    int windowHeight = 460;
    SDL_Rect gridRect = { 16, 44, windowWidth - 32, windowHeight - 60 };

    Shiro::Grid* g = new Shiro::Grid(gridRect.w / 16, gridRect.h / 16);
    SDL_Texture *paletteTex = assets->tets_bright_qs.tex;

    BindableInt paletteVar {"paletteVar", 0, 25};

    GuiGridCanvas *gridCanvas = new GuiGridCanvas {
        0,
        g,
        paletteVar,
        paletteTex,
        32, 32,
        gridRect
    };

    SDL_Rect windowRect = { 10, 10, windowWidth, windowHeight };
    // TODO: Fix how fixedsys is loaded/unloaded; currently, both SGUIL and
    // Shiromio try to free it, with SGUIL freeing it first in the GuiWindow
    // destructor.
    GuiWindow window {this, "Shiromino", &assets->fixedsys, GUI_WINDOW_CALLBACK_NONE, windowRect};

    window.addControlElement(gridCanvas);

    // SPM_Spec spec;
    QRS spec { qrs_variant_P, false };
    TestSPM SPMgame { *this, &spec };
    SPMgame.init();

    //p1game = qs_game_create(this, 0, MODE_PENTOMINO, NO_REPLAY);
    //p1game->init(p1game);

    double currentTime = static_cast<double>(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
    double timeAccumulator = 0.0;
// #define DEBUG_FRAME_TIMING
#ifdef DEBUG_FRAME_TIMING
    // Due to limitations in SDL's display refresh rate reporting, FPS
    // debugging can't be made to display the correct monitor FPS when vsync
    // and vsyncTimestep are enabled. So use the VIDEO_FPS INI option to get
    // correct FPS information.
    double videoFPS;
    {
        PDINI::INI ini;
        ini.read(configurationPath);
        if (!ini.get("SCREEN", "VIDEO_FPS", videoFPS) || videoFPS <= 0.0) {
            videoFPS = 0.0;
        }
    }
    double timeFromFrames = 0.0;
    const double fpsTimeFrameDuration = 1.0;
    double fpsTimeFrameStart = 0.0;
#endif

    while(running)
    {
        double newTime = static_cast<double>(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
        double renderFrameTime = newTime - currentTime;
        if (renderFrameTime > 0.25) {
            renderFrameTime = 0.25;
        }
        currentTime = newTime;
        timeAccumulator += renderFrameTime;

        unsigned newFrames = 0u;
        for (
#ifdef DEBUG_FRAME_TIMING
            double gameFrameTime = 1.0 / (settings.vsync && settings.vsyncTimestep && videoFPS > 0.0 ? videoFPS : fps);
#else
            double gameFrameTime = 1.0 / fps;
#endif
            timeAccumulator >= gameFrameTime || (settings.vsyncTimestep && settings.vsync && newFrames == 0u);
            timeAccumulator -= gameFrameTime,
            newFrames++,
            frames++
            ) {
            prev_keys_raw = keys_raw;
            prev_keys = keys;

            if (process_events(this)) {
                running = false;
            }

            handle_replay_input(this);

            update_input_repeat(this);
            update_pressed(this);

            gfx_buttons_input(this);

            /*
            SPMgame.input();
            SPMgame.frame();
            SPMgame.draw();
            */

            gfx.clearLayers();

            if (p1game) {
                if (!procgame(p1game, !button_emergency_override)) {
                    p1game->quit(p1game);
                    free(p1game);
                    p1game = NULL;

                    gfx_start_bg_fade_in(this, assets->bg_temp.tex);
                    break;
                }
            }
            if (menu && ((!p1game || menu_input_override) ? 1 : 0)) {
                if (!procgame(menu, !button_emergency_override)) {
                    menu->quit(menu);
                    free(menu);

                    menu = NULL;

                    if (!p1game) {
                        running = false;
                    }
                }
            }

            if (!menu && !p1game) {
                running = false;
            }
            else if (p1game) {
                p1game->frame_counter++;
            }
            else if (menu && (!p1game || menu_input_override)) {
                menu->frame_counter++;
            }

            gfx_updatebg(this);

            // TODO: Remove these OldGfx* types once all the old gfx_push* functions are replaced with calls of Gfx::push.
            struct OldGfxGraphic : public Shiro::Graphic {
                OldGfxGraphic() = delete;

                OldGfxGraphic(const std::function<void()> drawLambda) : drawLambda(drawLambda) {}

                void draw(const Shiro::Screen& screen) const {
                    drawLambda();
                }

                const std::function<void()> drawLambda;
            };

            class OldGfxEntity : public Shiro::Entity {
            public:
                OldGfxEntity(
                    const size_t layerNum,
                    const std::function<void()> drawLambda
                ) :
                    layerNum(layerNum),
                    drawLambda(drawLambda) {}

                bool update(Shiro::Layers& layers) {
                    layers.push(layerNum, std::make_shared<OldGfxGraphic>(drawLambda));
                    return false;
                }

            private:
                const size_t layerNum;
                const std::function<void()> drawLambda;
            };

            // TODO: Create entities in the code for the game and menu, then remove this.
            // Or perhaps have the game and menu code push entities, and no longer
            // have explicit game and menu drawing functions.
            gfx.push(std::make_unique<OldGfxEntity>(
                static_cast<size_t>(Shiro::GfxLayer::base),
                [this] {
                    if (p1game) {
                        p1game->draw(p1game);
                    }
                    else if (menu && ((!p1game || menu_input_override) ? 1 : 0)) {
                        menu->draw(menu);
                    }
                }
            ));

            // TODO: Create entities in the code for buttons, then remove this.
            gfx.push(std::make_unique<OldGfxEntity>(
                static_cast<size_t>(Shiro::GfxLayer::buttons),
                [this] { gfx_drawbuttons(this, 0); }
            ));

            // TODO: Create an entity for the background darkening, then remove this.
            if (button_emergency_override) {
                gfx.push(std::make_unique<OldGfxEntity>(
                    static_cast<size_t>(Shiro::GfxLayer::emergencyBgDarken),
                    [this] { gfx_draw_emergency_bg_darken(this); }
                ));
            }

            // TODO: Create entities in the code for emergency buttons, then remove this.
            gfx.push(std::make_unique<OldGfxEntity>(
                static_cast<size_t>(Shiro::GfxLayer::emergencyButtons),
                [this] { gfx_drawbuttons(this, EMERGENCY_OVERRIDE); }
            ));

            gfx.update();

#ifndef DEBUG_FRAME_TIMING
            gameFrameTime = 1.0 / fps;
#else
            gameFrameTime = 1.0 / (settings.vsync && settings.vsyncTimestep && videoFPS > 0.0 ? videoFPS : fps);
            timeFromFrames += gameFrameTime;
            // TODO: From testing with this, the game apparently doesn't reset
            // FPS to 60 when returning to the menu from a game, so fix that.
            // Though each mode does use its correct FPS.
            double realTime = static_cast<double>(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency() - startTime;
            std::cerr
                << "real FPS: " << (realTime / timeFromFrames) * (settings.vsync && settings.vsyncTimestep && videoFPS > 0.0
                    ? videoFPS
                    : fps
                ) << std::endl
                << "game FPS: " << settings.vsync && settings.vsyncTimestep && videoFPS > 0.0
                    ? videoFPS
                    : fps
                ) << std::endl;
            if (realTime - fpsTimeFrameStart >= fpsTimeFrameDuration) {
                timeFromFrames = realTime;
                fpsTimeFrameStart = realTime;
            }
#endif
            if (settings.vsync && settings.vsyncTimestep) {
                timeAccumulator = 0.0;
            }
        }

        SDL_SetRenderTarget(screen.renderer, screen.target_tex);
        SDL_RenderClear(screen.renderer);

        gfx_drawbg(this);

        gfx.draw();

#ifdef ENABLE_OPENGL_INTERPOLATION
        if (settings.interpolate) {
            SDL_RenderFlush(screen.renderer);
            SDL_SetRenderTarget(screen.renderer, NULL);

            glUseProgram(screen.interpolate_shading_prog);
            int w, h;
            SDL_GL_GetDrawableSize(screen.window, &w, &h);
            if ((SDL_GetWindowFlags(screen.window) & ~SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN) {
                double widthFactor, heightFactor;
                if (settings.videoStretch) {
                    widthFactor = w / 640.0;
                    heightFactor = h / 480.0;
                }
                else {
                    widthFactor = w / 640;
                    heightFactor = h / 480;
                    if (widthFactor > settings.videoScale) {
                        widthFactor = settings.videoScale;
                    }
                    if (heightFactor > settings.videoScale) {
                        heightFactor = settings.videoScale;
                    }
                }
                GLsizei viewportWidth, viewportHeight;
                if (widthFactor > heightFactor) {
                    viewportWidth = heightFactor * 640;
                    viewportHeight = heightFactor * 480;
                }
                else {
                    viewportWidth = widthFactor * 640;
                    viewportHeight = widthFactor * 480;
                }
                glViewport((w - viewportWidth) / 2, (h - viewportHeight) / 2, viewportWidth, viewportHeight);
                glUniform2f(glGetUniformLocation(screen.interpolate_shading_prog, "viewportSize"), viewportWidth, viewportHeight);
            }
            else {
                // TODO: Change this once a fullscreen resolution setting is supported.
                glViewport(0, 0, w, h);
                glUniform2f(glGetUniformLocation(screen.interpolate_shading_prog, "viewportSize"), w, h);
            }
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            if (SDL_GL_BindTexture(screen.target_tex, NULL, NULL) < 0) {
                std::cerr << "Failed to bind `target_tex`." << std::endl;
            }
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4u);
            glUseProgram(0u);
            if (SDL_GL_UnbindTexture(screen.target_tex) < 0) {
                std::cerr << "Failed to unbind `target_tex`." << std::endl;
            }

            SDL_GL_SwapWindow(screen.window);
        }
        else {
            SDL_RenderPresent(screen.renderer);
        }
#else
        SDL_RenderPresent(screen.renderer);
#endif

#if 0
        if (newFrames > 0u) {
            for (size_t i = 0; i < gfx_messages_max; i++) {
                if (gfx_messages[i].counter >= newFrames) {
                    gfx_messages[i].counter -= newFrames;
                }
                else {
                    gfx_messages[i].counter = 0u;
                }
            }
        }
#endif
        if (!settings.vsync) {
            SDL_Delay(settings.frameDelay);
        }
    }
}

static void load_image(CoreState *cs, gfx_image *img, const char *filename)
{
    std::filesystem::path path { cs->settings.basePath };
    if(!img_load(img, path / "assets" / "image" / filename, cs))
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

static void load_sfx(CoreState* cs, PDINI::INI& ini, Shiro::Sfx** s, const char* filename)
{
    std::filesystem::path basePath { cs->settings.basePath };
    *s = new Shiro::Sfx();
    if (!(*s)->load(basePath / "assets" / "audio" / filename)) {
        log_warn("Failed to load audio '%s'", filename);
    }
    float volume;
    if (!ini.get("", filename, volume) || volume < 0.0f || volume > 100.0f) {
        (*s)->volume = 100.0f;
    }
    else {
        (*s)->volume = volume;
    }
}

static void load_music(CoreState* cs, PDINI::INI& ini, Shiro::Music*& m, const char* name)
{
    std::filesystem::path basePath { cs->settings.basePath };
    m = new Shiro::Music();
    if (!m->load(basePath / "assets" / "audio" / name)) {
        log_warn("Failed to load music '%s'", name);
    }

    float volume;
    if (!ini.get("", name, volume) || volume < 0.0f || volume > 100.0f) {
        m->volume = 100.0f;
    }
    else {
        m->volume = volume;
    }
}

int load_files(CoreState *cs)
{
    if(!cs)
        return -1;

        // image assets

#define IMG(name) load_image(cs, &cs->assets->name, #name);
#define IMG_ARRAY(name, i) load_image(cs, &cs->assets->name[i], #name #i);
#include "images.h"
#undef IMG_ARRAY
#undef IMG

#define FONT(name, sheetName, outlineSheetName, charW, charH) \
    load_bitfont(&cs->assets->name, &cs->assets->sheetName, &cs->assets->outlineSheetName, charW, charH)
#include "fonts.h"
#undef FONT

        // audio assets

    {
        PDINI::INI ini(false);
        std::filesystem::path basePath { cs->settings.basePath };
        ini.read(basePath / "assets" / "audio" / "volume.ini");

#define MUSIC(name, i) load_music(cs, ini, cs->assets->name[i], #name #i);
#include "music.h"
#undef MUSIC

#define SFX(name) load_sfx(cs, ini, &cs->assets->name, #name);
#define SFX_ARRAY(name, i) load_sfx(cs, ini, &cs->assets->name[i], #name #i);
#include "sfx.h"
#undef SFX_ARRAY
#undef SFX
    }
    return 0;
}

void quit(CoreState *cs)
{
    scoredb_terminate(&cs->records);
    // scoredb_terminate(&cs->archive);

    if(cs->assets)
    {

        // Already destroyed in the BitFont destructor previously; this prevents a double-free.
        cs->assets->font_fixedsys_excelsior.tex = nullptr;

#define IMG(name) img_destroy(&cs->assets->name);
#define IMG_ARRAY(name, i) img_destroy(&cs->assets->name[i]);
#include "images.h"
#undef IMG_ARRAY
#undef IMG

        // All the textures have been destroyed, so prevent them being freed by
        // the GuiWindow destructor.
#define FONT(name, sheetName, outlineSheetName, charW, charH) \
        cs->assets->name.isValid = false; \
        cs->assets->name.sheet = nullptr; \
        cs->assets->name.outlineSheet = nullptr
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

    if(cs->joystick && SDL_JoystickGetAttached(cs->joystick))
    {
        SDL_JoystickClose(cs->joystick);
    }

    if(cs->p1game)
    {
        std::cerr << "quit(): Found leftover game struct, attempting ->quit" << std::endl;
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
}

void update_mouse(CoreState* cs, const int windowW, const int windowH) {
    if(cs->mouse_left_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
    {
        cs->mouse_left_down = 1;
    }
    if(cs->mouse_right_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
    {
        cs->mouse_right_down = 1;
    }

    if(cs->select_all)
    {
        cs->select_all = false;
    }
    if(cs->undo)
    {
        cs->undo = false;
    }
    if(cs->redo)
    {
        cs->redo = false;
    }

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
}

// TODO: Make this only read in events and update state; move out all logic interpreting the new state into the game/menu code, such as DAS processing. And rename this to poll_events once that's all done.
int process_events(CoreState *cs) {
    if (!cs) {
        return -1;
    }

    Shiro::KeyFlags *k = NULL;

    SDL_Keycode keyCode;
    SDL_Keymod keyMod;
    Shiro::KeyBindings& keyBindings = cs->settings.keyBindings;

    Shiro::ControllerBindings& controllerBindings = cs->settings.controllerBindings;

#if 0
    if(cs->mouse_left_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
    {
        cs->mouse_left_down = 1;
    }
    if(cs->mouse_right_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
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
#endif

    int windowW, windowH;
    SDL_GetWindowSize(cs->screen.window, &windowW, &windowH);
    SDL_Event event;
#define CHECK_BINDINGS(check) \
    check(left); \
    check(right); \
    check(up); \
    check(down); \
    check(start); \
    check(a); \
    check(b); \
    check(c); \
    check(d); \
    check(escape);
    while (SDL_PollEvent(&event)) {
        const auto currentDeadZone = Shiro::ControllerBindings::MAXIMUM_DEAD_ZONE * controllerBindings.deadZone;
        switch (event.type) {
            case SDL_QUIT:
                return 1;

            case SDL_JOYAXISMOTION:
                k = &cs->keys_raw;
                if (event.jaxis.which == controllerBindings.controllerID) {
                    if (event.jaxis.axis == controllerBindings.axes.x) {
                        if (event.jaxis.value * controllerBindings.axes.right > currentDeadZone) {
                            k->right = 1;
                            k->left = 0;
                        }
                        else if (event.jaxis.value * controllerBindings.axes.right < -currentDeadZone) {
                            k->left = 1;
                            k->right = 0;
                        }
                        else {
                            k->right = 0;
                            k->left = 0;
                        }
                    }
                    else if (event.jaxis.axis == controllerBindings.axes.y) {
                        if (event.jaxis.value * controllerBindings.axes.down > currentDeadZone) {
                            k->down = 1;
                            k->up = 0;
                        }
                        else if (event.jaxis.value * controllerBindings.axes.down < -currentDeadZone) {
                            k->up = 1;
                            k->down = 0;
                        }
                        else {
                            k->up = 0;
                            k->down = 0;
                        }
                    }
                }

                break;

            case SDL_JOYHATMOTION:
                k = &cs->keys_raw;

                if (event.jhat.which == controllerBindings.controllerID && event.jhat.hat == controllerBindings.hatIndex) {
                    if (event.jhat.value == SDL_HAT_CENTERED) {
                        k->right = 0;
                        k->left = 0;
                        k->up = 0;
                        k->down = 0;
                    }
                    else {
                        if ((event.jhat.value & (SDL_HAT_LEFT | SDL_HAT_RIGHT)) != (SDL_HAT_LEFT | SDL_HAT_RIGHT)) {
                            k->left = !!(event.jhat.value & SDL_HAT_LEFT);
                            k->right = !!(event.jhat.value & SDL_HAT_RIGHT);
                        }
                        else {
                            k->left = 0;
                            k->right = 0;
                        }

                        if ((event.jhat.value & (SDL_HAT_UP | SDL_HAT_DOWN)) != (SDL_HAT_UP | SDL_HAT_DOWN)) {
                            k->up = !!(event.jhat.value & SDL_HAT_UP);
                            k->down = !!(event.jhat.value & SDL_HAT_DOWN);
                        }
                        else {
                            k->up = 0;
                            k->down = 0;
                        }
                    }
                }

                break;

            case SDL_JOYBUTTONDOWN:
                k = &cs->keys_raw;
                if (event.jbutton.which == controllerBindings.controllerID) {
#define CHECK_JOYBUTTONDOWN(name) \
                    if (event.jbutton.button == controllerBindings.buttons.name) { \
                        k->name = 1; \
                    }
                    CHECK_BINDINGS(CHECK_JOYBUTTONDOWN);
#undef CHECK_JOYBUTTONDOWN
                }

                break;

            case SDL_JOYBUTTONUP:
                k = &cs->keys_raw;
                if (event.jbutton.which == controllerBindings.controllerID) {
#define CHECK_JOYBUTTONUP(name) \
                    if (event.jbutton.button == controllerBindings.buttons.name) { \
                        k->name = 0; \
                    }
                    CHECK_BINDINGS(CHECK_JOYBUTTONUP);
#undef CHECK_JOYBUTTONUP
                }

                break;
#if 0
#endif

            case SDL_KEYDOWN:
                if (event.key.repeat) {
                    break;
                }

                keyCode = event.key.keysym.sym;
                keyMod = SDL_GetModState();

                k = &cs->keys_raw;

#define CHECK_KEYDOWN(name) \
                if (keyCode == keyBindings.name) { \
                    k->name = 1; \
                }
                CHECK_BINDINGS(CHECK_KEYDOWN);
#undef CHECK_KEYDOWN

                switch (keyCode) {
                case SDLK_v:
                    if ((keyMod & KMOD_CTRL) && cs->text_editing && cs->text_insert) {
                        cs->text_insert(cs, SDL_GetClipboardText());
                    }
                    break;

                case SDLK_c:
                    if ((keyMod & KMOD_CTRL) && cs->text_editing && cs->text_insert) {
                        cs->text_copy(cs);
                    }
                    break;

                case SDLK_x:
                    if ((keyMod & KMOD_CTRL) && cs->text_editing && cs->text_cut) {
                        cs->text_cut(cs);
                    }
                    break;

                case SDLK_a:
                    if (keyMod & KMOD_CTRL) {
                        if (cs->text_editing && cs->text_select_all) {
                            cs->text_select_all(cs);
                        }
                        cs->select_all = true;
                    }
                    break;

                case SDLK_z:
                    if (keyMod & KMOD_CTRL) {
                        cs->undo = true;
                    }
                    break;

                case SDLK_y:
                    if (keyMod & KMOD_CTRL) {
                        cs->redo = true;
                    }
                    break;

                case SDLK_BACKSPACE:
                    cs->backspace_das = 1;
                    break;

                case SDLK_DELETE:
                    cs->delete_das = 1;
                    break;

                case SDLK_HOME:
                    if (cs->text_editing && cs->text_seek_home) {
                        cs->text_seek_home(cs);
                    }
                    break;

                case SDLK_END:
                    if (cs->text_editing && cs->text_seek_end) {
                        cs->text_seek_end(cs);
                    }
                    break;

                case SDLK_RETURN:
                    if (cs->text_toggle) {
                        cs->text_toggle(cs);
                    }
                    break;

                case SDLK_LEFT:
                    cs->left_arrow_das = 1;
                    break;

                case SDLK_RIGHT:
                    cs->right_arrow_das = 1;
                    break;

                case SDLK_F4:
                    if (keyMod & KMOD_ALT) {
                        return 1;
                    }
                    break;

                case SDLK_F8:
                    switch (cs->displayMode) {
                    case Shiro::DisplayMode::DEFAULT:
                        cs->displayMode = Shiro::DisplayMode::DETAILED;
                        break;

                    case Shiro::DisplayMode::DETAILED:
                        cs->displayMode = Shiro::DisplayMode::CENTERED;
                        break;

                    default:
                        cs->displayMode = Shiro::DisplayMode::DEFAULT;
                        break;
                    }
                    break;

                case SDLK_F9:
                    cs->motionBlur = !cs->motionBlur;
                    break;

                case SDLK_F11:
                    if (cs->settings.fullscreen) {
                        cs->settings.fullscreen = false;
                        SDL_SetWindowFullscreen(cs->screen.window, 0);
                        SDL_SetWindowSize(cs->screen.window, 640.0 * cs->settings.videoScale, 480.0 * cs->settings.videoScale);
                    }
                    else {
                        cs->settings.fullscreen = true;
                        SDL_SetWindowSize(cs->screen.window, 640, 480);
                        SDL_WindowFlags flags = (keyMod & KMOD_SHIFT) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
                        if (SDL_SetWindowFullscreen(cs->screen.window, flags) < 0) {
                            std::cout << "SDL_SetWindowFullscreen(): Error: " << SDL_GetError() << std::endl;
                        }
                    }
                    break;

                case SDLK_0:
                    cs->pressedDigits[0] = true;
                    break;

                case SDLK_1:
                    if (keyMod & KMOD_ALT) {
                        cs->settings.videoScale = 1;
                        SDL_SetWindowSize(cs->screen.window, 640, 480);
                    }
                    cs->pressedDigits[1] = true;
                    break;

                case SDLK_2:
                    if (keyMod & KMOD_ALT) {
                        cs->settings.videoScale = 2;
                        SDL_SetWindowSize(cs->screen.window, 2 * 640, 2 * 480);
                    }
                    cs->pressedDigits[2] = true;
                    break;

                case SDLK_3:
                    if (keyMod & KMOD_ALT) {
                        cs->settings.videoScale = 3;
                        SDL_SetWindowSize(cs->screen.window, 3 * 640, 3 * 480);
                    }
                    cs->pressedDigits[3] = true;
                    break;

                case SDLK_4:
                    if (keyMod & KMOD_ALT) {
                        cs->settings.videoScale = 4;
                        SDL_SetWindowSize(cs->screen.window, 4 * 640, 4 * 480);
                    }
                    cs->pressedDigits[4] = true;
                    break;

                case SDLK_5:
                    if (keyMod & KMOD_ALT) {
                        cs->settings.videoScale = 5;
                        SDL_SetWindowSize(cs->screen.window, 5 * 640, 5 * 480);
                    }
                    cs->pressedDigits[5] = true;
                    break;

                case SDLK_6:
                    cs->pressedDigits[6] = true;
                    break;

                case SDLK_7:
                    cs->pressedDigits[7] = true;
                    break;

                case SDLK_8:
                    cs->pressedDigits[8] = true;
                    break;

                case SDLK_9:
                    cs->pressedDigits[9] = true;
                    break;

                default:
                    break;
                }
                break;

            case SDL_KEYUP:
                keyCode = event.key.keysym.sym;

                k = &cs->keys_raw;

#define CHECK_KEYUP(name) \
                if (keyCode == keyBindings.name) { \
                    k->name = 0; \
                }
                CHECK_BINDINGS(CHECK_KEYUP);
#undef CHECK_KEYUP

                switch (keyCode) {
                case SDLK_LEFT:
                    cs->left_arrow_das = 0;
                    break;

                case SDLK_RIGHT:
                    cs->right_arrow_das = 0;
                    break;

                case SDLK_BACKSPACE:
                    cs->backspace_das = 0;
                    break;

                case SDLK_DELETE:
                    cs->delete_das = 0;
                    break;

                case SDLK_0:
                case SDLK_1:
                case SDLK_2:
                case SDLK_3:
                case SDLK_4:
                case SDLK_5:
                case SDLK_6:
                case SDLK_7:
                case SDLK_8:
                case SDLK_9:
                    cs->pressedDigits[keyCode - SDLK_0] = false;
                    break;

                default:
                    break;
                }

                break;

            case SDL_TEXTINPUT:
                if (cs->text_editing)
                {
                    if (!((event.text.text[0] == 'c' || event.text.text[0] == 'C') &&
                        (event.text.text[0] == 'v' || event.text.text[0] == 'V') &&
                        (event.text.text[0] == 'x' || event.text.text[0] == 'X') &&
                        (event.text.text[0] == 'a' || event.text.text[0] == 'A') && SDL_GetModState() & KMOD_CTRL))
                    {
                        if (cs->text_insert)
                            cs->text_insert(cs, event.text.text);
                    }
                }

                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    cs->mouse_left_down = Shiro::Magic::BUTTON_PRESSED_THIS_FRAME;
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                    cs->mouse_right_down = Shiro::Magic::BUTTON_PRESSED_THIS_FRAME;
                update_mouse(cs, windowW, windowH);
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    cs->mouse_left_down = 0;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    cs->mouse_right_down = 0;
                update_mouse(cs, windowW, windowH);
                break;

            case SDL_MOUSEMOTION:
                cs->mouse_x = event.motion.x;
                cs->mouse_y = event.motion.y;
                update_mouse(cs, windowW, windowH);
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    SDL_GetWindowSize(cs->screen.window, &windowW, &windowH);
                    update_mouse(cs, windowW, windowH);
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
        }
    }
#undef CHECK_BINDINGS

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
        SDL_Joystick *joy = cs->joystick;
        const uint8_t *keystates = SDL_GetKeyboardState(NULL);

        if (cs->settings.keyBindings) {
            k = &cs->keys_raw;
            kb = cs->settings.keyBindings;

            *k = (Shiro::KeyFlags) { 0 };

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

bool procgame(game_t *g, int input_enabled) {
    if(!g)
        return false;

    if(g->preframe)
    {
        if(g->preframe(g))
            return false;
    }

    if(g->input && input_enabled)
    {
        bool inputStatus = !g->input(g);
        if (!inputStatus) return false;
    }

    Uint64 benchmark = SDL_GetPerformanceCounter();

    if(g->frame)
    {
        if(g->frame(g))
            return false;
    }

    benchmark = SDL_GetPerformanceCounter() - benchmark;
//     std::cerr << (double) (benchmark) * 1000 / (double) SDL_GetPerformanceFrequency() << " ms" << std::endl;

    return true;
}

void handle_replay_input(CoreState *cs)
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

void update_input_repeat(CoreState *cs)
{
    Shiro::KeyFlags *k = &cs->keys;

    if(cs->hold_dir == Shiro::DASDirection::LEFT && k->right)
    {
        cs->hold_time = 0;
        cs->hold_dir = Shiro::DASDirection::RIGHT;
    }
    else if(cs->hold_dir == Shiro::DASDirection::RIGHT && k->left)
    {
        cs->hold_time = 0;
        cs->hold_dir = Shiro::DASDirection::LEFT;
    }
    else if(cs->hold_dir == Shiro::DASDirection::UP && k->down)
    {
        cs->hold_time = 0;
        cs->hold_dir = Shiro::DASDirection::DOWN;
    }
    else if(cs->hold_dir == Shiro::DASDirection::DOWN && k->up)
    {
        cs->hold_time = 0;
        cs->hold_dir = Shiro::DASDirection::UP;
    }

    if(cs->hold_dir == Shiro::DASDirection::LEFT && k->left)
        cs->hold_time++;
    else if(cs->hold_dir == Shiro::DASDirection::RIGHT && k->right)
        cs->hold_time++;
    else if(cs->hold_dir == Shiro::DASDirection::UP && k->up)
        cs->hold_time++;
    else if(cs->hold_dir == Shiro::DASDirection::DOWN && k->down)
        cs->hold_time++;
    else
    {
        if(k->left)
            cs->hold_dir = Shiro::DASDirection::LEFT;
        else if(k->right)
            cs->hold_dir = Shiro::DASDirection::RIGHT;
        else if(k->up)
            cs->hold_dir = Shiro::DASDirection::UP;
        else if(k->down)
            cs->hold_dir = Shiro::DASDirection::DOWN;
        else
            cs->hold_dir = Shiro::DASDirection::NONE;

        cs->hold_time = 0;
    }
}

void update_pressed(CoreState *cs)
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

int button_emergency_inactive(CoreState *cs)
{
    if(cs->button_emergency_override)
        return 0;
    else
        return 1;

    return 1;
}

int gfx_buttons_input(CoreState *cs)
{
    if(!cs)
        return -1;

    if(!cs->gfx_buttons.size())
        return 1;

    int i = 0;

    int scaled_x = 0;
    int scaled_y = 0;
    int scaled_w = 0;
    int scaled_h = 0;

    int scale = cs->settings.videoScale;

    for (auto it = cs->gfx_buttons.begin(); it != cs->gfx_buttons.end(); it++) {
        gfx_button& b = *it;
        scaled_x = scale * b.x;
        scaled_y = scale * b.y;
        scaled_w = scale * b.w;
        scaled_h = scale * b.h;

        if(cs->button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
        {
            b.highlighted = 0;
            if(b.delete_check)
            {
                if(b.delete_check(cs))
                {
                    cs->gfx_buttons.erase(it);
                }
            }

            continue;
        }

        if(cs->mouse_x < scaled_x + scaled_w && cs->mouse_x >= scaled_x && cs->mouse_y < scaled_y + scaled_h &&
           cs->mouse_y >= scaled_y)
            b.highlighted = 1;
        else
            b.highlighted = 0;

        if(b.highlighted && cs->mouse_left_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
        {
            if(b.action)
            {
                b.action(cs, b.data);
            }

            b.clicked = 4;
        }
        if (b.clicked) {
            b.clicked--;
        }

        if(b.delete_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check(cs))
            {
                cs->gfx_buttons.erase(it);
            }
        }
    }

    for (auto it = cs->gfx_buttons.begin(); it != cs->gfx_buttons.end(); it++) {
        gfx_button& b = cs->gfx_buttons[i];

        if(cs->button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check)
            {
                if(b.delete_check(cs))
                {
                    cs->gfx_buttons.erase(it);
                }
            }

            continue;
        }

        if(b.delete_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check(cs))
            {
                cs->gfx_buttons.erase(it);
            }
        }
    }

    return 0;
}

int request_fps(CoreState *cs, double fps)
{
    if(!cs)
        return -1;
    if(fps != 60.00 && fps != Shiro::RefreshRates::g1 && fps != Shiro::RefreshRates::g2 && fps != Shiro::RefreshRates::g3)
        return 1;

    cs->fps = fps;
    return 0;
}
