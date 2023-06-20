/* Super Space Acer DX */

/* Lots of trickery in here... how else to alter the behaviour of a */
/* game you have no source code for? */

#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include <jgmod.h>
#include "tiemul.h"

struct {
        int fire,start;
        int up,down,left,right;
        } snes;
int base;

#define RAMFILES 11
char RAMDISK[RAMFILES][10000]; /* blocks of 10k */
int  RAMSIZE[RAMFILES];
char RAMFAT[RAMFILES][80]={    "DSK1\\ACER_C", "DSK1\\ACER_P", "DSK1\\BOOT",
                        "DSK1\\SSC", "DSK1\\SSD", "DSK1\\SSE",
                        "DSK1\\SSA", "DSK1\\SSB", "MODS\\SSA_ROM.BIN",
                        "MODS\\AMI99DSK.BIN", "TIDATA" };

char misctext[80]=" ";
char qw[80];  /* temporary string */

char quitflag;  /* just what it says */

int redraw_needed; /* set after write access to VDP RAM or register */

int interrupt_needed;	/* set if we miss a VDP interrupt due to masking */

Byte CPU[65536], CPU2[8192], GROM[65536];
Byte VDP[16384], VDPREG[9];
/*   CPU m6emory- XB bank 2-  GROM memory- VDP memory- VDP registers */

Byte ROMMAP[65536], CRU[65536];
/* map of which CPU bytes are writeable, the CRU area */

Word PC, WP, X_flag;
/* program counter and workspace pointer , and X_flag for 'X' */

int bank=0,xb=0;
/* which bank is active, and is XB even loaded? */

Word ST;
/* status register. bit definitions follow */

int num_sprites,y_speed,x_speed,t1,t1a,t1b,t2,t2a,t2b,sprite_y,sprite_x,offset;
int ty,tx;

int snd_address;
Byte *sndlist, data;
/* used in console interrupt simulator */

int need_byte, last_byte;
/* used in sound routine */

int KEYS[8][8]= { 
/* Joy 2 */		KEY_ESC, KEY_ESC, KEY_ESC, KEY_ESC, KEY_ESC, KEY_ESC, KEY_ESC, KEY_ESC,

/* 0 */			KEY_M, KEY_J, KEY_U, KEY_7, KEY_4, KEY_F, KEY_R, KEY_V,
/* 1 */			KEY_SLASH, KEY_COLON, KEY_P, KEY_0, KEY_1, KEY_A, KEY_Q, KEY_Z,
/* 2 */			KEY_STOP, KEY_L, KEY_O, KEY_9, KEY_2, KEY_S, KEY_W, KEY_X,

/* Joy 1 */		KEY_TAB, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_ESC, KEY_ESC, KEY_ESC,
	
/* 3 */			KEY_COMMA, KEY_K, KEY_I, KEY_8, KEY_3, KEY_D, KEY_E, KEY_C,
/* 4 */			KEY_N, KEY_H, KEY_Y, KEY_6, KEY_5, KEY_G, KEY_T, KEY_B,
/* 5 */			KEY_EQUALS, KEY_SPACE, KEY_ENTER, KEY_ESC, KEY_ALT, KEY_LSHIFT, KEY_LCONTROL, KEY_ESC 
};


#define ST_LGT (ST&0x8000)
#define ST_AGT (ST&0x4000)
#define ST_EQ  (ST&0x2000)
#define ST_C   (ST&0x1000)
#define ST_OV  (ST&0x0800)
#define ST_OP  (ST&0x0400)
#define ST_X   (ST&0x0200)
#define ST_INTMASK (ST&0x000f)

#define set_LGT (ST=ST|0x8000)
#define set_AGT (ST=ST|0x4000)
#define set_EQ  (ST=ST|0x2000)
#define set_C   (ST=ST|0x1000)
#define set_OV  (ST=ST|0x0800)
#define set_OP  (ST=ST|0x0400)
#define set_X   (ST=ST|0x0200)

#define reset_LGT (ST=ST&0x7fff)
#define reset_AGT (ST=ST&0xbfff)
#define reset_EQ  (ST=ST&0xdfff)
#define reset_C   (ST=ST&0xefff)
#define reset_OV  (ST=ST&0xf7ff)
#define reset_OP  (ST=ST&0xfbff)
#define reset_X   (ST=ST&0xfdff)

Word VDPADD, GRMADD;   /* VDP and GROM address registers */

Word VDPST;           /* VDP status register */

Byte grmaccess,grmdata; /* used with GROM emulation (prefetch) */

Byte vdpaccess;         /* used with VDP address access */

unsigned int idxx;    /* general counter variable */

Word in,D,S,Td,Ts,B;   /* opcode in, and the breakdown of the opcode */

int debug,d2;         /* debugging flags */

int drawspeed=0;            /* flag for display drawing */
int kscan=0;				/* use KSCAN speed up */
int max_ipf=9999999;		/* 10 million instructions per frame.. way faster than my PC ;) */
int instruction_count;		/* used in speed control */

int cpucount, cpuframes;  /* used for instructions/second */
int last_count=0;				/* used to separate video redraws and VDP ints */
int timercount;           /* used to guestimate run-time */
int single_step=0;
int breakpoint=-1;		  /* breakpoint address */

void (*opcode[65536])(void); /* array for CPU table */

int ssaState;           /* game state */
int credits;            /* credits */
int attracttime;        /* attract timer */
int speedcount=0;
int score=0;
int NOSNESPAD=0;        /* set to disable SNES pads */
int ShipExplodeSound=0;
int dummy_voice;        /* used to pad modplayer */
int WatchForFlyOff=0;
int oldY=0;             /* used for fly off sound */
int attractXdir=0;
int attractYdir=0;      /* used to smooth out attract mode */
int attractLevel=1;     /* level to play demo on (1-5) */

JGMOD *MOD_Background, *MOD_Boss, *MOD_End, *MOD_GameOver;
SAMPLE *PlayerExplodes, *PlayerWarps, *BossExplodes, *ShipExplodes;
SAMPLE *CreditIn;
BITMAP *TitlePic;
RGB TitlePal[256];

#include "tivdp.c"

int main(int argc, char *argv[])
{
        FILE *fp;
        int idx, cnt;

                credits=0;      /* don't lose this value on reboot */

                setlpt(1);

		allegro_init();

		install_keyboard();
		install_timer();

                keyboard_callback=keycall;

                /* fill in the RAMdisk to prevent disk thrashing */
                for (idx=0; idx<RAMFILES; idx++) {
                  fp=fopen(&RAMFAT[idx][0], "rb");
                  if (fp) {
                    cnt=fread(&RAMDISK[idx][0], 1, 10000, fp);
                    RAMSIZE[idx]=cnt;
                    fclose(fp);
                    printf("Loaded %4d bytes of %s\n", cnt, &RAMFAT[idx][0]);
                  } else {
                    printf("* Couldn't load %s\n", &RAMFAT[idx][0]);
                  }
                }

                TitlePic=load_pcx("SSATITLE.PCX", TitlePal);

		buildcpu();

		startsound();
                wsndbyte(0x9f);         /* silence sound */
                wsndbyte(0xbf);
                wsndbyte(0xdf);
                wsndbyte(0xff);

		VDPBUFFER=SPRITEBUFFER=SCREENBUFFER=0;

		startvdp();

                readpad(0);     /* prime the SNES pad so it autodetects */
		
RESTART:
                ssaState=0;     /* booting */

		grmaccess=0;
		vdpaccess=0; /* no VDP address writes yet */
		quitflag=1;  /* no quit */
		interrupt_needed=0;	/* ok so far */

		/* clear memory banks */

                for (idxx=0; idxx<65536; idxx++)
                  CPU[idxx]=0;
                for (idxx=0; idxx<8192; idxx++)
                  CPU2[idxx]=0;
                for (idxx=0; idxx<65536; idxx++)
                  GROM[idxx]=0;
                for (idxx=0; idxx<16384; idxx++)
                  VDP[idxx]=0;
                for (idxx=0; idxx<4096; idxx++)
                  CRU[idxx]=1;

		debug=0;

		readroms();

		/* that SHOULD set up the PC. now go do the TI stuff! */

		cpucount=0;
		cpuframes=0;
		timercount=0;

		emulti();

                goto RESTART;
}

