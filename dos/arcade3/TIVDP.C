/* VDP routine for the TI-99/4a Emulator */
/* Ami99VDP by Mike Brent */

/* TI Palette */

#define BORDER 40
/* index of color of border to emulate call screen() */
#define TIPALETTE 41
/* first index of a fixed copy of the TI Palette (used for sprites) */

RGB TIPAL[16];
PALETTE vdp_pal;

int red[16]={ 0,0,8,24,8,16,40,16,56,56,48,48,8,48,40,56 };
int green[16]= { 0,0,48,56,8,24,8,48,8,24,48,48,32,16,40,56 };
int blue[16]= { 0,0,8,24,56,56,8,56,8,24,8,32,8,40,40,56 };
int col = 41;

BITMAP *VDPBUFFER, *SPRITEBUFFER, *SCREENBUFFER;

Word SIT, CT, PDT, SAL, SDT;
int i1,i2;
Word p_add;
Byte chr,fgcol,bgcol,x;
int Video_Debug;
unsigned int ad;
FILE *fp;

/* percentage of 3d effect, 100% is edge on */
/* the X co-ordinate is dependant on the scanline, */
/* but the Y co-ordinate isn't, so we save some memory */
/* there. */
#define RATIO 1
int ScreenLookupX[320][240], ScreenLookupY[240];
float ScreenRatio[240];

/* starfield */
#define NUMSTARS 128
int starx[NUMSTARS], stary[NUMSTARS];

