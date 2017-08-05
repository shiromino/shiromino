#include <stdint.h>
#include "grid.h"
#include "rotation_tables.h"

uint64_t qrs_piecemasks[18][4] = {
    {   // I
		0x44003E0000000000,
		0x4421084200000000,
		0x44003E0000000000,
		0x4421084200000000
	},
    {   // J
		0x44003C2000000000,
		0x442108C000000000,
		0x440021E000000000,
		0x4431084000000000
	},
    {   // L
		0x44003D0000000000,
		0x44C2108000000000,
		0x440005E000000000,
		0x444210C000000000
	},
    {   // X
		0x4402388000000000,
		0x4402388000000000,
		0x4402388000000000,
		0x4402388000000000
	},
    {   // S
		0x4404384000000000,
		0x4403118000000000,
		0x4404384000000000,
		0x4403118000000000
	},
    {   // Z
		0x4401390000000000,
		0x440610C000000000,
		0x4401390000000000,
		0x440610C000000000
	},
    {   // N
		0x4400386000000000,
		0x4421188000000000,
		0x440030E000000000,
		0x4423108000000000
	},
    {   // G
		0x44001D8000000000,
		0x4443084000000000,
		0x44000DC000000000,
		0x4442184000000000
	},
    {   // U
		0x4400394000000000,
		0x4406118000000000,
		0x440029C000000000,
		0x440310C000000000
	},
    {   // T
		0x4407108000000000,
		0x4401384000000000,
		0x440211C000000000,
		0x4404390000000000
	},
    {   // Fa
		0x4404388000000000,
		0x4403308000000000,
		0x4402384000000000,
		0x4402198000000000
	},
    {   // Fb
		0x4401388000000000,
		0x440230C000000000,
		0x4402390000000000,
		0x4406188000000000
	},
    {   // P
		0x440038C000000000,
		0x440118C000000000,
		0x440031C000000000,
		0x4403188000000000
	},
    {   // Q
		0x44001CC000000000,
		0x4403184000000000,
		0x440019C000000000,
		0x440218C000000000
	},
    {   // W
		0x4406184000000000,
		0x4401198000000000,
		0x440430C000000000,
		0x4403310000000000
	},
    {   // Ya
		0x44003C8000000000,
		0x4423084000000000,
		0x440009E000000000,
		0x4442188000000000
	},
    {   // Yb
		0x44003C4000000000,
		0x4421184000000000,
		0x440011E000000000,
		0x4443108000000000
	},
    {   // V
		0x4407210000000000,
		0x4407084000000000,
		0x440109C000000000,
		0x440421C000000000
	}
};

uint64_t ars_piecemasks[7][4] = {
	{	// I
		0x330F000000000000,
		0x3322220000000000,
		0x330F000000000000,
		0x3322220000000000
	},

	{	// T
		0x330E400000000000,
		0x334C400000000000,
		0x3304E00000000000,
		0x3346400000000000
	},

	{	// J
		0x330E200000000000,
		0x3344C00000000000,
		0x3308E00000000000,
		0x3364400000000000
	},

	{	// L
		0x330E800000000000,
		0x33C4400000000000,
		0x3302E00000000000,
		0x3344600000000000
	},

	{	// O
		0x3306600000000000,
		0x3306600000000000,
		0x3306600000000000,
		0x3306600000000000,
	},

	{	// S
		0x3306C00000000000,
		0x338C400000000000,
		0x3306C00000000000,
		0x338C400000000000
	},

	{	// Z
		0x330C600000000000,
		0x3326400000000000,
		0x330C600000000000,
		0x3326400000000000
	},
};

#define O 1

