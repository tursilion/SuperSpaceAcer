#ifndef GAME_H
#define GAME_H

// VRAM map:
// >0000	Screen Image Table
// >0300	Sprite Descriptor Table
// >0380	Color Table
// >03A0	(Unused)
// >03C0	Color Table 2 (all white on transparent)
// >03E0	(Unused)
// >0800	Sprite Pattern Table
// >1000	Pattern table (scroll 0)
// >1800	Pattern table (scroll 2)
// >2000	Pattern table (scroll 4)
// >2800	Pattern table (scroll 6)
// >3000	Screen Image Table 2
// >3300	(unused)
// >3D00	wraparound memory overwritten by boss draw code

#define uint8 unsigned char
#define int8 signed char
#define NULL (0)
#define abs(x) ((x)<0 ? -(x) : (x))

// these functions currently live in crt0_bios.s
void *memset (void *buf, unsigned int ch, unsigned int count);
void *memcpy (void *dst, const void *src, unsigned int count);
void *memmove (void *dst, const void *src, unsigned int count);
// these don't
int strlen(const char *s);
void strcpy(char *p, const char *s);

// bank switching - nOldBank is used to let a function restore the original bank
extern unsigned int nBank;
#define SWITCH_IN_PREV_BANK(nOldBank) (*(volatile unsigned char*)0)=(*(volatile unsigned char*)nOldBank); nBank=nOldBank;
#define SWITCH_IN_BANK1	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xfffe); nBank=(unsigned int)0xfffe; 
#define SWITCH_IN_BANK2	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFFD); nBank=(unsigned int)0xfffd;
#define SWITCH_IN_BANK3	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFFC); nBank=(unsigned int)0xfffc;
#define SWITCH_IN_BANK4	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFFB); nBank=(unsigned int)0xfffb;							
#define SWITCH_IN_BANK5	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFFA); nBank=(unsigned int)0xfffa;
#define SWITCH_IN_BANK6	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF9); nBank=(unsigned int)0xfff9;
#define SWITCH_IN_BANK7	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF8); nBank=(unsigned int)0xfff8;
#define SWITCH_IN_BANK8	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF7); nBank=(unsigned int)0xfff7;
#define SWITCH_IN_BANK9	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF6); nBank=(unsigned int)0xfff6;
#define SWITCH_IN_BANK10 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF5); nBank=(unsigned int)0xfff5;
#define SWITCH_IN_BANK11 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF4); nBank=(unsigned int)0xfff4;
#define SWITCH_IN_BANK12 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF3); nBank=(unsigned int)0xfff3;
#define SWITCH_IN_BANK13 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF2); nBank=(unsigned int)0xfff2;
#define SWITCH_IN_BANK14 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF1); nBank=(unsigned int)0xfff1;
#define SWITCH_IN_BANK15 (*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xFFF0); nBank=(unsigned int)0xfff0;

#define SET_COLECO_FONT_BANK SWITCH_IN_BANK7 

// NOTE: we have to do an assignment for the bank switch. SDCC does not recognize the potential
// of side effects on a read and so ignores the volatile flag, and optimizes the read away IF
// it decides there is no potential of other action. (For example, I had a function that set
// bank 5, read a byte from memory, then changed back to the old bank. The setting of bank 5
// was optimized away until a write occurred. This didn't happen in most cases because if
// a function call occurred, SDCC decided it couldn't be sure about side effects and did not
// optimize the read away.

// the lib also defines gXXX variables, but these will be faster here due to constant expression elimination
#define gIMAGE 0x0000
#define gIMAGE2 0x3000
#define gSPRITES 0x0300
#define gCOLOR 0x0380
#define gCOLOR2 0x03C0
#define gSPRITE_PATTERNS 0x0800
#define gPATTERN 0x1000

