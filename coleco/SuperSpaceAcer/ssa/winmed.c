// TODO: crashed after running the gnat win function from the sound test
// sounded like it didn't end when it was expected to, played though game over then crashed.

// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <ColecoSNPlay.h>

// game
#include "game.h"
#include "trampoline.h"
#include "music.h"

extern const char TXTDAT[];
extern const char WINTIPS[];
extern const unsigned char colecofont[];
char scorecountdown;

// VDP map for ending scroll text
// we have 8 pre-scrolled pattern tables, and
// overlap some (so only chars 0-63 are available, each takes 2)
// chars 128-255 are overwritten in the first table for the
// rest of the important tables. Got this from RasmusM's demo
// which in turn he creditted to The Mole
//	>0000				* Pattern table base 0
//	>0400				* Name table base 0
//	>0700				* Color table base
//	>0720				(unused)
//	>0780				* Sprite attribute table base
//	>0800				* Pattern table base 1
//	>0C00				* Name table base 1
//	>1000				* Pattern table base 2
//	>1000				* Sprite pattern table base (free from >1400)
//	>1800				* Pattern table base 3
//	>1C00				(technically unused)
//	>2000				* Pattern table base 4
//	>2400				(technically unused)
//	>2800				* Pattern table base 5
//	>2C00				(technically unused)
//	>3000				* Pattern table base 6
//	>3400				(technically unused)
//	>3800				* Pattern table base 7
//	>3C00				(technically unused)

// to have uppercase and lowercase fit in 64 characters, I need a limited font
// the scrolltext will probably need to be converted for performance's sake
// (but, maybe not, only 32 characters?)
// space must be first :) j and p are unused.
const char fontstr[] = " ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnoprstuvwxyz=!()-012369.`";

// at the end of the scroll text we will display a secret

void musicvdpsync(unsigned int reg1, unsigned int reg2) {
	// safely check if it's time to play music
	// writes the address passed (should normally be a register command)
	// followed by a poll which will destroy a real address set
	while (!(vdpLimi&0x80)) { }
	VDP_SET_ADDRESS(reg1);
	VDP_SET_ADDRESS(reg2);
	VDP_INT_POLL;
	doMusic();
}

// map characters into fontstr
void reparse(unsigned char *str, unsigned char offset) {
	unsigned char c, idx;
	const unsigned char *p;

	for (idx = 0; idx<32; idx++) {
		// this function needs to be fast, but this keeps the speed stable
		if ((idx == 8)||(idx==24)) musicvdpsync(0,0);

		c=0;		// space by default
		if ((str[idx]>='A')&&(str[idx]<'Z')) {
			c=str[idx]-'A'+1;
		} else if ((str[idx]>='a')&&(str[idx]<'q')) {
			c=str[idx]-'a'+26;
		} else if ((str[idx]>='r')&&(str[idx]<='z')) {
			c=str[idx]-'a'+25;
		} else {
			p=&fontstr[51];
			while (*p) {
				if (*p == str[idx]) {
					c = (p-fontstr);
					break;
				}
				++p;
			}
		}
		str[idx]=c+offset;
	}
}

void movesprites() {
	if (scorecountdown > 0) {
		// moving up the screen (4 sprites)
		unsigned char tmp = 0xc8-scorecountdown;
		VDP_SET_ADDRESS_WRITE(0x780);
		VDPWD=tmp;
		VDP_SAFE_DELAY();
		VDP_SET_ADDRESS_WRITE(0x784);
		VDPWD=tmp;
		VDP_SAFE_DELAY();
		VDP_SET_ADDRESS_WRITE(0x788);
		VDPWD=tmp;
		VDP_SAFE_DELAY();
		VDP_SET_ADDRESS_WRITE(0x78C);
		VDPWD=tmp;
		VDP_SAFE_DELAY();
		++scorecountdown;
	}
}

