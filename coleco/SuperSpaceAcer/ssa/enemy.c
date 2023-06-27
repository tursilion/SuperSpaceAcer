// TODO: need to tune generators for consistent flow
// TODO: why do I get more than 3 mines at at time?
// TODO: can we make the first stage with the beamgen start with just beamgens?
// TODO: saucers can start too close to the right edge?
// TODO: Helicopters that just appear near the bottom?

// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "enemy.h"
#include "trampoline.h"

// movement pattern for saucers (3-bits)
const signed char sinemove[32] = {	0,1,2,2,3,3,4,4,
									4,4,4,3,3,2,2,1,
									0,-1,-2,-2,-3,-3,-4,-4,
									-4,-4,-4,-3,-3,-2,-2,-1 
								 };

const unsigned char mincnt[7] = {
    5,  // saucer
    2,  // jet
    8,  // mine
    5,  // helicopter
    24, // beamgen
    4,  // swirly
    12  // bomb
};

// per level (first level is 1, so 0 is a dummy
// then 8 random numbers (0-7)
const unsigned char oddstable[6][8] = {
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_SAUCER, ENEMY_JET,     ENEMY_SAUCER,     ENEMY_JET,        ENEMY_SAUCER,     ENEMY_JET },
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_SAUCER, ENEMY_JET,     ENEMY_SAUCER,     ENEMY_JET,        ENEMY_SAUCER,     ENEMY_JET },
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_MINE,   ENEMY_JET,     ENEMY_SAUCER,     ENEMY_JET,        ENEMY_SAUCER,     ENEMY_JET },
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_MINE,   ENEMY_JET,     ENEMY_SAUCER,     ENEMY_HELICOPTER, ENEMY_SAUCER,     ENEMY_HELICOPTER },
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_MINE,   ENEMY_BEAMGEN, ENEMY_HELICOPTER, ENEMY_JET,        ENEMY_SAUCER,     ENEMY_HELICOPTER },
    {   ENEMY_SAUCER, ENEMY_JET, ENEMY_MINE,   ENEMY_BEAMGEN, ENEMY_HELICOPTER, ENEMY_SWIRLY,     ENEMY_BOMB,       ENEMY_SAUCER }
};

uint8 enr[12], enc[12];					// enemy row and col
unsigned char ent[12];					// enemy type
uint8 ech[12], eec[12], esc[12];		// enemy animation - character, end character, start character
char ers[12], ecs[12];					// enemy row speed, column speed - used for shots and sometimes flags
char ep[6];								// enemy power (hit points)
void (*en_func[12])(uint8);				// pointer to enemy handler function
unsigned char MineTipPos;

// generators
unsigned char genx[3], gentime[3], genmax[3], gencount[3], gentype[3];
signed char genspeed[3];

// set up generators for the stage
extern int distns;
void initGenerators() {
    for (unsigned char b=0; b<3; ++b) {
        gencount[b]=0;
        genspeed[b]=b;  // make sure these never start identical
    }
}

