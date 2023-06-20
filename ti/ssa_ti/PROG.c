/*program SUPER SPACE ACER design version 2.1*/
/*--- FIX LEVEL # IN LINE 47 ---*/
#include "dsk5.earefs/c"
#include "dsk5.grf1rf"
 
#asm
       COPY "DSK1.MOREDEFS"
STDAT# DATA >0500,>0800,>0000,>0F00,>0008
       TEXT 'DSK1.SSD'
#endasm
 
int joynum,s,a,b,r,c,x,y,flst;
int flag,k,pwrlvl,lives;
int level,shd[3],qw;
int str[4][6], stc[4][6], sto[4][6];
int shr[3], shc[3], enr[12], enc[12];
int ent[12], ech[12], eec[12], esc[12];
int ers[12], ecs[12], ep[3];
int shield,distns,ch;
int pcr4,ptp4,pr4,pc4,pec4,psc4;
int br,bc,bd;
char pname[15];
 
#include "dsk5.cutils/c"
#include "dsk5.random;c"
 
main()
{
spdall();
random();
intpic();
ldpic("ACER");
c=0;
while (c!=18)
{ joynum=1;
  c=key(joynum,&s);
  if (c!=18)
  { joynum=2;
    c=key(joynum,&s);
  }
}
grf1();
hotfix();
/*load VDP data*/
#asm
 LI R0,>3780
 LI R1,TEMP#1
 LI R2,>80
 BLWP @VMBW   *RESTORE DSR BLOCKS
 LI R0,>2000
 LI R1,STDAT#
 LI R2,20
 BLWP @VMBW
 LI R0,>2009
 MOV R0,@>8356
 BLWP @DSRLNK
 DATA 8
#endasm
sgrint();
while (1) { /*forever*/
k=key(5,&s);
if (k==65) ch=1; else ch=0;
pwrlvl=0;
lives=3;
level=1; distns=600;
while (level<6)
{space();
if (flag!=2) boss();
if (flag==2) gamovr();
if (flag!=2) {level++; distns=level*100+500; }
}
if (level==6) gamwin();
}}
 
space()
{ /* do a space level */
  ispace();
  flag=0;
  while (flag==0)
  { stars();
    player();
    enout();
    enemy();
    colchk();
    if (ch) cheat();
    if (!ch) plycol();
    distns--;
    if (distns==0) flag=1;
  }
}
 
cls()
{ /* 'clear' screen with 206 */
#asm
       CLR R0
       LI R1,>CE00
MY#2   BLWP @VSBW
       INC R0
       CI R0,768
       JNE MY#2
#endasm
}
 
ispace()
{/* init space level (stars, music, etc) */
  cls();
  spdall();
  screen(2);
  spmag(3);
  spmct(0);
  shutup();
  for (a=1; a<4; a++)
  { str[3][a]=rnd(23)+1;
    stc[3][a]=rnd(31)+1;
    hchar(str[3][a],stc[3][a],210,1);
    sto[3][a]=0;
  }
  for (a=1; a<3; a++)
  { str[2][a]=rnd(23)+1;
    stc[2][a]=rnd(31)+1;
    hchar(str[2][a],stc[2][a],208,1);
    sto[2][a]=0;
  }
  for (a=1; a<2; a++)
  { str[1][a]=rnd(23)+1;
    stc[1][a]=rnd(31)+1;
    hchar(str[1][a],stc[1][a],207,1);
    sto[1][a]=0;
  }
  #asm
       LI R0,>1100 *INTROM
       MOV R0,@>83CC
       MOVB @H01,@>83CE
       SOCB @H01,@>83FD
#endasm
  sprite(0,108,16,192,112);
  sprite(1,112,16,192,128);
  sprite(2,116,16,208,112);
  sprite(3,120,16,208,128);
  sprite(20,100,9,224,120);
  sprite(21,124,7,192,112);
  sprite(22,128,7,192,128);
  sprite(23,132,7,208,112);
  sprite(24,136,7,208,128);
  for (qw=1; qw<10; qw++)
  { spposn(21,&r,&c);
    r=r-6;
    playmv();
    stars();
  }
  flst=0;
  for (a=0; a<3; a++)
    shr[a]=0;
  for (a=0; a<12; a++)
    ent[a]=0;
  ptp4=0;
  distns=distns+100;
  shield=20;
}
 
