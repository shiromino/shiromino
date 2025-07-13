#pragma once
#include "SDL_render.h"
#include <cstddef>
#include <filesystem>
#include <memory>
#include "types.h"
#include "video/Gfx.h"

namespace Shiro {
    class AssetManager;
    struct Screen;
    
    struct AnimationGraphic : public Graphic {
        AnimationGraphic() = delete;

        AnimationGraphic(
            const Screen& screen,
            SDL_Texture* const frame,
            const int x,
            const int y,
            const u32 rgbaMod
        );

        void draw() const;

        const Screen& screen;
        SDL_Texture* frame;
        const int x;
        const int y;
        const u32 rgbaMod;
    };

    class AnimationEntity : public Entity {
    public:
        DEFINE_ENTITY_PUSH(AnimationEntity)

        AnimationEntity() = delete;

        AnimationEntity(
            const Screen& screen,
            AssetManager& assetMgr,
            const std::filesystem::path& frames,
            const std::size_t layerNum,
            const int x,
            const int y,
            const std::size_t numFrames,
            const std::size_t frameMultiplier,
            const u32 rgbaMod
        );

        bool update(Layers& layers);

    private:
        AssetManager& assetMgr;
        const std::filesystem::path frames;
        const std::size_t layerNum;
        std::size_t counter;
        const std::size_t numFrames;
        const std::size_t frameMultiplier;
        const std::shared_ptr<AnimationGraphic> graphic;
    };
}
