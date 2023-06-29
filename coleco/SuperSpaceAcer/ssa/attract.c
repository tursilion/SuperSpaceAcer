#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <ColecoSNPlay.h>

// game
#include "game.h"
#include "attract.h"
#include "trampoline.h"

#define BIN2INC_HEADER_ONLY
#include "qwertianc.c"
#include "qwertianp.c"


// story screens have 8 lines of 32 characters to say their peace
const char story[] = 
#if 0
//	012345678901234567890123456789012
	"It has been less than a century "
	"since humans finally broke free "
	"    of the Earth and entered    "
	"      interstellar space.       "
	"*"
	" The Qwertians were one of the  "
	"first alien species mankind made"
	"         contact with.          "
	"*"
	"Initially thought to be peaceful"
	" these doe-like creatures soon  "
	"demonstrated a deadly drive for "
	"      expansion and power.      "
	"*"
	"Thirty years ago Bob Shumway and"
	" his CD15 'Snowball' proved the "
	" ability of a single fighter to "
	"   infiltrate their defenses.   "
	"*"
	"  Now Bob and his friends must  "
	"undertake their greatest mission"
	"to infiltrate the Qwertian Fleet"
	" and destroy their battleships. "
	"*"
#else
//	012345678901234567890123456789012
	"Species Designation: QWERTIAN   "
	"First Contact: Epoch+1.9.87 yrs "
	"Disposition: -- HOSTILE --      "
	"                                "
	" Using a small fighter, mission "
	"is to fight through the Qwertian"
	" fleet and destroy their battle "
	"cruisers. Success at all costs! "
	"*"
#endif
;

unsigned char delayText(unsigned char frames) {
	unsigned char cnt;

	while (frames--) {
		cnt = VDP_INT_COUNTER;
		for (;;) {
			kscanfast(1);
			if (KSCAN_KEY == JOY_FIRE) return 1;
			VDP_INT_POLL;
			if (cnt != VDP_INT_COUNTER) break;

			kscanfast(2);
			if (KSCAN_KEY == JOY_FIRE) return 1;
			VDP_INT_POLL;
			if (cnt != VDP_INT_COUNTER) break;
		}
	}

	return 0;
}

const char *runText(const char *pTxt) {
	// assume SIT is at 0x1800
	int scrn = 0x1800+512;

	// first clear the bottom third of the screen
	vdpmemset(scrn, ' ', 32*8);

	// now loop, displaying characters and checking fire until
	// we get an asterisk.
	while (*pTxt != '*') {
		if (delayText(2)) return 0;
		vdpchar(scrn++, *(pTxt++));
	}
	return ++pTxt;
}

void LoadQwertian() {
	RLEUnpack(0x0000, QWERTP, 6144);
	RLEUnpack(0x2000, QWERTC, 6144);
}

void attractStory() {
	unsigned char i2;
	const char *pTxt;
	const unsigned char DELAYT = 240;

	i2=intpic();
	LoadQwertian();
    VDP_SET_REGISTER(7,COLOR_BLACK);    // make sure the screen is black

	// load the character set down into the third half of the screen
	// yes, three halves. Blame TI. ;)
	wrapLoadStoryFont();

	// and fix the colors to white on transparent
	vdpmemset(gColor+4096, 0xf0, 2048);
										
	// enable the screen
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);
	FIX_KSCAN(i2);

	// run the story inline - if start is pressed, exit
	pTxt = runText(story);
	if (0 == pTxt) return;
	if (delayText(DELAYT)) return;

}

// just a wrapper for attractStory so we can reboot on exit
void doAttract() {
	attractStory();
	reboot();
}

