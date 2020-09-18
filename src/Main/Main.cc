/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "CoreState.h"
#include "Main/Global.h"

int main(int argc, char* argv[]) {
    int returnCode;

    Shiro::Settings settings;
    if (settings.init(argc, argv)) {
        Shiro::GlobalInit(settings);

        {
            CoreState cs(settings);
            returnCode = cs.init() ? EXIT_SUCCESS : EXIT_FAILURE;
            if (returnCode == EXIT_SUCCESS) {
                cs.run();
            }
        }

        Shiro::GlobalQuit();
    }
    else {
        returnCode = EXIT_FAILURE;
    }

    return returnCode;
}