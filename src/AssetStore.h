#pragma once
#include "Audio.h"
#include "gfx_structures.h"
#include "SGUIL/SGUIL.h"
// TODO: Refactor lists of assets with a number appended into arrays.
namespace Shiro {
    struct AssetStore {
        gfx_image ASSET_IMG_NONE = { NULL };

        #define IMG(name) gfx_image name;
        #define DEF_ARRAY
        #define IMG_ARRAY(name, i) gfx_image name[i];
        #include "images.h"
        #undef IMG_ARRAY
        #undef DEF_ARRAY
        #undef IMG

        #define FONT(name, sheetName, outlineSheetName, charW, charH) BitFont name;
        #include "fonts.h"
        #undef FONT

        #define MUSIC(name, i) Shiro::Music* name[i];
        #define DEF_ARRAY
        #include "music.h"
        #undef DEF_ARRAY
        #undef MUSIC
    };
}