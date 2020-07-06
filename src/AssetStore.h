#pragma once
#include "Audio.hpp"
#include "gfx_structures.h"
#include "SGUIL/SGUIL.hpp"
// TODO: Refactor lists of assets with a number appended into arrays.
namespace Shiro {
    struct AssetStore {
        gfx_image ASSET_IMG_NONE = { NULL };

        #define IMG(name, filename) gfx_image name
        #include "images.h"
        #undef IMG

        #define FONT(name, sheetName, outlineSheetName, charW, charH) BitFont name
        #include "fonts.h"
        #undef FONT

        #define MUSIC(name, i) Shiro::Music* name[i];
        #define DEF_ARRAY
        #include "music.h"
        #undef DEF_ARRAY
        #undef MUSIC

        #define SFX(name) Shiro::Sfx* name;
        #define DEF_ARRAY
        #define SFX_ARRAY(name, i) Shiro::Sfx* name[i];
        #include "sfx.h"
        #undef SFX
        #undef DEF_ARRAY
        #undef SFX_ARRAY
    };
}