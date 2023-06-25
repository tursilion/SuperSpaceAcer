// enable this for a test mode - ships will fire constantly
// collisions are checked, but player never dies. You can
// start a game at high speed with the debugger open and
// just watch for VDP overruns
//#define TEST_MODE

// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "trampoline.h"
#include "enemy.h"

// used to get out of demo mode
static void (* const reboot)()=0x802d;

// color of player's ship (was white)
unsigned char playerColor;
char shotOffset;

// play shot types
#define PLAYER_SHOT_PULSE_BASE 88
#define PLAYER_SHOT_3WAY_STRAIGHT	208
#define PLAYER_SHOT_3WAY_LEFT1		212
#define PLAYER_SHOT_3WAY_LEFT2		216
#define PLAYER_SHOT_3WAY_RIGHT1		220
#define PLAYER_SHOT_3WAY_RIGHT2		224

// keypad keys for various cheats
#define CHEAT_NOSHIELD	'0'
#define CHEAT_SHIELDS	'2'
#define CHEAT_LIVES		'3'
#define CHEAT_BOSSWARP	'5'
#define CHEAT_POWERUP	'6'
#define CHEAT_KILLBOSS	'7'
#define CHEAT_OFF		'8'

// these are now just flags, though FLAME_BIG is used to init the sprite
// pattern 104 is no longer related though
#define FLAME_BIG 100
#define FLAME_SMALL 104

unsigned char joynum;
uint8 killedby,flst;		// enemy who hit us, flame status
uint8 pwrlvl;				// pwrlvl: 0-2 = pulse wave level 1,2,3, 4-6=3-way level 1,2,3  (3 is gnat)
char lives;
unsigned int score, oldscore;	// oldscore is used as a temporary during demo play
unsigned char scoremode;		// indicates bonus modes played via score's last digit. 0=normal, 1=gnat, 2=Selena, 3=invisible enemies
uint8 shr[NUM_SHOTS+1], shc[NUM_SHOTS];	// player shots row and col - shr is plus 1 so the last index can always be 0, faster searches
int8 shd[NUM_SHOTS];					// player shot x direction (y is constant)
unsigned int shield;
unsigned char hittime;				// mostly for cruiser - shakes the ship when hit
uint8 pcr4,ptp4,pr4,pc4,p4Time;		// powerup settings
unsigned char CanNotShoot;
unsigned char playerXspeed, playerYspeed;

// kind of a sine table, generated with blassic to look like joystick outputs
const signed char SINEISH[128] = {
	0,0,0,0,4,0,0,4,0,4,4,4,0,4,0,4,0,4,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	4,4,4,4,4,4,0,4,0,4,4,4,0,4,0,4,0,4,0,0,4,0,0,0,0,-0,-0,-0,-4,-0,-0,-4,-0,-4,-4,
	-4,-0,-4,-0,-4,-0,-4,-0,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,
	-4,-4,-4,-4,-4,-4,-4,-4,-0,-4,-0,-4,-4,-4,-0,-4,-0,-4,-0,-4,-0,-0,-4,-0,-0,-0
};

