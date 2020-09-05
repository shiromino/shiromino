/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
// TODO: Refactor the asset system to be more portable and implement Gfx code to have the ability to change the backend of Graphic subclasses.
#include "Gfx/Animation.h"

using namespace Shiro;
using namespace std;

namespace Shiro {
    struct AnimationGraphic : public Graphic {
        AnimationGraphic() = delete;

        AnimationGraphic(
            SDL_Renderer* const renderer,
            SDL_Texture* const frame,
            const int x,
            const int y,
            const Uint32 rgbaMod
        );

        void draw() const;

        SDL_Renderer* const renderer;
        SDL_Texture* frame;
        const int x;
        const int y;
        const Uint32 rgbaMod;
    };

    struct AnimationEntity::Impl {
        Impl() = delete;

        Impl(
            SDL_Renderer* const renderer,
            gfx_image* const frames,
            const size_t layerNum,
            const size_t numFrames,
            const size_t frameMultiplier,
            shared_ptr<AnimationGraphic> graphic
        );

        SDL_Renderer* const renderer;
        gfx_image* const frames;
        const size_t layerNum;
        size_t counter;
        const size_t numFrames;
        const size_t frameMultiplier;
        shared_ptr<AnimationGraphic> graphic;
    };
}

AnimationEntity::Impl::Impl(
    SDL_Renderer* const renderer,
    gfx_image* const frames,
    const size_t layerNum,
    const size_t numFrames,
    const size_t frameMultiplier,
    shared_ptr<AnimationGraphic> graphic
) :
    renderer(renderer),
    frames(frames),
    layerNum(layerNum),
    counter(0u),
    numFrames(numFrames),
    frameMultiplier(frameMultiplier),
    graphic(graphic) {}

AnimationGraphic::AnimationGraphic(
    SDL_Renderer* const renderer,
    SDL_Texture* const frame,
    const int x,
    const int y,
    const Uint32 rgbaMod
) :
    renderer(renderer),
    frame(frame),
    x(x),
    y(y),
    rgbaMod(rgbaMod) {}

void AnimationGraphic::draw() const {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    SDL_QueryTexture(frame, NULL, NULL, &dest.w, &dest.h);

    SDL_SetTextureColorMod(frame, R(rgbaMod), G(rgbaMod), B(rgbaMod));
    SDL_SetTextureAlphaMod(frame, A(rgbaMod));
    SDL_RenderCopy(renderer, frame, NULL, &dest);
    SDL_SetTextureAlphaMod(frame, 255);
    SDL_SetTextureColorMod(frame, 255, 255, 255);
}

AnimationEntity::AnimationEntity(
    SDL_Renderer* const renderer,
    gfx_image* const frames,
    const size_t layerNum,
    const int x,
    const int y,
    const size_t numFrames,
    const size_t frameMultiplier,
    const Uint32 rgbaMod
) :
    implPtr(
        make_shared<AnimationEntity::Impl>(
            renderer,
            frames,
            layerNum,
            numFrames,
            frameMultiplier,
            make_shared<AnimationGraphic>(renderer, nullptr, x, y, rgbaMod)
        )
    ) {}

bool AnimationEntity::update(Layers& layers) {
    implPtr->graphic->frame = implPtr->frames[implPtr->counter / implPtr->frameMultiplier].tex;
    layers.push(implPtr->layerNum, implPtr->graphic);
    if (++implPtr->counter < implPtr->frameMultiplier * implPtr->numFrames) {
        return true;
    }
    else {
        return false;
    }
}