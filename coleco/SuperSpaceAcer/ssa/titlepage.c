#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <player.h>

#include "game.h"
#include "trampoline.h"

#define BIN2INC_HEADER_ONLY
#include "title_c.c"
#include "title_p.c"
#include "..\title2\ship1_c.c"
#include "..\title2\ship1_p.c"
#include "..\title2\ship2_c.c"
#include "..\title2\ship2_p.c"
#include "..\title2\ship3_c.c"
#include "..\title2\ship3_p.c"
#include "..\title2\ship4_c.c"
#include "..\title2\ship4_p.c"

// only ship1 is on the same page as we are, 2-4 are on another page

// this is for bitmap mode - it does NOT use the globals,
// but uses fixed offsets
void ldpic() {
	// loads a picture - we aren't actually loading from disk
	// we are un-RLE-ing the image from ROM
	RLEUnpack(0x0000, TITLEP, 6144);
	RLEUnpack(0x2000, TITLEC, 6144);
}

void animate(unsigned char x) {
	unsigned char i;

	switch (x) {
		case 0:
			// first frame doesn't need a trampoline
			for (i=0; i<17; ++i) {
				vdpmemcpy(0x2000+6*32*8+16*8+(i*32*8), SHIP1C+(i*16*8), 16*8);
				vdpmemcpy(0x0000+6*32*8+16*8+(i*32*8), SHIP1P+(i*16*8), 16*8);
			}
			break;

		case 1:
			wrapCopyShip(SHIP2P,SHIP2C);
			break;

		case 2:
			wrapCopyShip(SHIP3P,SHIP3C);
			break;

		case 3:
			wrapCopyShip(SHIP4P,SHIP4C);
			break;
	}
}

void handleTitlePage() {
	unsigned char i;

	shutup();

	i=intpic();
	ldpic();
	spdall();
	// enable the screen
	VDP_SET_REGISTER(VDP_REG_MODE1, i);
	FIX_KSCAN(i);
	
	// display score in sprites, if not zero
	// assumes sprite patterns at 0x3800
	if (playership != 255) {
		wrapspritescore(0x3800, 0x1b00, 0xff, 103, 0);
	}

	joynum=1;
	do {
		seed++;			// random number seed

		// every 4 frames, update the graphic frame
		VDP_WAIT_VBLANK_CRU;
		if (0 == (seed&0x03)) {
			animate((seed>>2)&0x03);
		}
		VDP_CLEAR_VBLANK;

		joynum=joynum==1?2:1;
		kscanfast(joynum);
	} while (JOY_FIRE != KSCAN_KEY);
	
	// wipe the screen
	for (i=0; i<12; i++) {
		vdpmemset(0x1800+384+((int)i << 5), 0x00, 32);
		vdpmemset(0x1800+384-((int)i << 5), 0x00, 32);

		// wait for vblank
		vdpwaitvint();
	}
}

