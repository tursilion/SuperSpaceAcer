// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <ColecoSNPlay.h>

// game
#include "game.h"
#include "trampoline.h"
#include "music.h"

extern const unsigned char colecofont[];
unsigned char bottomsprite, bottomrow;
unsigned char gnaty;

const char selenaTxt[] = 
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

// text for the ladybug single line
//                       12345678901234567890123456789012
const char LadyText[] = "What do you think bugs dream of?";

const char gnatstory1[] = 
//	012345678901234567890123456789012
    "                                "
	"REPORTER: We are here at the    "
	"victory celebration for our new "
	"champion, the amazing pilot GNAT"
	"who has destroyed the Qwertian  "
	"threat! We expect them to arrive"
	"at any moment...                "
	"*";

const char gnatstory2[] = 
//	012345678901234567890123456789012
    "                                "
    "                                "
	"GNAT: I'm here! Yes! I'm here!  "
	"      I'm the GREATEST!!        "
    "                                "
	"*";

const char gnatstory3[] = 
//	012345678901234567890123456789012
    "                                "
    "                                "
	"GNAT: Hey, can't anyone hear me?"
	"      Hello?!?!                 "
    "                                "
    "                                "
    "*";

// 32x6 bytes
const unsigned char gnatSprites[] = {
//	Ship main
//Black:	0
	0x00, 0x00, 0x00, 0x00, 0x3C, 0x47, 0x78, 0x80,
	0x78, 0x27, 0x70, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xF0, 0x10, 0x10, 0x0C,
	0x04, 0xFC, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00,
//White:	4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x7F,
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xF8,
	0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//Blue5:	8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//ltyellow: 12
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

//Cockpit open
//Black		16
	0x00, 0x00, 0x20, 0x50, 0x2C, 0x17, 0x78, 0x80,
	0x78, 0x27, 0x70, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xF0, 0x10, 0x10, 0x0C,
	0x04, 0xFC, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00,
//Blue5		20
	0x00, 0x00, 0x00, 0x20, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

//Man
//Cyan7		24
	0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x50, 0x20,
	0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const signed char gnathop[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	-3, -5, -6, -4, -2, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	-3, -5, -6, -4, -2, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

unsigned int rndnum13()
{
	static unsigned int seed = 1;
	if (seed == 0) ++seed;	// 0 is illegal

	// these are all the masks
#if 0
/* 00 */       0x00,  //             0
/* 01 */       0x01,  //             1
/* 02 */       0x03,  //             3
/* 03 */       0x06,  //             7
/* 04 */       0x0C,  //            15
/* 05 */       0x14,  //            31
/* 06 */       0x30,  //            63
/* 07 */       0x60,  //           127
/* 08 */       0xB8,  //           255
/* 09 */     0x0110,  //           511
/* 10 */     0x0240,  //         1,023
/* 11 */     0x0500,  //         2,047
/* 12 */     0x0CA0,  //         4,095
/* 13 */     0x1B00,  //         8,191
/* 14 */     0x3500,  //        16,383
/* 15 */     0x6000,  //        32,767
/* 16 */     0xB400,  //        65,535
/* 17 */ 0x00012000,  //       131,071
/* 18 */ 0x00020400,  //       262,143
/* 19 */ 0x00072000,  //       524,287
/* 20 */ 0x00090000,  //     1,048,575
/* 21 */ 0x00140000,  //     2,097,151
/* 22 */ 0x00300000,  //     4,194,303
/* 23 */ 0x00400000,  //     8,388,607
/* 24 */ 0x00D80000,  //    16,777,215
/* 25 */ 0x01200000,  //    33,554,431
/* 26 */ 0x03880000,  //    67,108,863
/* 27 */ 0x07200000,  //   134,217,727
/* 28 */ 0x09000000,  //   268,435,575
/* 29 */ 0x14000000,  //   536,870,911
/* 30 */ 0x32800000,  // 1,073,741,823
/* 31 */ 0x48000000,  // 2,147,483,647
/* 32 */ 0xA3000000   // 4,294,967,295
#endif

	// trying the dreamcast one again, but 13-bit this time (0-8191)
	if (seed&1) {
		seed >>= 1;
		seed ^= 0x1B00;
	} else {
		seed >>= 1;
	}

	return seed;
}

void selenascroll() {
	// do a vertical scroll of the rightmost 12 characters
	// tmpbuf has the next 12 bytes to put at the bottom
	unsigned char idx;
	unsigned char row;
	unsigned int adr;
	unsigned char *pDat;

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
	// port this version of the code to F18A for a perfect scroll
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

void selenawin() {
	unsigned char i, i2;
	unsigned int adr;
	unsigned char *pTxt;

	i2=intpic();
	wrapLoadSelenaPic();
    VDP_SET_REGISTER(7,COLOR_BLACK);    // make sure the screen is black

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
	pTxt = (unsigned char*)selenaTxt;

	// set up the scroll - we put one row of the current
	// text into tmpbuf and then call selenascroll, over
	// and over until we're finished!
	while (*pTxt != '@') {
		// scroll eight times, then we need new text
		for (i=0; i<8; i++) {
			for (i2=0; i2<12; i2++) {	// always 12 characters across
				adr = ((*(pTxt+i2)-32)<<3) + i;
				wrapgetfontbytes(&tmpbuf[i2], &colecofont[adr], 1);
			}
			selenascroll();
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
		selenascroll();
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

void ladybugwin() {
	unsigned char i2;

	i2=intpic();
	// change the init of the image table to all zeros (it's at >1800)
	vdpmemset(0x1800, 0, 768);
	// load the bug game screen (it won't show due to the altered SIT)
	wrapLoadLadyScreen();
    VDP_SET_REGISTER(7,COLOR_BLACK);    // make sure the screen is black

	// enable the screen
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);
	FIX_KSCAN(i2);

	// start the music
	StartMusic(WINANIMMUS, 0);

	// we're going to scroll the screen into place, just as an analog for starting a new stage
	for (unsigned char i1=0; i1<24; ++i1) {
		// one frame
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it

		// update the SIT - there's no easy way to do a /correct/
		// scroll, but this somewhat deformed one actually looks
		// halfway decent! Completely wrong, but decent.
		vdpwriteinc(gImage, (23-i1)*32, (i1+1)*32);
		
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it
	}

	// wait just under two seconds
	for (unsigned char i1=0; i1<100; ++i1) {
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it
	}

	// to dither it in, we are going to need to use uncompressed image data

	// we'll fade in from the bottom left upwards
	// to maintain performance, we won't /fully/ run 8k over each step
	// but it should average out! Note the bottom row is blank in the data
	// so we can put the text there.
	// I really like this effect!!
	for (unsigned char step=0; step<32; ++step) {
		for (unsigned int i1=0; i1<512; ++i1) {
			unsigned int x = rndnum13();
			{
				unsigned char col = (x>>3)&0x1f;	// column is easy
				if (col < 31-step) continue;
			}
			{
				unsigned char row = ((x>>11)<<3)+((x&0x7ff)>>8);
				if ((step<23)&&(row < 23-step)) continue;
			}
			wrapLadyBugByte(x);
			musicsync();	// does not wait
		}
	}
	// the final full run to flesh it out
	for (unsigned int i1=0; i1<8191; ++i1) {
		unsigned int x = rndnum13();
		wrapLadyBugByte(x);
		musicsync();	// does not wait
	}
	wrapLadyBugByte(0);

	// wait just under two seconds
	for (unsigned char i1=0; i1<90; ++i1) {
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it
	}

	// draw the text on the bottom row
	{
		const char *pTxt = LadyText;
		int off = 0x1700;
		while (*pTxt) {
			wrapDrawLastRowText(*pTxt, off);
			++pTxt;
			off+=8;
			VDP_WAIT_VBLANK_CRU;
			musicsync();	// clears it
			VDP_WAIT_VBLANK_CRU;
			musicsync();	// clears it
		}
	}

	// wait for music to end
	while (isSNPlaying) {
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it
	}

	// set graphics mode back before we exit
	i2 = grf1();
	
	/*load VDP data */
	loadcharset();
	
	spdall();	// clears sprite table
	vdpmemset(gSPRITES, 0xd0, 128);	// clears VDP copy of sprite table (fixes initial gfx glitch)
	sgrint();	// fix color table
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);	// Switch screen on
	FIX_KSCAN(i2);
}

static void delayText(unsigned char frames) {
	static unsigned char hop = 0;
	unsigned char cnt;

	// get the correct y offset
	if (gnaty == 0) gnaty=vdpreadchar(0x1b0c);

	while (frames--) {
		cnt = VDP_INT_COUNTER;
		for (;;) {
			musicsync();	// clears it
			if (cnt != VDP_INT_COUNTER) break;
		}

		if (gnaty != 0xd0) {
			vdpchar(0x1B0C, gnaty+gnathop[(hop>>1)&0x3f]);
			++hop;
		}
	}
}

static void runText(const char *pTxt) {
	// assume SIT is at 0x1800
	int scrn = 0x1800+512;

	// first clear the bottom third of the screen
	vdpmemset(scrn, ' ', 32*8);

	// now loop, displaying characters and checking fire until
	// we get an asterisk.
	while (*pTxt != '*') {
		delayText(2);
		vdpchar(scrn++, *(pTxt++));
	}
}

// replacement sprite for the bitmap address of SAL
static void bmpsprite(unsigned char n, unsigned char ch, unsigned char col, unsigned char r, unsigned char c) {
	VDP_SET_ADDRESS_WRITE(n*4+0x1B00);
	VDP_SAFE_DELAY();
	VDPWD = r;
	VDP_SAFE_DELAY();
	VDPWD = c;
	VDP_SAFE_DELAY();
	VDPWD = ch;
	VDP_SAFE_DELAY();
	VDPWD = col;
}

void gnatwin() {
	unsigned char i2;
	unsigned int y;
	const unsigned char DELAYT = 180;

	i2=intpic();
	wraploadgnat1();
    VDP_SET_REGISTER(7,COLOR_BLACK);    // make sure the screen is black

	// load the character set down into the third half of the screen
	// yes, three halves. Blame TI. ;)
	wrapLoadStoryFont();

	// and fix the colors to green on transparent
	vdpmemset(gColor+4096, 0x30, 2048);

	// load the sprite patterns, too
	vdpmemcpy(0x3800, gnatSprites, 32*7);
										
	// enable the screen
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);
	FIX_KSCAN(i2);

	// start the music
	StartMusic(WINANIMMUS, 0);

	// run the story inline - if start is pressed, exit
	runText(gnatstory1);
	delayText(DELAYT);

	// wait, then clear the text
	VDP_WAIT_VBLANK_CRU;
	musicsync();	// clears it
	vdpmemset(0x1800+512, ' ', 32*8);

	// load the rest of the gnat frames
	wraploadgnat2();
	wraploadgnat3();
	wraploadgnat4();
	wraploadgnat5();

	// and fix the colors to white on transparent
	VDP_WAIT_VBLANK_CRU;
	musicsync();	// clears it
	vdpmemset(gColor+4096, 0xf0, 2048);

	// fly the gnat ship onscreen and pop him out!
	i2=0;
	y=32;
	bmpsprite(4, 20, COLOR_DKBLUE, 0xd0, 0);		// true end of list
	for (unsigned char i=255; i>110; i--) {
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it

		y+=(i2>>3);
		++i2;
		bmpsprite(3, 12, COLOR_LTYELLOW, y>>4, i);		// flame
		bmpsprite(2, 0, COLOR_BLACK, y>>4, i);
		bmpsprite(1, 4, COLOR_WHITE, y>>4, i);
		bmpsprite(0, 8, COLOR_LTBLUE, y>>4, i);
	}

	// open the cockpit
	VDP_WAIT_VBLANK_CRU;
	musicsync();	// clears it
	vdpchar(0x1b02, 20);	// window
	VDP_SAFE_DELAY();		// probably don't need this, but just in case
	vdpchar(0x1b0a, 16);	// outline

	// player down from ship and move over a bit
	y>>=4;	// lose the fraction
	for (unsigned char i=0; i<4; ++i) {
		for (unsigned char i2=0; i2<4; ++i2) {
			VDP_WAIT_VBLANK_CRU;
			musicsync();	// clears it
		}
		++y;
		bmpsprite(3, 24, COLOR_DKBLUE, y, 112);
	}

	// player to the right behind the ship
	for (unsigned char i=112; i<126; ++i) {
		for (unsigned char i2=0; i2<4; ++i2) {
			VDP_WAIT_VBLANK_CRU;
			musicsync();	// clears it
		}
		bmpsprite(3, 24, COLOR_DKBLUE, y, i);
	}
	gnaty = 0;

	runText(gnatstory2);
	delayText(DELAYT);

	runText(gnatstory3);
	delayText(DELAYT);

	// wait, then clear the text
	VDP_WAIT_VBLANK_CRU;
	musicsync();	// clears it
	vdpmemset(0x1800+512, ' ', 32*8);

	// wait for music to end
	while (isSNPlaying) {
		VDP_WAIT_VBLANK_CRU;
		musicsync();	// clears it
	}

	// set graphics mode back before we exit
	i2 = grf1();
	
	/*load VDP data */
	loadcharset();
	
	spdall();	// clears sprite table
	vdpmemset(gSPRITES, 0xd0, 128);	// clears VDP copy of sprite table (fixes initial gfx glitch)
	sgrint();	// fix color table
	VDP_SET_REGISTER(VDP_REG_MODE1, i2);	// Switch screen on
	FIX_KSCAN(i2);
}

void gamewinhard() {
	// just returns for now
	if (playership == SHIP_SELENA) selenawin();
	else if (playership == SHIP_LADYBUG) ladybugwin();
	else if (playership == SHIP_GNAT) gnatwin();

}
