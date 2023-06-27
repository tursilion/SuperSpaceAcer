// libti99 
#include <vdp.h>
#include <sound.h>
#include <kscan.h>

// game
#include "game.h"
#include "trampoline.h"
#include "enemy.h"
#include "music.h"

// enemy array usage:
// 0-2	engines
// 3-5	mines
// 6	cockpit (invisible collision sprite)
// 7-11	shots

// TODO: Sometimes runs too slowly with music active, need to free up some cycles! (getting there!)
// Jitters seen in BlueMSX seem mostly okay on hardware

//*BOSSES
// Number Rows, Number Columns
// 3 sets of Engine Row, Engine Column
// Color
const char BOSTAB[] = {
	8,11,	-8,17,	-8,49,	0,0,	2,
	11,11,	-5,14,	-7,34,	-5,52,	6,
	7,11,	-8,2,	-8,34,	-8,64,	5,
	12,11,	-8,-1,	-2,32,	-8,66,	10,
	9,15,	5,15,	6,50,	5,84,	13
};

char br,bd;			// these ones need to be signed
unsigned char bc;	// but this one doesn't
unsigned char BNR,BNC;
char bossminepower;
void (*bossdraw)();
unsigned char bosscnt=0;
unsigned char enginer[3], enginec[3];   // offsets

// boss draw functions
void draw1();
void draw2();
void draw3();
void draw4();
void draw5();

// this is only called once, the boss flash is handled by changing the color table
void bosscol(char col) {
	unsigned char i;

	// set boss color
	for (i=BOSS_START/8; i<32; i++) {
		color(i,col,0);
	}
}