void startsound()
{ /* start up the sound files */
	int i;
	char buf[80];

	int detect_digi_driver(int driver_id);

        reserve_voices(32,-1);

	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
        {       reserve_voices(-1,-1);
                if (install_sound(DIGI_NONE, MIDI_NONE, NULL) != 0)
			fail("Can't install sound");
	}

        MOD_Background=NULL;
        MOD_Boss=NULL;
        MOD_End=NULL;
        MOD_GameOver=NULL;
        if (install_mod(16) >=0 ) {
          MOD_Background=load_mod("aryx.s3m");
          MOD_Boss=load_mod("thrash.mod");
          MOD_End=load_mod("Acoustic.mod");
          MOD_GameOver=load_mod("Aftert.mod");
          set_mod_volume(200);
        }

	set_volume(255, -1);	/* max digi volume, ignore midi */

        dummy_voice=-1;
        PlayerExplodes=PlayerWarps=BossExplodes=ShipExplodes=NULL;
        CreditIn=NULL;

        PlayerExplodes=load_sample("PlayEx.wav");
        PlayerWarps=load_sample("PlayWarp.wav");
        BossExplodes=load_sample("BossEx.wav");
        ShipExplodes=load_sample("ShipEx.wav");
        CreditIn=load_sample("Credit.wav");

        dummy_voice=allocate_voice(PlayerWarps);

	need_byte=0;
	last_byte=0;
}

void startvdp()
{ /* set up VDP program */
VDPmain();
retrace_count=0;
}

void warn(x) char x[];
{ /* ***eventually*** pop up a warning window with the PC */
fail(x);
}

void fail(x) char x[];
{ /* report a fatal error */
char buffer[1024];

if (is_mod_playing()) stop_mod();

if (VDPBUFFER) destroy_bitmap(VDPBUFFER);
if (SPRITEBUFFER) destroy_bitmap(SPRITEBUFFER);
if (SCREENBUFFER) destroy_bitmap(SCREENBUFFER);
if (MOD_Background) destroy_mod(MOD_Background);
if (MOD_Boss) destroy_mod(MOD_Boss);
if (MOD_End) destroy_mod(MOD_End);
if (MOD_GameOver) destroy_mod(MOD_GameOver);
if (-1 != dummy_voice) deallocate_voice(dummy_voice);
if (PlayerExplodes) destroy_sample(PlayerExplodes);
if (PlayerWarps) destroy_sample(PlayerWarps);
if (BossExplodes) destroy_sample(BossExplodes);
if (ShipExplodes) destroy_sample(ShipExplodes);
if (TitlePic) destroy_bitmap(TitlePic);

allegro_exit();

if (x[0]!=0)
{
  printf("\n%s\n",x);
}

exit(0);
}

Word romword(x) Word x;
{ /* returns a word from ROM */
  x&=0xfffe;              
  return((Word)((rcpubyte(x)<<8)+rcpubyte(x+1)));
}

void wrword(x,y) Word x,y;
{ /* write a word to RAM */
  x&=0xfffe;
  wcpubyte(x,(Byte)(y>>8));
  wcpubyte(x+1,(Byte)(y&0xff));
}

void emulti()
{ /* emulate a TI99/4a */
/* First, read in the ROM stuff, using the data file TIDATA */
static int watch=0xa000;

WP=romword(0);
PC=romword(2);
X_flag=0;  /* not doing an 'X' right now */
quitflag=1;
while (quitflag)
{ 
  do1();

//  sprintf(misctext, "%04x=%04x %04x=%04x %04x=%04x", watch,romword(watch), watch+2, romword(watch+2), watch+4, romword(watch+4));
//  if (key[KEY_UP]) { watch+=6; while (key[KEY_UP]);}
//  if (key[KEY_DOWN]) { watch-=6; while (key[KEY_DOWN]); }

  if (key[KEY_ESC]) quitflag=0;
  if (key[KEY_F1])
  {
      BITMAP *bmp;
      PALETTE pal;

      get_palette(pal);
      bmp = create_sub_bitmap(screen, 0, 0, 320, 240);
      save_bitmap("dump.pcx", bmp, pal);
      destroy_bitmap(bmp);
  }

  if (quitflag==-1) break;
}

wsndbyte(0x9f);         /* silence sound */
wsndbyte(0xbf);
wsndbyte(0xdf);
wsndbyte(0xff);

BossMusicPlaying=0;

if (quitflag==0) fail("");       

}

void readroms()
{ /* this routine reads TIDATA, and uses it to load the read-only stuff */
char *fp;
char *fp2;
char type;
Byte c;
unsigned short address, length;

/* set all CPU memory as RAM */
for (idxx=0; idxx<65536; idxx++)
  ROMMAP[idxx]=0;

fp=GetPtr("TIDATA");
if (fp==NULL) fail("Can't open TIDATA file!");

while (fp)
{ fp=RAMfgets(qw,80,fp);
  if (qw[0]=='#')
  {
	  if (strncmp(qw,"#FRAMESKIP",10) == 0)
		drawspeed=atoi(&qw[10]);

	  if (strncmp(qw,"#KSCAN",6) == 0)
		  kscan=atoi(&qw[6]);

	  if (strncmp(qw,"#MAXIPF",7) == 0)
		  max_ipf=atoi(&qw[7]);

	  if (strncmp(qw,"#DEBUG",6) == 0)
	  {  debug=atoi(&qw[6]);
		 single_step=debug;
	  }
  }
  else
  { if ((fp!=NULL)&&(qw[0]!=';'))
    { type=qw[0];
      address=hex2dec(&qw[2]);
      length=hex2dec(&qw[7]);
      if ((type!='C')&&(type!='X')&&(type!='G')&&(type!='V')&&(type!='!'))
      { goto skipload;
      }
      idxx=12;
      while (qw[++idxx]>31);
        qw[idxx]=0;
      fp2=GetPtr(&qw[12]);
      if (fp2==NULL) fail("Can't open specified memory file!");
      
      if (qw[6]=='-')
      { 
        for (idxx=0; idxx<128; idxx++)
          fp2++;   /* skip TIFILES header (128 bytes) */
      }
      if (qw[11]=='-')
      { 
        for (idxx=0; idxx<6; idxx++)
          fp2++;   /* skip 6 bytes */
      }
      for (idxx=0; idxx<length; idxx++)
      { c=*(fp2++);
        switch(type)
        { case 'C': ROMMAP[address+idxx]=1;
                  case '!':     CPU[address+idxx]=c;
                    break;
          case 'X': CPU2[(address-0x6000)+idxx]=c;
                    xb=1;
                    break;
          case 'G': GROM[address+idxx]=c;
                    break;
          case 'V': VDP[address+idxx]=c;
                    break;
        }
      }
    skipload: ;
    }
  }
}
}

Word hex2dec(x) char x[];
{ /* convert 4 digit hex to decimal */
Word z,y,i,j;

z=0; j=4096;
for (i=0; i<4; i++)
{ y=x[i]-48;
  if (y>9) y=y-7;
  z=z+(y*j);
  j=j/16;
}
return(z);
}

