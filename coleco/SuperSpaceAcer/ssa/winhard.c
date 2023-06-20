// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "trampoline.h"

extern const unsigned char colecofont[];
unsigned char bottomsprite, bottomrow;

const char zenithTxt[] = 
//	 123456789012
	"Princess    "
	"Selena sent "
	"a final shot"
	"at the last "
	"remaining   "
	"probe, and  "
	"watched it  "
	"explode. She"
	"had driven  "
	"back the    "
	"invading    "
	"force and   "
	"her fellow  "
	"equines were"
	"safe once   "
	"again.      "
	"            "
	"Should the  "
	"villians    "
	"ever try to "
	"return, the "
	"Princess    "
	"will always "
	"stand ready,"
	"ready to    "
	"drive back  "
	"any who     "
	"would dare  "
	"violate the "
	"peace of her"
	"world.      "
	"@";

// predefined characters... 96 available in each character set (8*12) that's only 24 combinations with 2 pixel scrolling
// combinations in above text: 29 in just the first column. So we won't be doing that.

void musicsync() {
	// safely check if it's time to play music
	if (vdpLimi&0x80) {
		VDP_INT_POLL;
		doMusic();
	}
}

void zenithscroll() {
	// do a vertical scroll of the rightmost 12 characters
	// tmpbuf has the next 12 bytes to put at the bottom
	unsigned char idx;
	unsigned char row;
	unsigned int adr;
	unsigned char *pDat;

	// TODO: this is a little flickery on BlueMSX and goes out of sync at the top. 
	// What does hardware look like?
	// Flicker is probably as good as it will get with the unrolled loops. The out
	// of sync is harder to solve, it's caused by the screen refresh happening between
	// the pattern scroll and the sprite scroll. Moving the sprites along with the screen
	// might minimize it, but it seems to me hardware would be a bit more random than
	// BlueMSX is being?

	// the right half is a traditional bitmap scroll. The left half is
	// rotating sprites to reduce the number of writes to VDP for better
	// performance. Seems to work!

#if 0
	// No sprites, pure bitmap scrolling here
	// TODO: port this version of the code to F18A for a perfect scroll
	// using SpriteTab as a big char buffer to do a row at a time
	// using tmpbuf, one char at a time
	// do the topmost row (no previous cell)
	adr = 32*3*8 + 20*8 + 1;
	vdpmemread(adr, (unsigned char*)&SpriteTab, 95);
	vdpmemcpy(adr-1, (unsigned char*)&SpriteTab, 95);

	musicsync();

	// do the rest of the rows (with shift to previous cell)
	adr = 32*3*8 + 32*8 + 20*8;
	for (row = 0; row < 18; row ++) {
		vdpmemread(adr, (unsigned char*)&SpriteTab, 96);
		vdpmemcpy(adr, ((unsigned char*)&SpriteTab)+1, 95);

		for (idx=0; idx<96; idx+=8) {
			vdpchar(adr-(8*32-7), *(((unsigned char*)&SpriteTab)+idx));	// copy back the top rows to the previous cells
			adr+=8;
		}
		musicsync();
		adr += 8*32 - 8*12;
	}

	// copy in the newest data from tmpbuf
	adr=32*3*8 + 18*32*8+20*8+7;
	for (idx=0; idx<12; idx++) {
		vdpchar(adr, tmpbuf[idx]);
		adr+=8;
	}
	musicsync();
#else
	// using SpriteTab as a big char buffer to do a row at a time
	// do the topmost row (no previous cell)
	// this version is half sprites and half bitmap (but sprites are managed directly,
	// not double-buffered).
	adr = 32*4*8 + 26*8 + 1;
	vdpmemread(adr, (unsigned char*)&SpriteTab, 47);
	vdpmemcpy(adr-1, ((unsigned char*)&SpriteTab), 47);

	musicsync();

	// wipe the top row of data from the sprites
	if (bottomsprite == 27) {
		adr = bottomrow + 0x3800;
	} else {
		adr = (bottomsprite+3)*4*8+bottomrow + 0x3800;
	}
	for (idx = 0; idx<6; idx++) {
		vdpchar(adr, 0);
		adr += 16;
	}
	musicsync();

	// do the rest of the rows (with shift to previous cell)
	adr = 32*5*8 + 26*8;
	for (row = 0; row < 17; row ++) {
		vdpmemread(adr, (unsigned char*)&SpriteTab, 48);

		// multiple writes to reduce the flicker
		// using loops was too slow, but this appears to work.
		VDP_SET_ADDRESS_WRITE(adr);

		// we copy 7 bytes then zero the 8th, copying to the previous row happens
		// in a separate step
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+1);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+2);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+3);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+4);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+5);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+6);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+7);
		VDP_SAFE_DELAY();
		VDPWD=0;

		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+9);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+10);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+11);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+12);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+13);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+14);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+15);
		VDP_SAFE_DELAY();
		VDPWD=0;

		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+17);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+18);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+19);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+20);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+21);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+22);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+23);
		VDP_SAFE_DELAY();
		VDPWD=0;

		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+25);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+26);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+27);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+28);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+29);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+30);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+31);
		VDP_SAFE_DELAY();
		VDPWD=0;

		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+33);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+34);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+35);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+36);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+37);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+38);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+39);
		VDP_SAFE_DELAY();
		VDPWD=0;

		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+41);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+42);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+43);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+44);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+45);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+46);
		VDP_SAFE_DELAY();
		VDPWD=*(((unsigned char*)&SpriteTab)+47);
		VDP_SAFE_DELAY();
		VDPWD=0;