void boss()
{ /* boss routine */
	unsigned char i,p;
	unsigned char x_idx;
	unsigned char x_r;

	shutup();
	p=(level-1)*9;

	BNR=BOSTAB[p++];
	BNC=BOSTAB[p++];
	switch (level) {
		default:
		case 1:	bossdraw = draw1; break;
		case 2:	bossdraw = draw2; break;
		case 3:	bossdraw = draw3; break;
		case 4:	bossdraw = draw4; break;
		case 5:	bossdraw = draw5; break;
	}

	// try to mask the graphics prep a little
	wrapstars();

	// Boss pattern is now from BOSS_START to 255, and we reload the pattern here
	wrapunpackboss(level);
	wrapstars();

	for (i=0; i<3; i++) {
		enginer[i]=BOSTAB[p++];
		enginec[i]=BOSTAB[p++];
        enr[i]=enginer[i];
        enc[i]=enginec[i];
	}

	// work in silence to help mask any slowdown
	shutup();

	// clear sprites 
	DelSprButPlayer(255);
	// straighten player
	wrapplayerstraight();

	// clear enemy table (except engines)
	for (i=3; i<12; i++) {
		ent[i]=ENEMY_NONE;
	}
	// clear shot table
	for (i=0; i<=NUM_SHOTS; i++) {
		shr[i]=0;	// note: ONLY shr is legal in this loop because it includes NUM_SHOTS
	}
	// no powerup either (should already be gone though)
	ptp4=POWERUP_NONE;
	
	// center the player ship and build the boss tables
	for (x_idx=0; x_idx<3; x_idx++) {
		for (x_r=0; x_r<BNR; x_r++) {
			wrapplayer();
			wrapstars();	// calls waitforstep()

			PrepareBoss(x_idx, x_r);
			wrapstars();	// calls waitforstep()
		}
	}

	// start the music now
	StartMusic(BOSSMUS, 1);
	
	// erase the 'boss approaching' text
	hchar(11, 0, 32, 32);

	// setup boss
	bossminepower=10;	// starts the same as in a stage, but gets harder FAST
	bc=0; bd=0;
	br=-BNR;
	if (scoremode == 3) {
		// invisible enemies
		// TODO: if the background changes, it won't always be black
		bosscol(COLOR_BLACK);
	} else {
		bosscol(BOSTAB[p]);
	}

	// we set this up now, though it will be partially up again when the boss is fully onscreen
	// otherwise the enemy code crashes trying to process the enemy function
	ent[6]=ENEMY_NONE;		// not active yet however!
	en_func[6]=enemynull;	// an enemy who does nothing (I suppose the movement could go in there for consistency, but.. nah)
	ers[6]=0;	// ers and ecs no longer used, but we'll clear them
	ecs[6]=0;
	enr[6]=(BNR<<3)-32; 
	enc[6]=(BNC<<2)-12;	// <<3>>1 == <<2
	//sprite(6+ENEMY_SPRITE, 228, 0, enr[6], enc[6]); // it doesn't actually have to be on screen!

	// setup rest of boss's engines
	for (i=0; i<3; i++)	{
		if ((enr[i]==0)&&(enc[i]==0)) {
			// not defined, skip it
			ent[i]=ENEMY_NONE;
			ep[i]=0;
			continue;
		}
		ech[i]=76; esc[i]=76; eec[i]=80;
		ent[i]=ENEMY_ENGINE; ers[i]=0; ecs[i]=0;
		en_func[i]=enemyengine;     // just animates
		ep[i]=level*5;		// hit points (engine power)
		sprite(i+ENEMY_SPRITE,76,8,enr[i],enc[i]);
	}

	// load up the engine sprites
	wrapLoadEngineSprites();

	// finally, if the background scrolling isn't on SIT >0000,
	// call it once more to switch pages. We stay on >0000 through
	// the rest of the boss code.
	if (gImage != 0) wrapstars();

	// exit this loop when someone dies!
	flag=BOSS_LOOP_ACTIVE;
	while (flag == BOSS_LOOP_ACTIVE) {
		// frame 0
		wrapplayer();
		wrapcolchk(1);
		wrapstars();
		
		// frame 1
		mboss();
		wrapcolchk(0);
		wrapstars();

		// frame 2
		wrapenemy();

		if (ch) {
			wrapcheat();
		} else {
			wrapplycol();
			// oi.. this generic 'flag' system sucks....
			if (flag == MAIN_LOOP_ACTIVE) {
				// roll back a bit to repeat the boss battle
				flag=PLAYER_DIED_DURING_BOSS;
			}
		}

		whoded();
		if ((ep[0]==0)&&(ep[1]==0)&&(ep[2]==0)) {
			flag=MAIN_LOOP_ACTIVE;
		}
		wrapstars();

		if (bosscnt) {
			if (--bosscnt == 0) {
				// reset boss color set
				VDP_SET_ADDRESS(0x830e);		// CT at >0380
			}
		}
	}

	// reset boss color set in case it flashed (just always)
	VDP_SET_ADDRESS(0x830e);		// CT at >0380

	// check if player won the battle
	if (flag == MAIN_LOOP_ACTIVE) {
        if (joynum) {
	    	byboss();
        } else {
            // didn't really, but this will end the demo
            flag = PLAYER_DIED_DURING_BOSS;
        }
	}
}

