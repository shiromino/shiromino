/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Video/MessageEntity.h"

namespace Shiro {
	MessageEntity::MessageEntity(
		const Screen& screen,
		const FontAsset& font,
		const std::string& text,
		const int& x,
		const int& y,
		const int offsetX,
		const int offsetY,
		const float scale,
		const std::uint32_t color,
		const std::size_t numFrames,
		const std::size_t layerNum,
		const std::function<bool()> deleteCheck
	) :
		x(x),
		y(y),
		counter(numFrames),
		layerNum(layerNum),
		deleteCheck(deleteCheck),
		graphic(std::make_shared<TextGraphic>(screen, font, text, x, y, offsetX, offsetY, scale, color)) {}

	bool MessageEntity::update(Layers& layers) {
		graphic->x = x;
		graphic->y = y;
		layers.push(layerNum, graphic);
		return (counter != SIZE_MAX && --counter == 0u) || deleteCheck();
	}
}