void player()
{ /* move the player based on joystick 'joynum' */
	// animate flame
	if (playership != SHIP_SELENA) {
		if (flst == FLAME_SMALL) flst=FLAME_BIG; else flst=FLAME_SMALL;
	} else {
		// wing animation for Selena
		flst = (++flst)&0x03;
		switch (flst) {
		case 0:
		case 1:
			playerstraight();
			break;
		case 2:
			playerleft();
			break;
		case 3:
			playerright();
			break;
		}
	}

	// read controller
	if (joynum) {
		kscanfast(joynum);
		joystfast(joynum);
	} else {
		static unsigned char xcnt=0, ycnt=0;
		kscanfast((xcnt&1)+1);
		if (KSCAN_KEY == JOY_FIRE) {
			score = oldscore;
			reboot();
		}

		KSCAN_KEY = JOY_FIRE;
		KSCAN_JOYX = SINEISH[xcnt>>1];
		KSCAN_JOYY = SINEISH[ycnt>>1];
		xcnt+=3;
		ycnt+=7;
	}
	if (KSCAN_JOYX == JOY_LEFT) {
		if (playership != SHIP_SELENA) {
			playerleft();
		}
		if (SHIP_C > playerXspeed) SHIP_C-=playerXspeed;
	} else if (KSCAN_JOYX == JOY_RIGHT) {
		if (playership != SHIP_SELENA) {
			playerright();
		}
		if (SHIP_C < 224-playerXspeed) SHIP_C+=playerXspeed;
	} else {
		if (playership != SHIP_SELENA) {
			playerstraight();
		}
	}

	if (hittime) {
		// impact rocking for cruiser
		--hittime;
		switch (hittime&0x03) {
		case 0:	playerleft(); break;
		case 1: playerstraight(); break;
		case 2: playerright(); break;
		case 3: playerstraight(); break;
		}
	}

	if (KSCAN_JOYY == JOY_UP) {
		if (SHIP_R > playerYspeed) {
			SHIP_R-=playerYspeed;
			flst=FLAME_BIG;
		}
	} else if (KSCAN_JOYY == JOY_DOWN) {
		if (SHIP_R < 151-playerYspeed) {
			SHIP_R+=playerYspeed;
			flst=FLAME_SMALL;
		}	
	}

	if ((shield) && (playership != SHIP_CRUISER)) {
		--shield;
		// ladybug counts down faster, but has an offensive shield that can ram enemies
		// can't ram the boss engines, but can ram the boss itself ;)
		if ((shield) && (playership == SHIP_LADYBUG)) {
			--shield;
			wrapcheckdamage(SHIP_R+playerOffset, SHIP_C+8, 0);	// add damage to boss body
		}
	}
	if (flst == FLAME_BIG) {
		wrapPlayerFlameBig();
	} else {
		wrapPlayerFlameSmall();
	}
	playmv();

	// helper for the shooting code
	if (CanNotShoot) {
		CanNotShoot--;
	}
	if (!CanNotShoot) {
#ifdef TEST_MODE
		shoot();
#endif
		
		if (KSCAN_KEY == JOY_FIRE) {
			shoot();
		}
	}

	mvshot();
}
 
void shoot()
{ 
	uint8 a,a2,a3;
	uint8 truepwr = pwrlvl & 0x0f;
	uint8 r,c;

	/* make player shoot if a shot available */

	// set up the ROF delay
	switch (pwrlvl & 0x03) {
		case 0x00:
			// slowest
			CanNotShoot=4;
			break;

		case 0x01:
			// medium
			CanNotShoot=3;
			break;

		case 0x02:
		case 0x03:	
			// fastest
			CanNotShoot=2;
			break;
	}

	/* take pwrlvl into account */
	if (truepwr >= PWR3WAY) { 
		// 3-way shot - we need two or three available, depending on frame and power
		a=0;
		while (shr[a]!=0) a++;

		if ((truepwr == PWR3WAY) || ((truepwr == PWR3WAY+2) && (0 == (pwrlvl&PWRFRAME)))) {
			// only 2 shots needed
			a2=a;
		} else {
			a2=a+1;
			while (shr[a2]!=0) a2++;
		}
		a3=a2+1;
		while (shr[a3]!=0) a3++;
		// only need to test the last one!
		if (a3 < NUM_SHOTS) {
			// update the frame bit (used to alternate 3way)
			pwrlvl ^= PWRFRAME;
			spposn(PLAYER_SPRITE,r,c);
			c+=8;	// recenter
			r+=shotOffset;
			if (a2 == a) {
				// only two slightly angled shots
				sprite(PLAYER_SHOT+a,PLAYER_SHOT_3WAY_LEFT1,12,r,c);
				sprite(PLAYER_SHOT+a3,PLAYER_SHOT_3WAY_RIGHT1,12,r,c);
				shd[a]=-2;
				shd[a3]=2;
				shr[a]=r;
				shr[a3]=r;
				shc[a]=c;
				shc[a3]=c;
			} else {
				// three wider spread shots
				shd[a]=-4;
				shd[a2]=0;
				shd[a3]=4;
				shr[a]=shr[a2]=shr[a3]=r;
				shc[a]=shc[a2]=shc[a3]=c;
				sprite(PLAYER_SHOT+a,PLAYER_SHOT_3WAY_LEFT2,12,r,c);
				sprite(PLAYER_SHOT+a2,PLAYER_SHOT_3WAY_STRAIGHT,12,r,c);
				sprite(PLAYER_SHOT+a3,PLAYER_SHOT_3WAY_RIGHT2,12,r,c);
			}
		}
	} else {
		// pulse cannon shot
		a=0;
		while (shr[a]!=0) a++;
		if (a!=NUM_SHOTS) { 
			spposn(PLAYER_SPRITE,r,c);
			r+=shotOffset;
			c+=8;
			shr[a]=r;
			shc[a]=c;
			shd[a]=0;
			sprite(PLAYER_SHOT+a,PLAYER_SHOT_PULSE_BASE+(truepwr<<2),13,r,c);
		}
	}
}
 
