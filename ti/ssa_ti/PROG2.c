/* SSA PROG/C2...part 2 of C source */
 
pboom()
{/* do SSA ship explode routine */
for (a=4; a<25; a++)
 spdel(a);
for (qw=1; qw<4; qw++)
{ stars(); shutup();
  spcolr(0,16); spcolr(1,16);
  spcolr(2,16); spcolr(3,16);
  sh1(); delay(100);
  stars(); shutup();
  sh2(); delay(100);
  stars(); shutup();
  spcolr(0,9); spcolr(1,9);
  spcolr(2,9); spcolr(3,9);
  sh1(); delay(100);
  stars(); shutup();
  sh3(); delay(100);
  stars(); shutup();
}
#asm
 LI R0,>E600
 MOVB R0,@>8400
 LI R0,>F000
 MOVB R0,@>8400
#endasm
patcpy(255,248);
sppat(0,248); sppat(1,248);
sppat(2,248); sppat(3,248);
spmotn(0,-29,-29); spmotn(1,-29,29);
spmotn(2,29,-29); spmotn(3,29,29);
spmct(4);
for (qw=1; qw<16; qw++)
{ #asm
 LIMI 2
 LIMI 0
 LI R0,>F0
 A @QW,R0
 SWPB R0
 MOVB R0,@>8400
#endasm
delay(40);
stars(); shutup();
}
spmct(0);
}
 
gamovr()
{ /* do the GAME OVER thing */
for (a=0; a<9; a++)
  hchar(12,12+a,239+a,1);
#asm
 LI R0,>1400 *GAMOVM
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
#endasm
k=0;
while (k!=18)
{ k=key(joynum,&s);
#asm
 MOVB @>83CE,@>83CE
 JNE NSHUP#
#endasm
 shutup();
#asm
NSHUP#
#endasm
}
level=7;
}
 
sh1()
{ /*define ship shape 1*/
sppat(0,108); sppat(1,112);
sppat(2,116); sppat(3,120);
}
 
sh2()
{ /*shape 2*/
sppat(0,140); sppat(1,144);
sppat(2,148); sppat(3,152);
}
 
sh3()
{ /*#3*/
sppat(0,172); sppat(1,176);
sppat(2,180); sppat(3,184);
}
 
#asm
WN#DAT DATA >0500,>1000,>0000,>2000,>0008
       TEXT 'DSK1.SSE'
#endasm
 
gamwin()
{ /*load & run game win routine*/
shutup();
#asm
 LI R0,>3000
 LI R1,MX#1
MXLP#X MOV *R1+,*R0+
 CI R1,MX#2
 JNE MXLP#X
MX#1 LI R0,>0800
 LI R1,WN#DAT
 LI R2,20
 BLWP @VMBW
MX#LP
 LI R0,>0809
 MOV R0,@>8356
 BLWP @DSRLNK
 DATA 8
 LI R0,>1000
 LI R1,>2FF0
 LI R2,6
 BLWP @VMBR
 LI R0,>1006
 MOV @>2FF4,R1
 MOV @>2FF2,R2
 BLWP @VMBR
 LI R0,>1011
 BLWP @VSBR
 AI R1,>0100
 BLWP @VSBW
 MOV @>2FF0,R0
 MOV R0,R0
 JNE MX#LP
 B @>A000
MX#2
BSA BSS 2
BNR BSS 2
BNC BSS 2
#endasm
}
 
boss()
{ /* boss routine */
shutup();
x=(level-1)*20;
#asm
 LI R0,BOSTAB
 A @X,R0
 MOV *R0+,@BSA
 MOV *R0+,@BNR
 MOV *R0+,@BNC
 MOV R0,@QW
#endasm
for (a=0; a<3; a++)
{#asm
 MOV @QW,R0
 MOV *R0+,@X
 MOV R0,@QW
#endasm
 enr[a]=x+8;
#asm
 MOV @QW,R0
 MOV *R0+,@X
 MOV R0,@QW
#endasm
 enc[a]=x;
}
#asm
 MOV @QW,R0
 MOV *R0,@X
#endasm
 color(28,x,1);
#asm
 LI R0,>1500  * BOSSBM
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
#endasm
for (a=4; a<20; a++)
 spdel(a);
for (a=3; a<12; a++)
 ent[a]=0;
ptp4=0;
spposn(0,&r,&c);
while (c>112)
{ c=c-12; playmv(); stars(); delay(70); spposn(0,&r,&c);}
while (c<112)
{ c=c+12; playmv(); stars(); delay(70); spposn(0,&r,&c);}
while (r<144)
{ r=r+8; playmv(); stars(); delay(70); spposn(0,&r,&c); }
for (a=0; a<3; a++)
{ech[a]=76; esc[a]=76; eec[a]=80;
 ent[a]=10; ers[a]=0; ecs[a]=0;
 ep[a]=level*5;
 sprite(a+8,76,9,enr[a],enc[a]);
}
br=1; bc=0; bd=1;
drboss();
while (flag==1)
{ player();
  #asm
 LIMI 2
 LIMI 0
 #endasm
  mboss();
  stars();
#asm
 MOVB @>83CE,@>83CE
 JNE YAMY#X
 LI R0,>1500 *BOSSBM
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
YAMY#X
#endasm
 enemy();
 colchk();
 if (ch) cheat();
 if (!ch) plycol();
 whoded();
 if ((ep[0]==0)&&(ep[1]==0)&&(ep[2]==0))
  flag=0;
}
if (flag==0) byboss();
}
 
