// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "trampoline.h"
#include "enemy.h"
#include "human.h"

#define BIN2INC_HEADER_ONLY
#include "selena_end_c.c"
#include "selena_end_p.c"

// I don't THINK these need to be recursive, but just in case
#pragma nooverlay

void wrapenemy() {
	unsigned int old = nBank;
	SWITCH_IN_BANK1;
	enemy();
	SWITCH_IN_PREV_BANK(old);
}

void wrapnoen(uint8 x) {
	unsigned int old = nBank;
	SWITCH_IN_BANK1;
	noen(x);
	SWITCH_IN_PREV_BANK(old);
}

void wrapplayer() {
	unsigned int old = nBank;
	SWITCH_IN_BANK2;
	player();
	SWITCH_IN_PREV_BANK(old);
}

void wrapcheat() {
	unsigned int old = nBank;
	SWITCH_IN_BANK2;
	cheat();
	SWITCH_IN_PREV_BANK(old);
}

void wrapcolchk(uint8 x) {
	unsigned int old = nBank;
	SWITCH_IN_BANK2;
	colchk(x);
	SWITCH_IN_PREV_BANK(old);
}

void wrapplycol() {
	unsigned int old = nBank;
	SWITCH_IN_BANK2;
	plycol();
	SWITCH_IN_PREV_BANK(old);
}

void wrapplayerstraight() {
	unsigned int old = nBank;
	SWITCH_IN_BANK2;
	playerstraight();
	SWITCH_IN_PREV_BANK(old);
}

// boss chars run through to 255
void wrapunpackboss(unsigned char level) {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	switch (level) {
		case 1:	RLEUnpack(gPATTERN+BOSS_START*8, BOSS1, (BNR*BNC)<<3); break;
		case 2:	RLEUnpack(gPATTERN+BOSS_START*8, BOSS2, (BNR*BNC)<<3); break;
		case 3:	RLEUnpack(gPATTERN+BOSS_START*8, BOSS3, (BNR*BNC)<<3); break;
		case 4:	RLEUnpack(gPATTERN+BOSS_START*8, BOSS4, (BNR*BNC)<<3); break;
		case 5:	RLEUnpack(gPATTERN+BOSS_START*8, BOSS5, (BNR*BNC)<<3); break;
	}
	SWITCH_IN_PREV_BANK(old);
}

void winwrapgetscroll(unsigned char *dst, const char *src, unsigned int cnt) {
	// copies from src (dst is usually going to be tmpbuf)
	// used to get text out of the ending scrolltext
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	memcpy(dst, src, cnt);
	SWITCH_IN_PREV_BANK(old);
}

char winwrapgetbyte(const char *adr) {
	// another special one for win code - read one byte (so we can
	// get the termination character).
	unsigned int old = nBank;
	char ch;

	SWITCH_IN_BANK5;
	ch = *adr;
	SWITCH_IN_PREV_BANK(old);

	return ch;
}

void wrapstars() {
	unsigned int old = nBank;
	SWITCH_IN_BANK6;
	stars();
	SWITCH_IN_PREV_BANK(old);
}

void delaystars(unsigned char q)
{  /* delay for 'q' jiffies with star movement */
	unsigned int old = nBank;
	SWITCH_IN_BANK6;

	while (q--)	{ 
		stars();
	}

	SWITCH_IN_PREV_BANK(old);
}

void wrapinitstars() {
	unsigned int old = nBank;
	SWITCH_IN_BANK6;
	initstars();
	SWITCH_IN_PREV_BANK(old);
}

void wrapispace() {
	unsigned int old = nBank;
	// no switch needed, we're calling the fixed bank. But we need to restore the
	// right bank before we return, because ispace changes it.
	ispace();
	SWITCH_IN_PREV_BANK(old);
}
 
void wrapbackground() {
	unsigned int old = nBank;
	SWITCH_IN_BANK6;
	background();
	SWITCH_IN_PREV_BANK(old);
}

void wrapcheckdamage(uint8 sr, uint8 sc, uint8 pwr) {
	unsigned int old = nBank;
	SWITCH_IN_BANK7;
	// warning: checkdamage returns a return value, but we are ignoring it
	checkdamage(sr, sc, pwr);
	SWITCH_IN_PREV_BANK(old);
}

void wrapLoadSelenaPic() {
	unsigned int old = nBank;
	SWITCH_IN_BANK3;
	RLEUnpack(0x0000, selenaendP, 6144);
	RLEUnpack(0x2000, selenaendC, 6144);
	SWITCH_IN_PREV_BANK(old);
}

