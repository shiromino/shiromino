#pragma once

#include <array>

//extern int qrspent_yx_rotation_tables[18][4][5][5];
//extern int qrstet_yx_rotation_tables[7][4][4][4];

extern const std::array<std::array<std::array<std::array<bool, 5>, 5>, 4>, 18> qrspent_yx_rotation_tables;
extern const std::array<std::array<std::array<std::array<bool, 4>, 4>, 4>, 7> qrstet_yx_rotation_tables;