// sadly we are out of vblank before this function is done
void drboss() { 
	/*draw boss ship*/
	unsigned char ch=BOSS_START;		// boss chars run from BOSS_START-255
	unsigned char tc,to;
	unsigned char roff,coff;

	tc=bc>>2;
	to=bc&0x03;
	
	// update pattern table for scroll pos (0-3 subpixels)
	VDP_SET_ADDRESS(0x8402+to);

	// handle the inline draw function
	bossdraw();

	// engines (lowest offset is -8, highest is 20)
	roff=br<<3;
	// make sure offscreen engines don't wrap around to the bottom (192 is bottom)
	// but don't allow 208 (which would blank the rest of the sprite table)
	if (roff > 127) roff=200;	// chosen because all row values end up off the bottom but never equal 208
	coff=(bc<<1)+bd;
	if (ep[0]) {
        enr[0] = roff+enginer[0];
        enc[0] = coff+enginec[0];
		sploct(ENEMY_SPRITE,enr[0],enc[0]);
	}
	if (ep[1]) {
        enr[1] = roff+enginer[1];
        enc[1] = coff+enginec[1];
		sploct(1+ENEMY_SPRITE,enr[1],enc[1]);
	}
	if (ep[2]) {
        enr[2] = roff+enginer[2];
        enc[2] = coff+enginec[2];
		sploct(2+ENEMY_SPRITE,enr[2],enc[2]);
	}

	// move cockpit 'bullet' as close to player as we are allowed to go (so if they overwrite us, we get them)
	// row max = (BNR-1)*8
	// col = (BNC*2) to (BNC*2)+(BNC-1)*4-12
	// Couple of small bugs - on narrower bosses, if you sit on the right edge, the sprite
	// briefly wraps to the left of the boss when it's all the way left. This is of no consequence
	// because it does not hit you and could not have hit you at that point anyway.
	// Secondly, the final boss is so wide that you can sit on either edge and not be hit. Not
	// too important because you still have to dodge mines, and you will still take a hit if you
	// try to sweep across the boss.
	enr[6] = SHIP_R;
	if (enr[6] > ((BNR-1)<<3)+roff) enr[6]=((BNR-1)<<3)+roff;
	enc[6] = SHIP_C;
	if (enc[6] < (BNC<<1)+coff-12) enc[6]=(BNC<<1)+coff-12;
	else if (enc[6] > (BNC<<1)+((BNC-1)<<2)+coff) enc[6]=(BNC<<1)+((BNC-1)<<2)+coff;

	if (br >= 1) {
		// some events delete bullets, so just add it back (if it's active)
		ent[6]=ENEMY_SHOT;
	}
}
 
void erboss() { 
	/*erase boss*/
	unsigned char i;
	unsigned int p;

	p=gIMAGE+(bc>>2)+(br<<5);

	for (i=0; i<BNR; i++) {
		vdpmemset(p, 32, BNC);
		p+=32;
	}
}

