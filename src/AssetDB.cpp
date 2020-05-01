#include "AssetDB.hpp"
#include "Path.hpp"
#include "Debug.hpp"
#include "SDL_image.h"

using namespace Shiro;
using namespace std;

AssetDB::AssetDB(const string themePath, SDL_Renderer* renderer) :
    themePath(themePath),
    renderer(renderer) {}

AssetDB::~AssetDB() {
    unloadAllImages();
}

bool AssetDB::preloadImage(const string name) {
    if (!images.count(name)) {
        Path path(themePath);
        path << "gfx" << name;
        gfx_image&& image = gfx_image{ NULL };
        SDL_Surface* surface = NULL;

        string filename = string(path) + ".png";

        surface = IMG_Load(filename.c_str());
        if (!surface) {
            filename = string(path) + ".jpg";

            surface = IMG_Load(filename.c_str());
        }

        if (surface) {
            image.tex = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }

        if (image.tex != NULL) {
            images[name] = image;
            return true;
        }
        else {
            log_warn("Failed to load image '%s'", filename.c_str());
            return false;
        }
    }
    else {
        return true;
    }
}

bool AssetDB::preloadImages(const initializer_list<string> names) {
    bool preloadSuccess = true;
    for (auto& name : names) {
        if (!preloadImage(name)) {
            preloadSuccess = false;
        }
    }
    return preloadSuccess;
}

gfx_image& AssetDB::getImage(const string name) {
    if (images.count(name) || preloadImage(name)) {
        return images[name];
    }
    else {
        return gfx_image{ NULL };
    }
}

bool AssetDB::unloadImage(const string name) {
    if (images.count(name)) {
        SDL_DestroyTexture(images[name].tex);
        images.erase(name);
        return true;
    }
    else {
        return false;
    }
}

bool AssetDB::unloadImages(const initializer_list<string> names) {
    bool unloadSuccess = true;
    for (auto& name : names) {
        if (!unloadImage(name)) {
            unloadSuccess = false;
        }
    }
    return unloadSuccess;
}

void AssetDB::unloadAllImages() {
    for (auto& image : images) {
        SDL_DestroyTexture(image.second.tex);
    }
    images.clear();
}