void mvshot()
{ /* move the player's shots */

#if 1
	// WARNING: assumes NUM_SHOTS to be 9
	// constant offsets are more efficient than array lookups by a lot in SDCC
#define MOVE_ONE_SHOT(a)								\
	if (shr[a]) {										\
		shr[a]-=8;										\
		shc[a]+=shd[a];									\
		if ((shr[a]>191)||(shc[a]>250)||(shc[a]<5)) {	\
			spdel(PLAYER_SHOT+a);						\
			shr[a]=0;									\
		} else {										\
			sploct(PLAYER_SHOT+a,shr[a],shc[a]);		\
		}												\
	}
	MOVE_ONE_SHOT(0);
	MOVE_ONE_SHOT(1);
	MOVE_ONE_SHOT(2);
	MOVE_ONE_SHOT(3);
	MOVE_ONE_SHOT(4);
	MOVE_ONE_SHOT(5);
	MOVE_ONE_SHOT(6);
	MOVE_ONE_SHOT(7);
	MOVE_ONE_SHOT(8);

#undef MOVE_ONE_SHOT

#else
	uint8 a;
	for (a=0; a<NUM_SHOTS; a++) {
		if (shr[a]) { 
			shr[a]-=8;
			shc[a]+=shd[a];
			if ((shr[a]>191)||(shc[a]>250)||(shc[a]<5)) { 
				spdel(PLAYER_SHOT+a);  
				shr[a]=0; 
			} else {
				sploct(PLAYER_SHOT+a,shr[a],shc[a]);
			}
		}
	}
#endif
}
 
void cheat() { 
	/* process debugging keys */
	switch (KSCAN_KEY) {
		case CHEAT_SHIELDS:
			if (playership == SHIP_CRUISER) {
				shield+=25;
			} else {
				shield=shield+100;
			}
			break;

		case CHEAT_LIVES:
			lives++;
			break;

		case CHEAT_NOSHIELD:
			shield=0;
			break;

		case CHEAT_BOSSWARP:
			distns=1;
			break;

		case CHEAT_POWERUP:
			++pwrlvl; 
			if (pwrlvl>=7) {
				pwrlvl=0; 
			}
			break;

		case CHEAT_KILLBOSS:
			ep[0]=0; 
			ep[1]=0; 
			ep[2]=0;
			break;

		case CHEAT_OFF:
			ch=0;
			break;
	}
}
 
void colchk(uint8 half) { 
	/* check sprite collisions against player shots */
	uint8 a,b,k,s;
	uint8 k1,k2;

	// lopsided halves :)
	if (half) {
		k1=0; k2=4;
	} else {
		k1=4; k2=6;
	}

	for (k=k1; k<k2; k++) {
		if ((ent[k] >= ENEMY_SAUCER) && (ent[k] < ENEMY_MAX)) {
			for (s=0; s<NUM_SHOTS; s++) {
				if (shr[s]) {
					a=abs(enr[k]-shr[s]); 
					b=abs(enc[k]-shc[s]);
					if ((a<=15)&&(b<=15)) { 
						dyen(k);
						spdel(s+PLAYER_SHOT);
						shr[s]=0;
					}
				}
			}
		}
	}
}

