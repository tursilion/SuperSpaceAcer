// TODO: all game modes - need to add bonus points on game win for remaining lives - maybe 10k? (which is really only 100) (or shields for the cruiser)
// The end of game bonus tally could maybe look like the easy win screen, but without the simulation complete bit? :)
// TODO: Selena, with no shield or lives, can just have a fixed 10000 "Best Princess Bonus"

/* program SUPER SPACE ACER design version 2.2 */
/* ported to ColecoVision by M.Brent */

// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <ColecoSNPlay.h>

// game
#include "game.h"
#include "enemy.h"
#include "trampoline.h"
#include "human.h"
#include "music.h"
#include "attract.h"

// full software reboot vector
static void (* const hwreboot)()=0x802d;

// startup and init, central code
#define SIZE_OF_CHARS		160
#define SIZE_OF_SPRITES		2048
#define LIVESCHAR			19

// for sprite flicker, manage in CPU memory, copy up during vblank
struct _sprite SpriteTab[32];

// player data for weapons, but used in multiple places
const unsigned char damage[8] = { 3,4,5,1, 2,2,3,1 };

// distns table per stage (level-1)
const int stage_distns[5] = { 1100, 1150, 1200, 1400, 1650 };

// multi-purpose
int a,b,x,y;
uint8 flag;

// cheat flag
uint8 ch;

// which ship did the player choose?
uint8 playership = 255;
// how many pixels down does the sprite start? (for smaller ones)
uint8 playerOffset;
// old shield color - used to detect changes so we don't upload the graphics every frame
uint8 oldshield = 0;
// force bonus - if you never shoot except during the boss, and never miss, you get it
uint8 force = 0;

// function pointers for shield graphics swap functions
// these functions MUST exist in the fixed bank
void (*shieldsOn)() = shieldCruiser;
void (*shieldsOff)() = deShieldCruiser;

// stage and game related
unsigned char level;
int distns;
unsigned char nDifficulty;
unsigned char seed=1;
unsigned char attractShip = 0;

extern const unsigned char CHARS[];
extern const unsigned char ELECTRICWALL[];

// used for patcpy and scroll
unsigned char tmpbuf[32];

// used for bank switch returns
unsigned int nBank = 0xffff;

void sprStart0() __naked {
	// copy sprites 0-31 in order (simplest case)
	__asm
		ld hl,#_SpriteTab+0
		ld c,#_VDPWD
		.rept 128
		OUTI
		.endm

        ret
	__endasm;
}

void sprStart4() __naked {
	// copy sprites 4-31, then 0-3
	// decrement mode
	__asm
		ld hl,#_SpriteTab+124
		ld de,#-8
		ld c,#_VDPWD
		.rept 28
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

		ld hl,#_SpriteTab+12
		.rept 4
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

        ret
	__endasm;
}

void sprStart8() __naked {
	// copy sprites 8-31, then 0-7
	__asm
		ld hl,#_SpriteTab+32
		ld c,#_VDPWD
		.rept 96
		OUTI
		.endm

		ld hl,#_SpriteTab+0
		.rept 32
		OUTI
		.endm

        ret
	__endasm;
}

void sprStart12() __naked {
	// copy sprites 12-31, then 0-11
	// decrement mode
	__asm
		ld hl,#_SpriteTab+124
		ld de,#-8
		ld c,#_VDPWD
		.rept 20
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

		ld hl,#_SpriteTab+44
		.rept 12
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

        ret        
    __endasm;
}

void sprStart16() __naked {
	// copy sprites 16-31, then 0-15
	__asm
		ld hl,#_SpriteTab+64
		ld c,#_VDPWD
		.rept 64
		OUTI
		.endm

		ld hl,#_SpriteTab+0
		.rept 64
		OUTI
		.endm

        ret
	__endasm;
}

void sprStart20() __naked {
	// copy sprites 20-31, then 0-19
	// decrement mode
	__asm
		ld hl,#_SpriteTab+124
		ld de,#-8
		ld c,#_VDPWD
		.rept 12
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

		ld hl,#_SpriteTab+76
		.rept 20
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

        ret
	__endasm;
}

void sprStart24() __naked {
	// copy sprites 24-31, then 0-23
	__asm
		ld hl,#_SpriteTab+96
		ld c,#_VDPWD
		.rept 32
		OUTI
		.endm

		ld hl,#_SpriteTab+0
		.rept 96
		OUTI
		.endm

        ret        
    __endasm;
}