// dynamic sprite tables
extern const unsigned char SPRITES[],ALTSHIELDS[],PLAYERFLAMESMALL[];
extern const unsigned char SNOWBALL[],ALTSNOWBALL[];
extern const unsigned char LADYBUG[],ALTLADYBUG[];
extern const unsigned char GNAT[],ALTGNAT[];
extern const unsigned char SELENA[],ALTSELENA[],HOMING[];

// Address of Coleco ROM font
extern const unsigned char colecofont[];

// boss smooth scroll pattern table offset
#define SCROLL_OFFSET 0x0800

// player ships
#define SHIP_CRUISER 0
#define SHIP_SNOWBALL 1
#define SHIP_LADYBUG 2
#define SHIP_GNAT 3
#define SHIP_SELENA 4

// explosion chars 
// - char is the one that the patterns are copied into
// - copy is the one used for the SSA ship explosion
// The rest are the animation frames (must be 4)
#define EXPLOSION_FIRST 8
#define EXPLOSION_CHAR 12
#define EXPLOSION_COPY 9

// power up settings
#define PWRPULSE 0
#define PWRGNAT 3
#define PWR3WAY 4
#define PWRFRAME 0x80

// only exposing the none type for clearing
#define POWERUP_NONE	255

// player is 4 sprites
#define PLAYER_SPRITE 0

// player shots
#define NUM_SHOTS 9

// player has NUM_SHOTS shots
#define PLAYER_SHOT 22

// flame is 1 sprite
#define PLAYER_FLAME 17

// powerup is 1 sprite
#define POWERUP_SPRITE 4

// powerup colors
#define POWERUP_FIRST_COLOR 2
#define POWERUP_LAST_COLOR 15
// powerup change time in frames
#define POWERUP_TIME 30

// powerups (background characters) (shield must be first, must be contiguous)
#define POWERUP_SHIELD	16
#define POWERUP_WAVE	17
#define POWERUP_3WAY	18

// shield is 4 sprites
#define PLAYER_SHIELD 18

// boss data
#define BOSS_START 121
extern const unsigned char BOSS1[],BOSS2[],BOSS3[],BOSS4[],BOSS5[];

// player ships (damnit, we'll need different endings now...)
// function pointers are used for the init, set shield and set normal copies
// also define color 

// game flags
enum {
	MAIN_LOOP_ACTIVE,			// 0
	MAIN_LOOP_DONE,				// 1
	PLAYER_DIED,				// 2
	PLAYER_DIED_DURING_BOSS,	// 3
	BOSS_LOOP_ACTIVE,			// 4
};

// these are bitmasks for enout() to use
enum {
	DIFFICULTY_EASY = 1,
	DIFFICULTY_MEDIUM = 3,
	DIFFICULTY_HARD = 7
};

struct _sprite {
	unsigned char y, x, pat, col;
};
extern struct _sprite SpriteTab[32];

// some pointers above the stack to store data across reboots
#define SAVEDSCORE ((unsigned char*)0x73fa)
#define SAVEDMODE ((unsigned char*)0x73fe)
#define SAVEDATTRACT ((unsigned char*)0x73ff)

// functions
//void memset(char *p, unsigned char ch, int cnt);
void musicsync();
void spdall() ;
void loadcharset();
void chrdef(unsigned char n, char *sz) ;
void patcpy(uint8 from, uint8 to) ;
void patsprcpy(uint8 from, uint8 to) ;
unsigned char rndnum();
unsigned char intpic() ;
void RLEUnpack(unsigned int p, unsigned char *buf, unsigned int nMax);
void RLEUnpackInt(unsigned char *bufp, unsigned char *bufc, unsigned int nMax);
void ldpic() ;
unsigned char grf1() ;
void main() ;
void space();
void cls();
void ispace();
void sgrint();
void playmv();
void stars();
char target(unsigned char dest, unsigned char src);
void pwr(uint8 x);
void gamovr();
void gamwin();
void scrolltext();
void gamewineasy();
void gamewinmedium();
void gamewinhard();
void boss();
void drboss();
void erboss();
void mboss();
uint8 checkdamage(uint8 sr, uint8 sc, uint8 pwr);
void whoded();
void byboss();
void DoWinMusic() ;
void mainwin();
void read();
void centr(unsigned char row, const char *out);
void scroll();
void pause();
void nmi(void);
void AddDamage(unsigned int vptr);
void AddDestroyed(unsigned int vptr);
void PrepareBoss(unsigned char idx, unsigned char r);
void addscore(unsigned char val);
void getDifficulty();
void initstars();
void waitforstep();
void delaystars(unsigned char q);
void DelSprButPlayer(unsigned char x);
void background() ;
void initCruiser();
void initSnowball();
void initLadybug();
void initGnat();
void initSelena();
void shieldCruiser();
void shieldSnowball();
void shieldLadybug();
void shieldGnat();
void shieldSelena();
void deShieldCruiser();
void deShieldSnowball();
void deShieldLadybug();
void deShieldGnat();
void deShieldSelena();
void handleTitlePage();
void reboot();

