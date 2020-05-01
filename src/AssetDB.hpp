#pragma once

#include <string>
#include <unordered_map>
#include <initializer_list>

#include "SDL.h"
#include "gfx_structures.h"

// Call the preload functions so assets will already be loaded and
// ready to use when the get function is called for them. If an asset
// wasn't preloaded when a get function is called, it's loaded
// on-demand on the first call of the get function, and ready to use
// for subsequent get function calls.

// TODO: Remaining asset types.
// TODO: Change to using AngelCode BMFont format for fonts, so preloading is
// possible.
namespace Shiro {
    class AssetDB {
    public:
        AssetDB(const std::string themePath, SDL_Renderer* renderer);
        ~AssetDB();

        bool preloadImage(const std::string name);
        bool preloadImages(const std::initializer_list<std::string> names);
        gfx_image& getImage(const std::string name);
        bool unloadImage(const std::string name);
        bool unloadImages(const std::initializer_list<std::string> names);
        void unloadAllImages();

        AssetDB() = delete;
        AssetDB(const AssetDB&) = delete;
        AssetDB& operator=(const AssetDB&) = delete;

    private:
        const std::string themePath;
        SDL_Renderer* renderer;
        std::unordered_map<std::string, gfx_image> images;
    };
}