void mboss() { 
	/*boss control*/
	uint8 x,a;
	uint8 scaledLevel;

	// get the update out of the way first
	bc=bc+bd;
	if (br < 1) {
		br++;
		if (br == 1) {
			// clear the top row and the left most edge, some of the
			// bosses leave a little trail. hopefully the one-frame hiccup
			// is forgivable :)
			hchar(0, 0, 32, 32);
			hchar(1, 0, 32, 12);
			hchar(2, 0, 32, 12);
			hchar(3, 0, 32, 12);

			// now put the score back too ;)
			addscore(0);

			// activate the cockpit sprite
			ent[6] = ENEMY_SHOT;
		}
	}
	drboss();

	// now fix the direction for next time
	scaledLevel = level;
	if ((nDifficulty == DIFFICULTY_EASY)&&(scaledLevel > 2)) scaledLevel=2;
	if ((nDifficulty == DIFFICULTY_MEDIUM)&&(scaledLevel > 3)) scaledLevel=3;
	x=scaledLevel;
	if (bc>=(31-BNC)<<2) bd=-x;
	if (bc<=1) bd=x;

	if ((level==3)||(level==5)) { 
		// decide if mines come out
		if ((rndnum()&7)<3) {
			x=255;
			if (ent[3] == ENEMY_NONE) {
				x=3;
			}
			if ((nDifficulty >= DIFFICULTY_MEDIUM) && (ent[4] == ENEMY_NONE)) {
				x=4;
			}
			if ((nDifficulty == DIFFICULTY_HARD) && (ent[5] == ENEMY_NONE)) {
				x=5;
			}
			if (x<255) { 
				ent[x]=ENEMY_MINE;
				en_func[x]=enemymine;
				ep[x]=bossminepower++;	// muhaha - harder than in the levels, and worse the longer you stay
				if (bossminepower > 126) bossminepower=126;
				enr[x]=64+(br<<3);
				enc[x]=(bc<<1)+45;
				eec[x]=8; esc[x]=8; ech[x]=8;
				if (scoremode == 3) {
					sprite(x+ENEMY_SPRITE,8,COLOR_BLACK,enr[x],enc[x]);
				} else {
					sprite(x+ENEMY_SPRITE,8,COLOR_CYAN,enr[x],enc[x]);
				}
			}
		}
	}
	
	// decide if we shoot
	if ((ent[8]==ENEMY_NONE) && ((rndnum()&7)<3)) { 
		for (a=7; a<12; a++) { 
			ech[a]=84; eec[a]=84; esc[a]=84;
			ers[a]=3+scaledLevel; 
		}
		
		switch (level) { 
			case 1:
				ent[7]=ENEMY_SHOT; ent[8]=ENEMY_SHOT;
				en_func[7]=enemyshot; en_func[8]=enemyshot;
				enr[7]=56+(br<<3); enr[8]=enr[7];
				enc[7]=(bc<<1)+16; enc[8]=enc[7]+40;
				ecs[7]=0; ecs[8]=0;
				sprite(ENEMY_SPRITE+6+1,84,12,enr[7],enc[7]);
				sprite(ENEMY_SPRITE+6+2,84,12,enr[8],enc[8]);
				break;

			case 2:
				ecs[7]=-3; ecs[8]=0; ecs[9]=3;
				for (a=7; a<10; a++) { 
					if ((nDifficulty == DIFFICULTY_EASY) && (a != 8)) {
						continue;
					}
					ent[a]=ENEMY_SHOT; 
					en_func[a]=enemyshot;
					enr[a]=80+(br<<3);
					enr[8]=enr[a]+8;		// no need to do a conditional, just update it each time
					enc[a]=(bc<<1)+36;
					if (nDifficulty == DIFFICULTY_MEDIUM) {
						ecs[7]=-2;
						ecs[10]=2;
					}
					sprite(a+ENEMY_SPRITE,84,12,enr[a],enc[a]); 
				}
				break;

			case 4:
				for (a=7; a<11; a++) { 
					if ((nDifficulty == DIFFICULTY_EASY) && ((a==7)||(a==10))) {
						continue;
					}
					ent[a]=ENEMY_SHOT; 
					en_func[a]=enemyshot;
					enr[a]=88+(br<<3); 
				}
				enr[8]+=8; enr[9]=enr[8];
				enc[7]=(bc<<1)+24; enc[8]=enc[7]+8;
				enc[9]=enc[8]+8; enc[10]=enc[9]+8;
				ecs[7]=-4; ecs[8]=0; ecs[9]=0;
				ecs[10]=4;
				if (nDifficulty == DIFFICULTY_MEDIUM) {
					ecs[7]=-2;
					ecs[10]=2;
				}
				for (a=7; a<11; a++) {
					if (ent[a] != ENEMY_NONE) {
						sprite(a+ENEMY_SPRITE,84,12,enr[a],enc[a]);
					}
				}
				break;

			case 5:
				for (a=7; a<12; a++) {
					if ((nDifficulty < DIFFICULTY_HARD) && ((a==7)||(a==11))) {
						continue;
					}
					if ((nDifficulty == DIFFICULTY_EASY) && (a==9)) {
						continue;
					}
					ent[a]=ENEMY_SHOT; 
					en_func[a]=enemyshot;
				}
				enr[7]=56+(br<<3); enr[8]=enr[7]+8; enr[9]=enr[8]+8;
				enr[10]=enr[8]; enr[11]=enr[7];
				enc[7]=(bc<<1)+32; enc[8]=enc[7]+8;
				enc[9]=enc[8]+12; enc[10]=enc[9]+12;
				enc[11]=enc[10]+8; ecs[7]=-8;
				ecs[8]=-4; ecs[9]=0; ecs[10]=4;
				ecs[11]=8;
				for (a=7; a<12; a++) {
					if (ent[a] != ENEMY_NONE) {
						sprite(a+ENEMY_SPRITE,84,12,enr[a],enc[a]);
					}
				}
				break;
		}
	}
}

