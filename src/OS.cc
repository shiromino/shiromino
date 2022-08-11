#include "OS.h"
#include "SDL.h"
#include <filesystem>

namespace fs = std::filesystem;

const fs::path& Shiro::OS::getBasePath() {
    static char* basePathCStr = SDL_GetBasePath();
    static fs::path basePath;

    if (basePath.empty()) {
        if (!basePathCStr) {
            throw std::logic_error("Failed to get SDL base path.");
        }
        else {
            basePath = fs::path(basePathCStr);
            SDL_free(basePathCStr);
            basePathCStr = NULL;
        }
    }

    return basePath;
}