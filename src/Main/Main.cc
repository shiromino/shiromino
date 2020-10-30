/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "CoreState.h"
#include "Main/Startup.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int returnCode;

    Shiro::Settings settings;
    if (settings.init(argc, argv)) {
        std::cerr << "Configuration file: " << settings.configurationPath.string() << std::endl;
        std::cerr << "Base path: " << settings.basePath.string() << std::endl;

        Shiro::Startup(settings);
        CoreState cs(settings);
        returnCode = cs.init() ? EXIT_SUCCESS : EXIT_FAILURE;
        if (returnCode == EXIT_SUCCESS) {
            cs.run();
        }
    }
    else {
        returnCode = EXIT_FAILURE;
    }

    return returnCode;
}