// macros to look like the old c99
#define screen(x) VDP_SET_REGISTER(VDP_REG_COL, x)
// write a row of characters to the screen
#define hchar(r, c, ch, cnt) vdpmemset(gIMAGE+((r)<<5)+(c), ch, cnt)
// single character hchar
#define xchar(r, c, ch) vdpchar(gIMAGE+((r)<<5)+(c), ch)
// read a character from the screen
#define gchar(r, c) vdpreadchar(gIMAGE+((r)<<5)+(c))
// set up a new sprite
#define sprite(nn, chr, color, yy, xx) SpriteTab[nn].y=yy; SpriteTab[nn].x=xx; SpriteTab[nn].pat=chr; SpriteTab[nn].col=color
// get sprite position (note: not pointers anymore)
#define spposn(nn, rr, cc) rr=SpriteTab[nn].y; cc=SpriteTab[nn].x
// set sprite location
#define sploct(nn, rr, cc) SpriteTab[nn].y=rr; SpriteTab[nn].x=cc
// set sprite color (uses assembly color)
#define spcolr(n,c) SpriteTab[n].col = c
// set sprite pattern
#define sppat(n,chr) SpriteTab[n].pat=chr
// delete a sprite
#define spdel(n) SpriteTab[n].y=0xd1
// set main color
#define color(nSet, nFore, nBack) vdpchar(gCOLOR+nSet, (nFore<<4)|(nBack))
#define color2(nSet, nFore, nBack) vdpchar(gCOLOR2+nSet, (nFore<<4)|(nBack))

#define SHIP_R SpriteTab[PLAYER_SPRITE].y
#define SHIP_C SpriteTab[PLAYER_SPRITE].x

// shared variables
extern unsigned int score;
extern unsigned int oldscore;
extern unsigned char scoremode;
extern unsigned char joynum;
extern char lives;
extern unsigned char level;
extern unsigned char ent[12];
extern void (*en_func[12])(uint8);
extern const unsigned char *pLoopMus;
extern unsigned int  loopBank;
extern unsigned int  loopIdx;
extern uint8 ch;
extern uint8 enr[12], enc[12];
extern uint8 ech[12], eec[12], esc[12];
extern char ers[12], ecs[12];
extern char ep[6];		// was engine power, now general hitpoints, yes, want signed char
extern uint8 shr[NUM_SHOTS+1], shc[NUM_SHOTS];
extern uint8 pcr4,ptp4,pr4,pc4,p4Time;
extern uint8 flag;
extern unsigned char nDifficulty;
extern unsigned char tmpbuf[32];
extern const unsigned char damage[8];
extern int distns;
extern unsigned int shield;
extern unsigned char BNR,BNC;
extern unsigned int  musBank;
extern unsigned char playerColor;
extern unsigned char playerOffset;
extern char shotOffset;
extern uint8 playership;
extern unsigned char playerXspeed, playerYspeed;
extern uint8 oldshield;
extern unsigned char seed;
extern unsigned char force;


#endif
