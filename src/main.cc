#include "SDL_error.h"
#include "SDL_thread.h"
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>
#include "CoreState.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "Settings.h"
#include "random.h"

/*
* Frees several SDL-related resources.
*/
static void destroy() {
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}
/*
* Initializes several SDL-related resources.
*/
void initialize(const Shiro::Settings &settings) {
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
    if (Mix_OpenAudio(settings.samplingRate, MIX_DEFAULT_FORMAT, 2, settings.sampleSize) == -1) {
        std::cerr <<  "Mix_OpenAudio: Error: " << Mix_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Mix_AllocateChannels(32);
    g123_seeds_init();
    std::srand((unsigned int) std::time(0));
    std::atexit(destroy);
}
/*
* The game's entry point.
*/
int main(int argc, char *argv[]) {
    int returnCode = EXIT_FAILURE;
    Shiro::Settings settings;
    if (settings.init(argc, argv)) {
        std::cerr << "Configuration path: " << settings.configurationPath.string() << std::endl;
        std::cerr << "Cache path: " << settings.cachePath.string() << std::endl;
        std::cerr << "Share path: " << settings.sharePath.string() << std::endl;
        initialize(settings);
        CoreState cs(settings);
        if (cs.init()) {
            returnCode = EXIT_SUCCESS;
            cs.run();
        }
    }
    return returnCode;
}