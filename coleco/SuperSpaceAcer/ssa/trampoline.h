#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include "game.h"

// trampoline functions
void wrapenemy();
void wrapnoen(uint8 x);
void wrapplayer();
void wrapcheat();
void wrapcolchk(uint8 x);
void wrapplycol();
void wrapplayerstraight();
void wrapunpackboss(unsigned char level);
void winwrapgetscroll(unsigned char *dst, const char *src, unsigned int cnt);
char winwrapgetbyte(const char *adr);
void wrapstars();
void wrapinitstars();
void wrapispace();
void wrapbackground();
void wrapcheckdamage(uint8 sr, uint8 sc, uint8 pwr);
void wrapLoadSelenaPic();
void wrapgetfontbytes(unsigned char *dest, const unsigned char *src, unsigned int cnt);
void wrapspritescore(unsigned int sprpat, unsigned int sprtab, unsigned char row, unsigned char col, unsigned char ch);
void wrapLoadEngineSprites();
void wrapPlayerFlameBig();
void wrapPlayerFlameSmall();
void wrapGameWinHard();
void wrapGameWinMed();
void wrapCopyShip(const unsigned char *p, const unsigned char *c);
void wrapGamWin();

#endif