int qrspent_yx_rotation_tables[18][4][5][5] =
{
#define _ 0
    { // I
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, O},
            {_, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, O},
            {_, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _}
        }
    },

    { // J
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, _},
            {_, _, _, O, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, O, O, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // L
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {O, O, _, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, O, _},
            {O, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // X
        { // FLAT
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // S
        { // FLAT
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Z
        { // FLAT
            {_, _, _, _, _},
            {_, _, O, _, _},
            {O, O, O, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, O, _, _},
            {O, O, O, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // N
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, _, _},
            {_, _, O, O, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // G
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, O, O, O, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, O, O, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // U
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, _, _},
            {O, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, _, _, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, _, O, _, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // T
        { // FLAT
            {_, _, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, _, O, _, _},
            {O, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Fa
        { // FLAT
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {O, O, _, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Fb
        { // FLAT
            {_, _, _, _, _},
            {_, _, O, _, _},
            {O, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // P
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Q
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, O, O, O, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, O, O, _, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        }
    },

    { // W
        { // FLAT
            {_, _, _, _, _},
            {O, O, _, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {_, _, O, _, _},
            {_, O, O, _, _},
            {O, O, _, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, O, _, _, _},
            {_, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {_, O, O, _, _},
            {O, O, _, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Ya
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, O, _, _},
            {O, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // Yb
        { // FLAT
            {_, _, _, _, _},
            {_, _, _, _, _},
            {O, O, O, O, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, O, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, O, _, _, _},
            {O, O, O, O, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, O, _, _, _},
            {_, O, O, _, _},
            {_, O, _, _, _},
            {_, O, _, _, _},
            {_, _, _, _, _}
        }
    },

    { // V
        { // FLAT
            {_, _, _, _, _},
            {O, O, O, _, _},
            {O, _, _, _, _},
            {O, _, _, _, _},
            {_, _, _, _, _}
        },

        { // CW
            {_, _, _, _, _},
            {O, O, O, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {_, _, _, _, _}
        },

        { // FLIP
            {_, _, _, _, _},
            {_, _, O, _, _},
            {_, _, O, _, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        },

        { // CCW
            {_, _, _, _, _},
            {O, _, _, _, _},
            {O, _, _, _, _},
            {O, O, O, _, _},
            {_, _, _, _, _}
        }
    }
};
#undef _

int qrstet_yx_rotation_tables[7][4][4][4] =
{
#define _ 0
    { // I4
        { // FLAT
            {_, _, _, _},
            {O, O, O, O},
            {_, _, _, _},
            {_, _, _, _}
        },

        { // CW
            {_, _, O, _},
            {_, _, O, _},
            {_, _, O, _},
            {_, _, O, _}
        },

        { // FLIP
            {_, _, _, _},
            {O, O, O, O},
            {_, _, _, _},
            {_, _, _, _}
        },

        { // CCW
            {_, _, O, _},
            {_, _, O, _},
            {_, _, O, _},
            {_, _, O, _}
        }
    },

    { // T4
        { // FLAT
            {_, _, _, _},
            {O, O, O, _},
            {_, O, _, _},
            {_, _, _, _}
        },

        { // CW
            {_, O, _, _},
            {O, O, _, _},
            {_, O, _, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {_, O, _, _},
            {O, O, O, _},
            {_, _, _, _}
        },

        { // CCW
            {_, O, _, _},
            {_, O, O, _},
            {_, O, _, _},
            {_, _, _, _}
        }
    },

    { // J4
        { // FLAT
            {_, _, _, _},
            {O, O, O, _},
            {_, _, O, _},
            {_, _, _, _}
        },

        { // CW
            {_, O, _, _},
            {_, O, _, _},
            {O, O, _, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {O, _, _, _},
            {O, O, O, _},
            {_, _, _, _}
        },

        { // CCW
            {_, O, O, _},
            {_, O, _, _},
            {_, O, _, _},
            {_, _, _, _}
        }
    },

    { // L4
        { // FLAT
            {_, _, _, _},
            {O, O, O, _},
            {O, _, _, _},
            {_, _, _, _}
        },

        { // CW
            {O, O, _, _},
            {_, O, _, _},
            {_, O, _, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {_, _, O, _},
            {O, O, O, _},
            {_, _, _, _}
        },

        { // CCW
            {_, O, _, _},
            {_, O, _, _},
            {_, O, O, _},
            {_, _, _, _}
        }
    },

    { // O
        { // FLAT
            {_, _, _, _},
            {_, O, O, _},
            {_, O, O, _},
            {_, _, _, _}
        },

        { // CW
            {_, _, _, _},
            {_, O, O, _},
            {_, O, O, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {_, O, O, _},
            {_, O, O, _},
            {_, _, _, _}
        },

        { // CCW
            {_, _, _, _},
            {_, O, O, _},
            {_, O, O, _},
            {_, _, _, _}
        }
    },

    { // S4
        { // FLAT
            {_, _, _, _},
            {_, O, O, _},
            {O, O, _, _},
            {_, _, _, _}
        },

        { // CW
            {O, _, _, _},
            {O, O, _, _},
            {_, O, _, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {_, O, O, _},
            {O, O, _, _},
            {_, _, _, _}
        },

        { // CCW
            {O, _, _, _},
            {O, O, _, _},
            {_, O, _, _},
            {_, _, _, _}
        }
    },

    { // Z4
        { // FLAT
            {_, _, _, _},
            {O, O, _, _},
            {_, O, O, _},
            {_, _, _, _}
        },

        { // CW
            {_, _, O, _},
            {_, O, O, _},
            {_, O, _, _},
            {_, _, _, _}
        },

        { // FLIP
            {_, _, _, _},
            {O, O, _, _},
            {_, O, O, _},
            {_, _, _, _}
        },

        { // CCW
            {_, _, O, _},
            {_, O, O, _},
            {_, O, _, _},
            {_, _, _, _}
        }
    }
};
#undef _

#undef O

// TODO? maybe move all the tables to individual variables, probably not tho

// static int i5flat[5][5]
// static int j5flat[5][5]
// static int l5flat[5][5]
// static int x5flat[5][5]
// static int s5flat[5][5]
// static int z5flat[5][5]
// static int n5flat[5][5]
// static int g5flat[5][5]
// static int u5flat[5][5]
// static int t5flat[5][5]
// static int fa5flat[5][5]
// static int fb5flat[5][5]
// static int p5flat[5][5]
// static int q5flat[5][5]
// static int w5flat[5][5]
// static int ya5flat[5][5]
// static int yb5flat[5][5]
// static int v5flat[5][5]
// static int i4flat[4][4]
// static int t4flat[4][4]
// static int j4flat[4][4]
// static int l4flat[4][4]
// static int oflat[4][4]
// static int s4flat[4][4]
// static int z4flat[4][4]

/* grid_t qrspiece_rotation_table_grids[25][4] =
{
    { // QRS_I
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_I][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_I][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_I][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_I][3]
        }
    },

    { // QRS_J
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_J][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_J][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_J][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_J][3]
        }
    },

    { // QRS_L
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_L][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_L][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_L][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_L][3]
        }
    },

    { // QRS_X
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_X][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_X][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_X][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_X][3]
        }
    },

    { // QRS_S
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_S][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_S][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_S][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_S][3]
        }
    },

    { // QRS_Z
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Z][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Z][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Z][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Z][3]
        }
    },

    { // QRS_N
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_N][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_N][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_N][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_N][3]
        }
    },

    { // QRS_G
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_G][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_G][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_G][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_G][3]
        }
    },

    { // QRS_U
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_U][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_U][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_U][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_U][3]
        }
    },

    { // QRS_T
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_T][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_T][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_T][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_T][3]
        }
    },

    { // QRS_Fa
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fa][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fa][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fa][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fa][3]
        }
    },

    { // QRS_Fb
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fb][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fb][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fb][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Fb][3]
        }
    },

    { // QRS_P
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_P][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_P][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_P][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_P][3]
        }
    },

    { // QRS_Q
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Q][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Q][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Q][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Q][3]
        }
    },

    { // QRS_W
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_W][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_W][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_W][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_W][3]
        }
    },

    { // QRS_Ya
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Ya][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Ya][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Ya][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Ya][3]
        }
    },

    { // QRS_Yb
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Yb][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Yb][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Yb][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_Yb][3]
        }
    },

    { // QRS_V
        (grid_t) { // FLAT
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_V][0]
        },

        (grid_t) { // CW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_V][1]
        },

        (grid_t) { // FLIP
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_V][2]
        },

        (grid_t) { // CCW
            .w = 5,
            .h = 5,
            .grid = qrspent_yx_rotation_tables[QRS_V][3]
        }
    },

    // tetrominoes

    { // QRS_I4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_I][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_I][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_I][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_I][3]
        }
    },

    { // QRS_T4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_T][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_T][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_T][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_T][3]
        }
    },

    { // QRS_J4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_J][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_J][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_J][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_J][3]
        }
    },

    { // QRS_L4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_L][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_L][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_L][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_L][3]
        }
    },

    { // QRS_O
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_O][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_O][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_O][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_O][3]
        }
    },

    { // QRS_S4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_S][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_S][1]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_S][2]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = qrstet_yx_rotation_tables[QRS_ARS_S][3]
        }
    },

    { // QRS_Z4
        (grid_t) { // FLAT
            .w = 4,
            .h = 4,
            .grid = &qrstet_yx_rotation_tables[QRS_ARS_Z][0][0][0]
        },

        (grid_t) { // CW
            .w = 4,
            .h = 4,
            .grid = &qrstet_yx_rotation_tables[QRS_ARS_Z][1][0][0]
        },

        (grid_t) { // FLIP
            .w = 4,
            .h = 4,
            .grid = &qrstet_yx_rotation_tables[QRS_ARS_Z][2][0][0]
        },

        (grid_t) { // CCW
            .w = 4,
            .h = 4,
            .grid = &qrstet_yx_rotation_tables[QRS_ARS_Z][3]
        }
    }
}; */

/*
static piecedef qrs_piecedefs[25] = {
    { // QRS_I piecedef
        .qrs_id = QRS_I,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_I] }
    },

    { // QRS_J piecedef
        .qrs_id = QRS_J,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_J] }
    },

    { // QRS_L piecedef
        .qrs_id = QRS_L,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_L] }
    },

    { // QRS_X piecedef
        .qrs_id = QRS_X,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_X] }
    },

    { // QRS_S piecedef
        .qrs_id = QRS_S,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_S] }
    },

    { // QRS_Z piecedef
        .qrs_id = QRS_Z,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Z] }
    },

    { // QRS_N piecedef
        .qrs_id = QRS_N,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_N] }
    },

    { // QRS_G piecedef
        .qrs_id = QRS_G,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_G] }
    },

    { // QRS_U piecedef
        .qrs_id = QRS_U,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_U] }
    },

    { // QRS_T piecedef
        .qrs_id = QRS_T,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_T] }
    },

    { // QRS_Fa piecedef
        .qrs_id = QRS_Fa,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Fa] }
    },

    { // QRS_Fb piecedef
        .qrs_id = QRS_Fb,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Fb] }
    },

    { // QRS_P piecedef
        .qrs_id = QRS_P,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_P] }
    },

    { // QRS_Q piecedef
        .qrs_id = QRS_Q,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Q] }
    },

    { // QRS_W piecedef
        .qrs_id = QRS_W,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_W] }
    },

    { // QRS_Ya piecedef
        .qrs_id = QRS_Ya,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Ya] }
    },

    { // QRS_Yb piecedef
        .qrs_id = QRS_Yb,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_Yb] }
    },

    { // QRS_V piecedef
        .qrs_id = QRS_V,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_V] }
    },

    { // QRS_I4/QRS_ARS_I piecedef
        .qrs_id = QRS_I4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_I4] }
    },

    { // QRS_T4/QRS_ARS_T piecedef
        .qrs_id = QRS_T4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_T4] }
    },

    { // QRS_J4/QRS_ARS_J piecedef
        .qrs_id = QRS_J4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_J4] }
    },

    { // QRS_L4/QRS_ARS_L piecedef
        .qrs_id = QRS_L4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_L4] }
    },

    { // QRS_O/QRS_ARS_O piecedef
        .qrs_id = QRS_O,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_O] }
    },

    { // QRS_S4/QRS_ARS_S piecedef
        .qrs_id = QRS_S4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { qrspiece_rotation_table_grids[QRS_S4] }
    },

    { // QRS_Z4/QRS_ARS_Z piecedef
        .qrs_id = QRS_Z4,
        .flags = 0,
        .anchorx = ANCHORX_QRS,
        .anchory = ANCHORY_QRS,
        .rotation_tables = { { qrspiece_rotation_table_grids[QRS_Z4] } }
    }
}; */
