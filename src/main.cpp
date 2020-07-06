/*
   main.cpp - handle command line arguments, load game settings, manage main
   data structures
*/

#include "CoreState.h"
#include "random.h"

#include "Debug.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#ifdef VCPKG_TOOLCHAIN
#include <vorbis/vorbisfile.h>
#endif

using namespace Shiro;
using namespace std;

bool file_exists(const char *filename)
{
    struct stat buffer = {};
    return stat(filename, &buffer) == 0;
}

// When building with GCC on Windows, this fixes the error where WinMain is
// undefined.
#ifdef main
#undef main
#endif
int main(int argc, char** argv) {
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
            Settings* settings = new Settings();
            const char path[] = ".";

            string callingPath{ path };
            string iniFilename = "game.ini";
            string slash = "/";
            string cfg_filename;

            cs.calling_path = (char*)malloc(strlen(path) + 1);
            strcpy(cs.calling_path, path);

            g123_seeds_init();
            srand((unsigned int)time(0));

            // TODO: Use an argument handler library here, rather than hard-coded
            // logic.
            if (argc == 1) {
                if (!file_exists(iniFilename.c_str())) {
                    log_err("Couldn't find configuration file, aborting\n");
                    goto error;
                }

                if (settings->read(iniFilename)) {
                    printf("Using one or more default settings\n");
                }

                iniFilename = callingPath + slash + iniFilename;
                cs.iniFilename = (char*)malloc(iniFilename.size() + 1);
                strcpy(cs.iniFilename, iniFilename.c_str());
            }
            else if (argc >= 2) {
                if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
                    printf("Usage: %s [path to *.ini configuration file]\n", argv[0]);
                    CoreState_destroy(&cs);
                    return 0;
                }
                else if (strlen(argv[1]) >= 4 && strcmp(&argv[1][strlen(argv[1]) - 4], ".ini") != 0) {
                    printf("Usage: %s [path to *.ini configuration file]\n", argv[0]);
                    goto error;
                }

                if (settings->read(argv[1])) {
                    printf("Using one or more default settings\n");
                }

                cs.iniFilename = (char*)malloc(strlen(argv[1]) + 1);
                strcpy(cs.iniFilename, argv[1]);
            }

            printf("Finished reading configuration file: %s\n", cs.iniFilename);

            if (init(&cs, settings)) {
                printf("Initialization failed, aborting.\n");
                quit(&cs);
                CoreState_destroy(&cs);
                return 1;
            }

            {
                int status = run(&cs);

                quit(&cs);
                CoreState_destroy(&cs);

                if (status == 2) {
                    printf("Opening Lua modes menu.\n");
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