void do1()
{ /* do one instruction */
Byte x1;
int t1,t2;

cpucount++;

/******* check for video *****/
if ((retrace_count>drawspeed)||(interrupt_needed))
{
  if (retrace_count>drawspeed)
  {
    VDPdisplay();                                                                 /* this skews the accounting a bit, but */
    redraw_needed=0;								/* should help speed a lot.             */
    cpuframes++;
    last_count=retrace_count;
    while (retrace_count==last_count);
  }

  timercount+=retrace_count;
  if (retrace_count>drawspeed)
     retrace_count=0;
  last_count=retrace_count;

  instruction_count=0;

  VDPST |= 0x80;                        

  if (((ST&0x000f)==0)&&(VDPREG[1]&0x20))		/* if interrupt mask off and VDP ints on */
  {	  interrupt_needed=1;						/* we're going to miss this int, try when we can*/
  }
  else
  {	/* Simulate console interrupt routine */
	interrupt_needed=0;
	x1=CPU[0x83c2];
	if ((x1&0x80)==0)				/* if console routine on */
	{ /* console routine is active */
	    if ((x1&0x40)==0)
	    {	/* update sprites */

			/* the table offsets are hardcoded in the real TI		*/
			for (num_sprites=0; num_sprites<(CPU[0x837a]); num_sprites++)
			{	offset=num_sprites<<2;
				y_speed=(Byte)VDP[0x780+offset];
				x_speed=(Byte)VDP[0x781+offset];
				t1=(Byte)VDP[0x782+offset];
				t2=(Byte)VDP[0x783+offset];
				t1a=(y_speed&0xf0)>>4;
				t1b=(y_speed&0xf);
				t2a=(x_speed&0xf0)>>4;
				t2b=(x_speed&0xf);
			
				if (t1a>0x7)
				{	t1a-=0xf;
					if (t1b==0)
						t1a-=1;
				}
				if (t2a>0x7)
				{	t2a-=0xf;
					if (t2b==0)
						t2a-=1;
				}
	
				t1b=(y_speed&0xf);
				if ((y_speed>0x7f)&&(t1b))
					t1b=(t1b-0x10);
				if ((x_speed>0x7f)&&(t2b))
					t2b=(t2b-0x10);

				sprite_y=(Byte)VDP[0x300+offset];
				sprite_x=(Byte)VDP[0x301+offset];
				
				ty=sprite_y;
				tx=sprite_x;
			
				t1+=t1b;
				t2+=t2b;

				sprite_y+=t1a;
				sprite_x+=t2a;
	
				if (t1>0xf)
				{	t1-=0xf;
					sprite_y++;
				}
				if (t1<0)
				{	t1+=0xf;
					sprite_y--;
				}

				if (t2>0xf)
				{	t2-=0xf;
					sprite_x++;
				}
				if (t2<0)
				{	t2+=0xf;
					sprite_x--;
				}

				sprite_x&=0xff;
				sprite_y&=0xff;

				if ((sprite_y>=0xc0)&&(sprite_y<=0xe0))
				{	if (y_speed>0x7f)
						sprite_y-=0x20;
					else
						sprite_y+=0x20;
				}
	
				VDP[0x300+offset]=(Byte)sprite_y;
				VDP[0x301+offset]=(Byte)sprite_x;
				VDP[0x782+offset]=(Byte)t1;
				VDP[0x783+offset]=(Byte)t2;
			
				if ((ty!=sprite_y)||(tx!=sprite_x))
					redraw_needed=1;

			}
		}
    
            if ((x1&0x20)==0)
	    {
              /* update sound list */
	      if (CPU[0x83ce])
              {
                CPU[0x83ce]--;
                if (CPU[0x83ce]==0)
                {
                  /* update sound list */
                  snd_address=romword(0x83cc);
                  if (CPU[0x83fd] & 0x01)
                  {
                    /* sound list is in VDP RAM */
                    sndlist=(&VDP[snd_address]);
                  } else {
                    /* sound list is in GROM */
                    sndlist=(&GROM[snd_address]);
                  }
                  data=*sndlist++;
                  snd_address+=data;
                  if ((data!=0)&&(data!=0xff))
                  {
                    while (data--)
                    {
                      wsndbyte(*sndlist++);
                    }
                    data=*sndlist++;
                    snd_address+=2;
                    wrword(0x83cc,snd_address);
                    CPU[0x83ce]=data;
                  } else {
                    if (data==0xff)
                    {
                      if (CPU[0x83fd] & 0x1)
                        CPU[0x83fd]&=0xfe;
                      else
                        CPU[0x83fd]|=0x01;
                    }
                    CPU[0x83cc]=*sndlist++;
                    CPU[0x83cd]=*sndlist++;
                    CPU[0x83ce]=0x01;
                  }
                }
              }
	    }
	
	}

	/* the following parts can't be turned off, except by LIMI or disabling VDP ints */
  
  	wrword(0x83d6,romword(0x83d6)+2);	/* this blanks the screen when it */
										/* wraps around to zero */
	
	CPU[0x8379]+=CPU[0x83fc];			/* VDP Int Timer - not sure how it	*/
										/* gets this, but here it is. Add	*/
										/* byte from R14 of the GPL WP at	*/
										/* >83e0. Can't trace what it is.	*/
										/* E/A manual calls it System Status*/

	CPU[0x837b]=VDPST;		/* VDP Status register - copied to CPU RAM		*/
                                        /* This is where global sprite COINC goes               */
  }

}

/******* check for credits *****************/
if (credits<0) credits=0;

/******* state change?  ********************/
if ((ssaState==1)&&(VDPREG[1]&0x40)) {
  ssaState=2; /* title page */
  attracttime=0;
}

/* waiting for Game Over to end */
if (ssaState==5) {
  if ((MOD_End)&&(!is_mod_playing())) quitflag=-1;
  if ((NULL==MOD_End)&&(rcpubyte(0x83ce)==0)) quitflag=-1;
  if (credits>0) {
    readpad(0);
    if ((snes.start)||(key[KEY_1])) quitflag=-1;
  }
}

if (ssaState==7) {
  /* game win routine - reboot on fire or end of music */
  if ((MOD_GameOver)&&(!is_mod_playing())) quitflag=-1;
  if ((NULL==MOD_GameOver)&&(rcpubyte(0x83ce)==0)) quitflag=-1;
  readpad(0);
  if ((key[KEY_1])||(snes.start)) quitflag=-1;
}

/*** cheats! */
//if (romword(0xa292)<2) wrword(0xa292,2);      // inf lives
//if ((!key[KEY_A])&&(romword(0xa400)<2)) wrword(0xa400,100);    // inf shield

/** Attract mode hacks **/
/* Infinite lives and level jump is used in attract mode */
if (ssaState==3) {
  wrword(0xa292,2);
  wrword(0xa294,attractLevel);
}

/******* speed hack - one frame of SSA **********/
if ((ssaState<6)&&(PC==0xb20a))         
{
if (retrace_count+timercount<speedcount+4) return;
speedcount=timercount+retrace_count;
}

if ((ssaState==7)&&(PC==0xa4ec))        
{
if (retrace_count+timercount<speedcount+4) return;
speedcount=timercount+retrace_count;
}

/******* intercept patch for KSCAN *********/
/* Necessary for SSA due to lack of ROM */

if (PC==0x000e)       /* intercept KSCAN here */
{
        /* check keyboard. This is not even close to correct behaviour ;) */

	CPU[WP]=0xff;					/* set R0 (byte) with keycode */
	CPU[WP+12]=0;					/* CLR R6 (byte) */
	CPU[0x8375]=0xff;
	CPU[0x8376]=0;
	CPU[0x8377]=0;

        if (CPU[0x8374]!=1) goto skipScan;              /* we only want joystick scans */

        if (ssaState==4) {                              /* only if ingame */
                /* throw away mode 0/5 scanning */
                if (CPU[0x8374]==1)                             /* Joystick hack ;) */
                {
                        readpad(0);
                        if ((key[KEY_TAB])||(snes.fire))
                        {       CPU[0x8375]=18;                 /* fire */
                                VDPREG[8]=18;
                                CPU[WP]=18;
                                CPU[WP+12]=0x20;
                                wrword(0x83d6,0);
                        }
        
                        if ((key[KEY_UP])||(snes.up))
                                CPU[0x8376]=0x04;
                        if ((key[KEY_DOWN])||(snes.down))
                                CPU[0x8376]=0xfc;
                        if ((key[KEY_LEFT])||(snes.left))
                                CPU[0x8377]=0xfc;
                        if ((key[KEY_RIGHT])||(snes.right))
                                CPU[0x8377]=0x04;
                }
        }
        if ((ssaState==2)&&(credits>0)) { 
                readpad(0);
                if ((key[KEY_1])||(snes.start))
                {       CPU[0x8375]=18;          
                        VDPREG[8]=18;
                        CPU[WP]=18;
                        CPU[WP+12]=0x20;
                        wrword(0x83d6,0);
                        credits--;
                        ssaState=4;              /* enable controls */
                        score=0;
                        speedcount=timercount+retrace_count;
                }
        }
        if (ssaState==2) {
          attracttime++;
          if (attracttime>50000) {
            attracttime=0;
            ssaState=3;
            attractLevel=rand()%5+1;
            speedcount=timercount+retrace_count;
            CPU[0x8375]=18;                 /* fire */
            VDPREG[8]=18;
            CPU[WP]=18;
            CPU[WP+12]=0x20;
            wrword(0x83d6,0);
          }
        }
        if (ssaState==3) {
                readpad(0);
                if ((credits>0)&&((key[KEY_1])||(snes.start))) quitflag=-1;

                if (CPU[0x8374]==1)                             /* Joystick hack ;) */
                { 
                        if (rand()&0x01)
                        {       CPU[0x8375]=18;                 
                                VDPREG[8]=18;
                                CPU[WP]=18;
                                CPU[WP+12]=0x20;
                                wrword(0x83d6,0);
                        }

                        attractXdir+=(rand()%5)-2;
                        if ((attractXdir>10)||(attractXdir<-10)) attractXdir=0;
                        attractYdir+=(rand()%5)-2;
                        if ((attractYdir>10)||(attractYdir<-10)) attractYdir=0;

                        if (attractXdir<-2)
                          CPU[0x8376]=0xfc;
                        if (attractXdir>2)
                          CPU[0x8376]=0x04;
                        if (attractYdir<-2)
                          CPU[0x8377]=0xfc;
                        if (attractYdir>2)
                          CPU[0x8377]=0x04;
                }

          attracttime++;
          if (attracttime>1500) {
            attracttime=0;
            quitflag=-1;        /* restart */
          }
        }

skipScan:
	PC=romword(WP+22);				/* B *R11 - continue here */

}

/******* intercept patch for Ami99 DSK1 emulation ***********/
if (PC==0x4800)						/* special intercept address for emulated ROM */
{
	do_dsrlnk();
        PC=romword(WP+22)+2;                    /* return address in R11.. TI is a bit silly */
                                                /* and if we don't increment by 2, it's considered */
                                                /* an error condition */
}

/***** end of patches *****/

instruction_count++;

if (instruction_count<max_ipf)
{
	if (X_flag==0)
	{ in=romword(PC);    /* ie: not for an 'x' instruction */
          PC=PC+2;           /* thanks to Jeff Brown for explaining that! */
	}

	/* 'in' now has an instruction to execute */
	(*opcode[in])();
}
} /* end function */


