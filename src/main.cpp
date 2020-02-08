/*
   main.cpp - handle command line arguments, load game settings, manage main
   data structures
*/

#include "core.h"
#include "file_io.h"
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
// #include "tests.h"

using namespace std;

bool file_exists(const char *filename)
{
    struct stat buffer = {0};
    return stat(filename, &buffer) == 0;
}

// When building with GCC on Windows, this fixes the error where WinMain is
// undefined.
#ifdef main
#undef main
#endif
int main(int argc, char** argv) {
#ifdef VCPKG_TOOLCHAIN
    {
        // Hack to force vcpkg to copy over the OGG/Vorbis libraries. Pretty much a
        // no-op, so it has no performance penalty.
        OggVorbis_File vf;
        vf.seekable = 0;
        ov_info(&vf, 0);
    }
#endif
    coreState cs;
    coreState_initialize(&cs);
    Settings* settings = new Settings();
    const char path[] = ".";

    string callingPath {path};
    string iniFilename = "game.ini";
    string slash = "/";
    string cfg_filename;

    game_t *distr_test = NULL;

    cs.calling_path = (char *)malloc(strlen(path) + 1);
    strcpy(cs.calling_path, path);

    g123_seeds_init();
    srand(time(0));
    #if 0
    g2_output_seed_syncs();
    goto error;

    g2_output_sync_histogram();
    goto error;
    #endif

    // TODO: Use an argument handler library here, rather than hard-coded
    // logic.
    if (argc == 1) {
        if (!file_exists(iniFilename.c_str())) {
            log_err("Couldn't find configuration file, aborting\n");
            goto error;
        }

        if (settings->read(iniFilename)) {
            log_info("Using one or more default settings\n");
        }

        iniFilename = callingPath + slash + iniFilename;
        cs.iniFilename = (char*)malloc(iniFilename.length() + 1);
        strcpy(cs.iniFilename, iniFilename.c_str());
    }
    else if (argc >= 2) {
        if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [path to *.ini configuration file]\n", argv[0]);
            coreState_destroy(&cs);
            return 0;
        }
        else if (strlen(argv[1]) >= 4 && strcmp(&argv[1][strlen(argv[1]) - 4], ".ini") != 0) {
            printf("Usage: %s [path to *.ini configuration file]\n", argv[0]);
            goto error;
        }

        /*
        if (strcmp(argv[1], "--pento-distr-test") == 0) {
           //random_distr_test(cs, 0, 100000);
           goto error;
        } else if (strcmp(argv[1], "--list-tgm-seeds") == 0) {
           get_tgm_seed_count(0);
           goto error;
        } else if (strcmp(argv[1], "--output") == 0) {
           verify_tgm_rand_periodicity(0);
           goto error;
        } else if (strcmp(argv[1], "--seed-avg-sync") == 0) {
           seed_avg_sync(0x20);
           goto error;
        }*/

        // check(access(argv[1], F_OK) == 0, "File does not exist");

        if (settings->read(argv[1])) {
            log_info("Using one or more default settings\n");
        }

        cs.iniFilename = (char*)malloc(strlen(argv[1]) + 1);
        strcpy(cs.iniFilename, argv[1]);
    }

    printf("Finished reading configuration file: %s\n", cs.iniFilename);

    if (init(&cs, settings)) {
        printf("Initialization failed, aborting.\n");
        quit(&cs);
        coreState_destroy(&cs);
        return 1;
    }

    run(&cs);

    quit(&cs);
    coreState_destroy(&cs);

    return 0;

error:
    coreState_destroy(&cs);
    return 1;
}
