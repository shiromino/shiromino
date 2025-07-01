#include "video/MessageEntity.h"
#include "video/TextGraphic.h"

namespace Shiro {
	struct Screen;
	
	MessageEntity::MessageEntity(
		const Screen& screen,
		const FontAsset& font,
		const std::string& text,
		const int x,
		const int y,
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
		graphic(std::make_shared<TextGraphic>(screen, font, text, x, y, scale, color)) {}

	bool MessageEntity::update(Layers& layers) {
		layers.push(layerNum, graphic);
		return (counter != SIZE_MAX && --counter == 0u) || deleteCheck();
	}
}