// update and process the generators
void enout() {
    unsigned char maxgen = 3;
    if (nDifficulty != DIFFICULTY_HARD) maxgen=2;

    // check if we can start a generator
    if ((distns&0xf)==0) {                                                                                         
        // 16 frames - it is time
        // update the RND seed before we do any RNG
        seed = (unsigned char)(level*8+distns/32);
        for (unsigned char idx=0; idx<maxgen; ++idx) {
            if (gencount[idx] == 0) {
                // generators are always active
			    unsigned char a=(rndnum()&7);
                // this counts!
                genx[idx]=rndnum();                     // x pos
                genmax[idx]=(rndnum()&0x7)+mincnt[a];   // time between enemies
                gentime[idx]=genmax[idx];               // countdown timer
                gencount[idx]=(rndnum()&0x07);          // how many
				gentype[idx] = oddstable[level][a];     // rebase into an enemy type
                genspeed[idx]=rndnum()&0x87;            // max of 7 positive or negative, I think

                if (gentype[idx] == ENEMY_MINE) gencount[idx]=(rndnum()&0x03);
                if (gentype[idx] == ENEMY_BEAMGEN) gencount[idx]=(rndnum()&0x03);
                if (gentype[idx] == ENEMY_BOMB) gencount[idx]=1;
                if (gentype[idx] == ENEMY_JET) genspeed[idx]>>=2;

                // only set up one per time
                break;
            }
        }
    }

    // update any active generators
    for (unsigned char idx=0; idx<maxgen; ++idx) {
        if (gencount[idx]) {
            // TODO - debug info, clear old generator
            vdpchar(genx[idx]>>3, ' ');

            genx[idx]+=genspeed[idx];

            // TODO - debug info, show the generator onscreen
            vdpchar(genx[idx]>>3, '1'+idx);


            if (0 == --gentime[idx]) {
                // time to launch an enemy - maybe - make sure we aren't too close to edges or another active generator
                unsigned char k,c;
                unsigned char ok=1;

                // if we can't launch, we still count it
                gentime[idx] = genmax[idx];
                --gencount[idx];

                if ((genx[idx]<28)||(genx[idx]>227)) {
                    continue;  // too close to edge
                }
                for (unsigned char b=0; b<maxgen; ++b) {
                    if (b==idx) continue;
                    if (gencount[b]==0) continue;
                    if (abs(genx[idx]-genx[b]) < 16) {
                        // just a safety check to prevent lockstep
                        if (genspeed[idx]==genspeed[b]) ++genspeed[idx];
                        ok=0;
                        break;
                    }
                }
                if (!ok) {
                    continue;
                }

                k=255;
                for (unsigned char b=0; b<nDifficulty; ++b) {
                    if (b>5) break;
                    if (ent[b] == ENEMY_NONE) {
                        k=b;
                        break;
                    }
                }
                if (k == 255) {
                    // no free slot
                    continue;
                }

                // all right, load it up
                ent[k]=gentype[idx];
				enr[k]=1;
				enc[k]=genx[idx];
                c=COLOR_WHITE;  // why is this getting through?

                switch (gentype[idx]) {
				    // minimum player damage is 2 for spread, 3 for pulse
                    default:    //for (;;) { } // not sure why we need a default, but we seem to. So make it a saucer.
					case ENEMY_SAUCER:		eec[k]=0; esc[k]=0; ep[k]=2; c=COLOR_MEDGREEN; en_func[k]=enemysaucer; if (enc[k]<32) enc[k]+=32; if (enc[k]>216) enc[k]-=48; break;
					case ENEMY_JET:			eec[k]=4; esc[k]=4; ep[k]=3; ecs[k]=0; c=COLOR_LTBLUE; en_func[k]=enemyjet; break;
					case ENEMY_MINE:		eec[k]=8; esc[k]=8; ep[k]=10; c=COLOR_CYAN; en_func[k]=enemymine; break;
					case ENEMY_HELICOPTER:	eec[k]=24; esc[k]=12; ep[k]=5; c=COLOR_MAGENTA; ers[k]=(rndnum()&0x7f)+1; ecs[k]=(rndnum()&0x07); if (enc[k]>127) ecs[k]=-ecs[k]; en_func[k]=enemyhelicopter; break;
					case ENEMY_SWIRLY:		eec[k]=40; esc[k]=28; ep[k]=8; c=COLOR_MEDRED; en_func[k]=enemyswirly; break;
					case ENEMY_BOMB:		eec[k]=44; esc[k]=44; ep[k]=20; c=COLOR_DKYELLOW; en_func[k]=enemybomb; break;
					case ENEMY_BEAMGEN:		
						eec[k]=80; esc[k]=76; ep[k]=14; c=COLOR_GRAY; en_func[k]=enemybeamgen; 
						if (enc[k] > 144) enc[k]-=128;
						break;
				} 
				if (scoremode == 3) c=COLOR_BLACK;	// invisible enemies (TODO: if the background color changes, it may not be black)
				ech[k]=esc[k];
				sprite(k+ENEMY_SPRITE,ech[k],c,enr[k],enc[k]);
            }
        }
    }

	/*shoot?*/
    {
	    unsigned char k=rndnum()&15;
	    if (k<nDifficulty) {	// 1, 3 or 7
		    if ((ent[k+6]==ENEMY_NONE)&&((ent[k]==ENEMY_SAUCER)||(ent[k]==ENEMY_JET))) { 
			    shootplayer(k, k+6);
		    }
	    }
    }
}