uint8 checkdamage(uint8 sr, uint8 sc, uint8 pwr) {
	uint8 b;
	unsigned char rd,cd;
	unsigned int p;

	rd=sr>>3;
	if (rd <= br+BNR) {
		cd=(sc+4)>>3;
		b=gchar(rd,cd);
		if (b>=BOSS_START) {
			// potential - check the character pattern
			p=(b<<3)+gPATTERN+4;
			tmpbuf[0] = vdpreadchar(p);
			if (tmpbuf[0]) {
				// this block is solid, nuke it
				// need to read the whole block
				//p=(b<<3)+gPATTERN;
				p-=4;
				AddDamage(p);
				addscore(1);
				if (pwr==PWRPULSE+2) {
					// maximum pulse shot does double damage
					cd++;
					b=gchar(rd,cd);		// read new char, make sure it's really there
					if (b>=BOSS_START) {
						// repeat for the next char
						p=(b<<3)+gPATTERN;
						AddDamage(p);
						addscore(1);
					}
				}
				return 1;	// only one shot per frame hits the boss body (for performance's sake)
			}
		}
	}
	return 0;
}
 
void whoded() { 
	unsigned char rd,cd;
    unsigned char r,c;

	/*check boss specific collisions*/
	for (uint8 a=0; a<NUM_SHOTS; a++) {
		// check for valid shot
		if (!shr[a]) continue;

		// check if hit a piece of boss
		if (checkdamage(shr[a], shc[a], pwrlvl&0x0f)) {
			spdel(a+PLAYER_SHOT);
			shr[a]=0;
            continue;
		}

		// check if hit an engine
        for (uint8 b=0; b<3; ++b) {
            if (ent[b] != ENEMY_ENGINE) continue;
            spposn(b+ENEMY_SPRITE, r, c);
            rd = abs(r-shr[a]);
            if (rd <= 20) {
                cd = abs(c-shc[a]);
                if (cd <= 15) {
					VDP_SET_ADDRESS(0x830f);		// CT at >03C0 (makes boss flash white, everything else already is)
					bosscnt=3;						// how many cycles to stay white (should be 3 frames per cycle)
					ep[b]-=damage[pwrlvl&0x07];
					if (ep[b]<=0) { 
						addscore(5); 
						ep[b]=0; 
						enr[b]=192; 
                        ent[b]=ENEMY_NONE;
						spdel(b+ENEMY_SPRITE);
					}
					spdel(a+PLAYER_SHOT); 
					shr[a]=0; 
					break;
				}
			}
		}
	}
}
 
