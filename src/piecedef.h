#ifndef _piecedef_h
#define _piecedef_h

#include <stdint.h>
#include "grid.h"

#define PDNOWKICK			0x00000001
#define PDNOFKICK			0x00000002

#define PDBRACKETS			0x00000004
#define PDFLATFLOORKICKS 	0x00000008
#define PDONECELLFLOORKICKS 0x00000010
#define PDPREFERWKICK		0x00000020
#define PDAIRBORNEFKICKS	0x00000040

enum {
	FLAT = 0,
	CW = 1,
	FLIP = 2,
	CCW = 3
};

typedef struct {
	uint8_t qrs_id;		// minor cross-contamination (old: int color)
	unsigned int flags;
	int anchorx;
	int anchory;
	grid_t *rotation_tables[4]; // these grids technically don't have to be the same size
} piecedef;

piecedef *piecedef_create();
void piecedef_destroy(piecedef *pd);

piecedef *piecedef_cpy(piecedef *pd);

int pdsetw(piecedef *pd, int w);
int pdseth(piecedef *pd, int h);
int pdsetcell(piecedef *pd, int orientation, int x, int y);
//int pdchkflags(piecedef *pd, unsigned int tflags);
#endif