#if 0
void enout()
{ /* rndly bring out a new enemy */
	uint8 k, a;
	unsigned char c=0;

	k=rndnum()&nDifficulty;
	if (k<6) { 
		if (ent[k]==ENEMY_NONE) { 
			a=(rndnum()&7); if (a>6) a-=6;
			if (a<=level) {
				a+=ENEMY_SAUCER;	// rebase into an enemy type

				ent[k]=a;
				enr[k]=1;
				enc[k]=rndnum()&0xff; if (enc[k] > 249) enc[k]-=250;

				// minimum player damage is 2 for spread, 3 for pulse
				switch (a) { 
					case ENEMY_SAUCER:		eec[k]=0; esc[k]=0; ep[k]=2; c=COLOR_MEDGREEN; en_func[k]=enemysaucer; break;
					case ENEMY_JET:			eec[k]=4; esc[k]=4; ep[k]=3; ecs[k]=0; c=COLOR_LTBLUE; en_func[k]=enemyjet; break;
					case ENEMY_MINE:		eec[k]=8; esc[k]=8; ep[k]=10; c=COLOR_CYAN; en_func[k]=enemymine; break;
					case ENEMY_HELICOPTER:	eec[k]=24; esc[k]=12; ep[k]=5; c=COLOR_MAGENTA; ers[k]=(rndnum()&0x7f)+1; ecs[k]=(rndnum()&0x07); if (enc[k]>127) ecs[k]=-ecs[k]; en_func[k]=enemyhelicopter; break;
					case ENEMY_SWIRLY:		eec[k]=40; esc[k]=28; ep[k]=8; c=COLOR_MEDRED; en_func[k]=enemyswirly; break;
					case ENEMY_BOMB:		eec[k]=44; esc[k]=44; ep[k]=20; c=COLOR_DKYELLOW; en_func[k]=enemybomb; break;
					case ENEMY_BEAMGEN:		
						eec[k]=80; esc[k]=76; ep[k]=14; c=COLOR_GRAY; en_func[k]=enemybeamgen; 
						if (enc[k] > 144) enc[k]-=128;
						break;
				} 
				if (scoremode == 3) c=COLOR_BLACK;	// invisible enemies (TODO: if the background color changes, it may not be black)
				ech[k]=esc[k];
				sprite(k+ENEMY_SPRITE,ech[k],c,enr[k],enc[k]);
			}
		}
	}
}
#endif

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
	// since there's no animation esc is used as a flag that the turn has already happened
	// if esc==ech, it has not
	enr[x]+=8;
	enc[x] += ecs[x];

	if (esc[x] == ech[x]) {
		unsigned char r,c;
		spposn(PLAYER_SPRITE,r,c);
		r+=playerOffset; c+=8;		// better center (8 because add 16 for ship, -8 for enemy center)
		if (enr[x]+64 > r) {
			// make a choice - left, right or turn
			// in higher difficulties we turn towards the player more often
			switch (nDifficulty) {
				case DIFFICULTY_EASY:
					switch (rndnum()&3) {
						case 0:
							// turn left
							ecs[x]=-4;
							ech[x]=48;
							break;
						case 1:	
							// turn right
							ecs[x]=4;
							ech[x]=252;
							break;
						// 2/3 - do nothing
					}
					break;

				case DIFFICULTY_MEDIUM:
					switch (rndnum()&3) {
						case 0:
							// towards
							if (c-8 > enc[x]) {
								// turn right
								ecs[x]=4;
								ech[x]=252;
							} else if (c+8 < enc[x]) {
								// turn left
								ecs[x]=-4;
								ech[x]=48;
							}
							break;

						case 1:
							// left
							ecs[x]=-4;
							ech[x]=48;
							break;

						case 2:
							// right
							ecs[x]=4;
							ech[x]=252;
							break;

						// 3 - nothing
					}
					break;

				case DIFFICULTY_HARD:
					switch (rndnum()&3) {
						case 0:
						case 1:
						case 2:
							// towards
							if (c-8 > enc[x]) {
								// turn right
								ecs[x]=4;
								ech[x]=252;
							} else if (c+8 < enc[x]) {
								// turn left
								ecs[x]=-4;
								ech[x]=48;
							}
							break;

						// 3 - nothing
					}
			}
			++esc[x];	// flag that it's done
			// change shape now, we'll start moving next frame
			SpriteTab[x+ENEMY_SPRITE].pat=ech[x];
		}
	}

	if (enr[x]>191) noen(x);	// no worries for top row will invisibly wrap around!
	if (enc[x]>239) noen(x);	// wider on right edge due to hotspot of sprite (need this to catch the bomb explosion bits)
	if (enc[x]<5) noen(x);
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE, enr[x],enc[x]);
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
	static unsigned char cnt=0;

	/* used to count half frames */
	cnt++;

	// flash the enemy bullets
	SpriteTab[x+ENEMY_SPRITE].col = 12-(cnt&4);

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
	(void)x;
	// boss cockpit and others with no task to do - note, not even drawn!
	//sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
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

