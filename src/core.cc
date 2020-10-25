#include "CoreState.h"
#include "Game.h"
#include "Asset/Image.h"
#include "Asset/Font.h"
#include "Asset/Music.h"
#include "Asset/Sfx.h"
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
#include "Input/Mouse.h"
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
#include <utility>
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

bool CoreState::is_left_input_repeat(unsigned delay) {
    return keys.left && hold_dir == Shiro::DASDirection::LEFT && hold_time >= delay;
}

bool CoreState::is_right_input_repeat(unsigned delay) {
    return keys.right && hold_dir == Shiro::DASDirection::RIGHT && hold_time >= delay;
}

bool CoreState::is_up_input_repeat(unsigned delay) {
    return keys.up && hold_dir == Shiro::DASDirection::UP && hold_time >= delay;
}

bool CoreState::is_down_input_repeat(unsigned delay) {
    return keys.down && hold_dir == Shiro::DASDirection::DOWN && hold_time >= delay;
}

CoreState::CoreState(Shiro::Settings& settings) :
    screen(Shiro::Version::DESCRIPTOR, static_cast<unsigned>(settings.videoScale * 640.0f), static_cast<unsigned>(settings.videoScale * 480.0f)),
    settings(settings),
    bg(screen),
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

    bg.transition();

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
        assets->name.outlineSheet = nullptr;