sgrint()
{ /*init space colors */
  for (a=0; a<32; a++)
    color(a,16,1);
  color(31,10,1);
}
 
playmv()
{ /* locate the ship at r,c */
  sploct(0,r,c);
  sploct(1,r,c+16);
  sploct(2,r+16,c);
  sploct(3,r+16,c+16);
  sploct(20,r+32,c+8);
  sploct(21,r,c);
  sploct(22,r,c+16);
  sploct(23,r+16,c);
  sploct(24,r+16,c+16);
  x=2;
  if (shield>0) x=7;
  if (shield>40) x=11;
  if (shield>70) x=16;
  spcolr(21,x); spcolr(22,x); spcolr(23,x); spcolr(24,x);
}
 
stars()
{ /* move the stars, enable interrupts for music */
  for (a=3; a>0; a--)
    for (b=1; b<a+1; b++)
    { switch (a)
      { case 1:c=1; r=207; break;
        case 2:c=2; r=208; break;
        case 3:c=4; r=210;
      }
      x=gchar(str[a][b],stc[a][b]);
      if (x<214) hchar(str[a][b],stc[a][b],206,1);
      if (++sto[a][b]==c)
      { sto[a][b]=0;
        if (++str[a][b]==25) {str[a][b]=1; stc[a][b]=rnd(31)+1; }
      }
      x=gchar(str[a][b],stc[a][b]);
      if (x<214) hchar(str[a][b],stc[a][b],r+sto[a][b],1);
    #asm
    LIMI 2
    LIMI 0
#endasm
    }
if (flag!=1)
  #asm
  MOVB @>83CE,@>83CE
  JNE MY#1
  LI R0,>1200 *MAINBM
  MOV R0,@>83CC
  SOCB @H01,@>83FD
  MOVB @H01,@>83CE
MY#1
#endasm
#asm
  LI R0,>8000
  MOV R0,@>83D6  *screen blank time-out
#endasm
}
 
player()
{ /* move the player based on joystick 'joynum' */
  spposn(0,&r,&c);
  a=joyst(joynum,&x,&y);
  flst=flst+4;
  if (flst==8) flst=0;
  if (a)
  { if (c+x*3<223) if (c+x*3>0) c=c+x*3;
    if (r-y*2<151) if (r-y*2>0) r=r-y*2;
    if (y!=0) flst=-4*(y==-4);
  }
    switch (x)
    { case 4: sppat(0,172); sppat(1,176);
              sppat(2,180); sppat(3,184);
              sppat(21,188); sppat(22,192);
              sppat(23,196); sppat(24,200);
              break;
      case -4:sppat(0,140); sppat(1,144);
              sppat(2,148); sppat(3,152);
              sppat(21,156); sppat(22,160);
              sppat(23,164); sppat(24,168);
              break;
      default:sppat(0,108); sppat(1,112);
              sppat(2,116); sppat(3,120);
              sppat(21,124); sppat(22,128);
              sppat(23,132); sppat(24,136);
    }
  playmv();
  sppat(20,100+flst);
  k=key(joynum,&s);
  if (k==18) shoot();
  mvshot();
  if (shield>0) shield--;
}
 
shutup()
{ /*silence to music generators */
#asm
       LI R0,>1000  *STOPAM
       MOV R0,@>83CC
       SOCB @H01,@>83FD
       MOVB @H01,@>83CE
MY#3   MOVB @>83CE,@>83CE
       LIMI 2
       LIMI 0
       JNE MY#3
#endasm
 
}
 
shoot()
{ /* make player shoot if a shot available */
  /* take pwrlvl into account */
a=0;
while ((shr[a]!=0)&&(a<3)) a++;
if (a!=3)
{ spposn(0,&r,&c);
  shr[a]=r;
  shc[a]=c+8;
  sprite(5+a,88+pwrlvl*4,12+pwrlvl,shr[a],shc[a]);
  shd[a]=0;
  if (pwrlvl==1)
  { sprite(5,92,13,r,c+8);
    sprite(6,92,13,r,c+8);
    sprite(7,92,13,r,c+8);
    shd[0]=-4;
    shd[1]=0;
    shd[2]=4;
    shr[0]=r;
    shr[1]=r;
    shr[2]=r;
    shc[0]=c+8;
    shc[1]=c+8;
    shc[2]=c+8;
  }
}}
 
