// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "trampoline.h"

// movement pattern for saucers (3-bits)
const signed char sinemove[32] = {	0,1,2,2,3,3,4,4,
									4,4,4,3,3,2,2,1,
									0,-1,-2,-2,-3,-3,-4,-4,
									-4,-4,-4,-3,-3,-2,-2,-1 
								 };

uint8 enr[12], enc[12];					// enemy row and col
unsigned char ent[12];					// enemy type
uint8 ech[12], eec[12], esc[12];		// enemy animation - character, end character, start character
char ers[12], ecs[12];					// enemy row speed, column speed - used for shots and sometimes flags
char ep[6];								// enemy power (hit points)
void (*en_func[12])(uint8);				// pointer to enemy handler function
unsigned char MineTipPos;

void enemysaucer(uint8 x);
void enemyjet(uint8 x);
void enemymine(uint8 x);
void enemyhelicopter(uint8 x);
void enemyhelipause(uint8 x);
void enemyhelileave(uint8 x);
void enemyswirly(uint8 x);
void enemybomb(uint8 x);
void enemyshot(uint8 x);
void enemyengine(uint8 x);
void enemyexplosion(uint8 x);

void enout()
{ /* rndly bring out a new enemy */
	uint8 k, a;
	unsigned char c=0;

	k=rndnum()&nDifficulty;
	if (k<6) { 
		if (ent[k]==ENEMY_NONE) { 
			a=(rndnum()&7); if (a>5) a-=5;
			if (a<=level) {
				a+=ENEMY_SAUCER;	// rebase into an enemy type

				ent[k]=a;
				enr[k]=1;
				enc[k]=rndnum()&0xff; if (enc[k] > 249) enc[k]-=250;

				// minimum player damage is 2 for spread, 3 for pulse
				switch (a) { 
					case ENEMY_SAUCER:		eec[k]=0; esc[k]=0; ep[k]=2; c=COLOR_MEDGREEN; en_func[k]=enemysaucer; break;
					case ENEMY_JET:			eec[k]=4; esc[k]=4; ep[k]=3; c=COLOR_LTBLUE; en_func[k]=enemyjet; break;
					case ENEMY_MINE:		eec[k]=8; esc[k]=8; ep[k]=10; c=COLOR_CYAN; en_func[k]=enemymine; break;
					case ENEMY_HELICOPTER:	eec[k]=24; esc[k]=12; ep[k]=5; c=COLOR_MAGENTA; ers[k]=(rndnum()&0x7f)+1; ecs[k]=(rndnum()&0x07); if (enc[k]>127) ecs[k]=-ecs[k]; en_func[k]=enemyhelicopter; break;
					case ENEMY_SWIRLY:		eec[k]=40; esc[k]=28; ep[k]=8; c=COLOR_MEDRED; en_func[k]=enemyswirly; break;
					case ENEMY_BOMB:		eec[k]=44; esc[k]=44; ep[k]=20; c=COLOR_DKYELLOW; en_func[k]=enemybomb; break;
				} 
				if (scoremode == 3) c=COLOR_BLACK;	// invisible enemies (TODO: if the background color changes, it may not be black)
				ech[k]=esc[k];
				sprite(k+ENEMY_SPRITE,ech[k],c,enr[k],enc[k]);
			}
		}
	}

	/*shoot?*/
	k=rndnum()&7;
	if (k<5) { 
		if ((ent[k+6]==ENEMY_NONE)&&((ent[k]==ENEMY_SAUCER)||(ent[k]==ENEMY_JET))) { 
			shootplayer(k, k+6);
		}
	}
}

void enemysaucer(uint8 x) {
	enr[x]+=4;
	enc[x]+=sinemove[(enr[x]>>2)&0x1f];

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyjet(uint8 x) {
	enr[x]+=8;

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		SpriteTab[x+ENEMY_SPRITE].y = enr[x];
	}
}

