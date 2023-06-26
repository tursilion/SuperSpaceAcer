#ifndef MUSIC_H
#define MUSIC_H


#if 0
// musical numbers
extern const unsigned char songpack1[], songpack2[], songpack3[], songpack4[], songpack5[], songpack6[];

// songs used			address,bank,index
#define STAGE1MUS		songpack2,0xfffd,1
#define STAGE2MUS		songpack3,0xfffb,0
#define STAGE3MUS		songpack6,0xfff9,0
#define STAGE4MUS		songpack3,0xfffb,1
#define STAGE5MUS		songpack5,0xfff9,0
#define BOSSMUS			songpack4,0xfffc,0
#define GAMEOVERMUS		songpack1,0xfff4,2
#define WINSCROLLMUS	songpack4,0xfffc,1
#define WINANIMMUS		songpack2,0xfffd,2

// music not yet used
#define ATTRACTMUS      songpack1,0xfff4,0  /* evilboss */
#define GAMESTARTMUS    songpack1,0xfff4,1  /* select */
#define INFORMMUS       songpack1,0xfff4,3  /* informant */
#define FLYTOMUS        songpack2,0xfffd,0  /* freekd */
#else
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
#endif

void doMusic() ;
void StartMusic(const unsigned char *p, unsigned int musBank, unsigned char idx, unsigned char bLoop);
void shutup();


#endif