Byte rcpubyte(x) Word x;
{ /* read a byte from CPU memory */
/* TI CPU memory map
>0000 - >1fff  Console ROM
>2000 - >3fff  Low bank RAM
>4000 - >5fff  DSR ROMs
>6000 - >7fff  Cartridge expansion
>8000 - >9fff  Memory-mapped devices & CPU scratchpad
>a000 - >ffff  High bank RAM

All is fine and dandy, except the memory-mapped devices, and the
fact that writing to the cartridge port with XB in places causes
a bank switch. In this emulator, that will only happen if bank 2
has been loaded. A custom DSR will be written to be loaded which
will appear to the "TI" to support all valid devices, and it will
be loaded into the DSR space, which then will not be paged. */

if ((x<0x6000)||(x>0x9fff)) return(CPU[x]);             /* normal CPU RAM or DSR ROM (merged) */

if ((x>=0x8000)&&(x<=0x83ff))                                   /* scratchpad RAM */
{ x=0x8300+(x&0x00ff); /* make sure it maps into >8300->83ff */
  return(CPU[x]);
}

if ((x>=0x8800)&&(x<=0x8fff))                                   /* VDP RAM */
return(rvdpbyte(x));

/* Shouldn't be here */
return(0);
}

void wcpubyte(x,c) Word x; Byte c;
{ /* write a byte to CPU memory */

if (ROMMAP[x]) return; /* can't write to ROM! */

if ((x<0x8000)||(x>0x9fff))
{ CPU[x]=c;
  return;
}

if ((x>=0x8000)&&(x<=0x83ff))
{ x=0x8300+(x&0x00ff);
  CPU[x]=c;
  return;
}

if ((x>=0x8400)&&(x<=0x87ff))
{ wsndbyte(c);
  return;
}

if ((x>=0x8800)&&(x<=0x8fff))
{ wvdpbyte(x,c);
  return;
}

}

Byte rvdpbyte(x) Word x;
{ /* read a byte of VDP data. CPU address x */
unsigned short z;

if (x>=0x8c00) return(0); /* nothing there to read */

if (x&0x0002)
{/* read status */
z=VDPST;
VDPST=0;
return((Byte)z);
}
else
{ /* read data */
if (VDPADD>=0x8000) 
  { wVDPreg((VDPADD&0x0700)>>8,VDPADD&0x0ff);
    return((Byte)0);
  }
else
  { z=VDP[VDPADD&0x3fff];
    VDPADD++;
    return ((Byte)z);
  }
}}

void wvdpbyte(x,c) Word x; Byte c;
{ /* write a byte to VDP. CPU address x           */
Word tmp;

if (x<0x8c00) return; /* not going to write at that block */

if (x&0x0002)
{/* write address */
  VDPADD=(VDPADD>>8)|(c<<8);
  vdpaccess++;
  if (vdpaccess==2)
  { if (VDPADD&0x8000)
    { wVDPreg((Byte)((VDPADD&0x0700)>>8),(Byte)(VDPADD&0x00ff));
      redraw_needed=1;
    }
    vdpaccess=0;
  }
 }
else
{ /* write data */
  VDP[(VDPADD++)&0x3fff]=c;
  redraw_needed=1;

  if ((VDPADD&0x3fff)<=0x300) {
    if ((c==255)&&(ssaState==4)) {  /* boss explosion, should be many */
      BossMusicPlaying=0;
      score+=50;
      if ((ssaState==4)&&(BossExplodes)) {
        play_sample(BossExplodes, 255, 128, 1000, FALSE);
        WatchForFlyOff=1;
      }
    }
  }
}
}

void wVDPreg(r,v) Byte r,v;
{ /* write a value into a VDP register */
  int t;

  VDPREG[r]=v;
}

void wsndbyte(c) Byte c;
{ /* write a byte into the sound processor */
	unsigned int x;
	static int noise_type=1;
	static int freq3=22000;
	/* 'c' contains the byte currently being written to the sound chip	*/
	/* all functions are 1 or 2 bytes long, as follows					*/
	/* BYTE		BIT		PURPOSE											*/
	/*	1		0		always '1'										*/
	/*			1-3		Operation:	000 - tone 1 frequency				*/
	/*								001 - tone 1 volume					*/
	/*								010 - tone 2 frequency				*/
	/*								011 - tone 2 volume					*/
	/*								100 - tone 3 frequency				*/
	/*								101 - tone 3 volume					*/
	/*								110 - noise control					*/
	/*								111 - noise volume					*/
	/*			4-7		Least sig. frequency bits for tone, or volume	*/
	/*					setting (0-F), or type of noise.				*/
	/*					(volume F is off)								*/
	/*					Noise set:	4 - always 0						*/
	/*								5 - 0=periodic noise, 1=white noise */
	/*								6-7 - shift rate from table, or 11	*/
	/*									to get rate from voice 3.		*/
	/*	2		0-1		Always '0'. This byte only used for frequency	*/
	/*			2-7		Most sig. frequency bits						*/
	/* commands are instaneous. I'll use a switch for this.				*/

        return; /* ignore all bytes */

        if ((ssaState<4)&&((c&0x9f)!=0x9f)) return;      /* ignore byte if not a volume mute and game is not in play */

	switch (c&0xf0)
	{	case 0x80:	/* tone 1 frequency */
					last_byte=c;
					need_byte=1;
					break;			/* we need the next byte */

		case 0x90:	/* tone 1 volume */
					x=(c&0x0f)<<4;
					break;

		case 0xa0:	/* tone 2 frequency */
					last_byte=c;
					need_byte=2;	/* we need the next byte */
					break;

		case 0xb0:	/* tone 2 volume */
					x=(c&0x0f)<<4;
					break;

		case 0xc0:	/* tone 3 frequency */
					last_byte=c;
					need_byte=3;	/* we need the next byte */
					break;

		case 0xd0:	/* tone 3 volume */
					x=(c&0x0f)<<4;
					break;

		case 0xe0:	/* noise channel control */
                                        x=(c&0x07);             /* type of noise */
					noise_type=x+1;
					if ((noise_type==4) || (noise_type==8))
					{
                                                /* variable frequency noise */
					}
					break;

		case 0xf0:	/* noise channel volume */
					x=(c&0x0f)<<4;
					break;

		default:	if (need_byte)
					{	/* second byte of a frequency input */
						x=( (last_byte&0x0f) | ((c&0x3f)<<4) )+1;	/* frequency value */
						x=(111860/x)*22000;						/* get desired hz*sample rate */
						x/=550;									/* divide by set rate of sample */
						if (need_byte==3)
						{
							freq3=x;
							if ((noise_type==4) || (noise_type==8))
							{
                                                                /* also set noise channel */
							}
						}
						need_byte=0;
					}	/* if we didn't need one, ignore it */
					break;
	}

}

Byte rgrmbyte(x) Word x;
{ /* read a byte from GROM */
unsigned int z;

if (x>=0x9c00) return(0); /* no response */
if (x&0x0002)
{ /* read address */
grmaccess=2;
z=(GRMADD&0xff00)>>8;
GRMADD=GRMADD<<8;
GRMADD=GRMADD|(GRMADD>>8);
return((Byte)z);
}
else
{ /* read data */
  grmaccess=2;
  z=grmdata;
  grmdata=GROM[GRMADD++];
  return((Byte)z);
}
}

void wgrmbyte(x,c) Word x; Byte c;
{ /* write a byte to GROM */

if (x<0x9c00) return; /* no response */
if (x&0x0002)
{/* write address */
 GRMADD=(GRMADD<<8)|(c&0x00ff);
 grmaccess--;
 if (grmaccess==0)
 { grmaccess=2;
   grmdata=GROM[GRMADD++];
 }
}
else
{ /* write data */
  warn("Writing to GROM!!");
}
}

void wcru(ad,bt) Word ad; int bt;
{ /* write a "bit" to CRU */

}