void wrapgetfontbytes(unsigned char *dest, const unsigned char *src, unsigned int cnt) {
	unsigned int old = nBank;
	// this explicitly gets bytes from the font as requested	
	SET_COLECO_FONT_BANK;
	memcpy(dest, src, cnt);
	SWITCH_IN_PREV_BANK(old);
}

void wrapspritescore(unsigned int sprpat, unsigned int sprtab, unsigned char row, unsigned char col, unsigned char ch) {
	unsigned int x=score;
	unsigned int old = nBank;
	unsigned char i;

	vdpmemset(sprpat,0x00,16*8);	// zero out first 4 sprites

	// copy the correct digits into the sprite patterns (skip every other pattern)
	// sprite patterns at 0x3800
	SET_COLECO_FONT_BANK;

	vdpmemcpy(sprpat, (char*)(colecofont+(((x/10000)+16)<<3)), 8);
	x%=10000;
	vdpmemcpy(sprpat+0x10, (char*)(colecofont+(((x/1000)+16)<<3)), 8);
	x%=1000;
	vdpmemcpy(sprpat+0x20, (char*)(colecofont+(((x/100)+16)<<3)), 8);
	x%=100;
	vdpmemcpy(sprpat+0x30, (char*)(colecofont+(((x/10)+16)<<3)), 8);
	vdpmemcpy(sprpat+0x40, (char*)(colecofont+(((x%10)+16)<<3)), 8);
	vdpmemcpy(sprpat+0x50, (char*)(colecofont+(16<<3)), 8);	// 0
	vdpmemcpy(sprpat+0x60, (char*)(colecofont+(scoremode+16<<3)), 8);	// scoremode

	// draw the sprites on the the screen
	VDP_SET_ADDRESS_WRITE(sprtab);		// sprite attribute table is different in bitmap mode
	for (i=0; i<4; i++) {
		VDPWD=(row);			// row
		VDP_SAFE_DELAY();
		VDPWD=(col+(i<<4));		// col
		VDP_SAFE_DELAY();
		VDPWD=(ch+(i<<2));		// pattern 
		VDP_SAFE_DELAY();
		VDPWD=(15);				// color
		VDP_SAFE_DELAY();
	}

	// zero the rest of the sprite table
	VDPWD=0xd0;

	SWITCH_IN_PREV_BANK(old);
}

void wrapLoadEngineSprites() {
	// load up the engine sprites
	unsigned int old = nBank;

	SWITCH_IN_BANK5;
	vdpmemcpy(gSPRITE_PATTERNS+76*8, SPRITES+76*8, 2*4*8);
	SWITCH_IN_PREV_BANK(old);
}

void wrapPlayerFlameBig() {
	// set the player flame sprites to big
    // we assume that the init function sets to blank for gnat and selena
	unsigned int old = nBank;

    if (playership == SHIP_GNAT) {
        vdpchar(100*8+0x0800, 0x01);						// 1 pixel on for high flame
    } else if (playership != SHIP_SELENA) {
	    SWITCH_IN_BANK5;
	    vdpmemcpy(gSPRITE_PATTERNS+100*8, SPRITES+100*8, 4*8);
	    SWITCH_IN_PREV_BANK(old);
    }
} 

void wrapPlayerFlameSmall() {
	// set the player flame sprites to small
	unsigned int old = nBank;

    if (playership == SHIP_GNAT) {
        vdpchar(100*8+0x0800, 0x00);						// turn the 1 pixel off for low flame
    } else if (playership != SHIP_SELENA) {
	    SWITCH_IN_BANK5;
	    vdpmemcpy(gSPRITE_PATTERNS+100*8, PLAYERFLAMESMALL, 4*8);
	    SWITCH_IN_PREV_BANK(old);
    }
} 

void wrapCopyShip(const unsigned char *p, const unsigned char *c) {
	unsigned int old = nBank;
	unsigned char i;

	SWITCH_IN_BANK10;

	for (i=0; i<17; ++i) {
		vdpmemcpy(0x2000+6*32*8+16*8+(i*32*8), c+(i*16*8), 16*8);
		vdpmemcpy(0x0000+6*32*8+16*8+(i*32*8), p+(i*16*8), 16*8);
	}

	SWITCH_IN_PREV_BANK(old);
}

void wrapGamWin() {
	SWITCH_IN_BANK8;
	gamwin();	// never returns
}

void wrapLoadStoryFont() {
	// this loads the ColecoVision ROM font like loadcharset, but instead of
	// into the pattern table, it goes into the third bitmap table for story text
	unsigned int old = nBank;

	SET_COLECO_FONT_BANK;
	// we assume pattern table is at 0
	vdpmemcpy((32*8)+4096, colecofont, 768);

	SWITCH_IN_PREV_BANK(old);
}