mvshot()
{ /* move the player's shots */
for (a=0; a<3; a++)
  if (shr[a])
{ shr[a]=shr[a]-8;
  shc[a]=shc[a]+shd[a];
  if ((shr[a]<1)||(shc[a]>255)||(shc[a]<1))
  { spdel(5+a);  shr[a]=0; }
  if (shr[a])
    sploct(5+a,shr[a],shc[a]);
}}
 
cheat()
{ /* process debugging keys */
k=key(5,&s);
  if (k==83) shield=shield+100;
  if (k==76) lives++;
  if (k==48) shield=0;
  if (k==87) distns=1;
  if (k==80) { pwrlvl++; if (pwrlvl==3) pwrlvl=0; }
  if (k==66) { ep[0]=0; ep[1]=0; ep[2]=0;}
}
 
enout()
{ /* rndly bring out a new enemy */
k=rnd(10);
if (k<6)
{ if (ent[k]==0)
  {  a=rnd(6)+1;
     if (a<=level+1)
     { ent[k]=a;
       enr[k]=1;
       enc[k]=rnd(250);
       switch (a)
{ case 1: eec[k]=0; esc[k]=0; c=15; break;
  case 2: eec[k]=4; esc[k]=4; c=6; break;
  case 3: eec[k]=8; esc[k]=8; c=8; break;
  case 4: eec[k]=24; esc[k]=12; c=14; break;
  case 5: eec[k]=40; esc[k]=28; c=9; break;
  case 6: eec[k]=44; esc[k]=44; c=11; break;
} ech[k]=esc[k];
       sprite(k+8,ech[k],c,enr[k],enc[k]);
}}}
/*shoot?*/
k=rnd(10);
if (k<6)
{ if ((ent[k+6]==0)&&((ent[k]==1)||(ent[k]==2)||(ent[k]==4)))
  { ent[k+6]=7;
    eec[k+6]=84;
    esc[k+6]=84;
    ech[k+6]=84;
    enr[k+6]=enr[k];
    enc[k+6]=enc[k];
    spposn(0,&r,&c);
    r=r+8; c=c+8;
    ers[k+6]=sgn(r-enr[k])*4;
    ecs[k+6]=sgn(c-enc[k])*4;
    sprite(k+14,84,13,enr[k],enc[k]);
  }}
}
 
sgn(x) int x;
{ /* return -1,0,1 if # positive,etc */
if (x<0) return(-1);
if (x>0) return(1);
return(0);
}
 
enemy()
{ /*move enemies */
spposn(0,&r,&c);
r=r+8; c=c+8;
for (x=0; x<12; x++)
{ if (ent[x]!=0)
  {  switch(ent[x])
     { case 1: enr[x]=enr[x]+3;
               enc[x]=enc[x]+rnd(2)-1;
               break;
       case 2: enr[x]=enr[x]+8;
               break;
       case 3: enr[x]=enr[x]+sgn(r-enr[x])*2;
               enc[x]=enc[x]+sgn(c-enc[x])*2;
               break;
       case 4: enr[x]=enr[x]+4;
               enc[x]=enc[x]+sgn(128-enc[x])*2;
               break;
       case 5: enr[x]=enr[x]+5;
               enc[x]=enc[x]+sgn(c-enc[x])*3;
               break;
       case 6: enr[x]=enr[x]+3;
               enc[x]++;
               break;
       case 7:
       case 8: enr[x]=enr[x]+ers[x];
               enc[x]=enc[x]+ecs[x];
     }
     ech[x]=ech[x]+4;
     if (ech[x]>eec[x]) ech[x]=esc[x];
if ((ech[x]==esc[x])&&(ent[x]==9)) pwr(x);
     else
    {sppat(x+8,ech[x]);
     if (enr[x]>191) noen(x);
     if (enr[x]<1) noen(x);
     if (enc[x]>254) noen(x);
     if (enc[x]<1) noen(x);
     if ((ent[x]>0)&&(ent[x]<10))
       sploct(x+8,enr[x],enc[x]); }
     }
   }
if (ptp4!=0)
{ pr4++;
  sploct(4,pr4,pc4);
  pcr4++;
  if (pcr4>pec4) pcr4=psc4;
  spcolr(4,pcr4);
  if (pr4>192) {ptp4=0; spdel(4);}
}}
 
