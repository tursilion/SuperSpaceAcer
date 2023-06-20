// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <player.h>

// game
#include "game.h"
#include "trampoline.h"

// how to get back to the title page the lazy way
void (* const reboot)()=0x802d;

// lower case letters in the TI format of small uppercase (96-123)
const unsigned char smallcapsfont[] = {
	0x3C,0x66,0xDB,0xB1,0xB1,0xDB,0x66,0x3C,0x00,0x00,0x78,0xCC,0xFC,0xCC,0xCC,0x00,	// 00000200 `0........x..... //
	0x00,0x00,0xF8,0x6C,0x78,0x6C,0xF8,0x00,0x00,0x00,0x7C,0xC0,0xC0,0xC0,0x7C,0x00,	// 00000210 ...lxl....|...|. //
	0x00,0x00,0xF8,0x6C,0x6C,0x6C,0xF8,0x00,0x00,0x00,0xF8,0xC0,0xF0,0xC0,0xF8,0x00,	// 00000220 ...lll.......... //
	0x00,0x00,0xF8,0xC0,0xF0,0xC0,0xC0,0x00,0x00,0x00,0x7C,0xC0,0xFC,0xCC,0x78,0x00,	// 00000230 ..........|...x. //
	0x00,0x00,0xCC,0xCC,0xFC,0xCC,0xCC,0x00,0x00,0x00,0xFC,0x30,0x30,0x30,0xFC,0x00,	// 00000240 ...........000.. //
	0x00,0x00,0x78,0x30,0x30,0xF0,0xF0,0x00,0x00,0x00,0xD8,0xF0,0xE0,0xF0,0xD8,0x00,	// 00000250 ..x00........... //
	0x00,0x00,0xC0,0xC0,0xC0,0xC0,0xFC,0x00,0x00,0x00,0xCC,0xFC,0xFC,0xCC,0xCC,0x00,	// 00000260 ................ //
	0x00,0x00,0xCC,0xEC,0xFC,0xDC,0xCC,0x00,0x00,0x00,0xFC,0xCC,0xCC,0xCC,0xFC,0x00,	// 00000270 ................ //
	0x00,0x00,0xF8,0xCC,0xF8,0xC0,0xC0,0x00,0x00,0x00,0xFC,0xCC,0xFC,0xD8,0xF0,0x00,	// 00000280 ................ //
	0x00,0x00,0xFC,0xCC,0xFC,0xF0,0xD8,0x00,0x00,0x00,0x7C,0xC0,0x78,0x0C,0xF8,0x00,	// 00000290 ..........|.x... //
	0x00,0x00,0xFC,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x78,0x00,	// 000002A0 ...0000.......x. //
	0x00,0x00,0xCC,0xCC,0xD8,0xF0,0x60,0x00,0x00,0x00,0xCC,0xCC,0xFC,0xFC,0xCC,0x00,	// 000002B0 ......`......... //
	0x00,0x00,0xCC,0x70,0x30,0x70,0xCC,0x00,0x00,0x00,0xCC,0x78,0x30,0x30,0x30,0x00,	// 000002C0 ...p0p.....x000. //
	0x00,0x00,0xFC,0x18,0x30,0x60,0xFC,0x00,0x3C,0x60,0x30,0xE0,0x30,0x60,0x3C,0x00		// 000002D0 ....0`..<`0.0`<. //
};

const unsigned char easypats[] = {
	0xff,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
};

void gamwin() {	
	shutup();
	spdall();

	// select game win routine (for all difficulties)
	if (nDifficulty == DIFFICULTY_HARD) {
		gamewinhard();
	}

	shutup();

	// hard includes medium, which is the credits scroll
	if (nDifficulty >= DIFFICULTY_MEDIUM) {
		gamewinmedium();
	}

	shutup();

	// easy stands alone, though
	if (nDifficulty == DIFFICULTY_EASY) {
		gamewineasy();
	}

	// on end, reboot
	// to preserve the score while keeping the lazy reboot, we save it off in VDP right after the screen table
	// we save it twice, once inverted as a checksum, so the boot will always know it's right
	*((unsigned short*)tmpbuf) = score;
	*((unsigned short*)(&tmpbuf[2])) = ~score;
	vdpmemcpy(768, tmpbuf, 4);
	VDP_SAFE_DELAY();
	// also save off the scoremode
	vdpchar(772, scoremode);
	reboot();
}

void slowcenter(unsigned char row, const char *out) {
	// print&center the text in out with delays
	unsigned char c;
	unsigned int outp;

	c=16-((strlen(out)+1)>>1);
	outp = gIMAGE+(row<<5)+c;

	while (*out) {
		VDP_SET_ADDRESS_WRITE(outp++);
		VDP_SAFE_DELAY();
		VDPWD=(*(out++));
		waitforstep();
		waitforstep();
	}
}

// easy routine just draws a few gridlines and pretends it was all a simulation
void gamewineasy() {
	unsigned char idx;
	unsigned int x;

	vdpmemcpy(gPATTERN+96*8, smallcapsfont, 28*8);
	patcpy('y', '[');
	patcpy('z', '\\');
	vdpmemcpy(gPATTERN+121*8, easypats, 8*3);
	// and zero out whatever used to be the space character
	vdpmemset(gPATTERN+32*8, 0, 8);

	for (idx = 0; idx < 32; idx++) {
		vchar(1,idx,123,22);
		if (idx == 0) {
			xchar(0,0,121);
			xchar(23,0,122);
		} else if (idx == 31) {
			xchar(0,31,123);
		} else {
			xchar(0,idx,121);
			xchar(23,idx,122);
		}
		waitforstep();
		waitforstep();
		waitforstep();
	}
	for (idx = 0; idx<23; idx++) {
		hchar(idx, 0, 121, 31);
		waitforstep();
		waitforstep();
		waitforstep();
	}

	// get the score into a buffer (adapted from addscore)
	strcpy(tmpbuf, "Final score: 0000000");
	x=score;
	idx='0';
	while (x>=10000) {
		++idx;
		x-=10000;
	}
	tmpbuf[13]=idx;

	idx='0';
	while (x>=1000) {
		++idx;
		x-=1000;
	}
	tmpbuf[14]=idx;

	idx='0';
	while (x>=100) {
		++idx;
		x-=100;
	}
	tmpbuf[15]=idx;

	idx='0';
	while (x>=10) {
		++idx;
		x-=10;
	}
	tmpbuf[16]=idx;
	tmpbuf[17]=x+'0';
	//tmpbuf[18]='0';		// always
	tmpbuf[19]='0'+scoremode;

	slowcenter(5, "Congratulations!");
	slowcenter(7, tmpbuf);
	slowcenter(9, "Simulation complete!");
	slowcenter(11, "You are now read[ to challenge");
	slowcenter(13, "the higher difficult[ levels!");

	centr(17, "GAME OVER");
	StartMusic(GAMEOVERMUS, 0);

	while (!pDone) {
		waitforstep();
	}
	shutup();
}

