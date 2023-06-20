/* SSA game Win Routine! DSK1.SSE */
 
#include "dsk5.earefs/c"
#include "dsk5.grf1rf"
 
#asm
H01    BYTE >01
       EVEN
FINALM DATA >098C,>1A90,>AB1A,>B0CB,>23D0
       DATA >1805,>8D17,>AC17,>D118,>058C
       DATA >1AAB,>1AD2,>1805,>8D11,>AC11
       DATA >D018,>058C,>1AAB,>1AD1,>1801
       DATA >D218,>058D,>17AC,>17D3,>1805
       DATA >8C1A,>AB1A,>D418,>058D,>17AC
       DATA >17D5,>1805,>8C1A,>AB1A,>D618
       DATA >098E,>0F90,>AD0F,>B0C7,>2AD0
       DATA >1805,>8C1A,>AB1A,>D118,>01D2
       DATA >1805,>8D17,>AC17,>D318,>0780
       DATA >14AF,>13C1,>28D0,>1805,>8315
       DATA >A215,>D118,>058D,>17AC,>17D2
       DATA >1805,>8C1A,>AB1A,>D318,>01D3
       DATA >0000
LASNOT DATA >098C,>1A90,>AD1A,>B0CB,>23D0
       DATA >2D04,>9FBF,>DFFF,>0000
#endasm
 
char out[33];
char *txt;
int flag=1;
 
main()
{ int a;
  grf1();
#asm
 LI R0,>2000
 LI R1,FINALM
 LI R2,140
 BLWP @VMBW
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
 LI R0,TXTDAT
 MOV R0,@TXT
#endasm
chrdef(61,"0000ff55aaff0000");
chrdef(108,"081e3e203c3f3fea350a050c07070307");
chrdef(110,"00000000c0e0c000709040f0f0fedf87");
chrdef(96,"007c829aa29a827c191c4f1fbe2daf2f");
chrdef(98,"0000c0e030b1fffeaf37b91e5f1f4f0f");
chrdef(100,"fff18000e0f8fcfe2f0f27071707270f");
chrdef(102,"ffffffffffffffff0f0f1f1f1f3f3f3e");
chrdef(104,"fefcf0c0801c1e0e3e3e1e1f1f0f0703");
chrdef(106,"0e0f0f1ffffefcf8");
screen(2);
for (a=0; a<32; a++)
 color(a,16,1);
while (flag)
{ read();
  centr();
  pause();
  if (*txt=='@') flag=0;
}
#asm
MUS#L
 LIMI 2
 LIMI 0
 MOVB @>83CE,@>83CE
 JNE MUS#L
 LI R0,>2000
 LI R1,LASNOT
 LI R2,20
 BLWP @VMBW
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
MUS#L2
 LIMI 2
 LIMI 0
 JMP MUS#L2
#endasm
}
 
read()
{/*get from the text data to out[]*/
int c;
c=0;
while (*txt!=33)
 out[c++]=*txt++;
out[c]=0;
txt++;
}
 
centr()
{/* print&center the text in out[]*/
int c,a;
c=16-(strlen(out)/2);
a=-1;
scroll();
while (out[++a])
 hchar(24,c++,out[a],1);
}
 
strlen(a) char a[];
{/*return length*/
int x;
x=0;
while (a[x++]);
return(x);
}
 
scroll()
{/*'slow' scroll with interrupts*/
#asm
 LI R0,32
MY#1
 LI R1,BUF
 LI R2,32
 BLWP @VMBR
 AI R0,-32
 BLWP @VMBW
 LIMI 2
 LIMI 0
 AI R0,64
 CI R0,768
 JL MY#1
 LI R0,736
 LI R1,>2000
 LI R2,32
MY#2
 BLWP @VSBW
 INC R0
 DEC R2
 JNE MY#2
 LIMI 2
 LIMI 0
 MOVB @>83CE,@>83CE
 JNE MY#3
 LI R0,>2000
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
MY#3
#endasm
}
 
pause()
{/*slow it down*/
int a;
 for (a=0; a<1500; a++)
#asm
 LIMI 2
 LIMI 0
 MOVB @>83CE,@>83CE
 JNE MY#4
 LI R0,>2000
 MOV R0,@>83CC
 SOCB @H01,@>83FD
 MOVB @H01,@>83CE
MY#4
#endasm
}
 
#asm
BUF BSS 32
TXTDAT
 TEXT '==============================!!'
 TEXT 'CONGRADULATIONS!!'
 TEXT 'YOU HAVE DEFEATED!!'
 TEXT 'THE EVIL QWERTIANS!!'
 TEXT 'THE GALAXY IS SAFE!!'
 TEXT 'YOU ARE A TRUE HERO!!'
 TEXT '==============================!!'
 TEXT 'STAFF!!'
 TEXT '==============================!!'
 TEXT 'IDEA!!'
 TEXT '====!!'
 TEXT 'MIKE WARD!!'
 TEXT 'GORDON HADDRELL!!'
 TEXT '==============================!!'
 TEXT 'PROGRAM!!'
 TEXT '=======!!'
 TEXT 'MIKE WARD!!'
 TEXT 'WRITTEN IN C99!!'
 TEXT '==============================!!'
 TEXT 'GRAPHICS AND SOUND!!'
 TEXT '==================!!'
 TEXT 'MIKE WARD!!'
 TEXT 'GORDON HADDRELL!!'
 TEXT 'STEVE BRENT!!'
 TEXT 'JUSTIN GODDARD!!'
 TEXT '==============================!!'
 TEXT 'EXTRA THANKS!!'
 TEXT '============!!'
 TEXT 'JASON JONES!!'
 TEXT 'DARREN CRAWFORD!!'
 TEXT 'CHRIS JONANSEN!!'
 TEXT 'STEVE BURNS!!'
 TEXT 'LLOYD GALENZOSKI!!'
 TEXT 'SYSOPS OF:!!'
 TEXT 'TI SOURCE (TEXAMENTS)!!'
 TEXT '(516)475-6463!!'
 TEXT 'DIAL-A-TI!!'
 TEXT '(604)522-9830!!'
 TEXT 'WEBBS!!'
 TEXT '(DOWN)!!'
 TEXT '==============================!!'
 TEXT 'STARTED: JUNE 1990!!'
 TEXT 'FINISHED: MAY 1992!!'
 TEXT '==============================!'
 TEXT '!'
 TEXT '!'
 TEXT 'SUPER SPACE ACER!!'
 TEXT 'BY MIKE WARD!'
 BYTE 96
 TEXT ' 1992 BY JULIUS SOFTWARE!'
 TEXT '!'
 TEXT 'ab                     !'
 TEXT 'cd SEAHORSE    JULIUSln!'
 TEXT 'ef SOFTWARE  SOFTWAREmo!'
 TEXT 'gh                     !'
 TEXT 'ij                     !'
 TEXT '!'
 TEXT '!'
 TEXT 'JULIUS SOFTWARE!'
 TEXT 'C/O MIKE WARD!'
 TEXT '70-1406 MCEWEN AVE!'
 TEXT 'OTTAWA, ONT!'
 TEXT 'K2B 5M3!'
 TEXT '!'
 TEXT '(PRESS QUIT TO EXIT)!!'
 TEXT '==============================!'
 TEXT '@'
#endasm
 