void sprStart28() __naked {
	// copy sprites 28-31, then 0-27
	// decrement mode
	__asm
		ld hl,#_SpriteTab+112
		ld de,#-8
		ld c,#_VDPWD
		.rept 4
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

		ld hl,#_SpriteTab+108
		.rept 28
		OUTI
		OUTI
		OUTI
		OUTI
		add hl,de
		.endm

        ret
	__endasm;
}

// end of frame handler - handles sprite copy unless we were late
void waitforstep() {
	static unsigned char nSpriteOffset=0;

	// wait for one to occur
	// note, if we were late, this will just trigger
	// that makes the fast table copy unsafe. So, we
	// test a return value to know if it was real or late.
	if (vdpwaitvint()) {
		// missed - resync with the VDP rather than not updating sprites
		doMusic();		// harder to see it if you don't hear it ;)
		if (vdpwaitvint()) {
			return;		// it was late twice? That should not happen!
		}
	}

	// blit up the sprite table real quick - we should still be in blank
	VDP_SET_ADDRESS_WRITE(gSPRITES);

	// there are 8 possible sprite rotations, select the correct one
	// each function is a fully unrolled OUTI loop for the particular pattern
	// to get the sprite table to the VDP ASAP. Takes a bit over 1k for all that.
	// Note if nSpriteOffset is ever any other number, then we get no sprite updates
	// for that frame!! Be careful!
	switch (nSpriteOffset&0x07) {
	case 0:
		sprStart0();
		break;
	case 1:
		sprStart4();
		break;
	case 2:
		sprStart8();
		break;
	case 3:
		sprStart12();
		break;
	case 4:
		sprStart16();
		break;
	case 5:
		sprStart20();
		break;
	case 6:
		sprStart24();
		break;
	case 7:
		sprStart28();
		break;
	}

	// update sprite offset
	++nSpriteOffset;

	// play music
	doMusic();  
}

// loads a default ASCII character set from ROM
void loadcharset() {
	// this loads the ColecoVision ROM font - note we copy it into all tables
	// it was shifted and OR'd to make it look fatter - idea from Sometimes99er!
	unsigned int old = nBank;

	SET_COLECO_FONT_BANK;
	vdpmemcpy(gPATTERN+(32*8), colecofont, 768);
	vdpmemcpy(gPATTERN+(32*8)+SCROLL_OFFSET, colecofont, 768);
	vdpmemcpy(gPATTERN+(32*8)+(SCROLL_OFFSET*2), colecofont, 768);
	vdpmemcpy(gPATTERN+(32*8)+(SCROLL_OFFSET*3), colecofont, 768);

#if 0
	// makes the font, then we saved it off
	charsetlc();
	for (idx=gPattern+0x100; idx<gPattern+0x400; idx++) {
		unsigned char x = vdpreadchar(idx);
		x=(x) | (x>>1);
		vdpchar(idx, x);
	}
#endif

	// because 'y' and 'z' are overwritten by boss chars and color,
	// we move them down to '_' and '\' respectively
	patcpy('y', '[');
	patcpy('z', '\\');

	SWITCH_IN_PREV_BANK(old);
}

// copies a VDP character pattern to all four tables
// if from is 0, assumes that tmpbuf is already loaded
void patcpy(uint8 from, uint8 to) {
	if (from) {
		vdpmemread(((int)from<<3)+gPATTERN, tmpbuf, 8);
	}
	vdpmemcpy(((int)to<<3)+gPATTERN, tmpbuf, 8);
	vdpmemcpy(((int)to<<3)+gPATTERN+SCROLL_OFFSET, tmpbuf, 8);
	vdpmemcpy(((int)to<<3)+gPATTERN+(SCROLL_OFFSET*2), tmpbuf, 8);
	vdpmemcpy(((int)to<<3)+gPATTERN+(SCROLL_OFFSET*3), tmpbuf, 8);
}

// copies a VDP character pattern to sprite table
void patsprcpy(uint8 from, uint8 to) {
	vdpmemread((from<<3)+gPATTERN, tmpbuf, 8);
	vdpmemcpy((to<<3)+gSPRITE_PATTERNS, tmpbuf, 8);
}

void spdall() {
	unsigned char i;
	for (i=0; i<32; i++) {
		spdel(i);
	}
}

