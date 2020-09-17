#include "Main/Global.h"
#include "Version.h"
#include "random.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef VCPKG_TOOLCHAIN
#include <vorbis/vorbisfile.h>
#endif
void Shiro::GlobalInit(const Shiro::Settings& settings) {
#ifdef VCPKG_TOOLCHAIN
    {
        // Hack to force vcpkg to copy over the OGG/Vorbis libraries. Pretty much a
        // no-op, so it has no performance penalty.
        OggVorbis_File vf;
        vf.seekable = 0;
        ov_info(&vf, 0);
    }
#endif
#ifdef ENABLE_OPENGL_INTERPOLATION
    if (settings.interpolate) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    }
#endif
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init: Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_InitSubSystem: Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) != 0) {
        std::cerr << "Failed to set high thread priority; continuing without changing thread priority" << std::endl;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "IMG_Init: Failed to initialize PNG support: " << IMG_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        std::cerr << "Mix_Init: Failed to initialize OGG support: " <<  Mix_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        std::cerr <<  "Mix_OpenAudio: Error: " << Mix_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Mix_AllocateChannels(32);

    g123_seeds_init();
    srand((unsigned int)time(0));
}

void Shiro::GlobalQuit() {
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}