#ifndef ENEMY_H
#define ENEMY_H

#include "game.h"

// enemy types
enum {
	// non-active enemy types
	ENEMY_NONE,
	ENEMY_EXPLOSION,
	
	// enemy attacks (can't be shot)
	ENEMY_SHOT,
	ENEMY_BEAM,

	// enemy engines have special handling
	ENEMY_ENGINE,

	// active enemies, shootable (order dictates what level they come out)
	ENEMY_SAUCER,
	ENEMY_JET,
	ENEMY_MINE,
	ENEMY_HELICOPTER,
	ENEMY_BEAMGEN,		// complete beamgen, goes to DEADBEAM when shot
	ENEMY_SWIRLY,
	ENEMY_BOMB,

	ENEMY_DEADBEAM,	// left or right piece only, just completes its journey down

	// helicopter state machine, jumped into after helicopter reaches its target
	ENEMY_HELIPAUSE1,					// Helicopter pauses to shoot (counts up)
	ENEMY_HELISHOOT1=ENEMY_HELIPAUSE1+9,// first shot (skip to pause 4 on easy)
	ENEMY_HELIPAUSE2,
	ENEMY_HELISHOOT2=ENEMY_HELIPAUSE2+5,// second shot (skip to pause 4 on medium)
	ENEMY_HELIPAUSE3,
	ENEMY_HELISHOOT3=ENEMY_HELIPAUSE3+5,// third and last shot
	ENEMY_HELIPAUSE4,
	ENEMY_HELILEAVE=ENEMY_HELIPAUSE4+3,	// Helicopter is leaving

	ENEMY_MAX
};

// 6 enemies + 6 enemy shots in one table (12 total)
#define ENEMY_SPRITE 5

void enout();
void enemy();
void shootplayer(unsigned char en, unsigned char shot);
void helishoot(unsigned char en);
void enemyinit();
void enemyexplosion(uint8);
void enemyengine(uint8);
void enemyshot(uint8);
void enemyhominglaser(uint8);
void enemyhoming(uint8);
void enemymine(uint8);
void enemynull(uint8);
void enemysaucer(uint8 x);
void enemyjet(uint8 x);
void enemyhelicopter(uint8 x);
void enemyhelipause(uint8 x);
void enemyhelileave(uint8 x);
void enemybeamgen(uint8 x);
void enemydeadbeam(uint8 x);
void enemyhelileave(uint8 x);
void enemyswirly(uint8 x);
void enemybomb(uint8 x);

#endif
