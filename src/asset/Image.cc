#include "asset/Image.h"
#include "SDL_surface.h"
#include <cassert>
#include <iostream>
#include <string>
#include "SDL_image.h"
#include "video/Screen.h"

namespace Shiro {
    ImageAssetLoader::ImageAssetLoader(const std::filesystem::path &assetDirectory, const Screen &screen) :
        assetDirectory(assetDirectory),
        screen(screen) {}

    std::unique_ptr<Asset> ImageAssetLoader::create(const std::filesystem::path &location) const {
        return std::unique_ptr<Asset>(new ImageAsset(location));
    }

    bool ImageAssetLoader::load(Asset &asset) const {
        assert(asset.getType() == AssetType::image);
        ImageAsset &imageAsset = static_cast<ImageAsset&>(asset);
        SDL_Surface *surface;

        surface = IMG_Load((assetDirectory / imageAsset.location).concat(".png").string().c_str());
        if (surface) {
            imageAsset.texture = SDL_CreateTextureFromSurface(screen.renderer, surface);
            SDL_FreeSurface(surface);
            return imageAsset.texture != nullptr;
        }

        surface = IMG_Load((assetDirectory / imageAsset.location).concat(".jpg").string().c_str());
        if(surface) {
            imageAsset.texture = SDL_CreateTextureFromSurface(screen.renderer, surface);
            SDL_FreeSurface(surface);
            return imageAsset.texture != nullptr;
        }

        std::cerr << "Failed loading image \"" << imageAsset.location.string() << "\"" << std::endl;
        return false;
    }

    void ImageAssetLoader::unload(Asset &asset) const {
        assert(asset.getType() == AssetType::image);
        ImageAsset &imageAsset = static_cast<ImageAsset&>(asset);

        if (imageAsset.loaded()) {
            SDL_DestroyTexture(imageAsset.texture);
            imageAsset.texture = nullptr;
        }
    }

    AssetType ImageAssetLoader::getType() const {
        return AssetType::image;
    }

    SDL_Texture *ImageAsset::getTexture() const {
        return texture;
    }

    ImageAsset::ImageAsset(const std::filesystem::path &location) :
        Asset(location),
        texture(nullptr) {}

    ImageAsset::~ImageAsset() {}

    bool ImageAsset::loaded() const {
        return texture != nullptr;
    }

    AssetType ImageAsset::getType() const {
        return AssetType::image;
    }
}