#endif

#if 0
		for (idx=0; idx<48; idx+=8) {
			vdpchar(adr-(8*32-7), *(((unsigned char*)&SpriteTab)+idx));	// copy back the top rows to the previous cells
			adr+=8;
		}
#else
		// six writes to copy back rows to the previous cells
		vdpchar(adr-(8*32-7), *(((unsigned char*)&SpriteTab)));	// copy back the top rows to the previous cells
		VDP_SAFE_DELAY();
		vdpchar(adr-(8*32-7)+8, *(((unsigned char*)&SpriteTab)+8));	// copy back the top rows to the previous cells
		VDP_SAFE_DELAY();
		vdpchar(adr-(8*32-7)+16, *(((unsigned char*)&SpriteTab)+16));	// copy back the top rows to the previous cells
		VDP_SAFE_DELAY();
		vdpchar(adr-(8*32-7)+24, *(((unsigned char*)&SpriteTab)+24));	// copy back the top rows to the previous cells
		VDP_SAFE_DELAY();
		vdpchar(adr-(8*32-7)+32, *(((unsigned char*)&SpriteTab)+32));	// copy back the top rows to the previous cells
		VDP_SAFE_DELAY();
		vdpchar(adr-(8*32-7)+40, *(((unsigned char*)&SpriteTab)+40));	// copy back the top rows to the previous cells
#endif

		musicsync();
		adr += 8*32;
	}

	// shift all the sprites up
	adr = 0x1b00;
	pDat = ((unsigned char*)&SpriteTab) + 48;
	for (idx = 0; idx<30; idx++) {
		row = --(*pDat);
		// wrap down as needed
		if (row <= 15) {
			row = 22*8-1;
			*pDat = row;
		}
		vdpchar(adr, row);
		adr+=4;
		++pDat;
	}
	musicsync();

	// copy in the newest data from tmpbuf
	adr=32*3*8 + 18*32*8 + 26*8 + 7;
	for (idx=6; idx<12; idx++) {
		vdpchar(adr, tmpbuf[idx]);
		adr+=8;
	}
	musicsync();

	// add the data to the sprites too
	adr = bottomsprite*4*8 + bottomrow + 0x3800;
	for (idx = 0; idx<6; idx++) {
		vdpchar(adr, tmpbuf[idx]);
		adr += 16;
	}
	musicsync();

	++bottomrow;
	if (bottomrow >= 16) {
		bottomsprite += 3;
		if (bottomsprite > 27) bottomsprite = 0;
		bottomrow = 0;
	}
}

void zenithwin() {
	unsigned char i, i2;
	unsigned int adr;
	unsigned char *pTxt;

	i2=intpic();
	wrapLoadZenithPic();

	// set up 30 sprites to be half the scroll text
	VDP_SET_ADDRESS_WRITE(0x1b00);	// sprite table
	// we store the y coordinates in the top half of SpriteTab (as a dummy buffer)
	pTxt = ((unsigned char*)&SpriteTab) + 48;
	for (i=0; i<30; i++) {
		*pTxt = (i/3)*16+31;
		VDPWD = *(pTxt++);
		VDPWD = (i%3)*16+20*8;
		VDPWD = i*4;
		VDP_SAFE_DELAY();		// no math here
		VDPWD = COLOR_WHITE;
	}
	vdpmemset(0x3800, 0, 30*4*8);	// zero the pattern data

	bottomsprite = 27;
	bottomrow = 0;

	// enable the screen
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);
	FIX_KSCAN(i2);

	// fix the color table for the rightmost section
	adr = 20*8+0x2000;
	for (i=0; i<24; i++) {
		vdpmemset(adr, 0xf0, 12*8);
		adr+=32*8;
	}

	StartMusic(WINANIMMUS, 0);
	pTxt = zenithTxt;

	// set up the scroll - we put one row of the current
	// text into tmpbuf and then call zenithscroll, over
	// and over until we're finished!
	while (*pTxt != '@') {
		// scroll eight times, then we need new text
		for (i=0; i<8; i++) {
			for (i2=0; i2<12; i2++) {	// always 12 characters across
				adr = ((*(pTxt+i2)-32)<<3) + i;
				wrapgetfontbytes(&tmpbuf[i2], &colecofont[adr], 1);
			}
			zenithscroll();
		}
		// we can't use waitforstep for sync, because it does sprite copies
		// and the sprite table is in the middle of the image
		// because it takes about 3 frames to finish, the scroll function
		// will handle music.
		pTxt+=12;
	}

	// there's no more text, so we just need to scroll it off 
	memset(tmpbuf, 0, 12);
	for (i=0; i<180; i++) {
		zenithscroll();
	}

	// set graphics mode back before we exit
	i = grf1();
	
	/*load VDP data */
	loadcharset();
	
	spdall();	// clears sprite table
	vdpmemset(gSPRITES, 0xd0, 128);	// clears VDP copy of sprite table (fixes initial gfx glitch)
	sgrint();	// fix color table
	VDP_SET_REGISTER(VDP_REG_MODE1, i);	// Switch screen on
	FIX_KSCAN(i);

}

void gamewinhard() {
	// just returns for now
	if (playership == SHIP_ZENITH) zenithwin();
}
