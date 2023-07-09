 // libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <player.h>

// game
#include "game.h"

// Star patterns
#define SMALL_STAR_FIRST	0
#define SMALL_STAR_LAST		7
#define NUM_SMALL_STARS		6

uint8 stdata[NUM_SMALL_STARS*3];    // strided array so we can use faster(?) pointer math: row, col, offset
extern unsigned char tmpbuf[32];
unsigned char screenFlashCnt;

// do the background effect
// note the static counter cnt
void background() {
	// just stars today
	unsigned char b,x;
	static unsigned char cnt=0;

	/* used to count half frames */
	cnt++;

	/* move small stars */
	// Note that there are three speeds of small stars - 1/2 frame, 1 frame, and 2 per frame
	for (b=(cnt&1)?0:NUM_SMALL_STARS/3; b<NUM_SMALL_STARS; b++) {
        uint8 *p = &stdata[b*3];
		x=gchar(*p,*(p+1));
		if (b >= (NUM_SMALL_STARS/3)*2) (*(p+2))++;	// move 'med' stars by 2
		(*(p+2))++;
		if (*(p+2) > SMALL_STAR_LAST) {
			// erase old, if it's not hidden
			if (x < 8) {
				xchar(*p,*(p+1),32);
			}

			// update to new character
			*(p+2) = SMALL_STAR_FIRST;
			if (++(*p) == 24) {
				*p=0; 
				*(p+1)=(rndnum()&0x1f);
			}

			// get what's at the new position
			x=gchar(*p,*(p+1));
			if (x == 32) x=0;	// so the main case doesn't need an OR
		}
		// still have the old x if we didn't move, else we have the new one
		// draw if not obscured
		if (x<8) {
			xchar(*p,*(p+1),*(p+2));
		}
	}
}
 
void stars()
{ 
	// animate the background graphics
	background();
	
	// wait for vblank
	waitforstep();

	// reset screen color
	if (screenFlashCnt) {
		--screenFlashCnt;
		if (screenFlashCnt == 0) {
			screen(bgColor);
		}
	}
}
 
// note: does not draw the stars
void initstars() {
	unsigned char a;

	for (a=0; a<NUM_SMALL_STARS; a++) { 
        stdata[a*3]=rndnum()&0x1f; 
        if (stdata[a*3] > 23) stdata[a*3]-=24;
		stdata[a*3+1]=rndnum()&0x1f;
		stdata[a*3+2]=SMALL_STAR_FIRST;
	}
}
