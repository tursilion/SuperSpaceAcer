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

uint8 str[NUM_SMALL_STARS], stc[NUM_SMALL_STARS], sto[NUM_SMALL_STARS];		// star row, col, offset
extern unsigned char tmpbuf[32];

// do the background effect
// note the static counter cnt, and the bullet color animation
void background() {
	// just stars today
	unsigned char b,x;
	static unsigned char cnt=0;

	/* used to count half frames */
	cnt++;

	// flash the enemy bullets
	x=12-(cnt&4);
	SpriteTab[11].col = x;
	SpriteTab[12].col = x;
	SpriteTab[13].col = x;
	SpriteTab[14].col = x;
	SpriteTab[15].col = x;

	// TODO: just a dumb test to remove
	x=cnt&7;
	tmpbuf[(x+0)&7]=0;
	tmpbuf[(x+1)&7]=0;
	tmpbuf[(x+2)&7]=0;
	tmpbuf[(x+3)&7]=2;
	tmpbuf[(x+4)&7]=0;
	tmpbuf[(x+5)&7]=0;
	tmpbuf[(x+6)&7]=0x20;
	tmpbuf[(x+7)&7]=0;
	patcpy(0, 32);
	// end test

	/* move small stars */
	// Note that there are three speeds of small stars - 1/2 frame, 1 frame, and 2 per frame
	for (b=(cnt&1)?0:NUM_SMALL_STARS/3; b<NUM_SMALL_STARS; b++) {
		x=gchar(str[b],stc[b]);
		if (b >= (NUM_SMALL_STARS/3)*2) ++sto[b];	// move 'med' stars by 2
		++sto[b];
		if (sto[b] > SMALL_STAR_LAST) {
			// erase old, if it's not hidden
			if (x < 8) {
				xchar(str[b],stc[b],32);
			}

			// update to new character
			sto[b] = SMALL_STAR_FIRST;
			if (++str[b] == 24) {
				str[b]=0; 
				stc[b]=(rndnum()&0x1f);
			}

			// get what's at the new position
			x=gchar(str[b],stc[b]);
			if (x == 32) x=0;	// so the main case doesn't need an OR
		}
		// still have the old x if we didn't move, else we have the new one
		// draw if not obscured
		if (x<8) {
			xchar(str[b],stc[b],sto[b]);
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
	screen(1);
}
 
// note: does not draw the stars
void initstars() {
	unsigned char a;

	for (a=0; a<NUM_SMALL_STARS; a++) { 
		str[a]=rndnum()&0x1f; if (str[a] > 23) str[a]-=24;
		stc[a]=rndnum()&0x1f;
		sto[a]=SMALL_STAR_FIRST;
	}
}
