#ifndef HUMAN_H
#define HUMAN_H

void player();
void shoot();
void mvshot();
void homingshot();
void cheat();
void colchk(uint8 half);
void plycol();
void pdie();
void pboom();
void playerstraight();
void playerleft();
void playerright();
void playerinit();
void dyen(unsigned char x);

extern uint8 pwrlvl;
extern uint8 oldpwrlvl;
#endif
