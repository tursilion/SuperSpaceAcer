// libti99
#include <vdp.h>
#include <sound.h>
#include <kscan.h>
#include <player.h>

// game
#include "game.h"

// although the music lives in various banks, the
// player code is in the fixed bank, so we can
// just swap in before calling the playback code,
// then swap back as per normal.

// TODO: stages need to be lengthened to fit the music better

const unsigned char *pLoopMus;
unsigned int  loopBank;
unsigned int  loopIdx;
unsigned int  musBank;

// one interrupt of music
void doMusic() {
	unsigned int old = nBank;

	// no music in demo
	if (joynum == 0) {
		return;
	}

	// check whether we're playing
	if (pDone) {
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
	stplay();
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
	stinit((unsigned char*)p, idx);
	SWITCH_IN_PREV_BANK(old);
}

void shutup()
{ 
	/*silence to music generators */
	allstop();
	MUTE_SOUND();
	pLoopMus=NULL;
}