int SpriteScore[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int BossMusicPlaying=0;

#define min(x,y) (x>y ? y: x)

void gettables()
{
  /* Screen Image Table */
  SIT=(VDPREG[2]<<10)&0x3fff;

  /* Colour Table */
  CT=(VDPREG[3]<<6)&0x3fff;

  /* Pattern Descriptor Table */
  PDT=(VDPREG[4]<<11)&0x3fff;

  /* Sprite Attribute List */
  SAL=(VDPREG[5]<<7)&0x3fff;

  /* Sprite Descriptor Table */
  SDT=(VDPREG[6]<<11)&0x3fff;
}

void VDPdisplay()
{	/* determine which video mode to display */
        /* only supports bitmap and graphics mode, and blank screen */

        char buffer[1024];
	int i1;

	if (!(VDPREG[1] & 0x40))
	{
          rectfill(screen, 0, 0, 319, 199, 0);
          goto finish;
	}

	if (VDPREG[0] & 0x02)	// MODE BIT 3
	{
           VDPbitmap();
           goto finish;
	}

	VDPgraphics();

finish:
        sprintf(buffer, "Credits: %d", credits);
        textout(screen, font, buffer, 224, 230, 56);
        if ((ssaState<4)&&(credits>0))
          textout_centre(screen, font, "Press Start", 160, 112, 56);
        if ((ssaState==3)&&(credits==0))
          textout_centre(screen, font, "Insert Coin", 160, 112, 56);
        if (ssaState==5)
          textout_centre(screen, font, "Game Over", 160, 112, 56);

        textout_centre(screen, font, misctext, 160, 125, 56);

        i1=romword(0xa292);
        strcpy(buffer, "^^^^^");
        if ((i1>0)&&(i1<5)) buffer[romword(0xa292)]='\0';
        if ((i1>0)&&(ssaState==4))
          textout_centre(screen, font, buffer, 160, 230, 56);

        sprintf(buffer, "Score: %05d", score);
        textout(screen, font, buffer, 0, 230, 56);
}

void VDPmain()
{
  int x, y, startrow;

  set_color_depth(8);
  if (set_gfx_mode(GFX_MODEX,320,240,0,0)<0)
        fail("Can't set 320x240x8 graphics mode!");

  /* VDPBUFFER points to a bitmap sized to be 256 characters by */
  /* 1 character high. It's used to draw the character set for  */
  /* blitting to the screen. We draw 256 chars instead of 768.  */
  /* Gives a bit of speed boost, & reduces duplication of work  */
  VDPBUFFER=create_bitmap(2048,8);
  if (VDPBUFFER==NULL) fail("Can't create VDP workspace.");

  SPRITEBUFFER=create_bitmap(16,16);
  if (SPRITEBUFFER==NULL) fail("Can't create VDP Sprite workspace.");

  SCREENBUFFER=create_bitmap(320, 240);
  if (SCREENBUFFER==NULL) fail("Can't create VDP screen buffer.");

  clear_to_color(screen,0);

  redraw_needed=1;

  VDPREG[8]=0xff;   /* clear hacked-in keyboard buffer */

  /* When set, display the character set at the top of the screen */
  Video_Debug=0;

  text_mode(-1);

  /* calculate where we're translating every X and Y to */

  startrow=(240*RATIO)/100;

  for (y=0; y<240; y++) {
    ScreenLookupY[y]=startrow + ((y*((y*100.0)/240.0)/100.0) * (100.0-RATIO) / 100.0);
    ScreenRatio[y]=((100.0-RATIO)*(y/240.0))/100.0+0.2;
    for (x=0; x<320; x++) {
      ScreenLookupX[x][y]=((x-160.0) * ScreenRatio[y]) + 160;
    }
  }

  clear(screen);
  set_palette(TitlePal);

/* temp */
  for (i1=0; i1<16; i1++)
  { TIPAL[i1].r=red[i1];
    TIPAL[i1].b=blue[i1];
    TIPAL[i1].g=green[i1];
  }
}

void VDPgraphics()
{
/* this function to display the graphics once */
/* very simplified for SSA! */
int xx,yy,t,pat,col;
int i1,i2,i3,i4,t1,t2;

int cnt;
int vdp_drawn[256];		/* flags for which characters have been drawn */

for (i1=0; i1<16; i1++)
{
      vdp_pal[TIPALETTE+i1].r=TIPAL[i1].r;
      vdp_pal[TIPALETTE+i1].g=TIPAL[i1].g;
      vdp_pal[TIPALETTE+i1].b=TIPAL[i1].b;
}
/* update color palette */
for (i1=0; i1<32; i1++)
{	t=VDP[CT+i1];
	t1=t&0xf;
	t2=t>>4;
	i2=i1+128;
	vdp_pal[i1].r=TIPAL[t1].r;
	vdp_pal[i1].g=TIPAL[t1].g;
	vdp_pal[i1].b=TIPAL[t1].b;
	vdp_pal[i2].r=TIPAL[t2].r;
	vdp_pal[i2].g=TIPAL[t2].g;
	vdp_pal[i2].b=TIPAL[t2].b;
}

/* TI color 0 (transparent) is set to reflect the background color */
/* so we can use it here for the border color */
vdp_pal[BORDER].r=TIPAL[0].r;
vdp_pal[BORDER].g=TIPAL[0].g;
vdp_pal[BORDER].b=TIPAL[0].b;

set_palette(vdp_pal);

/* SSA uses the graphics mode screen for the starfield, the GAME OVER */
/* text, and the boss body itself. We'll draw the bosses as part of the */
/* sprites, but replace the starfield. */

gettables();

clear(SCREENBUFFER);

/* clear the vdp_drawn[] array */
for (i1=0; i1<256; i1++)
{
  vdp_drawn[i1]=0;
}

/* now blit each tile to the screen buffer */
/* calculate position in VDPBUFFER of base of table */

for (i1=0; i1<192; i1+=8)         /* y loop */
{ for (i2=0; i2<256; i2+=8)       /* x loop */
  {
    if (vdp_drawn[VDP[SIT]]==0)
	{	/* Now we draw a single character into VDPBUFFER. Then we set a flag		*/
		/* that is has been done, so that we don't do it twice this frame. We only	*/
		/* need to draw the characters we actually use this way.					*/

		xx=VDP[SIT]<<3;
		p_add=PDT+xx;
		
		for (i3=0; i3<8; i3++)
		{
                    t=VDP[p_add++];
		    putpixel(VDPBUFFER,xx,i3,(VDP[SIT]>>3)|(t&0x80 ? 0x80 : 0 ));
		    putpixel(VDPBUFFER,xx+1,i3,(VDP[SIT]>>3)|(t&0x40 ? 0x80 : 0 ));
		    putpixel(VDPBUFFER,xx+2,i3,(VDP[SIT]>>3)|(t&0x20 ? 0x80 : 0 ));
		    putpixel(VDPBUFFER,xx+3,i3,(VDP[SIT]>>3)|(t&0x10 ? 0x80 : 0 ));
		    putpixel(VDPBUFFER,xx+4,i3,(VDP[SIT]>>3)|(t&0x08 ? 0x80 : 0 ));
                    putpixel(VDPBUFFER,xx+5,i3,(VDP[SIT]>>3)|(t&0x04 ? 0x80 : 0 ));
                    putpixel(VDPBUFFER,xx+6,i3,(VDP[SIT]>>3)|(t&0x02 ? 0x80 : 0 ));
		    putpixel(VDPBUFFER,xx+7,i3,(VDP[SIT]>>3)|(t&0x01 ? 0x80 : 0 ));
		}
		
		vdp_drawn[VDP[SIT]]=1;
	}
        stretch_blit(VDPBUFFER,SCREENBUFFER,(VDP[SIT++]<<3),0,8,8,ScreenLookupX[i2+(i2>>2)][i1+(i1>>2)],ScreenLookupY[i1+(i1>>2)],8*ScreenRatio[i1+(i1>>2)]+1,8*ScreenRatio[i1+(i1>>2)]+1);
  }
}

/* now draw the sprites - each sprite drawn because of color and shape changes */

for (i1=0; i1<32; i1++)           /* 32 sprites */
{
  yy=VDP[SAL++]+1;                /* sprite Y, it's stupid, cause 255 is line 0 */
  if (yy>255) yy=0;
  xx=VDP[SAL++];                  /* sprite X */
  pat=VDP[SAL++];                 /* sprite pattern */
  col=VDP[SAL++]&0xf;             /* sprite early clock (not using yet) and color */

  /* SSA Hacks based on sprites */
  if (i1==0) {
    if (WatchForFlyOff) {
      if (yy<oldY) {
        WatchForFlyOff=0;
        if ((ssaState==4)&&(PlayerWarps)) {
          play_sample(PlayerWarps, 255, 128, 1000, FALSE);
          play_sample(PlayerWarps, 255, 128, 1000, FALSE);
          play_sample(PlayerWarps, 255, 128, 1000, FALSE);
        }
      }
    }
    oldY=yy;
  }

  if ((pat==56)&&(SpriteScore[i1]==0)) {
    if (ssaState==4) score+=100;
    SpriteScore[i1]=1;
    if ((ssaState==4)&&(ShipExplodes)) {
      play_sample(ShipExplodes, min(yy*2,255), 128, 1000, FALSE);
      play_sample(ShipExplodes, min(yy*2,255), 128, 1000, FALSE); /* beef it up */
    }
  }
  if (pat!=56) SpriteScore[i1]=0;
  if ((i1==0)&&(col!=15)) {
    BossMusicPlaying=0;
  }
  if ((pat==76)&&(BossMusicPlaying==0)) {
    /* boss engine */
    BossMusicPlaying=1;
    ShipExplodeSound=0;
    if (MOD_Boss) play_mod(MOD_Boss, FALSE);
  }
  if ((pat==248)&&(ShipExplodeSound==0)) {
    if (VDP[PDT+0x7c0]==0x14) {
      ShipExplodeSound=1;
      if ((ssaState==4)&&(PlayerExplodes)) {
        play_sample(PlayerExplodes, 255, 128, 1000, FALSE);
        play_sample(PlayerExplodes, 255, 128, 1000, FALSE);
      }
    }
  }

  /* End Hacks ***********************/

  if (yy==0xd1)
    i1=32;                  /* 0xd0 ends sprite list, add 1 cause I did above */

  if ((yy<192)&&(col&0xf))                                /* if on-screen and not transparent */
  {
    p_add=SDT+(pat<<3);
		
    clear(SPRITEBUFFER);

    for (i3=0; i3<8; i3++)
    {
      /* all SSA sprites are double size */

      t=VDP[p_add++];
      putpixel(SPRITEBUFFER,0,i3,(t&0x80 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,1,i3,(t&0x40 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,2,i3,(t&0x20 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,3,i3,(t&0x10 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,4,i3,(t&0x08 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,5,i3,(t&0x04 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,6,i3,(t&0x02 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,7,i3,(t&0x01 ? col+TIPALETTE : 0 ));

      t=VDP[p_add+7];
      putpixel(SPRITEBUFFER,0,i3+8,(t&0x80 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,1,i3+8,(t&0x40 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,2,i3+8,(t&0x20 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,3,i3+8,(t&0x10 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,4,i3+8,(t&0x08 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,5,i3+8,(t&0x04 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,6,i3+8,(t&0x02 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,7,i3+8,(t&0x01 ? col+TIPALETTE : 0 ));

      t=VDP[p_add+15];
      putpixel(SPRITEBUFFER,8,i3,(t&0x80 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,9,i3,(t&0x40 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,10,i3,(t&0x20 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,11,i3,(t&0x10 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,12,i3,(t&0x08 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,13,i3,(t&0x04 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,14,i3,(t&0x02 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,15,i3,(t&0x01 ? col+TIPALETTE : 0 ));

      t=VDP[p_add+23];
      putpixel(SPRITEBUFFER,8,i3+8,(t&0x80 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,9,i3+8,(t&0x40 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,10,i3+8,(t&0x20 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,11,i3+8,(t&0x10 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,12,i3+8,(t&0x08 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,13,i3+8,(t&0x04 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,14,i3+8,(t&0x02 ? col+TIPALETTE : 0 ));
      putpixel(SPRITEBUFFER,15,i3+8,(t&0x01 ? col+TIPALETTE : 0 ));
    }
    stretch_sprite(SCREENBUFFER,SPRITEBUFFER,ScreenLookupX[xx+(xx>>2)][yy+(yy>>2)],ScreenLookupY[yy+(yy>>2)],16*ScreenRatio[yy+(yy>>2)]+1, 16*ScreenRatio[yy+(yy>>2)]+1);
  }
}
blit(SCREENBUFFER, screen, 0, 0, 0, 0, 320, 240);
}

void VDPbitmap()
{
  /* Bitmap mode only displays a picture in this game... */

  if (TitlePic) {
    set_palette(TitlePal);
    blit(TitlePic, screen, 0,0, 0,0, 320,240);
  }
}
