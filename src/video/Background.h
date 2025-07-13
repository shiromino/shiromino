#pragma once
#include <cstdint>
#include <memory>
#include "asset/Image.h"

namespace Shiro {
    struct Screen;
    
    // TODO: Consider adding functionality to query current background state; if none of the code needs such a feature, don't add it.
    class Background {
    public:
        Background() = delete;

        static constexpr uint8_t defaultFadeRate = 25u;
        Background(const Screen& screen, const uint8_t shadeV = defaultFadeRate);

        /**
         * Transition to a new image.
         */
        void transition(const ImageAsset& nextImage);

        /**
         * Darken the current image to black, then remain black. If the
         * background is currently in the dark state, calling this has no
         * effect. Even if a transition to a new image is in progress, this will
         * still force the background to transition to displaying black.
         */
        void transition();

        /**
         * Update the state of the background. Call once per logical frame.
         */
        void update();

        /**
         * Draw the background. Call once per render frame.
         */
        void draw() const;

    private:
        struct Impl;
        std::shared_ptr<Impl> implPtr;
    };
}