void byboss() { 
	/*boss is dead...blow him up!*/
	int tmp, qw;
	uint8 a, x;
	unsigned char tc;
	unsigned char r,c;

	tc=bc>>2;

	shutup();
	// erase enemies and shots
	DelSprButPlayer(PLAYER_FLAME);

	// straighten player
	wrapplayerstraight();
	playmv();			// redraw the shield, DelSprButPlayer will have erased it

	// draw explosions over boss
	a=0;
	for (qw=0; qw<290; qw++) { 
		x=qw/20;
		SOUND=0xe5;
		SOUND=0x89+(qw&0xf);
		SOUND=0x3f;
		SOUND=0xa0+(qw&0xf);
		SOUND=0x3f;
		SOUND=0xc7+(qw&0xf);
		SOUND=0x3e;
		SOUND=0x90+x;
		SOUND=0xb0+x;
		SOUND=0xd0+x;
		SOUND=0xf0+x;

		// draw random explosion cell (or erase old one!)
		r=(rndnum()>>2)%BNR; c=(rndnum()>>2)%BNC;
		x=gchar(br+r,tc+c);
		if (x>127) {
			if (++a > 1) {
				xchar(br+r,tc+c,EXPLOSION_CHAR);		// draw explosion shape if boss
				a=0;
			}
		} else if (x==EXPLOSION_CHAR) {
			x=r*BNC+c+BOSS_START;					// draw blank if already explosion shape
			AddDestroyed((x<<3)+gPATTERN);
			xchar(br+r, tc+c, x);
		}

		// animate stars
		wrapstars();
		
		if ((qw&3)==1) {
			// animate explosions
			patcpy(EXPLOSION_FIRST+((qw>>2)&3), EXPLOSION_CHAR);
			// shake boss back and forth
			VDP_SET_ADDRESS(0x8402 + ((qw>>1)&0x2));
		}
	}
	// erase boss and delay
	erboss();
	shutup();

	// set pattern table back to default >1000
	VDP_SET_ADDRESS(0x8402);

	// boss destroyed announcement
	centr(11, "BOSS DESTROYED BONUS");
	tmp=100;
	if (nDifficulty >= DIFFICULTY_MEDIUM) tmp+=50;
	if (nDifficulty == DIFFICULTY_HARD) tmp+=50;

	for (qw=tmp; qw>=0; qw--) {
		int x = qw;
		VDP_SET_ADDRESS_WRITE((int)(gIMAGE+384+13));
		VDPWD=((x/100)+'0');
		x%=100;
		VDPWD=((x/10)+'0');
		VDPWD=((x%10)+'0');
		VDP_SAFE_DELAY();
		VDPWD=('0');
		VDP_SAFE_DELAY();
		if (qw == tmp) {		// interleaving for VDP writes
			VDPWD=('0');
			delaystars(30);
		} else {
			VDPWD=('0');
			delaystars(1);
			addscore(1);
		}
	}

	delaystars(10);

	// fly off screen
	for (qw=0; qw<81; qw++){ 
		x=11186/(qw+200);
		a=x&0xf;	// low nibble of freq

		tmp=(x>>4)+(a<<8)+0xc000;
		SOUND=0xf0;
		SOUND=(tmp>>8);
		SOUND=tmp&0xff;
		SOUND=0xe7;
		SOUND=0xdf;

		wrapPlayerFlameBig();
		if ((SHIP_R<192)||(SHIP_R>200)) { 
			SHIP_R-=6; playmv(); 
		} else {
			spdall();
		}
		wrapstars();
		SOUND=0xff;
	}
	flag=MAIN_LOOP_ACTIVE;
}
 
// damage pattern for smooth horizontal scrolling for the boss
void AddDamage(unsigned int ptr) {
 	// add 8 bytes of random noise at ptr, but we
 	// also need to shift it through the other 3 tables
 	unsigned short *mask;	// needs 16 bytes
 	unsigned char idx,idx2;

	// Tried unrolling and making this explicit - but it was exactly
	// the same speed, even over 15,000 iterations. So left the loops.

	// alias mask to the second half of tmpbuf, which is 32 bytes
	mask=(unsigned short*)&tmpbuf[16];

	// do initial pattern
	vdpmemread(ptr, tmpbuf, 8);
	tmpbuf[4]=0;

	for (idx=0; idx<8; idx++) {
 		mask[idx]=rndnum()&0xff;
 		tmpbuf[idx]&=mask[idx];
		mask[idx]<<=8;
		mask[idx]|=0xff;
 	}
 	mask[4]=0x00ff;				// test line, wipe it

 	vdpmemcpy(ptr, tmpbuf, 8);

	// do the rest of the patterns
	for (idx=0; idx<3; idx++) {
 		ptr+=SCROLL_OFFSET;				// offset is to the next table
 		vdpmemread(ptr, tmpbuf, 16);
 		for (idx2=0; idx2<8; idx2++) {
 			mask[idx2]>>=2;		// shift 2 pixels right
 			mask[idx2]|=0xc000;	// preserve shifted in pixels
 			tmpbuf[idx2]&=(mask[idx2]>>8)&0xff;
 			tmpbuf[idx2+8]&=mask[idx2]&0xff;
 		}
 		vdpmemcpy(ptr,tmpbuf,16);
 	}
}

