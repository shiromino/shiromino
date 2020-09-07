/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Gfx/MessageEntity.h"
#include "Gfx/TextGraphic.h"

using namespace Shiro;
using namespace std;

namespace Shiro {
    struct MessageEntity::Impl {
        Impl(
            const size_t layerNum,
            const size_t numFrames,
            const function<bool()> deleteCheck,
            const shared_ptr<TextGraphic> graphic
        );

        const size_t layerNum;
        size_t counter;
        const function<bool()> deleteCheck;
        const shared_ptr<TextGraphic> graphic;
    };
}

MessageEntity::Impl::Impl(
    const size_t layerNum,
    const size_t numFrames,
    const function<bool()> deleteCheck,
    const shared_ptr<TextGraphic> graphic
) :
    layerNum(layerNum),
    counter(numFrames),
    deleteCheck(deleteCheck),
    graphic(graphic) {}

MessageEntity::MessageEntity(
    SDL_Renderer *const renderer,
    const std::string text,
    const size_t layerNum,
    const int x,
    const int y,
    const png_monofont& font,
    const text_formatting& fmt,
    const size_t numFrames,
    const function<bool()> deleteCheck
) :
    implPtr(make_shared<Impl>(
        layerNum,
        numFrames == SIZE_MAX ? SIZE_MAX : numFrames + 1,
        deleteCheck,
        make_shared<TextGraphic>(
            renderer,
            text,
            x,
            y,
            font,
            fmt
        )
    )) {}

bool MessageEntity::update(Shiro::Layers& layers) {
    layers.push(implPtr->layerNum, implPtr->graphic);
    return (implPtr->counter == SIZE_MAX || --implPtr->counter != 0u) && !implPtr->deleteCheck();
}