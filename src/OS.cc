#include "OS.h"
#include "SDL.h"
#include "definitions.h"
#include <filesystem>

namespace fs = std::filesystem;

const fs::path& Shiro::OS::getBasePath() {
    static fs::path basePath;

    if (basePath.empty()) {
#ifdef APPIMAGE_BASE_PATH
		char* const appImageFilename = getenv("APPIMAGE");
		if (appImageFilename == NULL) {
			throw std::logic_error("Failed to get AppImage base path.");
		}
		else {
			basePath = fs::path(appImageFilename).remove_filename();
		}
#else
		char* basePathCStr = NULL;
		if (!(basePathCStr = SDL_GetBasePath())) {
            throw std::logic_error("Failed to get SDL base path.");
        }
        else {
            basePath = fs::path(basePathCStr);
            SDL_free(basePathCStr);
            basePathCStr = NULL;
        }
#endif
    }

    return basePath;
}
