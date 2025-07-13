// TODO: Refactor the asset system to be more portable and implement Gfx code to have the ability to change the back end of graphic subclasses.
#include "Animation.h"
#include "SDL_rect.h"
#include "asset/Image.h"
#include "types.h"
#include "video/Render.h"

namespace Shiro {
	class AssetManager;
	struct Screen;

	AnimationGraphic::AnimationGraphic(
		const Screen& screen,
		SDL_Texture* const frame,
		const int x,
		const int y,
		const u32 rgbaMod
	) :
		screen(screen),
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
		RenderCopy(screen, frame, NULL, &dest);
		SDL_SetTextureAlphaMod(frame, 255);
		SDL_SetTextureColorMod(frame, 255, 255, 255);
	}

	AnimationEntity::AnimationEntity(
		const Screen& screen,
		AssetManager& assetMgr,
		const std::filesystem::path& frames,
		const size_t layerNum,
		const int x,
		const int y,
		const std::size_t numFrames,
		const std::size_t frameMultiplier,
		const u32 rgbaMod
	) :
		assetMgr(assetMgr),
		frames(frames),
		layerNum(layerNum),
        counter(0u),
		numFrames(numFrames),
		frameMultiplier(frameMultiplier),
		graphic(std::make_shared<AnimationGraphic>(
            screen,
			nullptr,
			x,
			y,
			rgbaMod
        )) {}

	bool AnimationEntity::update(Layers& layers) {
		graphic->frame = ImageAsset::get(assetMgr, frames, counter / frameMultiplier).getTexture();
		layers.push(layerNum, graphic);
        return ++counter >= frameMultiplier * numFrames;
	}
}