void gamewinmedium() { 
	unsigned char idx,i2,c;
	unsigned int srcCpy, dstCpy;
	unsigned char finalcountdown = 0;
	const char *txt;
	const char *oldtxt;

	// prepare score handler
	scorecountdown = 0;

	// set up the VDP registers, it's all different
	VDP_SET_ADDRESS(0x81a2);	// 16k,ints,Mag2,graphics mode (disabled screen)
	VDP_SET_ADDRESS(0x8201);	// SIT at >0400
	VDP_SET_ADDRESS(0x831c);	// CT at >0700
	VDP_SET_ADDRESS(0x8400);	// PT at >0000
	VDP_SET_ADDRESS(0x850f);	// SAL at >0780
	VDP_SET_ADDRESS(0x8602);	// SDT at >1000
	screen(COLOR_BLACK);

	// We can't use most of the helper facilities, so we do the VDP work inline
	// we can't use waitforstep either because the sprite table's in the wrong place

	// clear screens
	vdpmemset(0x400, 0, 768);	// current screen
	vdpmemset(0xc00, 0, 768);	// other screen

	// turn off sprites
	vdpchar(0x0780, 0xd0);

	// set colors
	for (idx=0; idx<32; idx++) {
		vdpchar(0x0700+idx, 0xf0);	// all sets white on transparent
	}

	// copy and pre-shift the character set
	// this is a painfully slow way to load the font, but
	// it will save trampoline space. If it's fast enough
	// (even a second or so is okay) then it's okay.
	for (idx=0; idx<64; idx++) {
		c = fontstr[idx]-32;									// which character to process
		memset(tmpbuf, 0, 16);									// prepare the work buf
		wrapgetfontbytes(&tmpbuf[8], colecofont+(c<<3), 8);		// get the whole character into the work buffer (now we have a 16 byte char)

		// now we write into VDP memory 8 times, shifting a bit each time
		// using memmove is probably a little slower, too, but hopefully it's fast enough,
		// since this part is not realtime
		for (i2 = 0; i2<8; i2++) {
			vdpmemcpy((idx*8)+0+(0x800*i2), tmpbuf, 8);
			vdpmemcpy((idx*8)+64*8+(0x800*i2), &tmpbuf[8], 8);

			memmove(tmpbuf, &tmpbuf[1], 15);
			tmpbuf[15]=0;
		}
	}

	// okay, now we have a character set prepared with all 8 shift patterns

	// Set up sprites to be the score digits at the end
	// create 4 sprites just like on the title page,
	// but at the bottom of the screen
	//	>0780				* Sprite attribute table base
	//	>1000				* Sprite pattern table base (free from >1400)
	wrapspritescore(0x1400, 0x0780, 0xc0, 19*8, 128);

	// we could start the music early if we liked...
	StartMusic(WINSCROLLMUS, 1);
	txt=TXTDAT; 
	oldtxt = txt;

	// turn the screen on
	VDP_SET_ADDRESS(0x81e2);	// 16k,enabled,mag2,ints,graphics mode

	while (1) { 
		// first, we flip through 8 frames of pattern
		srcCpy = 0x0420;
		dstCpy = 0x0c20-32;	// deliberate underrun to force the scroll up
							// Remember! Screen table is in the middle of the
							// pattern table! We can't safely underrun it! But this works.

		// flip the screen for a few frames (only 1 frame needed, it appears)
		musicvdpsync(0x8201,0x8400);

		// for each frame, we copy one row of text to the other screen
		for (idx = 0; idx<23; idx++) {
			if ((idx%3) == 0) {
				// move sprites
				movesprites();
			}
			musicvdpsync(0x8201,0x8400+(idx/3));	// sets the pattern
			// copy one line to the other buffer
			vdpmemread(srcCpy, tmpbuf, 32);
			vdpmemcpy(dstCpy, tmpbuf, 32);
			srcCpy+=32;
			dstCpy+=32;
		}

		// we may be done
		if (finalcountdown) break;

		// before we flip, copy the new line of text over
		winwrapgetscroll(tmpbuf, txt, 32);

		// maybe temporary if too slow
		// this will pause the appropriate number of frames too
		reparse(tmpbuf,0);

		// write it out
		vdpmemcpy(dstCpy, tmpbuf, 32);

		// flip the screen
		srcCpy = 0x0c20;
		dstCpy = 0x0420-32;		// deliberate underrun to force the scroll up
		
		// frame delay (appears only one needed)
		musicvdpsync(0x8203,0x8400);

		// ** second loop **

		// for each frame, we copy one row of text to the other screen
		for (idx = 0; idx<23; idx++) {
			if ((idx%3) == 0) {
				// move sprites
				movesprites();
			}
			musicvdpsync(0x8203,0x8400+(idx/3));	// sets the pattern
			// copy one line to the other buffer
			vdpmemread(srcCpy, tmpbuf, 32);
			vdpmemcpy(dstCpy, tmpbuf, 32);
			srcCpy+=32;
			dstCpy+=32;
		}

		// before we flip, copy the new line of text over
		winwrapgetscroll(tmpbuf, txt, 32);

		// maybe temporary if too slow
		reparse(tmpbuf,64);

		// write it out
		vdpmemcpy(dstCpy, tmpbuf, 32);

		// update the score digits
		if (scorecountdown < 0) {
			// counting up to the final position
			++scorecountdown;
			if (scorecountdown == 0) scorecountdown = 1;	// time to move
		}

		// restore the txt pointer -- except for the hint line, this should do nothing
		txt = oldtxt;

		// check if we're done
		txt += 32;
		winwrapgetscroll(tmpbuf, txt, 1);
		if (tmpbuf[0] == '@') {
			// scroll just a little more, then exit
			finalcountdown = 1;
		}
		if (tmpbuf[0] == '#') {
			int n = rndnum()&0x07;	// 0-7
			// filter out hints that are clearly unnecessary
			if ((nDifficulty == DIFFICULTY_HARD) && (n == 0)) ++n;		// don't display 'try in hard mode' in hard mode
			if ((nDifficulty == DIFFICULTY_MEDIUM) && (n == 7)) --n;	// don't display cheat mode hint in medium mode
			if (((scoremode == 1) || (scoremode == 2)) && (n == 1)) ++n;	// don't display 'hold fire' if already playing a hidden ship 
			if ((scoremode == 3) && (n == 2)) ++n;						// don't display 'hold down' in invisible enemy mode
			if ((scoremode == 9) && (n == 7)) --n;						// don't display cheat mode hint when cheating
			++txt;
			scorecountdown = -7;
			// save off the text pointer and point to our hint line instead
			oldtxt = txt;
			txt = &WINTIPS[n<<5];
		} else {
			// we ALWAYS need to update oldtxt
			oldtxt = txt;
		}
	}

//	while (!pDone) {
    while (isSNPlaying) {
		musicvdpsync(0,0);
	}
}  