// handle smooth horizontal scrolling for the boss
void AddDestroyed(unsigned int ptr) {
 	// clear out 8 bytes at ptr (like AddDamage, but no noise or random)
 	// also need to shift it through the other 3 tables
 	unsigned char idx,idx2;
	unsigned short mask = 0x00ff;

	// do initial pattern
	memset(tmpbuf, 0, 8);
	vdpmemset(ptr, 0, 8);

	// do the rest of the patterns
	for (idx=0; idx<3; idx++) {
 		ptr+=SCROLL_OFFSET;				// offset is to the next table
 		vdpmemread(ptr, tmpbuf, 16);
		mask>>=2;		// shift 2 pixels right
		mask|=0xc000;	// preserve shifted in pixels
 		for (idx2=0; idx2<8; idx2++) {
 			tmpbuf[idx2]&=(mask>>8)&0xff;
 			tmpbuf[idx2+8]&=mask&0xff;
 		}
 		vdpmemcpy(ptr,tmpbuf,16);
 	}
}

// copies the boss patterns scrolled into the next cell
// idx indicates the target scroll table (0-2)
// r indicates the boss row (0-(BNR-1))
void PrepareBoss(unsigned char idx, unsigned char r) {
	unsigned char idx2;
	char c;
	unsigned int p,basep;

	// get start address (last two characters of first row)
	basep=((BOSS_START+BNC-2)<<3)+gPATTERN;

	// calculate the offset for this pass
	for (idx2=0; idx2<idx; idx2++) {
		basep+=SCROLL_OFFSET;
	}
	p=basep;
	for (idx2=0; idx2<(unsigned)r; idx2++) {
		p+=BNC<<3;
	}

	// this seems okay for time now. If not, break up the columns
	// using any interrupt method other than waitforpoll can screw
	// up the timing for the sprite copy and cause corruption
	
	for (c=BNC-1; c>0; c--) {
		vdpmemread(p, tmpbuf, 16);	// read character and its neighbor
		// we need to shift 2 pixels from the neighbor for each row
		for (idx2=0; idx2<8; idx2++) {
			// first, shift ourselves (discard the loss)
			tmpbuf[8+idx2]>>=2;
			// next, get the two LSBs from the neighbor and drop them in place
			tmpbuf[8+idx2]|=(tmpbuf[idx2]&0x03)<<6;
		}
		// now only write ourselves back, but in the next table
		vdpmemcpy(p+8+SCROLL_OFFSET, tmpbuf+8, 8);
		// except if column 0 was involved (saves an extra read/write)
		if (c == 1) {
			for (idx2=0; idx2<8; idx2++) {
				// just need to shift, nothing new to come in
				tmpbuf[idx2]>>=2;
			}
			vdpmemcpy(p+SCROLL_OFFSET, tmpbuf, 8);
		}
		// decrement the pointer
		p-=8;
	}
}

// hard-coded boss draw functions are nearly twice as
// fast as the looping version, and we optimize a bit
// by skipping empty space.
// Note that the draw functions all have one more character
// than the count, because the shifting makes all chars 2 wide
// the compiler is optimizing these as "ld a,#<constant> ; out (_VDPWD),a"
// To that end, maybe we can save a few nops? ld a,# is 7 cycles, each
// nop is 4. out is 11. We need 29. So we have 18 cycles in instruction,
// and need 3 nops to get to 30. Safe delay is 5, so let's do our own
// and get the boss draw just a bit faster.

inline void BOSS_SAFE_DELAY(void) {	
__asm
	nop
	nop
	nop
__endasm;
}

#define EDGEBLANKA			\
	if (bd>0) {				\
		VDPWD=32;			\
		BOSS_SAFE_DELAY();	\
		VDPWD=32;			\
		BOSS_SAFE_DELAY();	\
	}

#define EDGEBLANKB			\
	if (bd<0) {				\
		VDPWD=32;			\
		BOSS_SAFE_DELAY();	\
		VDPWD=32;			\
	}