int rcru(ad) Word ad;
{ /* read a "bit" from CRU. Zero is true */
	return(1);
}

/* one function for each opcode (the mneumonic prefixed with "op_") */

void fixDS()
{ /* fix up D(est) and S(ource) based on Td (type) and Ts */
Word temp,t2;

switch (Ts)
{ case 0: S=WP+(S<<1); break;  /* register */
  case 1: S=romword(WP+(S<<1)); break; /* register indirect */
  case 2: if (S)
          { S=romword(PC)+romword(WP+(S<<1)); PC+=2;} /* idxxed */
          else
          { S=romword(PC); PC+=2; } /* symbolic */
          break;
  case 3: t2=WP+(S<<1); temp=romword(t2); wrword(t2,temp+(2-B));
          S=temp; break;     /* register indirect autoincrement */
}

switch (Td)
{ case 0: D=WP+(D<<1); break;   /* register */
  case 1: D=romword(WP+(D<<1)); break; /* register indirect */
  case 2: if (D)
          { D=romword(PC)+romword(WP+(D<<1)); PC+=2;} /* idxxed */
          else
          { D=romword(PC); PC+=2; } /* symbolic */
          break;
  case 3: t2=WP+(D<<1); temp=romword(t2); wrword(t2,temp+(2-B));
          D=temp; break;    /* register indirect autoincrement */
}
}

/* Note: the format code letters are the official notation from Texas */
/* instruments. See their TMS9900 documentation for details. */
/* (Td, Ts, D, S, B, etc) */

void parity(x) Byte x;
{ /* count parity and set parity bit in ST */
int z,y;

  z=0;
  for (y=0; y<8; y++)
  { if (x>127) z=z++;
    x=x<<1;
  }
  if (z&1) set_OP; else reset_OP;
}

void op_a()
{ Word x1,x2,x3;

FormatI;
x1=romword(S); x2=romword(D);

x3=x2+x1; 
wrword(D,x3);
if (x3>0) set_LGT; else reset_LGT;
if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
if (x3==0) set_EQ; else reset_EQ;
if (x3<x2) set_C; else reset_C;
if (((x1&0x8000)==(x2&0x8000))&&((x3&0x8000)!=(x2&0x8000))) set_OV; else reset_OV;
}

void op_ab()
{ Byte x1,x2,x3;

FormatI;
x1=rcpubyte(S); x2=rcpubyte(D);

x3=x2+x1;
wcpubyte(D,x3);
if (x3>0) set_LGT; else reset_LGT;
if ((x3>0)&&(x3<0x80)) set_AGT; else reset_AGT;
if (x3==0) set_EQ; else reset_EQ;
if (x3<x2) set_C; else reset_C;
if (((x1&0x80)==(x2&0x80))&&((x3&0x80)!=(x2&0x80))) set_OV; else reset_OV;
parity(x3);
}

void op_abs()
{ Word x1,x2;

FormatVI;
x1=x2=romword(S);

if (x1&0x8000) x2=~x1+1;
wrword(S,x2);
if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) set_EQ; else reset_EQ;
if (x1==0x8000) set_OV; else reset_OV;
}

void op_ai()
{ Word x1,x3;

FormatVIII_1;
x1=romword(D);

x3=x1+S;
wrword(D,x3);
if (x3>0) set_LGT; else reset_LGT;
if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
if (x3==0) set_EQ; else reset_EQ;
if (x3<x1) set_C; else reset_C;
if (((x1&0x8000)==(S&0x8000))&&((x3&0x8000)!=(S&0x8000))) set_OV; else reset_OV;
}

void op_dec()
{ Word x1;

FormatVI;
x1=romword(S);

x1--;
wrword(S,x1);

if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) set_EQ; else reset_EQ;
if (x1!=0xffff) set_C; else reset_C;
if (x1==0x7fff) set_OV; else reset_OV;
}

void op_dect()
{ Word x1;

FormatVI;
x1=romword(S);

x1=x1-2;
wrword(S,x1);
if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) set_EQ; else reset_EQ;
if (x1<0xfffe) set_C; else reset_C;
if ((x1==0x7fff)||(x1=0x7ffe)) set_OV; else reset_OV;
}

void op_div()
{ Word x1,x2; unsigned long x3;

FormatIX;
x2=romword(S);
D=WP+(D<<1);
x3=romword(D);

if (x2>x3)
{ x3=(x3<<16)+romword(D+2);
  x1=(Word)x3/x2;
  wrword(D,x1);
  x1=(Word)x3%x2;
  wrword(D+2,x1);
  reset_OV;
}
else
  set_OV;
}

void op_inc()
{ Word x1;

FormatVI;
x1=romword(S);
x1++;
wrword(S,x1);
if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) { set_EQ; set_C; } else { reset_EQ; reset_C; }
if (x1==0x8000) set_OV; else reset_OV;
}

void op_inct()
{ Word x1;

FormatVI;
x1=romword(S);
x1+=2;
wrword(S,x1);
if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) set_EQ; else reset_EQ;
if ((x1==0)||(x1==1)) set_C; else reset_C;
if ((x1==0x8000)||(x1==0x8001)) set_OV; else reset_OV;
}

void op_mpy()
{ Word x1; unsigned long x3;

FormatIX;
x1=romword(S);
D=WP+(D<<1);
x3=romword(D);
x3=x3*x1;
wrword(D,(Word)(x3>>16)); wrword(D+2,(Word)(x3&0xffff));
}

void op_neg()
{ Word x1,x2;

FormatVI;
x1=x2=romword(S);
x1=~x1+1;
wrword(S,x1);
if (x1>0) set_LGT; else reset_LGT;
if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
if (x1==0) set_EQ; else reset_EQ;
if (x2==0x8000) set_OV; else reset_OV;
}

void op_s()
{ Word x1,x2,x3;

FormatI;
x1=romword(S); x2=romword(D);
x3=x2-x1;
wrword(D,x3);
if (x3>0) set_LGT; else reset_LGT;
if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
if (x3==0) set_EQ; else reset_EQ;
if (x3<=x2) set_C; else reset_C;
if (((x1&0x8000)!=(x2&0x8000))&&((x3&0x8000)!=(x2&0x8000))) set_OV; else reset_OV;
}

void op_sb()
{ Byte x1,x2,x3;

FormatI;
x1=rcpubyte(S); x2=rcpubyte(D);
x3=x2-x1;
wcpubyte(D,x3);
if (x3>0) set_LGT; else reset_LGT;
if ((x3>0)&&(x3<0x80)) set_AGT; else reset_AGT;
if (x3==0) set_EQ; else reset_EQ;
if (x3<=x2) set_C; else reset_C;
if (((x1&0x80)!=(x2&0x80))&&((x3&0x80)!=(x2&0x80))) set_OV; else reset_OV;
parity(x3);
}

void op_b()
{ FormatVI;
  PC=S;
}

void op_bl()
{ FormatVI;
  wrword(WP+22,PC);
  PC=S;
}

void op_blwp()
{ Word x1;
  int i;

	if (kscan)
	{
                /* EA VDP Utils speedup - no sanity checks!!! */
		if (S == 0x210c)
		{
                        /* VSBW - R0 = address, R1 MSB = byte */
			VDP[romword(WP)]=CPU[WP+2];
			redraw_needed=1;
			return;
		}
		if (S == 0x2110)
		{
                        /* VMBW - R0 = address, R1 = CPU buffer, R2 = # bytes */
			for (i=0; i<romword(WP+4); i++)
				VDP[romword(WP)+i]=CPU[romword(WP+2)+i];
			redraw_needed=1;
			return;
		}
		if (S == 0x2114)
		{
                        /* VSBR - R0 = address, R1 MSB = byte */
			CPU[WP+2]=VDP[romword(WP)];
			redraw_needed=1;
			return;
		}
		if (S == 0x2118)
		{
                        /* VMBR - R0 = address, R1 = CPU buffer, R2 = # bytes */
			for (i=0; i<romword(WP+4); i++)
				CPU[romword(WP+2)+i]=VDP[romword(WP)+i];
			redraw_needed=1;
			return;
		}
		if (S == 0x211c)
		{
                        /* VWTR - R0 MSB = register, R0 LSB = data */
			VDPREG[CPU[WP]] = CPU[WP+1];
			redraw_needed=1;
			return;
		}
	}

	FormatVI;
	x1=WP;
	WP=romword(S);
	wrword(WP+26,x1);
	wrword(WP+28,PC);
	wrword(WP+30,ST);
	PC=romword(S+2);
}

void op_jeq()
{ char x1;

  FormatII;
  x1=(char)D;
  if (ST_EQ) PC+=x1+x1;
}