void enemybeamgen(uint8 x) {
	unsigned char max, min;

	enr[x]++;	// always slow downward movement

	// toggle which side it's on
	// This is necessary because it allows the collision detection to work,
	// but the sprite draw is a little more complex
	if (ech[x] == esc[x]) {
		ech[x] = eec[x];
		min = enc[x];
		enc[x] += 96;		// really important that the init be correct here ;)
		max = enc[x];
	} else {
		ech[x] = esc[x];
		max = enc[x];
		enc[x] -= 96;
		min = enc[x];
	}

	// handle the beam. If the helicopter stole it, wait for it to be free
	if (ent[x+6] == ENEMY_NONE) {
		// set up the beam shot
		ent[x+6] = ENEMY_BEAM;
		ers[x+6] = 0;
		ecs[x+6] = 8;	// start moving right
		en_func[x+6] = enemynull;	// just a dummy function
		eec[x+6] = 228;
		esc[x+6] = 228;
		ech[x+6] = 228;
		enr[x+6] = enr[x];
		enc[x+6] = min+8;
	} else if (ent[x+6] == ENEMY_BEAM) {
		// handle motion
		enr[x+6] = enr[x];
		if (ecs[x+6] > 0) {
			enc[x+6] += ecs[x+6];
			if (enc[x+6] >= max-8) {
				ech[x+6] = 104;
				ecs[x+6] = -8;
			}
		} else {
			enc[x+6] += ecs[x+6];
			if (enc[x+6] <= min+8) {
				ech[x+6] = 228;
				ecs[x+6] = 8;
			}
		}
	}

	if (enr[x]>191) {
		noen(x);	// no worries for top row will invisibly wrap around!
		noen(x+6);
	}
	if (ent[x] != ENEMY_NONE) {
        unsigned char c = COLOR_GRAY;
        if (scoremode == 3) c=COLOR_BLACK;

		if (ent[x+6] != ENEMY_BEAM) {
			// we don't have both sprites available, just draw the one we have
			sprite(x+ENEMY_SPRITE, ech[x], c, enr[x], enc[x]);
		} else {
			// we have a 4 frame cycle to flicker the two sprites over 3 positions
			// the beam alternates every frame, while the generators get 2
			// solid frames in a row
			// we'll just use the row
			switch (enr[x]&3) {
				case 0:	// left and beam
					sprite(x+ENEMY_SPRITE, 76, c, enr[x], min);
					sprite(x+6+ENEMY_SPRITE, ech[x+6], COLOR_LTYELLOW, enr[x+6], enc[x+6]);
					break;
				case 3:
				case 1:	// left and right
					sprite(x+ENEMY_SPRITE, 76, c, enr[x], min);
					sprite(x+6+ENEMY_SPRITE, 80, c, enr[x], max);
					break;
				case 2:	// right and beam
					sprite(x+ENEMY_SPRITE, 80, c, enr[x], max);
					sprite(x+6+ENEMY_SPRITE, ech[x+6], COLOR_LTYELLOW, enr[x+6], enc[x+6]);
					break;
			}
		}
	}
}

void enemydeadbeam(uint8 x) {
	// whatever it was, we're set up with the right position and pattern,
	// so we just keep moving down
	enr[x]++;	// always slow downward movement
	if (enr[x]>191) {
		noen(x);	// no worries for top row will invisibly wrap around!
	}
	if (ent[x] != ENEMY_NONE) {
		sploct(x+ENEMY_SPRITE,enr[x],enc[x]); 
	}
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
			if (nDifficulty != DIFFICULTY_EASY) {
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
			} else {
				// on easy mode, they only shoot straight lines
				ers[shot]=4;
				if (abs(c-enc[en])<16) {
					ecs[shot]=0;
				} else if (c>enc[en]) {
					ecs[shot]=4;
				} else {
					ecs[shot]=-4;
				}
			}

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

    initGenerators();
}

void pwr(uint8 x)
{ 
	/* decides if pwr up to come out*/
	if ((playership != SHIP_GNAT)&&(playership != SHIP_SELENA)) {		// no powerups for the gnat or Selena
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