// warning: this seed is reset by the level code, so this game is
// intentionally not very random.
unsigned char rndnum()
{
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

	// trying the dreamcast one again, but 8-bit this time
	if (seed&1) {
		seed >>= 1;
		seed ^= 0xb8;
	} else {
		seed >>= 1;
	}

	return seed;
}

void cls()
{ /* 'clear' screen with 32 */
	vdpmemset(gIMAGE, 32, 768);
}

// leave this one in the fixed bank
unsigned char intpic() {
	unsigned char i;

	// set the mode (returns disabled and uninitialized)
	i = set_bitmap(VDP_SPR_16x16);
	// one difference
	VDP_SET_REGISTER(VDP_REG_SDT, 0x07);	gSpritePat = 0x3800;

	// disable sprite table
	vdpmemset(gSprite, 0xd0, 128);

	// fill the image table with three sets of 0-255
	vdpwriteinc(gImage, 0, 768);

	// return the magic value
	return i;
}

void RLEUnpack(unsigned int p, unsigned char *buf, unsigned int nMax) {
	int cnt;
	unsigned char z;

	VDP_SET_ADDRESS_WRITE(p);
	cnt=nMax;
	while (cnt > 0) {
		z=*buf;
		if (z&0x80) {
			// run of bytes
			buf++;
			z&=0x7f;
			raw_vdpmemset(*buf, z);
			buf++;
		} else {
			// sequence of data
			buf++;
			raw_vdpmemcpy(buf, z);
			buf+=z;
		}
		cnt-=z;
	}
}

char target(unsigned char dest, unsigned char src)
{ 
	// similar to sgn(), but designed to work with unsigned values
	// without wraparound
	if (dest > src) return 1;
	if (dest < src) return -1;
	return 0;
}

unsigned char grf1() {
	int x;

	x = set_graphics_raw(VDP_SPR_16x16);
	// we separate sprites and patterns
	VDP_SET_REGISTER(VDP_REG_PDT, 0x02);	gPattern = 0x1000;

	// clear the screen
	cls();

	// clear the sprite attribute table
	spdall();

	// return the magic word
	return x;
}

void DelSprButPlayer(unsigned char x) {
	// delete all sprites but main player sprites, and 'x'ception
	// warning: deletes shield sprites too
	unsigned char i;

	for (i=0; i<32; i++) {
		if (i<PLAYER_SPRITE+4) continue;
		if (i==x) continue;
		spdel(i);
	}
}

// these are in here instead of in player to avoid the bank switch overhead when copying
// graphics swap functions for the player ships.
void initCruiser() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(108*8+0x0800, &SPRITES[108*8], 24*4*8);	// ship sprites
	SWITCH_IN_PREV_BANK(old);

    wrapPlayerFlameSmall();
	
	shieldsOn = shieldCruiser;
	shieldsOff = deShieldCruiser;
	playerColor = COLOR_MEDRED;
	playerOffset = 8;
	shotOffset = -7;
	playerXspeed = 10;
	playerYspeed = 6;
}

void initSnowball() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(108*8+0x0800, SNOWBALL, 24*4*8);			// ship sprites
	SWITCH_IN_PREV_BANK(old);
	
    wrapPlayerFlameSmall();

    shieldsOn = shieldSnowball;
	shieldsOff = deShieldSnowball;
	playerColor = COLOR_GRAY;
	playerOffset = 8;
	shotOffset = -8;
	playerXspeed = 12;
	playerYspeed = 8;
}

void initLadybug() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(108*8+0x0800, LADYBUG, 24*4*8);			// ship sprites
	SWITCH_IN_PREV_BANK(old);
	
    wrapPlayerFlameSmall();

    shieldsOn = shieldLadybug;
	shieldsOff = deShieldLadybug;
	playerColor = COLOR_MEDRED;
	playerOffset = 16;
	shotOffset = 16;

	playerXspeed = 12;
	playerYspeed = 8;
}

void initGnat() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(108*8+0x0800, GNAT, 24*4*8);				// ship sprites
	SWITCH_IN_PREV_BANK(old);
	
	vdpmemset(100*8+0x0800, 0, 4*8);					// zero the flame sprites by default

	shieldsOn = shieldGnat;
	shieldsOff = deShieldGnat;
	playerColor = COLOR_MEDGREEN;
	playerOffset = 22;
	shotOffset = 23;

	playerXspeed = 12;
	playerYspeed = 8;
}