noen(x) int x;
{ /* remove enemy x from service */
spdel(x+8);
ent[x]=0;
}
 
colchk()
{ /* check sprite collisions */
int a,b;
for (k=0; k<6; k++)
if ((ent[k]>0)&&(ent[k]<7))
 for (s=0; s<3; s++)
 if (shr[s])
 {#asm
 LIMI 2
 LIMI 0
#endasm
   a=abs(enr[k]-shr[s]); b=abs(enc[k]-shc[s]);
   if ((a<=15)&&(b<=15))
   { dyen(k);
     spdel(s+5);
     shr[s]=0;
     s=2;
   }
 }
}
 
dyen(x) int x;
{ /* enemy has been shot */
  int r,c,k;
if (ent[x]!=6)
{ent[x]=9;
 ech[x]=52;
 eec[x]=72;
 esc[x]=52;
 spcolr(x+8,10);
} else
{ spposn(x+8,&r,&c);
  screen(16);
  #asm
 LI R0,>E600
 MOVB R0,@>8400
 LI R0,>F000
 MOVB R0,@>8400
#endasm
  for (k=0; k<12; k++)
    noen(k);
  for (k=0; k<9; k++)
  { ent[k]=8;
    enr[k]=r; enc[k]=c;
    ech[k]=48;
    eec[k]=48;
    esc[k]=48;
    sprite(k+8,48,12,r,c);
    if (k<3) ers[k]=-9;
    if ((k>2)&&(k<6)) ers[k]=0;
    if (k>5) ers[k]=9;
    if (k%3==0) ecs[k]=-9;
    if ((k-1)%3==0) ecs[k]=0;
    if ((k+1)%3==0) ecs[k]=9;
  }
  ent[4]=9;
  ech[4]=52;
  eec[4]=72;
  esc[4]=52;
  sprite(12,52,10,r,c);
  #asm
  LI R0,>FF00
  MOVB R0,@>8400
#endasm
screen(2);
}}
 
abs(x) int x;
{ /* returns absolute value of x */
if (x<0) x=-x;
return(x);
}
 
pwr(x) int x;
{ /* decides if pwr up to come out*/
if ((flag!=1)&&(rnd(20)<3)&&(ptp4==0))
{ ptp4=rnd(2)+1;
  patcpy(203+ptp4,248);
  sprite(4,248,3,enr[x],enc[x]);
  pcr4=3; pr4=enr[x]; pc4=enc[x];
  pec4=16; psc4=3; /*used for color, not character*/
}
noen(x);
}
 
plycol()
{/*check player collisions */
spposn(0,&r,&c);
/*pwrup?*/
if (ptp4)
{ x=pr4-r; y=pc4-c;
 if ((x<25)&&(y<25)&&(x>=0)&&(y>=0))
 { if (ptp4==2)
   { pwrlvl++;
     if (pwrlvl==3) pwrlvl=2;
   } else
     shield=shield+100;
   ptp4=0;
   spdel(4);
}}
/*enemies?*/
r=r-8; c=c-8;
if (shield<1)
{for (a=0; a<12; a++)
 if ((ent[a]>0)&&(ent[a]<9))
 { x=enr[a]-r; y=enc[a]-c;
   if ((x<25)&&(y<25)&&(x>=0)&&(y>=0))
    a=99;
 }
 if (a>=99) pdie();
}}
 
pdie()
{/* player dies */
shutup();
pboom();
spdall();
delay(2000);
lives--;
pwrlvl=0;
if (lives<0) flag=2;
 else ispace();
}
 
delay(q) int q;
{ /*short pause*/
while (q--)
{ q++; q--; }
}
 
#include "dsk1.prog2/c"
