#ifndef MUSIC_H
#define MUSIC_H

extern const unsigned char outpack0[], outpack1[], outpack2[], outpack3[];

#define STAGE1MUS		outpack1,0xfffc,0   /* driven */
#define STAGE2MUS		outpack2,0xfff9,0   /* invasion */
#define STAGE3MUS		outpack0,0xfffb,0   /* boxxIX */
#define STAGE4MUS		outpack1,0xfffc,2   /* oddity */
#define STAGE5MUS		outpack3,0xfff9,0   /* mystery */
#define BOSSMUS			outpack2,0xfff9,1   /* storm */
#define GAMEOVERMUS		outpack0,0xfffb,1   /* tursi gameover */
#define WINSCROLLMUS	outpack1,0xfffc,1   /* what */
#define WINANIMMUS		outpack0,0xfffb,2   /* gameover */

extern const unsigned char *pLoopMus;

extern void (*doMusic)(void);
void doAllMusic();
void doSfxInstead();
void StartMusic(const unsigned char *p, unsigned int musBank, unsigned char idx, unsigned char bLoop);
void shutup();
void initSound();

void playsfx_armor();
void playsfx_explosion();
void playsfx_hitboss();
void playsfx_nukebomb();
void playsfx_shipdead();
void playsfx_shielddown();
void playsfx_shieldwarn();
void playsfx_shieldup();
void playsfx_pwrpulse();
void playsfx_pwrwide();

#endif
