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
int main(int argc, char *argv[]) {
    int returnCode = EXIT_FAILURE;
    Shiro::Settings settings;
    if (settings.init(argc, argv)) {
        std::cerr << "Configuration path: " << settings.configurationPath.string() << std::endl;
        std::cerr << "Cache path: " << settings.cachePath.string() << std::endl;
        std::cerr << "Share path: " << settings.sharePath.string() << std::endl;
        Shiro::Startup(settings);
        CoreState cs(settings);
        if (cs.init()) {
            returnCode = EXIT_SUCCESS;
            cs.run();
        }
    }
    return returnCode;
}