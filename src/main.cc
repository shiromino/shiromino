/*
   main.cpp - handle command line arguments, load game settings, manage main
   data structures
*/
#include "CoreState.h"
#include "Debug.h"
#include "random.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/stat.h>
#ifdef VCPKG_TOOLCHAIN
#include <vorbis/vorbisfile.h>
#endif
// When building with GCC on Windows, this fixes the error where WinMain is
// undefined.
#ifdef main
#undef main
#endif
void printHelp(const char* argv[]) {
    std::cerr << "Usage: " << argv[0] << " --configuration-file <configuration file>" << std::endl;
}
void handleCommandLineArguments(int argc, const char* argv[], CoreState& coreState, Shiro::Settings& settings) {
    /* TODO: Use an argument handler library here */
    if (argc == 1) {
        const auto configurationPath = settings.basePath / "game.ini";
        if (!std::filesystem::exists(configurationPath)) {
            std::cerr << "Couldn't find configuration file, aborting" << std::endl;
            CoreState_destroy(&coreState);
            std::exit(EXIT_FAILURE);
        }
        if (settings.read(configurationPath.string())) {
            std::cerr << "Using one or more default settings" << std::endl;
        }
        coreState.configurationPath = configurationPath;
    }
    else if (argc == 3) {
        const auto firstArgument = std::string(argv[1]);
        const auto secondArgument = std::string(argv[2]);
        if (firstArgument == "--configuration-file" || firstArgument == "-c") {
            const auto configurationPath = std::filesystem::path(secondArgument);
            coreState.configurationPath = configurationPath;
            auto basePath = std::filesystem::canonical(configurationPath);
            basePath.remove_filename();
            settings.basePath = basePath;
            if (settings.read(configurationPath.string())) {
                std::cerr << "Using one or more default settings" << std::endl;
            }
        }
        else {
            printHelp(argv);
            CoreState_destroy(&coreState);
            std::exit(EXIT_SUCCESS);
        }
    }
    else {
        printHelp(argv);
        CoreState_destroy(&coreState);
        std::exit(EXIT_FAILURE);
    }
    std::cerr << "Configuration file: " << coreState.configurationPath << std::endl;
    std::cerr << "Base path: " << settings.basePath << std::endl;
}
int main(int argc, const char* argv[]) {
#ifdef VCPKG_TOOLCHAIN
    {
        // Hack to force vcpkg to copy over the OGG/Vorbis libraries. Pretty much a
        // no-op, so it has no performance penalty.
        OggVorbis_File vf;
        vf.seekable = 0;
        ov_info(&vf, 0);
    }
#endif

    CoreState cs;
    CoreState_initialize(&cs);
    auto executablePath = std::filesystem::path(argv[0]);
    const auto basePath = std::filesystem::canonical(executablePath.remove_filename());
    auto settings = Shiro::Settings(basePath);
    g123_seeds_init();
    srand((unsigned int)time(0));
    handleCommandLineArguments(argc, argv, cs, settings);
    if (init(&cs, &settings, argv[0])) {
        std::cerr << "Initialization failed, aborting." << std::endl;
        quit(&cs);
        CoreState_destroy(&cs);
        return EXIT_FAILURE;
    }
    int status = run(&cs);
    quit(&cs);
    CoreState_destroy(&cs);
}