void op_jgt()
{ char x1; 

  FormatII;
  x1=(char)D;
  if (ST_AGT) PC+=x1+x1;
}

void op_jhe()
{ char x1;

  FormatII;
  x1=(char)D;
  if ((ST_LGT)||(ST_EQ)) PC+=x1+x1;
}

void op_jh()
{ char x1;

  FormatII;
  x1=(char)D;
  if ((ST_LGT)&&(!ST_EQ)) PC+=x1+x1;
}

void op_jl()
{ char x1;

  FormatII;
  x1=(char)D;
  if ((!ST_LGT)&&(!ST_EQ)) PC+=x1+x1;
}

void op_jle()
{ char x1;

  FormatII;
  x1=(char)D;
  if ((!ST_LGT)||(ST_EQ)) PC+=x1+x1;
}

void op_jlt()
{ char x1;

  FormatII;
  x1=(char)D;
  if ((!ST_AGT)&&(!ST_EQ)) PC+=x1+x1;
}

void op_jmp()
{ char x1;

  FormatII;
  x1=(char)D;
  PC+=x1+x1;
}

void op_jnc()
{ char x1;

  FormatII;
  x1=(char)D;
  if (!ST_C) PC+=x1+x1;
}

void op_jne()
{ char x1;

  FormatII;
  x1=(char)D;
  if (!ST_EQ) PC+=x1+x1;
}

void op_jno()
{ char x1;

  FormatII;
  x1=(char)D;
  if (!ST_OV) PC+=x1+x1;
}

void op_jop()
{ char x1;

  FormatII;
  x1=(char)D;
  if (ST_OP) PC+=x1+x1;
}

void op_joc()
{ char x1;

  FormatII;
  x1=(char)D;
  if (ST_C) PC+=x1+x1;
}

void op_rtwp()
{ ST=romword(WP+30);
  PC=romword(WP+28);
  WP=romword(WP+26);
}

void op_x()
{ /* don't let X call itself. That would likely work, but be weird. */
  if (X_flag!=0) warn("Recursive X instruction!!!!!");
  
  FormatVI;
  in=romword(S);
  X_flag=1;
  do1();		/* This is recursive! */
  X_flag=0;		/* all done, clean up the pointer */
}

void op_xop()
{ Word x1;

  FormatIX;
  x1=WP;
  set_X;
  WP=romword(0x0040+(D<<2));
  wrword(WP+22,S);
  wrword(WP+26,x1);
  wrword(WP+28,PC);
  wrword(WP+30,ST);
  PC=romword(0x0042+(D<<2));
}

void op_c()
{ short x1,x2; Word x3,x4;

  FormatI;
  x3=romword(S); x1=x3;
  x4=romword(D); x2=x4;
  if (x3>x4) set_LGT; else reset_LGT;
  if (x1>x2) set_AGT; else reset_AGT;
  if (x3==x4) set_EQ; else reset_EQ;
}

void op_cb()
{ char x1,x2; Byte x3,x4;

  FormatI;
  x3=rcpubyte(S); x1=x3;
  x4=rcpubyte(D); x2=x4;
  if (x3>x4) set_LGT; else reset_LGT;
  if (x1>x2) set_AGT; else reset_AGT;
  if (x3==x4) set_EQ; else reset_EQ;
  parity(x3);
}

void op_ci()
{ short x1,x2; Word x3;

  FormatVIII_1;
  x3=romword(D); x1=x3;
  x2=S;
  if (x3>S) set_LGT; else reset_LGT;
  if (x1>x2) set_AGT; else reset_AGT;
  if (x3==S) set_EQ; else reset_EQ;
}

void op_coc()
{ Word x1,x2,x3;

  FormatIII;
  x1=romword(S);
  x2=romword(D);
  x3=x1&x2;
  if (x3==x1) set_EQ; else reset_EQ;
}

void op_czc()
{ Word x1,x2,x3;

  FormatIII;
  x1=romword(S);
  x2=romword(D);
  x3=x1&x2;
  if (x3==0) set_EQ; else reset_EQ;
}

void op_ldcr()
{ Word x1,x3; int x2;

  FormatIV;
  if (D==0) D=16;
  if ((S&1)&&(D>8)) S-=1;
  x1=(D<9 ? rcpubyte(S) : romword(S));
  x3=1;
  for (x2=0; x2<D; x2++)
  { wcru((romword(WP+24)>>1)+x2, (x1&x3) ? 1 : 0);
    x3=x3<<1;
  }
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<(D<9 ? 0x80 : 0x8000))) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
  if (D<9) parity(x1&0xff);
}

void op_sbo()
{ char x1;

  FormatII;
  x1=(char)D;
  wcru((romword(WP+24)>>1)+x1,1);
}

void op_sbz()
{ char x1;

  FormatII;
  x1=(char)D;
  wcru((romword(WP+24)>>1)+x1,0);
}

void op_stcr()
{ Word x1,x3,x4; int x2;

  FormatIV;
  if (D==0) D=16;
  if ((S&1)&&(D>8)) S=S-1;
  x1=0; x3=1;
  for (x2=0; x2<D; x2++)
  { x4=rcru((romword(WP+24)>>1)+x2);
    if (x4) x1=x1|x3;
    x3=x3<<1;
  }
  if (D<9) 
    wcpubyte(S,(Byte)x1);  
  else 
    wrword(S,x1);
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<(D<9 ? 0x80 : 0x8000))) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
  if (D<9) parity((x1&0xff00)>>8);
}

void op_tb()
{ char x1;

  FormatII;
  x1=(char)D;

  if (rcru((romword(WP+24)>>1)+x1)) set_EQ; else reset_EQ;

}

void op_ckof()
{ warn("ClocK OFf instruction encountered!");
}

void op_ckon()
{ warn("ClocK ON instruction encountered!");
}

void op_idle()
{ warn("IDLE instruction encountered!");
}

void op_rset()
{ warn("ReSET instruction encountered!");
}

void op_lrex()
{ warn("Load or REstart eXecution instruction encountered!");
}

void op_li()
{ FormatVIII_1;
  wrword(D,S);
  if (S>0) set_LGT; else reset_LGT;
  if ((S>0)&&(S<0x8000)) set_AGT; else reset_AGT;
  if (S==0) set_EQ; else reset_EQ;
}

void op_limi()
{ FormatVIII_1;
  ST=(ST&0xfff0)|(S&0xf);
}

void op_lwpi()
{ FormatVIII_1;
  WP=S;
}

void op_mov()
{ Word x1;

  FormatI;
  x1=romword(S);
  wrword(D,x1);

  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_movb()
{ Byte x1;

  FormatI;
  x1=rcpubyte(S);
  wcpubyte(D,x1);
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x80)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
  parity(x1);

  /* trap ending of all music */
  /* oddly, all the legal endings are at PC 0xb950 */
  if ((PC==0xb950)&&(ssaState==4)&&(x1==0)&&(D==0x83ce)) {
    if (is_mod_playing()) pause_mod();
    BossMusicPlaying=0;
  }

  /* Trap playing of Game Over music */
  if ((ssaState==4)&&(x1>=1)&&(D==0x83ce)) {
    if (romword(0x83cc)==0x1400) {
      if (MOD_GameOver) play_mod(MOD_GameOver, FALSE);
      /* game over is being played */
      ssaState=5;
      BossMusicPlaying=0;
      ShipExplodeSound=0;
    }
  }

  /* Trap playing of Main BG music */
  if ((ssaState==4)&&(x1>=1)&&(D==0x83ce)) {
    if (romword(0x83cc)==0x1100) {
      int xx, yy, col, pat;
      if (MOD_Background) play_mod(MOD_Background, TRUE);
      BossMusicPlaying=0;
      ShipExplodeSound=0; /* toggle that explosion is OK         */
      /* SSA starts the music before clearing the sprites, so    */
      /* sometimes triggers the explosion sound twice (depending */
      /* on when the screen is redrawn). When this happens, the  */
      /* explosion sound is set as being played, and never reset */
      /* So we go through the sprite list here and pre-emptively */
      /* remove any explosion sprites...                         */
      gettables();
      for (i1=0; i1<32; i1++)           /* 32 sprites */
      {
        yy=VDP[SAL];                    /* sprite Y, it's stupid, cause 255 is line 0 */
        xx=VDP[SAL+1];                  /* sprite X */
        pat=VDP[SAL+2];                 /* sprite pattern */
        col=VDP[SAL+3];                 /* sprite early clock (not using yet) and color */
        if (pat==248) VDP[SAL+2]=244;
        SAL+=4;
      }
    }
  }

  if ((ssaState==6)&&(x1>=1)&&(D==0x83ce)) {
    /* starting game win music */
    if (MOD_End) play_mod(MOD_End, FALSE);
    ssaState=7;
    speedcount=timercount+retrace_count;
    BossMusicPlaying=0;
  }
}