void plycol() {
	unsigned char x,y;
	unsigned char r,c;
	uint8 a;

	/*check player collisions */
	spposn(PLAYER_SPRITE,r,c);
	r+=playerOffset;
	c+=8;
	
	/*pwrup?*/
	if (ptp4 != POWERUP_NONE) { 
		y=abs(pr4-r); x=abs(pc4-c);
		if ((x<=16)&&(y<=16)) {
			// Player collides with powerup
			if (ptp4==POWERUP_SHIELD) { 
				// Shield power increase
				if (playership == SHIP_CRUISER) {
					if (shield < 75) shield += 25;	// maximum shields set on Cruiser
				} else {
					if (nDifficulty == DIFFICULTY_EASY) shield+=50;
					if (nDifficulty <= DIFFICULTY_MEDIUM) shield+=50;
					shield+=100;
				}
			} else if (ptp4 == POWERUP_WAVE) {
				if ((pwrlvl&0x0f) < PWR3WAY) {
					if ((pwrlvl&0x03) < 2) {
						pwrlvl++;
					}
				} else {
					pwrlvl=PWRPULSE;
					if (playership == SHIP_SELENA) pwrlvl+=2;
				}
			} else if (ptp4 == POWERUP_3WAY) {
				if ((pwrlvl&0x0f) >= PWR3WAY) {
					if ((pwrlvl&0x03) < 2) {
						pwrlvl++;
					}
				} else {
					pwrlvl=PWR3WAY;
					if (playership == SHIP_SELENA) pwrlvl+=2;
				}
			}
			// remove powerup
			ptp4=POWERUP_NONE;
			spdel(POWERUP_SPRITE);
			addscore(2);
		}
	}

	/*enemies?*/
	if (playership != SHIP_GNAT) {
		for (a=0; a<12; a++) {
			if ((ent[a]>ENEMY_EXPLOSION)&&(ent[a]<ENEMY_MAX)) { 
				y=abs(enr[a]-r); x=abs(enc[a]-c);
				if ((x<=8)&&(y<=8)&&(hittime==0)) {
					if (shield < 1) {
						killedby=a;
#ifndef TEST_MODE
						a=99;
#endif
					} else if (playership == SHIP_CRUISER) {
						shield -= 25;
						if (shield > 75) shield = 0;
						hittime = 10;
						// kill off the enemy
						if (ent[a] < ENEMY_SAUCER) {
							wrapnoen(a);
						} else {
							ep[a]=0;
							dyen(a);
						}
						// TODO: trigger sfx
					} else if (playership == SHIP_LADYBUG) {
						if (ent[a] >= ENEMY_SAUCER) {
							ep[a]=0;
							dyen(a);
							shield+=100;	// ladybug recharges shield by hitting enemies
						} else {
							wrapnoen(a);
						}
					} else {
						// still kill off whatever we ran into
						if (ent[a] >= ENEMY_SAUCER) {
							ep[a]=0;
							dyen(a);
						} else {
							wrapnoen(a);
						}
					}
				}
			}
		}
	} else {
		for (a=0; a<12; a++) {
			if ((ent[a]>ENEMY_EXPLOSION)&&(ent[a]<ENEMY_MAX)) { 
				y=abs(enr[a]-r); x=abs(enc[a]-c);
				if ((x<=3)&&(y<=2)) {
					// this is the gnat, so there's no point checking the cruiser shields ;)
					if (shield < 1) {
						killedby=a;
#ifndef TEST_MODE
						a=99;
#endif
					} else {
						if (ent[a] >= ENEMY_SAUCER) {
							ep[a]=0;
							dyen(a);
						} else {
							wrapnoen(a);
						}
					}
				}
			}
		}
	}
	if (a>=99) pdie();
}
 
void pdie()
{
	/* player dies */
	unsigned char oldflag=flag;

	shutup();
	pboom();
	spdall();
	delaystars(40);		// really a wrap function
	lives--;
	if (playership != SHIP_GNAT) {
		pwrlvl=PWRPULSE;
		if (playership == SHIP_SELENA) pwrlvl+=2;
	} else {
		pwrlvl=PWRGNAT;
	}
	if (lives<0) {
		flag=PLAYER_DIED;
	} else {
		flag=MAIN_LOOP_ACTIVE;
		if ((oldflag==MAIN_LOOP_ACTIVE) || (oldflag == MAIN_LOOP_DONE)) {
			// ispace is in the fixed bank, but it changes banks many times
			wrapispace();
		}
	}
}
 
