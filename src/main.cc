/*
   main.cpp - handle command line arguments, load game settings, manage main
   data structures
*/
#include "CoreState.h"
#include "Debug.h"
#include "random.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/stat.h>
#ifdef VCPKG_TOOLCHAIN
#include <vorbis/vorbisfile.h>
#endif
// When building with GCC on Windows, this fixes the error where WinMain is
// undefined.
#ifdef main
#undef main
#endif
int main(int argc, const char* argv[]) {
    bool luaModes = false;
    bool running = true;

#ifdef VCPKG_TOOLCHAIN
    {
        // Hack to force vcpkg to copy over the OGG/Vorbis libraries. Pretty much a
        // no-op, so it has no performance penalty.
        OggVorbis_File vf;
        vf.seekable = 0;
        ov_info(&vf, 0);
    }
#endif

    while (running) {
        if (luaModes) {
            running = false;
        }
        else {
            CoreState cs;
            CoreState_initialize(&cs);
            auto executablePath = std::filesystem::path(argv[0]);
            const auto basePath = std::filesystem::canonical(executablePath.remove_filename());
            auto settings = Shiro::Settings(basePath);
            const char path[] = ".";
            const auto configurationPath = basePath / "game.ini";

            g123_seeds_init();
            srand((unsigned int)time(0));

            // TODO: Use an argument handler library here, rather than hard-coded
            // logic.
            if (argc == 1) {
                if (!std::filesystem::exists(configurationPath)) {
                    std::cerr << "Couldn't find configuration file, aborting" << std::endl;
                    goto error;
                }

                if (settings.read(configurationPath.string())) {
                    std::cerr << "Using one or more default settings" << std::endl;
                }

                cs.configurationPath = configurationPath;
            }
            else if (argc >= 2) {
                if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
                    std::cerr << "Usage: " << argv[0] << " [path to *.ini configuration file]" << std::endl;
                    CoreState_destroy(&cs);
                    return 0;
                }
                else if (strlen(argv[1]) >= 4 && strcmp(&argv[1][strlen(argv[1]) - 4], ".ini") != 0) {
                    std::cerr << "Usage: " << argv[0] << " [path to *.ini configuration file]" << std::endl;
                    goto error;
                }

                if (settings.read(argv[1])) {
                    std::cerr << "Using one or more default settings" << std::endl;
                }

                cs.configurationPath = configurationPath;
            }

            std::cerr << "Finished reading configuration file: " << configurationPath << std::endl;

            if (init(&cs, &settings, argv[0])) {
                std::cerr << "Initialization failed, aborting." << std::endl;
                quit(&cs);
                CoreState_destroy(&cs);
                return 1;
            }

            {
                int status = run(&cs);

                quit(&cs);
                CoreState_destroy(&cs);

                if (status == 2) {
                    std::cerr << "Opening Lua modes menu." << std::endl;
                    luaModes = true;
                    continue;
                }
                else {
                    break;
                }
            }

        error:
            CoreState_destroy(&cs);
            return 1;
        }
    }
    return 0;
}