void op_stst()
{ FormatVIII_0;
  wrword(D,ST);
}

void op_stwp()
{ FormatVIII_0;
  wrword(D,WP);
}

void op_swpb()
{ Word x1,x2;

  FormatVI;
  x1=romword(S);
  x2=((x1&0xff)<<8)|((x1&0xff00)>>8);
  wrword(S,x2);
}

void op_andi()
{ Word x1,x2;

  FormatVIII_1;
  x1=romword(D);
  x2=x1&S;
  wrword(D,x2);
  if (x2>0) set_LGT; else reset_LGT;
  if ((x2>0)&&(x2<0x8000)) set_AGT; else reset_AGT;
  if (x2==0) set_EQ; else reset_EQ;
}

void op_ori()
{ Word x1,x2;

  FormatVIII_1;
  x1=romword(D);
  x2=x1|S;
  wrword(D,x2);
  if (x2>0) set_LGT; else reset_LGT;
  if ((x2>0)&&(x2<0x8000)) set_AGT; else reset_AGT;
  if (x2==0) set_EQ; else reset_EQ;
}

void op_xor()
{ Word x1,x2,x3;

  FormatIII;
  x1=romword(S);
  x2=romword(D);
  x3=x1^x2;
  wrword(D,x3);
  if (x3>0) set_LGT; else reset_LGT;
  if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
  if (x3==0) set_EQ; else reset_EQ;
}