void initSelena() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(108*8+0x0800, SELENA, 24*4*8);			// ship sprites
    vdpmemcpy(96*8+0x0800, HOMING, 4*8);                // homing shot
	SWITCH_IN_PREV_BANK(old);
	
	vdpmemset(100*8+0x0800, 0, 4*8);					// zero the flame sprites

    shieldsOn = shieldSelena;
	shieldsOff = deShieldSelena;
	playerColor = COLOR_LTBLUE;
	playerOffset = 8;
	shotOffset = -8;

	playerXspeed = 12;
	playerYspeed = 8;
}

void shieldCruiser() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, ALTSHIELDS, 4*4*8);			// straight
	vdpmemcpy(156*8+0x0800, &ALTSHIELDS[4*4*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &ALTSHIELDS[8*4*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void shieldSnowball() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, ALTSNOWBALL, 4*4*8);			// straight
	vdpmemcpy(156*8+0x0800, &ALTSNOWBALL[4*4*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &ALTSNOWBALL[8*4*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void shieldLadybug() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, ALTLADYBUG, 4*4*8);			// straight
	vdpmemcpy(156*8+0x0800, &ALTLADYBUG[4*4*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &ALTLADYBUG[8*4*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void shieldGnat() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, ALTGNAT, 4*4*8);			// straight
	vdpmemcpy(156*8+0x0800, &ALTGNAT[4*4*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &ALTGNAT[8*4*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void shieldSelena() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, ALTSELENA, 4*4*8);			// straight
	vdpmemcpy(156*8+0x0800, &ALTSELENA[4*4*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &ALTSELENA[8*4*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void deShieldCruiser() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, &SPRITES[124*8], 4*4*8);	// straight
	vdpmemcpy(156*8+0x0800, &SPRITES[156*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &SPRITES[188*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void deShieldSnowball() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, &SNOWBALL[16*8], 4*4*8);	// straight
	vdpmemcpy(156*8+0x0800, &SNOWBALL[48*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &SNOWBALL[80*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void deShieldLadybug() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, &LADYBUG[16*8], 4*4*8);	// straight
	vdpmemcpy(156*8+0x0800, &LADYBUG[48*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &LADYBUG[80*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void deShieldGnat() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, &GNAT[16*8], 4*4*8);	// straight
	vdpmemcpy(156*8+0x0800, &GNAT[48*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &GNAT[80*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

void deShieldSelena() {
	unsigned int old = nBank;
	SWITCH_IN_BANK5;
	vdpmemcpy(124*8+0x0800, &SELENA[16*8], 4*4*8);	// straight
	vdpmemcpy(156*8+0x0800, &SELENA[48*8], 4*4*8);	// left
	vdpmemcpy(188*8+0x0800, &SELENA[80*8], 4*4*8);	// right
	SWITCH_IN_PREV_BANK(old);
}

extern void draw1();
extern char br,bc,bd;

#if 0
// 5000 cycle music test			//	hit count
unsigned int mycnt_newcmd = 0;		//	 1410
unsigned int mycnt_run = 0;			//	16949
unsigned int mycnt_fixed = 0;		//	 1962
unsigned int mycnt_inc = 0;			//	 4805
									//  25126 hits total - avg 5 calls per cycle (over 12 streams)
void getcnts(unsigned int *i1, unsigned int *i2, unsigned int *i3, unsigned int *i4);
#endif

extern void selenawin();
void main() {
	unsigned char i;

#if 0
	// test performance of boss damage code
	unsigned int bigi;
	score=0;
	VDP_SET_REGISTER(VDP_REG_MODE1,VDP_MODE1_INT|VDP_MODE1_16K);
	for (bigi=0; bigi<15000; bigi++) {
		AddDamage(0);
		i=VDPST;
	}
#endif

	// init this just once (why doesn't auto-init work? Probably because my CRT0 isn't loading it... and this still does not work.)
	playership = 255;

	// check for a score saved off above stack. If it's there, we can load it and also zero playership
	memcpy(tmpbuf, SAVEDSCORE, 4);
	score = *((unsigned short*)tmpbuf);
	if (~score == *((unsigned short*)(&tmpbuf[2]))) {
		// it matches!
		playership = 0;
		scoremode = *SAVEDMODE;
		seed = (scoremode>>4)&0x0f;
		scoremode &= 0x0f;
		attractShip = *SAVEDATTRACT;
		*SAVEDATTRACT = attractShip & 1;
		attractShip >>= 4;
	} else {
		// it was junk
		score = 0;
		scoremode = 0;
	}
	if (seed == 0) ++seed;

    initSound();

titleagain:
	SWITCH_IN_BANK9;
	handleTitlePage();

	if (joynum == 0) {
		// this is an attract timeout
		// we'll use a flag in VDP above the score to note
		// gameplay attract or story attract
		if (!(*SAVEDATTRACT)) {
			*SAVEDATTRACT = 1;
			SWITCH_IN_BANK12;
			doAttract();	// never returns
		} else {
			// we'll do the demo play
			*SAVEDATTRACT = 0;
		}
	}

	// set regular graphics mode
	i = grf1();
	
	/*load VDP data */
	loadcharset();
	
	SWITCH_IN_BANK5;
	vdpmemcpy(gPATTERN, CHARS, SIZE_OF_CHARS);
	vdpmemcpy(gPATTERN+SCROLL_OFFSET, CHARS, SIZE_OF_CHARS);
	vdpmemcpy(gPATTERN+(SCROLL_OFFSET*2), CHARS, SIZE_OF_CHARS);
	vdpmemcpy(gPATTERN+(SCROLL_OFFSET*3), CHARS, SIZE_OF_CHARS);

	spdall();	// clears sprite table
	vdpmemset(gSPRITES, 0xd0, 128);	// clears VDP copy of sprite table (fixes initial gfx glitch)
	VDP_SET_REGISTER(VDP_REG_MODE1, i);	// Switch screen on
	FIX_KSCAN(i);

	sgrint();	// sets color table

	SWITCH_IN_BANK4;
	scoremode = 0;
	if (joynum != 0) {
        playership = 0;     // always reset to cruiser on entry
		getDifficulty();	// sets scoremode
	} else {
		// demo mode
		nDifficulty = DIFFICULTY_EASY;
		playership = attractShip++;
		if (attractShip > 4) attractShip=0;
		oldscore = score;
	}
	cls();
	spdall();
	// set background to black
	screen(COLOR_BLACK);

	// load in the sprites (patterns were modified by getDifficulty)
	SWITCH_IN_BANK5;
	vdpmemcpy(gSPRITE_PATTERNS, SPRITES, SIZE_OF_SPRITES);
	// also, manually clear out sprite patterns 248-251 (powerup,etc)
	vdpmemset(gSPRITE_PATTERNS+(248*8), 0, 32);
	spdall();	// clears sprite table
	vdpmemset(gSPRITES, 0xd0, 128);	// clears VDP copy of sprite table (fixes initial gfx glitch)

	// load the correct ship
	switch (playership) {
	case SHIP_CRUISER:
		initCruiser();
		break;
	case SHIP_SNOWBALL:
		initSnowball();
		break;
	case SHIP_LADYBUG:
		initLadybug();
		break;
	case SHIP_GNAT:
		initGnat();
		break;
	case SHIP_SELENA:
		initSelena();
		break;
	}

	while (1) {		/*forever*/
		score=0;
		lives=3;
		level=1;
		if (playership != SHIP_GNAT) {
			pwrlvl=PWRPULSE; 
            oldpwrlvl = PWR3WAY;
			if (playership == SHIP_SELENA) {
				pwrlvl+=2;
				lives = 0;
			} else if (playership == SHIP_CRUISER) {
				lives = 0;
				shield = 75;
			}
		} else {
			pwrlvl=PWRGNAT;		// this was originally a bug in cheat mode, but I like it!!
		}
		if (joynum == 0) {
			lives = 0;
            level = rndnum()%5+1;
		}

		distns=stage_distns[0];
		if (joynum == 0) {
			// start at an offset to make attract mode a little more random
			distns -= rndnum();
			distns -= rndnum();
		}

		while (level<6)
		{
            force = 1;  // force is okay
			space();
			if (flag != PLAYER_DIED) {
				// means game over
				SWITCH_IN_BANK7;
                if (force) force = 2;
				boss();
			}
			// have to check again in case you died facing a boss
			if (joynum) {
				if (flag == PLAYER_DIED) {
					// 2 means game over
					gamovr();
				}
			} else {
				// back to title screen
				level = 9;
			}

			// 3 means died during boss battle
			// the distns was already rolled back, so we can do nothing and just loop
			if ((flag != PLAYER_DIED) && (flag != PLAYER_DIED_DURING_BOSS)) {
				// Boss was defeated!
				distns=stage_distns[level];		// do this BEFORE we increment, cause it's -1
				level++; 
			}
		}
		if ((level==6)&&(joynum)) {
			SWITCH_IN_BANK8;
			gamwin();
		}
		if (level==9) {
			if (joynum == 0) {
				score = oldscore;
			}
			goto titleagain;		// level 9 is a flag from gamovr()
		}
	}
}
 
void space()
{ /* do a space level */
	unsigned char idx;

	ispace();
	flag=MAIN_LOOP_ACTIVE;
	while ((flag == MAIN_LOOP_ACTIVE) || (flag == MAIN_LOOP_DONE)) { 
		// frame 0
		SWITCH_IN_BANK2;
		player();			// handles player and player shots
		colchk(0);			// first half - this means the gnat may still beat mines..
		
		SWITCH_IN_BANK6;
		stars();

		// frame 1
		SWITCH_IN_BANK1;		// covers enout and enemy
		if (flag == MAIN_LOOP_ACTIVE) {
			enout();
		}
		enemy();

		SWITCH_IN_BANK6;
		stars();

		// frame 2
		SWITCH_IN_BANK2;
		if (ch) {
			cheat();
		} else {
			plycol();		// player collision first, otherwise the mines /never/ hit the gnat if it's shooting
		}
		colchk(1);			// second half
		
		SWITCH_IN_BANK6;
		stars();
		
		if (flag == MAIN_LOOP_DONE) {
			// check if all enemies dead (ent[0-5])
			for (idx=0; idx<5; idx++) {
				if (ent[idx]!=(unsigned)ENEMY_NONE) break;
			}
			if ((idx >= 5) && (ptp4 == POWERUP_NONE)) break;
		} else {
			distns--;
			if (distns==0) {
				flag=MAIN_LOOP_DONE;
				pLoopMus = NULL;
				// fix up all mines into helicopters (typewise) so they leave the screen
				for (idx=0; idx<6; idx++) {
					if (ent[idx] == ENEMY_MINE) {
						ent[idx] = ENEMY_HELILEAVE;
						en_func[idx] = enemyhelileave;
					}
				}
				centr(11, "WARNING!! BOSS APPROACHING!");
			}
		}
	}
}
 
void ispace()
{
	/* init space level (stars, music, etc) */
	cls();
	spdall();
	screen(1);
	shutup();

	// load the electric wall sprites
	SWITCH_IN_BANK5;
	vdpmemcpy(gSPRITE_PATTERNS+76*8, ELECTRICWALL, 2*4*8);

	// small stars first
	SWITCH_IN_BANK6;
	initstars();

	// show score
	addscore(0);
	if ((playership != SHIP_SELENA) && (playership != SHIP_CRUISER)) {
		hchar(23, 2, LIVESCHAR, lives);
	}

	// set up main music
	switch(level) {
	case 1:	StartMusic(STAGE1MUS, 1); break;
	case 2:	StartMusic(STAGE2MUS, 1); break;
	case 3:	StartMusic(STAGE3MUS, 1); break;
	case 4:	StartMusic(STAGE4MUS, 1); break;
	case 5:	StartMusic(STAGE5MUS, 1); break;
	}

	SWITCH_IN_BANK2;
	playerinit();
	
	SWITCH_IN_BANK1;
	enemyinit();

	distns=distns+100*level;
}
 
void gamovr()
{ /* do the GAME OVER thing */
	int nCnt;
	unsigned char btnok=0;

	centr(11,"GAME OVER");
	StartMusic(GAMEOVERMUS, 0);
	
	nCnt=300;
	level=7;			// flag for exit to restart
	spdall();

	for (;;) {
    //	if (pDone) {
        if (!(isSNPlaying)) {
			shutup();
		} else {
			nCnt=240;
		}
	
		SWITCH_IN_BANK6;
		stars();
		 
		nCnt--;
		if (nCnt == 0) {
			level=9;	// change flag to go back to title pic
			break;
		}

		kscanfast(joynum);
		if (KSCAN_KEY == JOY_FIRE) {
			if (btnok) {
				break;
			}
		} else {
			btnok = 1;
		}
	}
}

// used by boss and player in multiple places to position and center player sprites
void playmv()
{ 
	/* locate the ship at SHIP_R,SHIPC (PLAYER_SPRITE already moved) */
	uint8 x;

	//sploct(PLAYER_SPRITE,r,c);
	sploct(PLAYER_SHIELD,SHIP_R,SHIP_C);
	sploct(PLAYER_SPRITE+1,SHIP_R,SHIP_C+16);
	sploct(PLAYER_SHIELD+1,SHIP_R,SHIP_C+16);
	sploct(PLAYER_SPRITE+2,SHIP_R+16,SHIP_C);
	sploct(PLAYER_SHIELD+2,SHIP_R+16,SHIP_C);
	sploct(PLAYER_SPRITE+3,SHIP_R+16,SHIP_C+16);
	sploct(PLAYER_SHIELD+3,SHIP_R+16,SHIP_C+16);
	sploct(PLAYER_FLAME,SHIP_R+32,SHIP_C+8);
	
	// set shield color
	if (shield != oldshield) {
		if (oldshield == 0) {
			// new shield must not be zero, so turn on the shield patterns
			shieldsOn();
		}

		if (shield > 70) {
			x=COLOR_WHITE;
		} else if (shield > 40) {   // also find the code that plays sfx_shieldwarn
			x=COLOR_DKYELLOW;
		} else if (shield > 0) {
			x=COLOR_DKRED;
		} else {
			x=COLOR_BLACK;

			// done with shield, back to normal
			shieldsOff();
		}

		spcolr(PLAYER_SHIELD,x); 
		spcolr(PLAYER_SHIELD+1,x); 
		spcolr(PLAYER_SHIELD+2,x); 
		spcolr(PLAYER_SHIELD+3,x);

		oldshield = shield;
	}
}

// adds to score and displays the result
void addscore(unsigned char val) {
	unsigned int x;
	unsigned char c;

	score+=val;
	x=score;

	VDP_SET_ADDRESS_WRITE((unsigned int)(gIMAGE+13));

#if 0
	// This version does 1000 passes with values 0-255 in 179 ticks - 2983uS average
	// write the correct digits
	VDPWD=((x/10000)+'0');
	x%=10000;
	VDPWD=((x/1000)+'0');
	x%=1000;
	VDPWD=((x/100)+'0');
	x%=100;
	VDPWD=((x/10)+'0');
	// should be enough calculation delay here
	VDPWD=((x%10)+'0');
#else
	// this version does 1000 passes with 0-255 in 29 ticks - 483uS average (6x faster!)
	c='0';
	while (x>=10000) {
		++c;
		x-=10000;
	}
	VDPWD=c;

	c='0';
	while (x>=1000) {
		++c;
		x-=1000;
	}
	VDPWD=c;

	c='0';
	while (x>=100) {
		++c;
		x-=100;
	}
	VDPWD=c;

	c='0';
	while (x>=10) {
		++c;
		x-=10;
	}
	VDPWD=c;
	VDP_SAFE_DELAY();
	VDPWD=x+'0';
#endif

	VDP_SAFE_DELAY();
	VDPWD=('0');
//	VDP_SAFE_DELAY();
	VDPWD=('0'+scoremode);
}

// displays centered, NUL terminated text
void centr(unsigned char row, const char *out) {
	/* print&center the text in out[]*/
	unsigned char c;

	c=15-(strlen(out)>>1);
	VDP_SET_ADDRESS_WRITE(gIMAGE+(row<<5)+c);

	while (*out) {
		VDPWD=(*(out++));
	}
}

void sgrint()
{ /*init space colors */
	unsigned char a;

	for (a=0; a<32; a++) {
		color(a,15,0);
		color2(a,15,0);
	}

	// red color set for explosion, everything else is white (not in set 2)
	color(1,9,0);
}

// reboot wrapper that saves off some handy variables and restarts the game
void reboot() {
	unsigned char x;

	// to preserve the score while keeping the lazy reboot, we save it off above the stack
	// we save it twice, once inverted as a checksum, so the boot will always know it's right
	*((unsigned short*)tmpbuf) = score;
	*((unsigned short*)(&tmpbuf[2])) = ~score;
	memcpy(SAVEDSCORE, tmpbuf, 4);

	// also save off the scoremode (only 0-9, so we can store a count here too to init the seed with)
	*SAVEDMODE = ((*SAVEDMODE+0x10)&0xf0) | scoremode;
	// save off the last attract ship too
	x = *SAVEDATTRACT;
	x = (attractShip<<4)|(x&1);
	*SAVEDATTRACT = x;

	hwreboot();	// never returns
}
