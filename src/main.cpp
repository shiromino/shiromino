/*
   main.c - handle command line arguments, load
   game settings, manage main data structures
*/

#include "core.h"
#include "file_io.h"
#include "random.h"

#include "zed_dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
// #include "tests.h"

using namespace std;

bool file_exists(const char *filename)
{
    struct stat buffer = {0};
    return stat(filename, &buffer) == 0;
}

int main(int argc, char *argv[])
{
    coreState *cs = coreState_create();
    struct settings *s = NULL;
    const char path[] = ".";

    string calling_path {path};
    string cfg = "game.cfg";
    string slash = "/";
    string cfg_filename;

    game_t *distr_test = NULL;

    cs->calling_path = (char *)malloc(strlen(path) + 1);
    strcpy(cs->calling_path, path);

    g123_seeds_init();
    /*
       g2_output_seed_syncs();
       goto error;

       g2_output_sync_histogram();
       goto error;
    */
    switch(argc)
    {
        case 1:
            if(!file_exists(cfg.c_str()))
            {
                log_err("Couldn't find configuration file , aborting\n");
                goto error;
            }

            s = parse_cfg(cfg.c_str());
            if(!s)
            {
                log_info("Using default settings\n");
            }

            cfg_filename = calling_path;
            cfg_filename.append(slash);
            cfg_filename.append(cfg);

            cs->cfg_filename = (char *)(cfg_filename.c_str());
            break;

        case 2:
            if(strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            {
                printf("Usage: %s [path to config file]\n", argv[0]);
                coreState_destroy(cs);
                return 0;
            }

            /*
            if(strcmp(argv[1], "--pento-distr-test") == 0) {
               //random_distr_test(cs, 0, 100000);
               goto error;
            } else if(strcmp(argv[1], "--list-tgm-seeds") == 0) {
               get_tgm_seed_count(0);
               goto error;
            } else if(strcmp(argv[1], "--output") == 0) {
               verify_tgm_rand_periodicity(0);
               goto error;
            } else if(strcmp(argv[1], "--seed-avg-sync") == 0) {
               seed_avg_sync(0x20);
               goto error;
            }*/

            // check(access(argv[1], F_OK) == 0, "File does not exist");

            s = parse_cfg(argv[1]);
            check(s, "File could not be opened for reading");

            cs->cfg_filename = (char *)malloc(strlen(argv[1]) + 1);
            strcpy(cs->cfg_filename, argv[1]);
            break;

        default:
            printf("Usage: %s [path to config file]\n", argv[0]);
            goto error;
    }

    printf("Finished reading configuration file: %s\n", cs->cfg_filename);

    if(init(cs, s))
    {
        printf("Initialization failed, aborting.\n");
        quit(cs);
        coreState_destroy(cs);
        return 1;
    }

    run(cs);

    quit(cs);
    coreState_destroy(cs);

    return 0;

error:
    coreState_destroy(cs);
    return 1;
}