void pboom()
{
	/* do SSA ship explode routine */
	unsigned char qw;
	int lp1, lp2;		// used to move instead of automotion

	// delete 'boss approaching' if it's up
	if (flag != BOSS_LOOP_ACTIVE) {
		// the blimp gets corrupted by this - any other time worst case is we lose a star, I don't care about that :)
		hchar(11, 0, 32, 32);
	}
	DelSprButPlayer(killedby+ENEMY_SPRITE);

	// stop audio
	shutup();

	for (qw=1; qw<4; qw++) { 
		wrapstars();							// move stars
		spcolr(PLAYER_SPRITE,playerColor); 
		spcolr(PLAYER_SPRITE+1,playerColor);	// set ship normal
		spcolr(PLAYER_SPRITE+2,playerColor); 
		spcolr(PLAYER_SPRITE+3,playerColor);
		playerstraight(); delaystars(4);		// straight
		playerleft(); delaystars(4);			// tilt left
		spcolr(PLAYER_SPRITE,9); 
		spcolr(PLAYER_SPRITE+1,9);				// set ship lt red
		spcolr(PLAYER_SPRITE+2,9); 
		spcolr(PLAYER_SPRITE+3,9);
		playerstraight(); delaystars(4);		// straight
		playerright(); delaystars(4);			// tilt right
	}

	SOUND=0xe6;									// force a noise sound for explosion
	SOUND=0xf0;

	if (playership != SHIP_GNAT) {
		patsprcpy(EXPLOSION_COPY,248);					// copy the boss explosion character in to sprite space
	} else {
		// gnat gets tiny explosion sprites too
		vdpmemset(248*8+0x0800, 0, 32);					// zero the pattern
		vdpchar(248*8+0x0800+8, 0x01);					// one pixel near the center
	}
	sppat(PLAYER_SPRITE,248); sppat(PLAYER_SPRITE+1,248);			// overwrite all four player sprites
	sppat(PLAYER_SPRITE+2,248); sppat(PLAYER_SPRITE+3,248);			// shield is off, so we ignore it

	// fake the automotion for a few frames
	for (lp1=1; lp1<16; lp1++)
	{ 
		unsigned char r,c;
		for (lp2=PLAYER_SPRITE; lp2<PLAYER_SPRITE+4; lp2++) {
			SOUND=0xf0+lp1;
			spposn(lp2, r, c);
			if (lp2&1) {
				c+=4;
			} else {
				c-=4;
			}
			if (lp2&2) {
				r+=4;
			} else {
				r-=4;
			}
			sploct(lp2, r, c);
		}
		delaystars(2);
	}
}
 
void playerstraight()
{	
	/*define ship shape 1 - straight */
	sppat(PLAYER_SPRITE,108); sppat(PLAYER_SPRITE+1,112);
	sppat(PLAYER_SPRITE+2,116); sppat(PLAYER_SPRITE+3,120);
	sppat(PLAYER_SHIELD,124); sppat(PLAYER_SHIELD+1,128);
	sppat(PLAYER_SHIELD+2,132); sppat(PLAYER_SHIELD+3,136);
}
 
void playerleft()
{ 
	/*shape 2 - left */
	sppat(PLAYER_SPRITE,140); sppat(PLAYER_SPRITE+1,144);
	sppat(PLAYER_SPRITE+2,148); sppat(PLAYER_SPRITE+3,152);
	sppat(PLAYER_SHIELD,156); sppat(PLAYER_SHIELD+1,160);
	sppat(PLAYER_SHIELD+2,164); sppat(PLAYER_SHIELD+3,168);
}
 
void playerright()
{	
	/*#3 - right */
	sppat(PLAYER_SPRITE,172); sppat(PLAYER_SPRITE+1,176);
	sppat(PLAYER_SPRITE+2,180); sppat(PLAYER_SPRITE+3,184);
	sppat(PLAYER_SHIELD,188); sppat(PLAYER_SHIELD+1,192);
	sppat(PLAYER_SHIELD+2,196); sppat(PLAYER_SHIELD+3,200);
}
 
