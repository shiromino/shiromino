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
            const FontAsset& font,
            const string text,
            //const shared_ptr<pair<int, int>> pos,
            const int& x,
            const int& y,
            const int offsetX,
            const int offsetY,
            const float scale,
            const uint32_t color,
            const size_t numFrames,
            const size_t layerNum,
            const function<bool()> deleteCheck
        ) :
            font(font),
            text(text),
            /*
            oldPos(*pos),
            pos(pos),
            */
            x(x),
            y(y),
            offsetX(offsetX),
            offsetY(offsetY),
            scale(scale),
            color(color),
            counter(numFrames),
            layerNum(layerNum),
            deleteCheck(deleteCheck),
            textGraphic(shared_ptr<Graphic>(new TextGraphic(font, text, x, y, offsetX, offsetY, scale, color))) {}

        const FontAsset& font;
        const string text;
        //pair<int, int> oldPos;
        //const shared_ptr<pair<int, int>> pos;
        const int& x;
        const int& y;
        const int offsetX;
        const int offsetY;
        const float scale;
        const uint32_t color;
        size_t counter;
        const size_t layerNum;
        const function<bool()> deleteCheck;

        shared_ptr<Graphic> textGraphic;
    };
}

MessageEntity::MessageEntity(
    const FontAsset& font,
    const string& text,
    //const shared_ptr<pair<int, int>> pos,
    const int& x,
    const int& y,
    const int offsetX,
    const int offsetY,
    const float scale,
    const uint32_t color,
    const size_t numFrames,
    const size_t layerNum,
    const function<bool()> deleteCheck
) :
    implPtr(make_shared<Impl>(
        font,
        text,
        //pos,
        x,
        y,
        offsetX,
        offsetY,
        scale,
        color,
        numFrames == SIZE_MAX ? SIZE_MAX : numFrames + 1,
        layerNum,
        deleteCheck
    )) {}

bool MessageEntity::update(Shiro::Layers& layers) {
    auto& graphic = static_cast<TextGraphic&>(*implPtr->textGraphic);
    graphic.x = implPtr->x;
    graphic.y = implPtr->y;
    layers.push(implPtr->layerNum, implPtr->textGraphic);
    return (implPtr->counter == SIZE_MAX || --implPtr->counter != 0u) && !implPtr->deleteCheck();
}