#include "video/Background.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <queue>
#include "video/Render.h"
#include "video/Screen.h"

using namespace Shiro;
using namespace std;

namespace Shiro {
    enum class BackgroundState {
        /**
         * Current background darkening to black; if there's no image to transition
         * to, the next state will be dark. If there is a new image to transition
         * to, the image at the front of the queue is popped and the next state will
         * be brighten.
         */
        darken,

        /**
         * No background image to draw; draw black to the whole screen until a
         * transition to a new image is requested, at which point the next state
         * would be brighten. A newly constructed background will start in the dark
         * state until a transition to a new image is requested.
         */
        dark,

        /**
         * Current background brightening to normal brightness; once fully
         * brightened, the next state will be bright.
         */
        brighten,

        /**
         * Current background will remain displayed at full brightness, until a
         * transition is requested, at which point the next state would be darken.
         */
        bright
    };

    struct Background::Impl {
        Impl() = delete;

        Impl(
            const Screen& screen,
            const uint8_t shadeV = defaultFadeRate
        ) :
             screen(screen),
             state(BackgroundState::dark),
             drawImage(false),
             shade(0u),
             shadeV(shadeV) {}

        const Screen& screen;
        BackgroundState state;
        bool drawImage;
        queue<const ImageAsset*> images;
        uint8_t shade;
        const uint8_t shadeV;
    };
}

Background::Background(const Screen& screen, const uint8_t shadeV) :
    implPtr(make_shared<Impl>(screen, shadeV)) {}

void Background::transition(const ImageAsset& nextImage) {
    implPtr->drawImage = true;
    implPtr->images.push(&nextImage);

    switch (implPtr->state) {
    // If the background is brightening, this will force immediate transition to
    // the next image, darkening from the current intermediate brightness.
    case BackgroundState::brighten:
    case BackgroundState::bright:
        implPtr->state = BackgroundState::darken;
        break;

    case BackgroundState::dark:
        implPtr->state = BackgroundState::brighten;
        break;

    // Make no change to state if the state is currently darken; the
    // transition to the next image will be handled by the update function.
    case BackgroundState::darken:
        break;
    }
}

void Background::transition() {
    implPtr->drawImage = false;

    switch (implPtr->state) {
    case BackgroundState::brighten:
    case BackgroundState::bright:
        implPtr->state = BackgroundState::darken;
        break;

    default:
        break;
    }
}

void Background::update() {
    switch (implPtr->state) {
    case BackgroundState::darken:
        if (implPtr->shade <= implPtr->shadeV) {
            if (implPtr->drawImage) {
                implPtr->shade = implPtr->shadeV - implPtr->shade;
                while (implPtr->images.size() > 1u) {
                    implPtr->images.pop();
                }
                implPtr->state = BackgroundState::brighten;
            }
            else {
                implPtr->shade = 0u;
                while (implPtr->images.size() != 0u) {
                    implPtr->images.pop();
                }
                implPtr->state = BackgroundState::dark;
            }
        }
        else {
            implPtr->shade -= implPtr->shadeV;
        }
        break;

    case BackgroundState::dark:
        break;

    case BackgroundState::brighten:
        if (implPtr->shade >= 255u - implPtr->shadeV) {
            implPtr->shade = 255u;
            implPtr->state = BackgroundState::bright;
        }
        else {
            implPtr->shade += implPtr->shadeV;
        }
        break;

    case BackgroundState::bright:
        break;
    }
}

void Background::draw() const {
    switch (implPtr->state) {
    case BackgroundState::dark: {
        Uint8 oldDrawColor[4];
        SDL_GetRenderDrawColor(implPtr->screen.renderer, &oldDrawColor[0], &oldDrawColor[1], &oldDrawColor[2], &oldDrawColor[3]);
        SDL_SetRenderDrawColor(implPtr->screen.renderer, 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE);
        Shiro::RenderFillRect(implPtr->screen, nullptr);
        SDL_SetRenderDrawColor(implPtr->screen.renderer, oldDrawColor[0], oldDrawColor[1], oldDrawColor[2], oldDrawColor[3]);
        break;
    }

    default: {
        Uint8 oldColorMod[3];
        SDL_GetTextureColorMod(implPtr->images.front()->getTexture(), &oldColorMod[0], &oldColorMod[1], &oldColorMod[2]);
        SDL_SetTextureColorMod(implPtr->images.front()->getTexture(), implPtr->shade, implPtr->shade, implPtr->shade);

        SDL_Texture *theRenderTarget = SDL_GetRenderTarget(implPtr->screen.renderer);
        SDL_SetRenderTarget(implPtr->screen.renderer, NULL);
        Shiro::RenderCopy(implPtr->screen, implPtr->images.front()->getTexture(), nullptr, nullptr);
        SDL_SetRenderTarget(implPtr->screen.renderer, theRenderTarget);

        SDL_SetTextureColorMod(implPtr->images.front()->getTexture(), oldColorMod[0], oldColorMod[1], oldColorMod[2]);
        break;
    }
    }
}