#define LINECHAR		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR2		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR3		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR4		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR5		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR6		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR7		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR8		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR9		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR10		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR12		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

#define LINECHAR14		\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();	\
	VDPWD=ch++;			\
	BOSS_SAFE_DELAY();

inline void BOSS_SET_ADDRESS_WRITE(unsigned int x)	{	VDPWA=((x)&0xff); VDPWA=((((x)>>8)&0x3f)|0x40); }

void draw1() {
	unsigned char ch=BOSS_START+1;	// skipping the first 1
	unsigned int p;

	// gIMAGE /must/ stay at >0000 for this code to work
	p=(bc>>2)+(br<<5)+gIMAGE;	// note: this wraps negative badly and requires the special addres write macro
								// that performs masking to ensure it stays in the right memory range
	if (bd>0) {
		p-=2;	// for leading edge
	}

	BOSS_SET_ADDRESS_WRITE(p+1);
	EDGEBLANKA;
	LINECHAR8;
	EDGEBLANKB;
	ch+=2+0-1;      // add characters spare at end of this line, plus leading characters spare next line, -1
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	ch+=1+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=3+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+4-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+4);
	EDGEBLANKA;
	LINECHAR3;
	EDGEBLANKB;
}

void draw2() {
	unsigned char ch=BOSS_START;
	unsigned int p;

	p=(((unsigned)(bc))>>2)+(((unsigned)(br))<<5)+gIMAGE;
	if (bd>0) {
		p-=2;	// for leading edge
	}

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch++;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	ch+=1+1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+1);
	EDGEBLANKA;
	LINECHAR8;
	EDGEBLANKB;
	ch+=2+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+4-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+4);
	EDGEBLANKA;
	LINECHAR3;
	EDGEBLANKB;
	ch+=4+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=3+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+4-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+4);
	EDGEBLANKA;
	LINECHAR3;
	EDGEBLANKB;
}

void draw3() {
	unsigned char ch=BOSS_START;	// no skip at all
	unsigned int p;

	p=(((unsigned)(bc))>>2)+(((unsigned)(br))<<5)+gIMAGE;
	if (bd>0) {
		p-=2;	// for leading edge
	}

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	ch+=1+1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+1);
	EDGEBLANKA;
	LINECHAR9;
	EDGEBLANKB;
	ch+=1+1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+1);
	EDGEBLANKA;
	LINECHAR8;
	EDGEBLANKB;
}

void draw4() {
	unsigned char ch=BOSS_START;
	unsigned int p;

	p=(((unsigned)(bc))>>2)+(((unsigned)(br))<<5)+gIMAGE;
	if (bd>0) {
		p-=2;	// for leading edge
	}

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	//ch+=1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR10;
	EDGEBLANKB;
	ch+=1+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
	ch+=4+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR5;
	EDGEBLANKB;
	ch+=4+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=3+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=3+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=3+2-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+2);
	EDGEBLANKA;
	LINECHAR5;
	EDGEBLANKB;
}

void draw5() {
	unsigned char ch=BOSS_START;
	unsigned int p;

	p=(((unsigned)(bc))>>2)+(((unsigned)(br))<<5)+gIMAGE;
	if (bd>0) {
		p-=2;	// for leading edge
	}

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR14;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR14;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR14;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR14;
	EDGEBLANKB;
	//ch+=1+0-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p);
	EDGEBLANKA;
	LINECHAR14;
	EDGEBLANKB;
	ch+=1+1-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+1);
	EDGEBLANKA;
	LINECHAR12;
	EDGEBLANKB;
	ch+=2+3-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+3);
	EDGEBLANKA;
	LINECHAR8;
	EDGEBLANKB;
	ch+=4+4-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+4);
	EDGEBLANKA;
	LINECHAR6;
	EDGEBLANKB;
	ch+=5+5-1;
	p+=32;

	BOSS_SET_ADDRESS_WRITE(p+5);
	EDGEBLANKA;
	LINECHAR4;
	EDGEBLANKB;
}
