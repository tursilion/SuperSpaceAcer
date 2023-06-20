/*MAKE FILE 'SSD'*/
#include "dsk5.grf1rf"
 
#asm
       REF VMBW,DSRLNK
       COPY "DSK1.DEFS"
       COPY "DSK1.MOREDEFO"
PAB    DATA >0600,>0800,>0000,>0F00,>0008
       TEXT 'DSK1.SSD'
#endasm
 
int r,c,a;
 
main()
{ grf1();
r=10; c=1;
for (a=0; a<256; a++)
{ hchar(r,c++,a,1);
  if (c==33) {c=1; r++;} }
patcpy(71,239);
patcpy(65,240);
patcpy(77,241);
patcpy(69,242);
patcpy(32,243);
patcpy(79,244);
patcpy(86,245);
patcpy(69,246);
patcpy(82,247);
#asm
       LI R0,>800
       LI R1,SAUCER
       LI R2,1768
       BLWP @VMBW
       LI R0,>1000
       LI R1,STOPAM
       LI R2,8
       BLWP @VMBW
       LI R0,>1100
       LI R1,INTROM
       LI R2,110
       BLWP @VMBW
       LI R0,>1200
       LI R1,MAINBM
       LI R2,348
       BLWP @VMBW
       LI R0,>1400
       LI R1,GAMOVM
       LI R2,120
       BLWP @VMBW
       LI R0,>1500
       LI R1,BOSSBM
       LI R2,160
       BLWP @VMBW
       LI R0,>1600
       LI R1,FINALM
       LI R2,140
       BLWP @VMBW
#endasm
patcpy(206,255);
patcpy(216,231);
patcpy(217,230);
patcpy(218,229);
patcpy(219,228);
patcpy(220,227);
chrdef(206,"00");
patcpy(206,249);
patcpy(249,250);
patcpy(250,251);
#asm
       LI R0,>2000
       LI R1,PAB
       LI R2,20
       BLWP @VMBW
       LI R0,>2009
       MOV R0,@>8356
       BLWP @DSRLNK
       DATA 8
#endasm
}