void enemymine(uint8 x) {
	unsigned char r,c;

	/*move enemies */
	spposn(PLAYER_SPRITE,r,c);
	r+=playerOffset; c+=8;		// better center (8 because add 16 for ship, -8 for enemy center)

	enr[x]+=target(r, enr[x])<<1;
	enc[x]+=target(c, enc[x])<<1;

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyhelicopter(uint8 x) {
	if (ers[x] <= 4) {
		enr[x]+=ers[x]; 
		ers[x] = 0;
	} else {
		enr[x]+=4;
		ers[x] -= 4;
	}
	enc[x]+=ecs[x];
	if (0 == ers[x]) {
		ent[x] = ENEMY_HELIPAUSE1;
		en_func[x] = enemyhelipause;
	}

	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
	}
	sppat(x+ENEMY_SPRITE,ech[x]);

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyhelipause(uint8 x) {
	// handle helicopter frames
	++ent[x];
	if (ent[x] == ENEMY_HELISHOOT1) {
		helishoot(x);
		if (nDifficulty == DIFFICULTY_EASY) ent[x]=ENEMY_HELIPAUSE4;
	} else if (ent[x] == ENEMY_HELISHOOT2) {
		helishoot(x);
		if (nDifficulty == DIFFICULTY_MEDIUM) ent[x]=ENEMY_HELIPAUSE4;
	} else if (ent[x] == ENEMY_HELISHOOT3) {
		helishoot(x);
	} else if (ent[x] == ENEMY_HELILEAVE) {
		// flip around and give us a new angle
		eec[x]=244; esc[x]=232; 
		ecs[x]=(rndnum()&0x07);
		ech[x]=esc[x];
		en_func[x] = enemyhelileave;
	}

	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
	}
	sppat(x+ENEMY_SPRITE,ech[x]);

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyhelileave(uint8 x) {
	enr[x]-=4;
	enc[x]+=ecs[x];

	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
	}
	sppat(x+ENEMY_SPRITE,ech[x]);

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyswirly(uint8 x) {
	unsigned char c;

	/*move enemies */
	c = SpriteTab[PLAYER_SPRITE].x;
	c+=8;		// better center (8 because add 16 for ship, -8 for enemy center)

	enr[x]+=5;
	enc[x]+=target(c, enc[x])<<2;

	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
	}
	sppat(x+ENEMY_SPRITE,ech[x]);

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemybomb(uint8 x) {
	enr[x]+=3;
	enc[x]++;

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyshot(uint8 x) {
	// also shrapnel
	enr[x]+=ers[x];
	enc[x]+=ecs[x];

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
}

void enemyengine(uint8 x) {
	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
	}
	sppat(x+ENEMY_SPRITE,ech[x]);
}
 
void enemynull(uint8 x) {
	// boss cockpit and others with no task to do
	sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
}

void enemyexplosion(uint8 x) {
	ech[x]+=4;
	if (ech[x]>eec[x]) {
		ech[x]=esc[x];
		pwr(x);		// will delete this enemy after deciding if a powerup comes out
		return;
	}
	sppat(x+ENEMY_SPRITE,ech[x]);
}

void enemy()
{ 
	uint8 x;

	// move enemies according to their handlers
	for (x=0; x<12; x++) {
		if (ent[x] != ENEMY_NONE) {
			en_func[x](x);
		}
	}

	// move powerup
	if (ptp4!=POWERUP_NONE) { 
		if (pr4>192) {
			ptp4=POWERUP_NONE; 
			spdel(POWERUP_SPRITE);			// remove if offscreen
		} else {
			pr4++;
			SpriteTab[POWERUP_SPRITE].y = pr4;	// move down
			if (p4Time > 10) {
				// stop flickering right before it changes
				pcr4++;
				if (pcr4>POWERUP_LAST_COLOR) pcr4=POWERUP_FIRST_COLOR;
			}
			spcolr(POWERUP_SPRITE,pcr4);		// rotate colors
			p4Time--;
			if (p4Time == 0) {
				p4Time = POWERUP_TIME;
				ptp4++;
				if (ptp4 > POWERUP_SHIELD+2) ptp4=POWERUP_SHIELD;
				patsprcpy(ptp4,248);
			}
		}
	}

	// move mine tips
	MineTipPos++;
	if (MineTipPos >= 6) {
		MineTipPos=0;
	}
	if (ent[MineTipPos] == ENEMY_MINE) {
		sprite(31, 204, COLOR_LTYELLOW, enr[MineTipPos], enc[MineTipPos]);
	} else {
		spdel(31);
	}
}
 
void noen(uint8 x) { 
	/* remove enemy x from service */
	spdel(x+ENEMY_SPRITE);
	ent[x]=ENEMY_NONE;
}
 
void shootplayer(unsigned char en, unsigned char shot) {
	int z,y;
	unsigned char r,c;

	spposn(PLAYER_SPRITE,r,c);
	r=r+12; c=c+16;
	// disallow shooting backwards at you
	if (enr[en] < r) {
		// this code allows better aim
		z=abs(r-enr[en])+abs(c-enc[en]);
		if (z > 8) {
			// don't shoot if too close to the player
			if (z > 2) {
				// this gets us a divisor - smaller numbers make slower speed overall
				z>>=1;
			} else {
				z=1;
			}
			y=((r-enr[en])<<2)/z;
			ers[shot]=(char)y;
			y=((c-enc[en])<<2)/z;
			ecs[shot]=(char)y;

			// now fill in the rest of the stuff
			ent[shot]=ENEMY_SHOT;
			en_func[shot]=enemyshot;
			eec[shot]=84;
			esc[shot]=84;
			ech[shot]=84;
			enr[shot]=enr[en];
			enc[shot]=enc[en];
			sprite(shot+ENEMY_SPRITE,84,12,enr[en],enc[en]);
		}
	}
}

void helishoot(unsigned char en) {
	// helicopters steal shots from other enemies
	// so this is just a little search
	unsigned char i;

	// enemy shots from 6-10
	for (i=11; i>5; i--) {
		if (ent[i] == ENEMY_NONE) {
			shootplayer(en, i);
			return;
		}
	}

	// couldn't find a free slot to shoot, skip it
}

void enemyinit() {
	unsigned char a;

	for (a=0; a<12; a++) {
		ent[a]=ENEMY_NONE;
	}
	MineTipPos=0;
}

void pwr(uint8 x)
{ 
	/* decides if pwr up to come out*/
	if (playership != SHIP_GNAT) {		// no powerups for the gnat
		if ((flag != MAIN_LOOP_DONE)&&((rndnum()&0x0f)<3)&&(ptp4==POWERUP_NONE)) { 
			ptp4=(rndnum()&3)+POWERUP_SHIELD;		// gives one of 3
			if (ptp4 == POWERUP_SHIELD+3) ptp4=POWERUP_SHIELD;
			patsprcpy(ptp4,248);		
			sprite(POWERUP_SPRITE,248,2,enr[x],enc[x]);
			pcr4=POWERUP_FIRST_COLOR; pr4=enr[x]; pc4=enc[x];
			p4Time=POWERUP_TIME;
		}
	}
	noen(x);
}