#include "fonts.h"
#undef FONT

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

    delete screenManager;

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
        assetMgr.addLoader(Shiro::AssetType::image, std::unique_ptr<Shiro::AssetLoader>(new Shiro::ImageAssetLoader(settings.basePath / "assets" / "image", screen)));
        assetMgr.addLoader(Shiro::AssetType::font, std::unique_ptr<Shiro::AssetLoader>(new Shiro::FontAssetLoader(settings.basePath / "assets" / "font", screen)));
        assetMgr.addLoader(Shiro::AssetType::music, std::unique_ptr<Shiro::AssetLoader>(new Shiro::MusicAssetLoader(settings.basePath / "assets" / "audio")));
        assetMgr.addLoader(Shiro::AssetType::sfx, std::unique_ptr<Shiro::AssetLoader>(new Shiro::SfxAssetLoader(settings.basePath / "assets" / "audio")));

        assetMgr.preload({
            {Shiro::AssetType::image, "title"},
            {Shiro::AssetType::image, "title_emboss"},
            {Shiro::AssetType::image, "tetrion_qs_white"},
            {Shiro::AssetType::image, "tets_bright_qs"},
            {Shiro::AssetType::image, "tets_bright_qs_small"},
            {Shiro::AssetType::image, "tets_dark_qs"},
            {Shiro::AssetType::image, "tets_jeweled"},
            //{Shiro::AssetType::image, "g2_tets_bright"},
            //{Shiro::AssetType::image, "g2_tets_bright_small"},
            //{Shiro::AssetType::image, "g2_tets_dark"},
            {Shiro::AssetType::image, "playfield_grid"},
            {Shiro::AssetType::image, "playfield_grid_alt"},
            {Shiro::AssetType::image, "misc"},
            {Shiro::AssetType::image, "medals"},

            {Shiro::AssetType::image, "bg0"},
            {Shiro::AssetType::image, "bg1"},
            {Shiro::AssetType::image, "bg2"},
            {Shiro::AssetType::image, "bg3"},
            {Shiro::AssetType::image, "bg4"},
            {Shiro::AssetType::image, "bg5"},
            {Shiro::AssetType::image, "bg6"},
            {Shiro::AssetType::image, "bg7"},
            {Shiro::AssetType::image, "bg8"},
            {Shiro::AssetType::image, "bg9"},
            {Shiro::AssetType::image, "bg_temp"},
            {Shiro::AssetType::image, "bg_darken"},

            {Shiro::AssetType::image, "animation_lineclear0"},
            {Shiro::AssetType::image, "animation_lineclear1"},
            {Shiro::AssetType::image, "animation_lineclear2"},
            {Shiro::AssetType::image, "animation_lineclear3"},
            {Shiro::AssetType::image, "animation_lineclear4"},

            {Shiro::AssetType::image, "g1_tetrion"},
            {Shiro::AssetType::image, "g2_tetrion_death"},
            {Shiro::AssetType::image, "g2_tetrion_master"},
            {Shiro::AssetType::image, "g3_tetrion_terror"},


            {Shiro::AssetType::font, "fixedsys"},
            // TODO: Convert remaining fonts to AngelCode bitmap font format.


            {Shiro::AssetType::music, "tracks0"},
            {Shiro::AssetType::music, "tracks1"},
            {Shiro::AssetType::music, "tracks2"},
            {Shiro::AssetType::music, "tracks3"},

            {Shiro::AssetType::music, "g1_tracks0"},
            {Shiro::AssetType::music, "g1_tracks1"},

            {Shiro::AssetType::music, "g2_tracks0"},
            {Shiro::AssetType::music, "g2_tracks1"},
            {Shiro::AssetType::music, "g2_tracks2"},
            {Shiro::AssetType::music, "g2_tracks3"},

            {Shiro::AssetType::music, "g3_tracks0"},
            {Shiro::AssetType::music, "g3_tracks1"},
            {Shiro::AssetType::music, "g3_tracks2"},
            {Shiro::AssetType::music, "g3_tracks3"},
            {Shiro::AssetType::music, "g3_tracks4"},
            {Shiro::AssetType::music, "g3_tracks5"},


            {Shiro::AssetType::sfx, "menu_choose"},

            {Shiro::AssetType::sfx, "ready"},
            {Shiro::AssetType::sfx, "go"},
            {Shiro::AssetType::sfx, "prerotate"},
            {Shiro::AssetType::sfx, "land"},
            {Shiro::AssetType::sfx, "lock"},
            {Shiro::AssetType::sfx, "lineclear"},
            {Shiro::AssetType::sfx, "dropfield"},
            {Shiro::AssetType::sfx, "newsection"},
            {Shiro::AssetType::sfx, "medal"},
            {Shiro::AssetType::sfx, "gradeup"},

            {Shiro::AssetType::sfx, "pieces0"},
            {Shiro::AssetType::sfx, "pieces1"},
            {Shiro::AssetType::sfx, "pieces2"},
            {Shiro::AssetType::sfx, "pieces3"},
            {Shiro::AssetType::sfx, "pieces4"},
            {Shiro::AssetType::sfx, "pieces5"},
            {Shiro::AssetType::sfx, "pieces6"} //,

            //{Shiro::AssetType::sfx, "clear_single"},
            //{Shiro::AssetType::sfx, "clear_double"},
            //{Shiro::AssetType::sfx, "clear_triple"},
            //{Shiro::AssetType::sfx, "clear_tetris"}
        });

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

        load_files();

        check(Gui_Init(screen.renderer, NULL), "Gui_Init() returned failure\n");
        check(gfx_init(this) == 0, "gfx_init returned failure\n");

        bg.transition(Shiro::ImageAsset::get(assetMgr, "bg_temp"));
        // blank = Shiro::ImageAsset::get(assetMgr, "blank").getTexture();

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
    SDL_Texture *paletteTex = Shiro::ImageAsset::get(assetMgr, "tets_bright_qs").getTexture();

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
            running && (timeAccumulator >= gameFrameTime || (settings.vsyncTimestep && settings.vsync && newFrames == 0u));
            timeAccumulator -= gameFrameTime,
            newFrames++,
            frames++
            ) {
            prev_keys_raw = keys_raw;
            prev_keys = keys;

            running = process_events();

            handle_replay_input();

            update_input_repeat();
            update_pressed();

            gfx_buttons_input();

            /*
            SPMgame.input();
            SPMgame.frame();
            SPMgame.draw();
            */

            gfx.clearLayers();

            if (p1game) {
                if (!p1game->update(!button_emergency_override)) {
                    p1game->quit(p1game);
                    free(p1game);
                    p1game = NULL;

                    bg.transition(Shiro::ImageAsset::get(assetMgr, "bg_temp"));
                    break;
                }
            }
            if (menu && ((!p1game || menu_input_override) ? 1 : 0)) {
                if (!menu->update(!button_emergency_override)) {
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

            bg.update();

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
                Shiro::GfxLayer::base,
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
                Shiro::GfxLayer::buttons,
                [this] { gfx_drawbuttons(this, 0); }
            ));

            // TODO: Create entities in the code for emergency buttons, then remove this.
            gfx.push(std::make_unique<OldGfxEntity>(
                Shiro::GfxLayer::emergencyButtons,
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

        bg.draw();
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
                    viewportWidth = static_cast<GLsizei>(heightFactor * 640.0);
                    viewportHeight = static_cast<GLsizei>(heightFactor * 480.0);
                }
                else {
                    viewportWidth = static_cast<GLsizei>(widthFactor * 640.0);
                    viewportHeight = static_cast<GLsizei>(widthFactor * 480.0);
                }
                glViewport((w - viewportWidth) / 2, (h - viewportHeight) / 2, viewportWidth, viewportHeight);
                glUniform2f(glGetUniformLocation(screen.interpolate_shading_prog, "viewportSize"), static_cast<GLfloat>(viewportWidth), static_cast<GLfloat>(viewportHeight));
            }
            else {
                // TODO: Change this once a fullscreen resolution setting is supported.
                glViewport(0, 0, w, h);
                glUniform2f(glGetUniformLocation(screen.interpolate_shading_prog, "viewportSize"), static_cast<GLfloat>(w), static_cast<GLfloat>(h));
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

void CoreState::load_files() {
    // image assets
#define IMG(name) load_image(this, &assets->name, #name);
#define IMG_ARRAY(name, i) load_image(this, &assets->name[i], #name #i);
#include "images.h"
#undef IMG_ARRAY
#undef IMG

#define FONT(name, sheetName, outlineSheetName, charW, charH) \
    load_bitfont(&assets->name, &assets->sheetName, &assets->outlineSheetName, charW, charH);
#include "fonts.h"
#undef FONT
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
        cs->assets->name.outlineSheet = nullptr;
#include "fonts.h"
#undef FONT

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

// TODO: Make this only read in events and update state; move out all logic interpreting the new state into the game/menu code, such as DAS processing. And rename this to pollEvents once that's all done.
bool CoreState::process_events() {
    Shiro::KeyFlags *k = NULL;

    SDL_Keycode keyCode;
    SDL_Keymod keyMod;
    Shiro::KeyBindings& keyBindings = settings.keyBindings;

    Shiro::ControllerBindings& controllerBindings = settings.controllerBindings;

#if 0
    if(mouse_left_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
    {
        mouse_left_down = 1;
    }
    if(mouse_right_down == Shiro::Magic::BUTTON_PRESSED_THIS_FRAME)
    {
        mouse_right_down = 1;
    }

    if(select_all)
    {
        select_all = 0;
    }
    if(undo)
    {
        undo = 0;
    }
    if(redo)
    {
        redo = 0;
    }

    SDL_GetMouseState(&mouse_x, &mouse_y);

    int windowW;
    int windowH;
    SDL_GetWindowSize(screen.window, &windowW, &windowH);

    if(windowW == (windowH * 4) / 3)
    {
        float scale_ = (float)windowW / 640.0;
        mouse.logicalX = (int)((float)mouse_x / scale_);
        mouse.logicalY = (int)((float)mouse_y / scale_);
    }
    else if(windowW < (windowH * 4) / 3) // squished horizontally (results in horizontal bars on the top and bottom of window)
    {
        float scale_ = (float)windowW / 640.0;
        int yOffset = (windowH - ((windowW * 3) / 4)) / 2;
        if(mouse_y < yOffset || mouse_y >= windowH - yOffset)
        {
            mouse.logicalY = -1;
        }
        else
        {
            mouse.logicalY = (int)((float)(mouse_y - yOffset) / scale_);
        }

        mouse.logicalX = (int)((float)mouse_x / scale_);
    }
    else
    {
        float scale_ = (float)windowH / 480.0;
        int xOffset = (windowW - ((windowH * 4) / 3)) / 2;
        if(mouse_x < xOffset || mouse_x >= windowW - xOffset)
        {
            mouse.logicalX = -1;
        }
        else
        {
            mouse.logicalX = (int)((float)(mouse_x - xOffset) / scale_);
        }

        mouse.logicalY = (int)((float)mouse_y / scale_);
    }
#endif

    int windowW, windowH;
    SDL_GetWindowSize(screen.window, &windowW, &windowH);
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
                return false;

            case SDL_JOYAXISMOTION:
                k = &keys_raw;
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
                k = &keys_raw;

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
                k = &keys_raw;
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
                k = &keys_raw;
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

                k = &keys_raw;

#define CHECK_KEYDOWN(name) \
                if (keyCode == keyBindings.name) { \
                    k->name = 1; \
                }
                CHECK_BINDINGS(CHECK_KEYDOWN);
#undef CHECK_KEYDOWN

                switch (keyCode) {
                case SDLK_v:
                    if ((keyMod & KMOD_CTRL) && text_editing && text_insert) {
                        text_insert(this, SDL_GetClipboardText());
                    }
                    break;

                case SDLK_c:
                    if ((keyMod & KMOD_CTRL) && text_editing && text_insert) {
                        text_copy(this);
                    }
                    break;

                case SDLK_x:
                    if ((keyMod & KMOD_CTRL) && text_editing && text_cut) {
                        text_cut(this);
                    }
                    break;

                case SDLK_a:
                    if (keyMod & KMOD_CTRL) {
                        if (text_editing && text_select_all) {
                            text_select_all(this);
                        }
                        select_all = true;
                    }
                    break;

                case SDLK_z:
                    if (keyMod & KMOD_CTRL) {
                        undo = true;
                    }
                    break;

                case SDLK_y:
                    if (keyMod & KMOD_CTRL) {
                        redo = true;
                    }
                    break;

                case SDLK_BACKSPACE:
                    backspace_das = 1;
                    break;

                case SDLK_DELETE:
                    delete_das = 1;
                    break;

                case SDLK_HOME:
                    if (text_editing && text_seek_home) {
                        text_seek_home(this);
                    }
                    break;

                case SDLK_END:
                    if (text_editing && text_seek_end) {
                        text_seek_end(this);
                    }
                    break;

                case SDLK_RETURN:
                    if (text_toggle) {
                        text_toggle(this);
                    }
                    break;

                case SDLK_LEFT:
                    left_arrow_das = 1;
                    break;

                case SDLK_RIGHT:
                    right_arrow_das = 1;
                    break;

                case SDLK_F4:
                    if (keyMod & KMOD_ALT) {
                        return false;
                    }
                    break;

                case SDLK_F8:
                    switch (displayMode) {
                    case Shiro::DisplayMode::DEFAULT:
                        displayMode = Shiro::DisplayMode::DETAILED;
                        break;

                    case Shiro::DisplayMode::DETAILED:
                        displayMode = Shiro::DisplayMode::CENTERED;
                        break;

                    default:
                        displayMode = Shiro::DisplayMode::DEFAULT;
                        break;
                    }
                    break;

                case SDLK_F9:
                    motionBlur = !motionBlur;
                    break;

                case SDLK_F11:
                    if (settings.fullscreen) {
                        settings.fullscreen = false;
                        SDL_SetWindowFullscreen(screen.window, 0);
                        SDL_SetWindowSize(screen.window, static_cast<int>(640.0 * settings.videoScale), static_cast<int>(480.0 * settings.videoScale));
                    }
                    else {
                        settings.fullscreen = true;
                        SDL_SetWindowSize(screen.window, 640, 480);
                        SDL_WindowFlags flags = (keyMod & KMOD_SHIFT) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
                        if (SDL_SetWindowFullscreen(screen.window, flags) < 0) {
                            std::cout << "SDL_SetWindowFullscreen(): Error: " << SDL_GetError() << std::endl;
                        }
                    }
                    break;

                case SDLK_0:
                    pressedDigits[0] = true;
                    break;

                case SDLK_1:
                    if (keyMod & KMOD_ALT) {
                        settings.videoScale = 1;
                        SDL_SetWindowSize(screen.window, 640, 480);
                    }
                    pressedDigits[1] = true;
                    break;

                case SDLK_2:
                    if (keyMod & KMOD_ALT) {
                        settings.videoScale = 2;
                        SDL_SetWindowSize(screen.window, 2 * 640, 2 * 480);
                    }
                    pressedDigits[2] = true;
                    break;

                case SDLK_3:
                    if (keyMod & KMOD_ALT) {
                        settings.videoScale = 3;
                        SDL_SetWindowSize(screen.window, 3 * 640, 3 * 480);
                    }
                    pressedDigits[3] = true;
                    break;

                case SDLK_4:
                    if (keyMod & KMOD_ALT) {
                        settings.videoScale = 4;
                        SDL_SetWindowSize(screen.window, 4 * 640, 4 * 480);
                    }
                    pressedDigits[4] = true;
                    break;

                case SDLK_5:
                    if (keyMod & KMOD_ALT) {
                        settings.videoScale = 5;
                        SDL_SetWindowSize(screen.window, 5 * 640, 5 * 480);
                    }
                    pressedDigits[5] = true;
                    break;

                case SDLK_6:
                    pressedDigits[6] = true;
                    break;

                case SDLK_7:
                    pressedDigits[7] = true;
                    break;

                case SDLK_8:
                    pressedDigits[8] = true;
                    break;

                case SDLK_9:
                    pressedDigits[9] = true;
                    break;

                default:
                    break;
                }
                break;

            case SDL_KEYUP:
                keyCode = event.key.keysym.sym;

                k = &keys_raw;

#define CHECK_KEYUP(name) \
                if (keyCode == keyBindings.name) { \
                    k->name = 0; \
                }
                CHECK_BINDINGS(CHECK_KEYUP);
#undef CHECK_KEYUP

                switch (keyCode) {
                case SDLK_LEFT:
                    left_arrow_das = 0;
                    break;

                case SDLK_RIGHT:
                    right_arrow_das = 0;
                    break;

                case SDLK_BACKSPACE:
                    backspace_das = 0;
                    break;

                case SDLK_DELETE:
                    delete_das = 0;
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
                    pressedDigits[keyCode - SDLK_0] = false;
                    break;

                default:
                    break;
                }

                break;

            case SDL_TEXTINPUT:
                if (text_editing)
                {
                    if (!((event.text.text[0] == 'c' || event.text.text[0] == 'C') &&
                        (event.text.text[0] == 'v' || event.text.text[0] == 'V') &&
                        (event.text.text[0] == 'x' || event.text.text[0] == 'X') &&
                        (event.text.text[0] == 'a' || event.text.text[0] == 'A') && SDL_GetModState() & KMOD_CTRL))
                    {
                        if (text_insert)
                            text_insert(this, event.text.text);
                    }
                }

                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse.leftButton = Shiro::Mouse::Button::pressedThisFrame;
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                    mouse.rightButton = Shiro::Mouse::Button::pressedThisFrame;
                select_all = false;
                undo = false;
                redo = false;
                mouse.update(windowW, windowH);
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    mouse.leftButton = Shiro::Mouse::Button::notPressed;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    mouse.rightButton = Shiro::Mouse::Button::notPressed;
                select_all = false;
                undo = false;
                redo = false;
                mouse.update(windowW, windowH);
                break;

            case SDL_MOUSEMOTION:
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;
                select_all = false;
                undo = false;
                redo = false;
                mouse.update(windowW, windowH);
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    SDL_GetWindowSize(screen.window, &windowW, &windowH);
                    select_all = false;
                    undo = false;
                    redo = false;
                    mouse.update(windowW, windowH);
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

    if(left_arrow_das)
    {
        if(left_arrow_das == 1 || left_arrow_das == 30)
        {
            if(text_editing && text_seek_left)
                text_seek_left(this);
            if(left_arrow_das == 1)
                left_arrow_das = 2;
        }
        else
        {
            left_arrow_das++;
        }
    }

    if(right_arrow_das)
    {
        if(right_arrow_das == 1 || right_arrow_das == 30)
        {
            if(text_editing && text_seek_right)
                text_seek_right(this);
            if(right_arrow_das == 1)
                right_arrow_das = 2;
        }
        else
        {
            right_arrow_das++;
        }
    }

    if(backspace_das)
    {
        if(backspace_das == 1 || backspace_das == 30)
        {
            if(text_editing && text_backspace)
                text_backspace(this);
            if(backspace_das == 1)
                backspace_das = 2;
        }
        else
        {
            backspace_das++;
        }
    }

    if(delete_das)
    {
        if(delete_das == 1 || delete_das == 30)
        {
            if(text_editing && text_delete)
                text_delete(this);
            if(delete_das == 1)
                delete_das = 2;
        }
        else
        {
            delete_das++;
        }
    }

    /*
        SDL_Joystick *joy = joystick;
        const uint8_t *keystates = SDL_GetKeyboardState(NULL);

        if (settings.keyBindings) {
            k = &keys_raw;
            kb = settings.keyBindings;

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

            keys = keys_raw;
        }
    */

    keys = keys_raw;

    return true;
}

void CoreState::handle_replay_input() {
    game_t *g = p1game;
    if (g != NULL) {
        qrsdata *q = (qrsdata *)g->data;

        if (q == NULL) {
            return;
        }

        if (q->playback) {
            if ((unsigned int)(q->playback_index) == q->replay->len) {
                qrs_end_playback(g);
            }
            else {
                unpack_input(q->replay->pinputs[q->playback_index], &keys);

                q->playback_index++;
            }
        }
        else if(q->recording) {
            q->replay->pinputs[q->replay->len] = pack_input(&keys_raw);

            q->replay->len++;
        }
    }
}

void CoreState::update_input_repeat() {
    Shiro::KeyFlags *k = &keys;

    if(hold_dir == Shiro::DASDirection::LEFT && k->right)
    {
        hold_time = 0;
        hold_dir = Shiro::DASDirection::RIGHT;
    }
    else if(hold_dir == Shiro::DASDirection::RIGHT && k->left)
    {
        hold_time = 0;
        hold_dir = Shiro::DASDirection::LEFT;
    }
    else if(hold_dir == Shiro::DASDirection::UP && k->down)
    {
        hold_time = 0;
        hold_dir = Shiro::DASDirection::DOWN;
    }
    else if(hold_dir == Shiro::DASDirection::DOWN && k->up)
    {
        hold_time = 0;
        hold_dir = Shiro::DASDirection::UP;
    }

    if(hold_dir == Shiro::DASDirection::LEFT && k->left)
        hold_time++;
    else if(hold_dir == Shiro::DASDirection::RIGHT && k->right)
        hold_time++;
    else if(hold_dir == Shiro::DASDirection::UP && k->up)
        hold_time++;
    else if(hold_dir == Shiro::DASDirection::DOWN && k->down)
        hold_time++;
    else
    {
        if(k->left)
            hold_dir = Shiro::DASDirection::LEFT;
        else if(k->right)
            hold_dir = Shiro::DASDirection::RIGHT;
        else if(k->up)
            hold_dir = Shiro::DASDirection::UP;
        else if(k->down)
            hold_dir = Shiro::DASDirection::DOWN;
        else
            hold_dir = Shiro::DASDirection::NONE;

        hold_time = 0;
    }
}

void CoreState::update_pressed() {
    pressed.left = (keys.left == 1 && prev_keys.left == 0) ? 1 : 0;
    pressed.right = (keys.right == 1 && prev_keys.right == 0) ? 1 : 0;
    pressed.up = (keys.up == 1 && prev_keys.up == 0) ? 1 : 0;
    pressed.down = (keys.down == 1 && prev_keys.down == 0) ? 1 : 0;
    pressed.start = (keys.start == 1 && prev_keys.start == 0) ? 1 : 0;
    pressed.a = (keys.a == 1 && prev_keys.a == 0) ? 1 : 0;
    pressed.b = (keys.b == 1 && prev_keys.b == 0) ? 1 : 0;
    pressed.c = (keys.c == 1 && prev_keys.c == 0) ? 1 : 0;
    pressed.d = (keys.d == 1 && prev_keys.d == 0) ? 1 : 0;
    pressed.escape = (keys.escape == 1 && prev_keys.escape == 0) ? 1 : 0;
}

bool CoreState::button_emergency_inactive() {
    return !button_emergency_override;
}

void CoreState::gfx_buttons_input() {
    if (!gfx_buttons.size()) {
        return;
    }

    int i = 0;

    int scaled_x = 0;
    int scaled_y = 0;
    int scaled_w = 0;
    int scaled_h = 0;

    for (auto it = gfx_buttons.begin(); it != gfx_buttons.end(); it++) {
        gfx_button& b = *it;
        scaled_x = static_cast<int>(settings.videoScale * static_cast<float>(b.x));
        scaled_y = static_cast<int>(settings.videoScale * static_cast<float>(b.y));
        scaled_w = static_cast<int>(settings.videoScale * static_cast<float>(b.w));
        scaled_h = static_cast<int>(settings.videoScale * static_cast<float>(b.h));

        if(button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
        {
            b.highlighted = 0;
            if(b.delete_check)
            {
                if(b.delete_check(this))
                {
                    gfx_buttons.erase(it);
                }
            }

            continue;
        }

        if(mouse.x < scaled_x + scaled_w && mouse.x >= scaled_x && mouse.y < scaled_y + scaled_h &&
           mouse.y >= scaled_y)
            b.highlighted = 1;
        else
            b.highlighted = 0;

        if(b.highlighted && mouse.leftButton == Shiro::Mouse::Button::pressedThisFrame)
        {
            if(b.action)
            {
                b.action(this, b.data);
            }

            b.clicked = 4;
        }
        if (b.clicked) {
            b.clicked--;
        }

        if(b.delete_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check(this))
            {
                gfx_buttons.erase(it);
            }
        }
    }

    for (auto it = gfx_buttons.begin(); it != gfx_buttons.end(); it++) {
        gfx_button& b = gfx_buttons[i];

        if(button_emergency_override && !(b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check)
            {
                if(b.delete_check(this))
                {
                    gfx_buttons.erase(it);
                }
            }

            continue;
        }

        if(b.delete_check && (!b.clicked || b.flags & BUTTON_EMERGENCY))
        {
            if(b.delete_check(this))
            {
                gfx_buttons.erase(it);
            }
        }
    }
}

int CoreState::request_fps(double fps)
{
    if(fps != 60.00 && fps != Shiro::RefreshRates::g1 && fps != Shiro::RefreshRates::g2 && fps != Shiro::RefreshRates::g3)
        return 1;

    this->fps = fps;
    return 0;
}
