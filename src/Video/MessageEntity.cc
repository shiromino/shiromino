/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Video/MessageEntity.h"
#include "Video/TextGraphic.h"

using namespace Shiro;
using namespace std;

namespace Shiro {
    struct MessageEntity::Impl {
        Impl(
            const string text,
            const size_t layerNum,
            const shared_ptr<pair<int, int>> pos,
            const int offsetX,
            const int offsetY,
            const png_monofont& font,
            const text_formatting& fmt,
            const size_t numFrames,
            const function<bool()> deleteCheck
        ) :
            counter(numFrames),
            text(text),
            layerNum(layerNum),
            pos(pos),
            offsetX(offsetX),
            offsetY(offsetY),
            font(font),
            fmt(fmt),
            deleteCheck(deleteCheck) {}

        size_t counter;
        const string text;
        const size_t layerNum;
        const shared_ptr<pair<int, int>> pos;
        const int offsetX;
        const int offsetY;
        const png_monofont& font;
        const text_formatting fmt;
        const function<bool()> deleteCheck;
    };
}

MessageEntity::MessageEntity(
    const string text,
    const size_t layerNum,
    const shared_ptr<pair<int, int>> pos,
    const int offsetX,
    const int offsetY,
    const png_monofont& font,
    const text_formatting& fmt,
    const size_t numFrames,
    const function<bool()> deleteCheck
) :
    implPtr(make_shared<Impl>(
        text,
        layerNum,
        pos,
        offsetX,
        offsetY,
        font,
        fmt,
        numFrames == SIZE_MAX ? SIZE_MAX : numFrames + 1,
        deleteCheck
    )) {}

bool MessageEntity::update(Shiro::Layers& layers) {
    layers.push(implPtr->layerNum,
        make_shared<TextGraphic>(
            implPtr->text,
            implPtr->pos->first + implPtr->offsetX,
            implPtr->pos->second + implPtr->offsetY,
            implPtr->font,
            implPtr->fmt
        )
   );
    return (implPtr->counter == SIZE_MAX || --implPtr->counter != 0u) && !implPtr->deleteCheck();
}