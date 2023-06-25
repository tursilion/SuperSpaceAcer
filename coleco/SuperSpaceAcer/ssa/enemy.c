// Looks like we have lost:
// - enemy generators
// - beamgen enemy (except for the definition)
// - jets diving left or right as they get close
// my notes from November 2016:
/*
So how do the generators work? The game supports 1-3 at a time, depending on the difficulty and the level you're on. 
Each generator is mostly independent, more on that in a moment. Each generator further tracks the following data: 
horizontal position, countdown timer, maximum timer, movement speed, count of enemies remaining, and type of enemy 
to generate. They are always positioned at the top of the screen because, in SSA, all enemies begin life at the top 
of the screen.

In order to guarantee the RNG, the generators are checked at fixed points in the level progression -- right now every 
32 frames the next generator is checked. (Although because I do this with bitmasking, and there are only 3 generators, 
every fourth check does nothing. Also, in levels that don't have all three generators out, those idle generators end 
up doing nothing as well. This is partly for the difficulty curve and partly to control CPU time.)

If we check and determine that the generator of interest this frame is idle, and it's allowed to start (again, due to 
level and difficulty), then we set the RNG based (mostly) on the current level distance counter. Then we create the 
generator randomly, setting up its position, timer, speed, count and type. The position is where enemies will be 
dispatched (horizontally), the timer is how many frames between enemy dispatches, the speed is how far (and which 
way) the generator moves every frame, the count is how many enemies will be dispatched before the generator returns
to idle, and the type is what type of enemy that will be. These last two values are constrained by the level and
difficulty settings. The timer is also constrained by the enemy type in order to keep a reasonable minimum spacing. 
In effect, it's a rudimentary particle system generator.

There's one additional catch -- to avoid overlap of enemies from being too common a thing, we check the spacing 
against the other generators and reset the position if it's too close to one of them.

During the enemy processing frame, generators are updated. The update is very simple - the speed is added to the 
position (and wraps around natively with no need to check, since we have 256 pixels across), then the timer is 
counted down. When it reaches zero, the generator calls the enemy spawn function, and decrements its count. When 
the count reaches zero, the generator goes idle and will be awakened the next time the distance update reaches it.

Enemy spawn is straight-forward now. We know the type of enemy, and we know where it will start. We also know 
it's legal for the level and don't need to check that anymore either. But there are still two things to check.
One is whether there is a legal slot to spawn the enemy in -- both the game engine and the difficulty constrain
this. If a free slot is not available, then the opportunity to spawn is discarded. (This is why on the easier 
difficulties you will see more enemies if you shoot them than if you don't!) A second check is also performed 
for overlap - if the generator is too close to one of the other generators, it's deferred rather than discarded, 
on the assumption that the generators will move apart.

The overlap avoidance code actually created an interesting bug that only occurred in my testing midway through 
the medium difficulty level (it might still be there, I didn't finish fixing it... but I didn't see it tonight.) 
Anyway... about halfway through the stage, enemies stopped appearing, and only spawned one enemy every long time 
right at the edge until the boss came out. I was a little baffled and added debug to show me where the generators
were. And at the magic point, I watched generators 1 and 2 chase each other across the screen. They had started
wrapped around (255 and 1) so passed the creation range check, but were moving the same direction at the same 
speed, and so were always too close to spawn -- except for the one frame where they were split at the edge of 
the screen again. (Earlier, before I put the test in the generator startup code, I'd seen a similar case where
two generators started right beside each other with a speed of zero. Never moved, and couldn't spawn, so the
stage stayed empty.)

Anyway, I'm also proud of the beamgens. SSA uses all the available sprites and the game engine is limited 
in the number of enemies it allows - 6 enemies, each with their own shot. It was not intended to support a 
single enemy with two widely spread parts, so I was going to implement it as two enemies. But I was getting 
really concerned about how to find empty slots and tracking the two indexes, and ultimately I went for a 
flicker approach where the generator parts alternate each frame. This worked surprisingly well (and you'll 
see as a result that they still flicker on the F18A, even though no other sprites do ;) ). The beam itself 
is just the enemy's bullet, so that was a guaranteed spare.

Testing collision on the main sprite was easy, but I had to think a bit about the secondary one.. ultimately, 
I decided to just try actually moving the enemy back and forth, and seeing if collisions lasted long enough 
that either could be shot. Happily, the answer was yes, so all I had to do in the enemy death function was 
check against the base X position to see whether the right or left generator was the one destroyed, and then 
set up the enemy as a dud that looked like the remaining one. I did something very similar to this in Mario 
Bros, actually. The bonus stage broke my enemy engine by having too many coins to collide with. Rather than 
add a bunch of special cases, it flickers just four coins through all 10 visible ones for the sake of the 
collision engine. It's something like 1 in 3 frames on the worst case, but the slope of Mario's jump guarantees
contact for that long for all but the most contrived cases. (Of course, each coin you collect improves this as
well, once you have two coins it's 1 frame in 2, and when there are only four coins left they are testing
collision every frame).

*/


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

uint8 enr[12], enc[12];					// enemy row and col
unsigned char ent[12];					// enemy type
uint8 ech[12], eec[12], esc[12];		// enemy animation - character, end character, start character
char ers[12], ecs[12];					// enemy row speed, column speed - used for shots and sometimes flags
char ep[6];								// enemy power (hit points)
void (*en_func[12])(uint8);				// pointer to enemy handler function
unsigned char MineTipPos;

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

	/*shoot?*/
	k=rndnum()&15;
	if (k<nDifficulty) {	// 1, 3 or 7
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
	// boss cockpit and others with no task to do
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
		en_func[x+6] = enemynull;	// we'll handle it in here
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
		if (ent[x+6] != ENEMY_BEAM) {
			// we don't have both sprites available, just draw the one we have
			sprite(x+ENEMY_SPRITE, ech[x], COLOR_GRAY, enr[x], enc[x]);
		} else {
			// we have a 4 frame cycle to flicker the two sprites over 3 positions
			// the beam alternates every frame, while the generators get 2
			// solid frames in a row
			// we'll just use the row
			switch (enr[x]&3) {
				case 0:	// left and beam
					sprite(x+ENEMY_SPRITE, 76, COLOR_GRAY, enr[x], min);
					sprite(x+6+ENEMY_SPRITE, ech[x+6], COLOR_LTYELLOW, enr[x+6], enc[x+6]);
					break;
				case 3:
				case 1:	// left and right
					sprite(x+ENEMY_SPRITE, 76, COLOR_GRAY, enr[x], min);
					sprite(x+6+ENEMY_SPRITE, 80, COLOR_GRAY, enr[x], max);
					break;
				case 2:	// right and beam
					sprite(x+ENEMY_SPRITE, 80, COLOR_GRAY, enr[x], max);
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