void op_inv()
{ Word x1;

  FormatVI;
  x1=romword(S);
  x1=~x1;
  wrword(S,x1);
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_clr()
{ FormatVI;
  wrword(S,0);
}

void op_seto()
{ FormatVI;
  wrword(S,0xffff);
}

void op_soc()
{ Word x1,x2,x3;

  FormatI;
  x1=romword(S);
  x2=romword(D);
  x3=x1|x2;
  wrword(D,x3);
  if (x3>0) set_LGT; else reset_LGT;
  if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
  if (x3==0) set_EQ; else reset_EQ;
}

void op_socb()
{ Byte x1,x2,x3;

  FormatI;
  x1=rcpubyte(S);
  x2=rcpubyte(D);
  x3=x1|x2;
  wcpubyte(D,x3);
  if (x3>0) set_LGT; else reset_LGT;
  if ((x3>0)&&(x3<0x80)) set_AGT; else reset_AGT;
  if (x3==0) set_EQ; else reset_EQ;
  parity(x3);
}

void op_szc()
{ Word x1,x2,x3;

  FormatI;
  x1=romword(S);
  x2=romword(D);
  x3=(~x1)&x2;
  wrword(D,x3);
  if (x3>0) set_LGT; else reset_LGT;
  if ((x3>0)&&(x3<0x8000)) set_AGT; else reset_AGT;
  if (x3==0) set_EQ; else reset_EQ;
}

void op_szcb()
{ Byte x1,x2,x3;

  FormatI;
  x1=rcpubyte(S);
  x2=rcpubyte(D);
  x3=(~x1)&x2;
  wcpubyte(D,x3);
  if (x3>0) set_LGT; else reset_LGT;
  if ((x3>0)&&(x3<0x80)) set_AGT; else reset_AGT;
  if (x3==0) set_EQ; else reset_EQ;
  parity(x3);
}

void op_sra()
{ Word x1,x3,x4; int x2;

  FormatV;
  if (D==0)
  { D=romword(WP)&0xf;
    if (D==0) D=16;
  }
  x1=romword(S);
  x4=x1&0x8000;
  for (x2=0; x2<D; x2++)
  { x3=x1&1;   /* save carry */
    x1=x1>>1;  /* shift once */
    x1=x1|x4;  /* extend sign bit */
  }
  wrword(S,x1);
  if (x3) set_C; else reset_C;
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_srl()
{ Word x1,x3; int x2;

  FormatV;
  if (D==0)
  { D=romword(WP)&0xf;
    if (D==0) D=16;
  }
  x1=romword(S);
  for (x2=0; x2<D; x2++)
  { x3=x1&1;
    x1=x1>>1;
  }
  wrword(S,x1);
  if (x3) set_C; else reset_C;
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_sla()
{ Word x1,x3,x4; int x2;

  FormatV;
  if (D==0)
  { D=romword(WP)&0xf;
    if (D==0) D=16;
  }
  x1=romword(S);
  x4=x1&0x8000;
  for (x2=0; x2<D; x2++)
  { x3=x1&0x8000;
    x1=x1<<1;
  }
  wrword(S,x1);
  if (x3) set_C; else reset_C;
  if (x4!=(x1&0x8000)) set_OV; else reset_OV;
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_src()
{ Word x1,x3,x4; int x2;

  FormatV;
  if (D==0)
  { D=romword(WP)&0xf;
    if (D==0) D=16;
  }
  x1=romword(S);
  for (x2=0; x2<D; x2++)
  { x3=x1&0x8000;
    x4=x1&0x1;
    x1=x1>>1;
    if (x4) x1=x1|0x8000;
  }
  wrword(S,x1);
  if (x3) set_C; else reset_C;
  if (x1>0) set_LGT; else reset_LGT;
  if ((x1>0)&&(x1<0x8000)) set_AGT; else reset_AGT;
  if (x1==0) set_EQ; else reset_EQ;
}

void op_bad()
{ warn("Illegal opcode!");
}

void buildcpu()
{ /* set up the CPU array */
Word in,x;
unsigned int i;

/*printf("\nBuilding TMS9900 instruction table\n");*/

for (i=0; i<65536; i++)
{ in=(Word)i;

/*if ((i/820)*820==i) printf(".");*/

/* 'in' now has an instruction to parse. break it down! */

x=(in&0xf000)>>12;

switch(x)
{ case 0: opcode0(in); break;
  case 1: opcode1(in); break;
  case 2: opcode2(in); break;
  case 3: opcode3(in); break;
  case 4: opcode[in]=op_szc; break;
  case 5: opcode[in]=op_szcb; break;
  case 6: opcode[in]=op_s; break;
  case 7: opcode[in]=op_sb; break;
  case 8: opcode[in]=op_c; break;
  case 9: opcode[in]=op_cb; break;
  case 10:opcode[in]=op_a; break;
  case 11:opcode[in]=op_ab; break;
  case 12:opcode[in]=op_mov; break;
  case 13:opcode[in]=op_movb; break;
  case 14:opcode[in]=op_soc; break;
  case 15:opcode[in]=op_socb; break;
  default: opcode[in]=op_bad;
} /*switch*/
} /* for */

/*printf("\n");*/
} /* fctn */

void opcode0(Word in)
{ /* lots of these. Break it again */
unsigned short x;

x=(in&0x0f00)>>8;

switch(x)
{ case 2: opcode02(in); break;
  case 3: opcode03(in); break;
  case 4: opcode04(in); break;
  case 5: opcode05(in); break;
  case 6: opcode06(in); break;
  case 7: opcode07(in); break;
  case 8: opcode[in]=op_sra; break;
  case 9: opcode[in]=op_srl; break;
  case 10:opcode[in]=op_sla; break;
  case 11:opcode[in]=op_src; break;
  default: opcode[in]=op_bad;
}}

void opcode02(Word in)
{ /* more breaking up the opcode */
unsigned short x;

x=(in&0x00e0)>>4;

switch(x)
{ case 0: opcode[in]=op_li; break;
  case 2: opcode[in]=op_ai; break;
  case 4: opcode[in]=op_andi; break;
  case 6: opcode[in]=op_ori; break;
  case 8: opcode[in]=op_ci; break;
  case 10:opcode[in]=op_stwp; break;
  case 12:opcode[in]=op_stst; break;
  case 14:opcode[in]=op_lwpi; break;
  default: opcode[in]=op_bad;
}}

void opcode03(Word in)
{ unsigned short x;

x=(in&0x00e0)>>4;

switch(x)
{ case 0: opcode[in]=op_limi; break;
  case 4: opcode[in]=op_idle; break;
  case 6: opcode[in]=op_rset; break;
  case 8: opcode[in]=op_rtwp; break;
  case 10:opcode[in]=op_ckon; break;
  case 12:opcode[in]=op_ckof; break;
  case 14:opcode[in]=op_lrex; break;
  default: opcode[in]=op_bad;
}}

void opcode04(Word in)
{ unsigned short x;

x=(in&0x00c0)>>4;

switch(x)
{ case 0: opcode[in]=op_blwp; break;
  case 4: opcode[in]=op_b; break;
  case 8: opcode[in]=op_x; break;
  case 12:opcode[in]=op_clr; break;
  default: opcode[in]=op_bad;
}}

void opcode05(Word in)
{ unsigned short x;

x=(in&0x00c0)>>4;

switch(x)
{ case 0: opcode[in]=op_neg; break;
  case 4: opcode[in]=op_inv; break;
  case 8: opcode[in]=op_inc; break;
  case 12:opcode[in]=op_inct; break;
  default: opcode[in]=op_bad;
}}

void opcode06(Word in)
{ unsigned short x;

x=(in&0x00c0)>>4;

switch(x)
{ case 0: opcode[in]=op_dec; break;
  case 4: opcode[in]=op_dect; break;
  case 8: opcode[in]=op_bl; break;
  case 12:opcode[in]=op_swpb; break;
  default: opcode[in]=op_bad;
}}

void opcode07(Word in)
{ unsigned short x;

x=(in&0x00c0)>>4;

switch(x)
{ case 0: opcode[in]=op_seto; break;
  case 4: opcode[in]=op_abs; break;
  default: opcode[in]=op_bad;
}}

void opcode1(Word in)
{ unsigned short x;

x=(in&0x0f00)>>8;

switch(x)
{ case 0: opcode[in]=op_jmp; break;
  case 1: opcode[in]=op_jlt; break;
  case 2: opcode[in]=op_jle; break;
  case 3: opcode[in]=op_jeq; break;
  case 4: opcode[in]=op_jhe; break;
  case 5: opcode[in]=op_jgt; break;
  case 6: opcode[in]=op_jne; break;
  case 7: opcode[in]=op_jnc; break;
  case 8: opcode[in]=op_joc; break;
  case 9: opcode[in]=op_jno; break;
  case 10:opcode[in]=op_jl; break;
  case 11:opcode[in]=op_jh; break;
  case 12:opcode[in]=op_jop; break;
  case 13:opcode[in]=op_sbo; break;
  case 14:opcode[in]=op_sbz; break;
  case 15:opcode[in]=op_tb; break;
  default: opcode[in]=op_bad;
}}

void opcode2(Word in)
{ unsigned short x;

x=(in&0x0c00)>>8;

switch(x)
{ case 0: opcode[in]=op_coc; break;
  case 4: opcode[in]=op_czc; break;
  case 8: opcode[in]=op_xor; break;
  case 12:opcode[in]=op_xop; break;
  default: opcode[in]=op_bad;
}}

void opcode3(Word in)
{ unsigned short x;

x=(in&0x0c00)>>8;

switch(x)
{ case 0: opcode[in]=op_ldcr; break;
  case 4: opcode[in]=op_stcr; break;
  case 8: opcode[in]=op_mpy; break;
  case 12:opcode[in]=op_div; break;
  default: opcode[in]=op_bad;
}}


void do_dsrlnk()
{
        /* performs file i/o using the PAB passed as per a normal */
        /* dsrlnk call. *Only* does LOAD image reads from         */
        /* DSK1, because I don't need more than that for SSA      */
        /* DSK1 is defined as a subdirectory off the current      */
        /* path called "DSK1"                                     */
	
        /* address of length byte passed in CPU >8356 */

	Word PAB, buffer, max_bytes, len;
	char filename[80];
	int idx, b;
        char *fp;

        PAB = romword(0x8356)-14;       /* base address of PAB in VDP RAM */

	if (VDP[PAB] == 0x01)
	{
                /* we'll support CLOSE (0x01) just because it happens in case of Errors */
                /* but we won't do anything about it */
		return;
	}

        if (VDP[PAB] != 0x05)           /* LOAD */
	{
                /* bad opcode. Boy, this DSR sucks ;)  */
                /* set return byte in PAB and exit     */

                VDP[PAB+1] = 0x040;             /* Bad open attribute */
                CPU[0x837c] |= 0x20;            /* are we supposed to set this bit? */
		return;
	}

	buffer=(VDP[PAB+2]<<8) | VDP[PAB+3];		// VDP buffer
	max_bytes=(VDP[PAB+6]<<8) | VDP[PAB+7];		// maximum bytes to read
        len=VDP[PAB+9] - 5;                             // Filename length minus 'DSK1.' bit
        strcpy(filename,"DSK1\\");                      // set up path
	strncat(filename, &VDP[PAB+15], len);		// copy name after DSK1.

        if (strcmp(filename, "DSK1\\ACER_C")==0) {
          ssaState=1;
        }       // we've loaded title page

        if (strcmp(filename, "DSK1\\SSE")==0) {
          ssaState=6;
        }       // loaded game win routine - all bets are off


        fp=GetPtr(filename);
	if (NULL == fp)
	{
		// couldn't open the file
                VDP[PAB+1] = 0xe0;      // file error
		CPU[0x837c] |= 0x20;	// are we supposed to set this bit?
		return;
	}

        b=*(fp++);
	if ((0 != b) && (0xff != b))
	{
		// we don't have a proper flag byte at the start, assume there is
		// a TIFILES or V9T9 header and skip 128 bytes, then carry on
		// this could cause problems with bad files
		for (idx=0; idx<128; idx++)
                        b=*(fp++);  // this reads the next desired byte again
	}

	max_bytes--;

	for (idx = 0; idx < max_bytes; idx++)
	{
                VDP[buffer+idx] = b;    // read byte
                b = *(fp++);

	}

	VDP[PAB+1] = 0x00;			// no errors
        CPU[0x837c] &= 0xdf;                    // clear COND bit
}

int keycall(int key) {
  if ((key & 0xff) == '5') {
    credits++;
    play_sample(CreditIn, 255, 128, 1000, FALSE);
  }

  return key;
}

char *GetPtr(char *szName) {
  /* return a pointer to a file in the RAM disk */
  int idx;
  char *ret;

  ret=NULL;
  for (idx=0; idx<RAMFILES; idx++) {
    if (strcmp(&RAMFAT[idx][0], szName)==0) {
      ret=&RAMDISK[idx][0];
    }
  }

  return ret;
}

char *RAMfgets(char *buffer, int length, char *fp)
{
/* do psuedo-fgets from the RAMdisk */
int buf;
int ret;
int cnt;

/* figure out which buffer we're reading */
buf=-1;
for (ret=0; ret<RAMFILES; ret++) {
  if ((fp>=&RAMDISK[ret][0])&&(fp<=(&RAMDISK[ret+1][0])))
    buf=ret;
}
if (buf==-1) return 0;
if (fp-(&RAMDISK[buf][0])>=RAMSIZE[buf]) return 0;

cnt=0;
while ((fp<(&RAMDISK[buf][0])+RAMSIZE[buf])&&(*fp!='\n')&&(cnt<length)) {
  *(buffer++)=*(fp++);
  cnt++;
}
*(buffer)='\0';
if ('\n'==*fp) fp++;
return fp;
}

/************************************************************************/
// SNESpad class
// Version 1.0
// Kerry High

#define SNES_PWR (128+64+32+16+8)
#define SNES_CLK 1 // base+0
#define SNES_LAT 2 // base+0
#define SIN (negate?((inp(base+1)&snes_din)?1:0):((inp(base+1)&snes_din)?0:1))

// Set base by lpt#: 1=0x378 2=0x278 3=0x3BC
void setlpt(int lpt_number)
{
switch(lpt_number)
{
case 1: setbase(0x378); break;
case 2: setbase(0x278); break;
case 3: setbase(0x3bc); break;
default:
printf("SNESpad: LPT%d invalid!\n",lpt_number);
printf("Defaulting to LPT1\n");
setbase(0x378);
}
return;
}

// Set to any base
void setbase(int port_number)
{
base=port_number;
return;
}

// Read pad number pad_number
// Pads are numbered from 0 to 4
// Original C code by Earle F. Philhower, III.
void readpad(int pad_number)
{
const int DIN[5] = {64,32,16,8,128};
const int NEGATE[5]={0, 0, 0,0, 1};

int snes_din=DIN[pad_number];
int negate=NEGATE[pad_number];

if (NOSNESPAD) {
memset(&snes, 0, sizeof(snes));
return;
}

outp(base, SNES_PWR+SNES_CLK); // Power up!
outp(base, SNES_PWR+SNES_LAT+SNES_CLK); // Latch it!

snes.fire=0;
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.start = SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.up = SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.down = SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.left = SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.right = SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, SNES_PWR);
outp(base, SNES_PWR+SNES_CLK);
snes.fire |= SIN;
outp(base, 0); // Power it down

if (snes.fire && snes.start && snes.up && snes.down && snes.left && snes.right) {
  /* if EVERYTHING is on, there's likely no pad */
  NOSNESPAD=1;
  memset(&snes, 0, sizeof(snes));
}

}

