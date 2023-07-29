#pragma once
#include "SPM_Structures.h"
#include "SPM_Graphics.h"
#include <cstdint>

namespace G2 {
    enum class FieldEncoding : int32_t {
        mono = -1,
        bone = -2,
        clearedCell = -3,
        fieldLimiter = -100,

        item_deathBlock = -201,
        item_negaField = -202,
        item_180Field = -203,
        item_shotgun = -204,
        item_hardBlock = -205,
        item_laserBlock = -206,
        item_rollroll = -207,
        item_transform = -208,
        item_xray = -209,
        item_colorBlock = -210,
        item_darkBlock = -211,
        item_mirrorBlock = -212,

        item_delFieldUpper = -213,
        item_delFieldLower = -214,
        item_movFieldRight = -215,
        item_movFieldLeft = -216,
        item_delEven = -217,

        item_freeFall = -218,
        item_exchangeField = -219
    };
}