drboss()
{ /*draw boss ship*/
x=bc;
#asm
 MOV @BNR,R3
 MOV @BSA,R1
 MOV @X,R0
 AI R0,32
 MOV @BNC,R2
TIJAL
 BLWP @VMBW
 AI R0,32
 A @BNC,R1
 DEC R3
 JNE TIJAL
#endasm
for (a=0; a<3; a++)
 if (ep[a])
  sploct(a+8,enr[a],bc*8+enc[a]);
}
 
#asm
BLANK DATA >CECE,>CECE,>CECE,>CECE,>CECE
      DATA >CECE,>CECE,>CECE
#endasm
 
erboss()
{ /*erase boss*/
x=bc;
#asm
 MOV @BNR,R3
 MOV @X,R0
 AI R0,32
 LI R1,BLANK
 MOV @BNC,R2
TIYAL
 BLWP @VMBW
 AI R0,32
 DEC R3
 JNE TIYAL
#endasm
}
 
mboss()
{ /*boss control*/
erboss();
bc=bc+bd;
if (bc>17) bd=-1;
if (bc<1) bd=1;
drboss();
if ((level==3)||(level==5))
{ if (rnd(5)<2)
  {for (a=3; a<6; a++)
    if (ent[a]==0) { x=a; a=9;}
   if (a>=9)
   { ent[x]=3;
     enr[x]=62;
     enc[x]=bc*8+45;
     eec[x]=8; esc[x]=8; ech[x]=8;
     sprite(x+8,8,8,enr[x],enc[x]);
} }}
if (ent[8]==0)
 if (rnd(5)<3)
{ for (a=7; a<12; a++)
  { ech[a]=84; eec[a]=84; esc[a]=84;
    ers[a]=8; }
  switch (level)
{ case 1:ent[7]=7; ent[8]=7;
         enr[7]=56; enr[8]=enr[7];
         enc[7]=bc*8+16; enc[8]=bc*8+56;
         ecs[7]=0; ecs[8]=0;
         sprite(15,84,13,enr[7],enc[7]);
         sprite(16,84,13,enr[8],enc[8]);
         break;
  case 2:for (a=7; a<10; a++)
         { ent[a]=7; enr[a]=80;
           enc[a]=bc*8+36;
           enr[8]=88;
           sprite(a+8,84,13,enr[a],enc[a]); }
         ecs[7]=-4; ecs[8]=0; ecs[9]=4;
         break;
  case 4:for (a=7; a<11; a++)
       { ent[a]=7; enr[a]=88; }
       enr[8]=96; enr[9]=96;
       enc[7]=bc*8+24; enc[8]=enc[7]+8;
       enc[9]=enc[8]+8; enc[10]=enc[9]+8;
       ecs[7]=-4; ecs[8]=0; ecs[9]=0;
       ecs[10]=4;
       for (a=7; a<11; a++)
         sprite(a+8,84,13,enr[a],enc[a]);
       break;
  case 5:for (a=7; a<12; a++)
       { ent[a]=7; enr[a]=((abs(a-9)-1)*-8)+64; }
       enc[7]=bc*8+32; enc[8]=enc[7]+8;
       enc[9]=enc[8]+12; enc[10]=enc[9]+12;
       enc[11]=enc[10]+8; ecs[7]=-8;
       ecs[8]=-4; ecs[9]=0; ecs[10]=4;
       ecs[11]=8;
       for (a=7; a<12; a++)
         sprite(a+8,84,13,enr[a],enc[a]);
}}}
 
whoded()
{ /*check boss specific collisions*/
for (a=0; a<3; a++)
  for (b=0; b<3; b++)
{ x=bc*8+enc[b];
   k=abs(enr[b]-shr[a]); s=abs(x-shc[a]);
   if ((k<=20)&&(s<=15)&&(shr[a]>0))
   {  screen(16);
      ep[b]=ep[b]-(pwrlvl+1)*2;
      if (ep[b]<=0) { ep[b]=0; spdel(b+8);}
      spdel(a+5); shr[a]=0; b=2;
      screen(2);
   }
}}
 
byboss()
{ /*boss is dead...blow him up!*/
flag=1;
shutup();
for (a=5; a<20; a++)
  spdel(a);
sppat(0,108); sppat(1,112); sppat(2,116);
sppat(3,184); sppat(21,188); sppat(22,192);
sppat(23,196); sppat(24,200);
for (qw=0; qw<290; qw++)
{ x=qw/20;
#asm
 LI R0,>E500
 MOVB R0,@>8400
 LI R0,>893F
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
 LI R0,>A03F
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
 LI R0,>C73E
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
 MOV @X,R0
 SWPB R0
 AI R0,>90B0
 A @X,R0
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
 MOV @X,R0
 SWPB R0
 AI R0,>D0F0
 A @X,R0
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
#endasm
 r=rnd(12); c=rnd(14);
 x=gchar(br+r,bc+c);
 if (x>226) hchar(br+r,bc+c,255,1);
 if (x==255) hchar(br+r,bc+c,206,1);
 if ((qw/3*3)==qw) stars();
}
 erboss();
 shutup();
 for (qw=0; qw<12; qw++)
   stars();
 for (qw=0; qw<61; qw++)
{ x=(qw*10+2000)/10;
  x=11186/x;
  a=x%16;
#asm
 MOV @X,R0
 SRL R0,4
 MOV @A,R1
 SLA R1,8
 A R1,R0
 AI R0,>C000
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
 LI R0,>E700
 MOVB R0,@>8400
 LI R0,>DFF0
 MOVB R0,@>8400
 SWPB R0
 MOVB R0,@>8400
#endasm
 sppat(20,100);
 spposn(0,&r,&c);
 if ((r<192)||(r>200))
{ r=r-6; playmv(); }
 else spdall();
 stars();
}
 flag=0;
}
