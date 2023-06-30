// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <ColecoSNPlay.h>

// game
#include "game.h"
#include "music.h"

extern const unsigned char sfx_armor[];
extern const unsigned char sfx_explosion[];
extern const unsigned char sfx_hitboss[];
extern const unsigned char sfx_nukebomb[];
extern const unsigned char sfx_shipdead[];
extern const unsigned char sfx_shielddown[];
extern const unsigned char sfx_shieldwarn[];
extern const unsigned char sfx_pwrwide[];
extern const unsigned char sfx_shieldup[];
extern const unsigned char sfx_pwrpulse[];

// although the music lives in various banks, the
// player code is in the fixed bank, so we can
// just swap in before calling the playback code,
// then swap back as per normal.

const unsigned char *pLoopMus;
unsigned int  loopBank;
unsigned int  loopIdx;
unsigned int  musBank;
const unsigned char *pSfx;
const unsigned char *pShoot;
unsigned char blockSfx;

// one interrupt of music
void doMusic() {
	unsigned int old = nBank;

	// no music in demo, but sfx are okay
	if (joynum != 0) {
	    // check whether we're playing
        if (!(isSNPlaying)) {
		    // loop music if needed
		    if (pLoopMus != NULL) {
			    MUTE_SOUND();	// don't carry over any old tones
			    StartMusic(pLoopMus, loopBank, loopIdx, 1);
			    // we'll try not returning for smoother transition to intros,
			    // but, this player is still kind of heavy-weight.
		    } else {
			    shutup();
			    return;
		    }
	    }

	    SWITCH_IN_PREV_BANK(musBank);
        CALL_PLAYER_SN;
    }
     
    // run sound effects at 30 hz
    if (VDP_INT_COUNTER & 1) {
        if (NULL != pSfx) {
            // SFX data format:
            // number registers, [register number, register data]
            // SWITCH_IN_BANKxx;
            unsigned char regs = *(pSfx++);
            if (0 == regs) {
                pSfx = NULL;
                blockSfx = 0;
            } else {
                while (regs--) {
                    AY_REGISTER = *(pSfx++);
                    AY_DATA_WRITE = *(pSfx++);
                }
            }
        }
    } else {
        if (NULL != pShoot) {
            // SFX data format:
            // number registers, [register number, register data]
            // SWITCH_IN_BANKxx;
            unsigned char regs = *(pShoot++);
            if (0 == regs) {
                pShoot = NULL;
            } else {
                while (regs--) {
                    AY_REGISTER = *(pShoot++);
                    AY_DATA_WRITE = *(pShoot++);
                }
            }
        }
    }

	SWITCH_IN_PREV_BANK(old);
}

void StartMusic(const unsigned char *p, unsigned int inBank, unsigned char idx, unsigned char bLoop) {
	unsigned int old = nBank;

	// no music in demo
	if (joynum == 0) {
		shutup();
		pLoopMus = NULL;
		return;
	}

	musBank = inBank; 
	if (bLoop) {
		pLoopMus = p;
		loopBank = inBank;
		loopIdx  = idx;
	} else { 
		pLoopMus = NULL;
	}
	SWITCH_IN_PREV_BANK(musBank);
	//stinit((unsigned char*)p, idx);
    StartSong(p, idx);
	SWITCH_IN_PREV_BANK(old);
}

void shutup()
{ 
	/*silence to music generators */
	//allstop();
    StopSong();
	MUTE_SOUND();
	pLoopMus=NULL;

    pSfx = NULL;
    pShoot = NULL;
    AY_REGISTER = AY_VOLA;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLB;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLC;
    AY_DATA_WRITE = 0x0;
}

// do any necessary sound chip initialization
void initSound() {
    shutup();
    // set up the AY so A and B are tone channels, and C is noise
    AY_REGISTER = AY_MIXER;
    AY_DATA_WRITE = 0x1C;
}

void playsfx_armor() {
    pSfx = sfx_armor;
    blockSfx = 1;
}
void playsfx_explosion() {
    pSfx = sfx_explosion;
    blockSfx = 1;
}
void playsfx_hitboss() {
    if (!blockSfx) pSfx = sfx_hitboss;
}
void playsfx_nukebomb() {
    pSfx = sfx_nukebomb;
    blockSfx = 1;
}
void playsfx_shipdead() {
    if (!blockSfx) pSfx = sfx_shipdead;
}

// player sfxs run separately and sparingly
void playsfx_shielddown() {
    pShoot = sfx_shielddown;
}

void playsfx_shieldup() {
    pShoot = sfx_shieldup;
}

void playsfx_shieldwarn() {
    pShoot = sfx_shieldwarn;
}

void playsfx_pwrpulse() {
    pShoot = sfx_pwrpulse;
}

void playsfx_pwrwide() {
    pShoot = sfx_pwrwide;
}