void playerinit() {
	unsigned char a;

	sprite(PLAYER_SPRITE,108,playerColor,192,112);
	sprite(PLAYER_SPRITE+1,112,playerColor,192,128);
	sprite(PLAYER_SPRITE+2,116,playerColor,208,112);
	sprite(PLAYER_SPRITE+3,120,playerColor,208,128);
	sprite(PLAYER_FLAME,FLAME_BIG,11,224,120);
	sprite(PLAYER_SHIELD,124,COLOR_BLACK,192,112);
	sprite(PLAYER_SHIELD+1,128,COLOR_BLACK,192,128);
	sprite(PLAYER_SHIELD+2,132,COLOR_BLACK,208,112);
	sprite(PLAYER_SHIELD+3,136,COLOR_BLACK,208,128);
	oldshield = 0;		// force an update for cruiser

	for (a=1; a<10; a++)	{ 
		SHIP_R-=6;
		playmv();
		wrapstars();
	}

	for (a=0; a<NUM_SHOTS; a++) {
		shr[a]=0;
	}

	flst=0;
	CanNotShoot=0;
	ptp4=POWERUP_NONE;
	hittime = 0;
	if (playership != SHIP_CRUISER) {
		// set initial shield
		shield=20;
	}
}

void dyen(unsigned char x)
{ /* enemy has been shot */
	uint8 r,c,k;

	if (ep[x] > 0) {
		// count down armor
		// todo: sfx here
		ep[x] -= damage[pwrlvl&0x07];
	}
	if (ep[x] <= 0) {
		// no armor left
		addscore(ent[x]);
		if (ent[x] == ENEMY_BEAMGEN) {
			// figure out which one we destroyed
			// TODO: sfx here
			// based on which character it is, we know if it's left or right

			// make bullet into explosion
			// Possible bug: we assume we own the bullet by now. Only the helicopter
			// can steal our bullet, and it shouldn't last that long...
			ent[x+6] = ENEMY_EXPLOSION;
			enr[x+6]=enr[x];
			enc[x+6]=enc[x];
			ech[x+6]=52;
			eec[x+6]=72;
			esc[x+6]=52;
			en_func[x+6] = enemyexplosion;
			sprite(x+6+ENEMY_SPRITE,ech[x+6],9, enr[x+6], enc[x+6]);

			if (ech[x] == 76) {
				// left destroyed
				// make active on right
				enc[x]+=96;
				ech[x]=eec[x];
			} else {
				// right destroyed
				enc[x]-=96;
				ech[x]=esc[x];
			}
			ent[x]=ENEMY_DEADBEAM;
			en_func[x]=enemydeadbeam;
			sppat(x+ENEMY_SPRITE, ech[x]);
			sploct(x+ENEMY_SPRITE, enr[x], enc[x]);
		} else if (ent[x] != ENEMY_BOMB) {
			// TODO: sfx here
			ent[x]=ENEMY_EXPLOSION;
			ech[x]=52;
			eec[x]=72;
			esc[x]=52;
			en_func[x] = enemyexplosion;
			spcolr(x+ENEMY_SPRITE,9);
		} else {
			// TODO: special sfx here for ENEMY_BOMB
			spposn(x+ENEMY_SPRITE,r,c);
			screen(15);

			SOUND=0xe6;		// TODO: delete these three SOUNDS (two here, one at end) when SFX is in
			SOUND=0xf0;

			for (k=0; k<12; k++) {
				wrapnoen(k);
			}
			for (k=0; k<9; k++)	{ 
				ent[k]=ENEMY_SHOT;
				en_func[k]=enemyshot;
				enr[k]=r; enc[k]=c;
				ech[k]=84;
				eec[k]=84;
				esc[k]=84;
				sprite(k+ENEMY_SPRITE,84,11,r,c);
				if (k<3) {
					ers[k]=-9;
				}
				if ((k>2)&&(k<6)) {
					ers[k]=0;
				}
				if (k>5) {
					ers[k]=9;
				}
				if (k%3==0) {
					ecs[k]=-9;
				}
				if ((k-1)%3==0) {
					ecs[k]=0;
				}
				if ((k+1)%3==0) {
					ecs[k]=9;
				}
			}
			ent[4]=ENEMY_EXPLOSION;
			en_func[4] = enemyexplosion;
			ech[4]=52;
			eec[4]=72;
			esc[4]=52;
			sprite(ENEMY_SPRITE+4,52,9,r,c);

			SOUND=0xff;
		}
	}
}
 
