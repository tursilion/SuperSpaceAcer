CC = D:/work/coleco/sdcc-4.2.0/bin/sdcc.exe
CFLAGS = -mz80 --vc -c "-Id:/work/coleco/libti99coleco" "-ID:/work/TI/vgmcomp2/Players/libcolecovgm2" -DENABLEFX --std-sdcc11 --opt-code-speed --less-pedantic --fsigned-char
AS = D:/work/coleco/sdcc-4.2.0/bin/sdasz80
AFLAGS = -plosgff

.PHONY: all clean

# crt0 must be first!
objs = crt0.rel ssd_inc.rel superspaceacer.rel trampolines.rel bg.rel boss.rel enemy.rel music.rel human.rel wineasy.rel winhard.rel winmed.rel \
		title_c.rel title_p.rel boss1.rel boss2.rel boss3.rel boss4.rel boss5.rel outfile0.rel outfile1.rel outfile2.rel outfile3.rel \
		font.rel difficulty.rel selena_end_c.rel selena_end_p.rel titlepage.rel ship1_c.rel ship1_p.rel ship2_c.rel ship2_p.rel \
		ship3_c.rel ship3_p.rel ship4_c.rel ship4_p.rel qwertianc.rel qwertianp.rel attract.rel sfxs.rel ladybugc.rel ladybugp.rel \
		ladybugscreenc.rel ladybugscreenp.rel

all: buildcol
	d:/work/coleco/tursi/makemegacart/release/makemegacart.exe -map crt0.s crt0.ihx ssa.rom

# crt0.s contains a reference to each segment in order, so they can be linked up - put files into segments there
# if makemegacart complains about data in RAM, see if a segment is missing there

buildcol: $(objs)
	$(CC) -mz80 --vc --no-std-crt0 $(BANKS) --code-loc 0x8100 --data-loc 0x7000 -l"d:/work/coleco/libti99coleco/libti99.a" -l"D:\work\TI\vgmcomp2\Players\libcolecovgm2\libcolecovgm2.a" $(objs)
	"../parseMap.exe" crt0.map

# banks - all defined the same way, we just need to declare them
BANKS = "-Wl -b_bank1=0xc000" "-Wl -b_bank2=0xc000" "-Wl -b_bank3=0xc000" "-Wl -b_bank4=0xc000" "-Wl -b_bank5=0xc000" "-Wl -b_bank6=0xc000" "-Wl -b_bank7=0xc000" \
		"-Wl -b_bank8=0xc000" "-Wl -b_bank9=0xc000" "-Wl -b_bank10=0xc000" "-Wl -b_bank11=0xc000" "-Wl -b_bank12=0xc000" "-Wl -b_bank13=0xc000" "-Wl -b_bank14=0xc000" "-Wl -b_bank15=0xc000"

clean:
	cmd //c del *.rel *.map *.lst *.sym *.asm *.ihx *.rom boss1.c boss2.c boss3.c boss4.c boss5.c

ssd_inc.rel: ssd_inc.c
	$(CC) $(CFLAGS) -o ssd_inc.rel ssd_inc.c --codeseg ssd --constseg ssd

bg.rel: bg.c game.h
	$(CC) $(CFLAGS) -o bg.rel bg.c --codeseg bg --constseg bg

boss.rel: boss.c game.h
	$(CC) $(CFLAGS) -o boss.rel boss.c --codeseg boss --constseg boss

enemy.rel: enemy.c game.h
	$(CC) $(CFLAGS) -o enemy.rel enemy.c --codeseg enemy --constseg enemy

music.rel: music.c game.h
	$(CC) $(CFLAGS) -o music.rel music.c --codeseg music --constseg music

human.rel: human.c game.h
	$(CC) $(CFLAGS) -o human.rel human.c --codeseg human --constseg human

superspaceacer.rel: superspaceacer.c game.h enemy.h trampoline.h human.h music.h attract.h
	$(CC) $(CFLAGS) -o superspaceacer.rel superspaceacer.c --codeseg ssa --constseg ssa

titlepage.rel: titlepage.c game.h
	$(CC) $(CFLAGS) -o titlepage.rel titlepage.c --codeseg titlepage --constseg titlepage

trampolines.rel: trampolines.c game.h trampoline.h
	$(CC) $(CFLAGS) -o trampolines.rel trampolines.c --codeseg tramp --constseg tramp

wineasy.rel: wineasy.c game.h
	$(CC) $(CFLAGS) -o wineasy.rel wineasy.c --codeseg wineasy --constseg wineasy

winmed.rel: winmed.c game.h
	$(CC) $(CFLAGS) -o winmed.rel winmed.c --codeseg winmed --constseg winmed

winhard.rel: winhard.c game.h
	$(CC) $(CFLAGS) -o winhard.rel winhard.c --codeseg winhard --constseg winhard

title_c.rel: title_c.c
	$(CC) $(CFLAGS) -o title_c.rel title_c.c --codeseg titlec --constseg titlec

title_p.rel: title_p.c
	$(CC) $(CFLAGS) -o title_p.rel title_p.c --codeseg titlep --constseg titlep

boss1.rel: boss1.bmp
	D:/work/SuperSpaceAcer/coleco/SuperSpaceAcer/packboss/Debug/packboss.exe boss1.bmp boss1.c BOSS1
	$(CC) $(CFLAGS) -oboss1.rel boss1.c --codeseg boss1 --constseg boss1

boss2.rel: boss2.bmp
	D:/work/SuperSpaceAcer/coleco/SuperSpaceAcer/packboss/Debug/packboss.exe boss2.bmp boss2.c BOSS2
	$(CC) $(CFLAGS) -oboss2.rel boss2.c --codeseg boss2 --constseg boss2

boss3.rel: boss3.bmp
	D:/work/SuperSpaceAcer/coleco/SuperSpaceAcer/packboss/Debug/packboss.exe boss3.bmp boss3.c BOSS3
	$(CC) $(CFLAGS) -oboss3.rel boss3.c --codeseg boss3 --constseg boss3

boss4.rel: boss4.bmp
	D:/work/SuperSpaceAcer/coleco/SuperSpaceAcer/packboss/Debug/packboss.exe boss4.bmp boss4.c BOSS4
	$(CC) $(CFLAGS) -oboss4.rel boss4.c --codeseg boss4 --constseg boss4

boss5.rel: boss5.bmp
	D:/work/SuperSpaceAcer/coleco/SuperSpaceAcer/packboss/Debug/packboss.exe boss5.bmp boss5.c BOSS5
	$(CC) $(CFLAGS) -oboss5.rel boss5.c --codeseg boss5 --constseg boss5

#songpack1.rel: songpack1.c
#	$(CC) $(CFLAGS) -o songpack1.rel songpack1.c --codeseg songpack1 --constseg songpack1
#
#songpack2.rel: songpack2.c
#	$(CC) $(CFLAGS) -o songpack2.rel songpack2.c --codeseg songpack2 --constseg songpack2
#
#songpack3.rel: songpack3.c
#	$(CC) $(CFLAGS) -o songpack3.rel songpack3.c --codeseg songpack3 --constseg songpack3
#
#songpack4.rel: songpack4.c
#	$(CC) $(CFLAGS) -o songpack4.rel songpack4.c --codeseg songpack4 --constseg songpack4
#
#songpack5.rel: songpack5.c
#	$(CC) $(CFLAGS) -o songpack5.rel songpack5.c --codeseg songpack5 --constseg songpack5
#
#songpack6.rel: songpack6.c
#	$(CC) $(CFLAGS) -o songpack6.rel songpack6.c --codeseg songpack6 --constseg songpack6

outfile0.rel: outfile0.c
	$(CC) $(CFLAGS) -o outfile0.rel outfile0.c --codeseg outfile0 --constseg outfile0
outfile1.rel: outfile1.c
	$(CC) $(CFLAGS) -o outfile1.rel outfile1.c --codeseg outfile1 --constseg outfile1
outfile2.rel: outfile2.c
	$(CC) $(CFLAGS) -o outfile2.rel outfile2.c --codeseg outfile2 --constseg outfile2
outfile3.rel: outfile3.c
	$(CC) $(CFLAGS) -o outfile3.rel outfile3.c --codeseg outfile3 --constseg outfile3

ship1_c.rel: ../title2/ship1_c.c
	$(CC) $(CFLAGS) -o ship1_c.rel ../title2/ship1_c.c --codeseg ship1c --constseg ship1c
ship1_p.rel: ../title2/ship1_p.c
	$(CC) $(CFLAGS) -o ship1_p.rel ../title2/ship1_p.c --codeseg ship1p --constseg ship1p

ship2_c.rel: ../title2/ship2_c.c
	$(CC) $(CFLAGS) -o ship2_c.rel ../title2/ship2_c.c --codeseg ship2c --constseg ship2c
ship2_p.rel: ../title2/ship2_p.c
	$(CC) $(CFLAGS) -o ship2_p.rel ../title2/ship2_p.c --codeseg ship2p --constseg ship2p

ship3_c.rel: ../title2/ship3_c.c
	$(CC) $(CFLAGS) -o ship3_c.rel ../title2/ship3_c.c --codeseg ship3c --constseg ship3c
ship3_p.rel: ../title2/ship3_p.c
	$(CC) $(CFLAGS) -o ship3_p.rel ../title2/ship3_p.c --codeseg ship3p --constseg ship3p

ship4_c.rel: ../title2/ship4_c.c
	$(CC) $(CFLAGS) -o ship4_c.rel ../title2/ship4_c.c --codeseg ship4c --constseg ship4c
ship4_p.rel: ../title2/ship4_p.c
	$(CC) $(CFLAGS) -o ship4_p.rel ../title2/ship4_p.c --codeseg ship4p --constseg ship4p

font.rel: font.c
	$(CC) $(CFLAGS) -o font.rel font.c --codeseg font --constseg font

difficulty.rel: difficulty.c
	$(CC) $(CFLAGS) -o difficulty.rel difficulty.c --codeseg difficulty --constseg difficulty

selena_end_c.rel: selena_end_c.c
	$(CC) $(CFLAGS) -o selena_end_c.rel selena_end_c.c --codeseg selenac --constseg selenac

selena_end_p.rel: selena_end_p.c
	$(CC) $(CFLAGS) -o selena_end_p.rel selena_end_p.c --codeseg selenap --constseg selenap

qwertianp.rel: qwertianp.c
	$(CC) $(CFLAGS) -o qwertianp.rel qwertianp.c --codeseg qwertianp --constseg qwertianp

qwertianc.rel: qwertianc.c
	$(CC) $(CFLAGS) -o qwertianc.rel qwertianc.c --codeseg qwertianc --constseg qwertianc

attract.rel: attract.c attract.h
	$(CC) $(CFLAGS) -o attract.rel attract.c --codeseg attract --constseg attract

sfxs.rel: sfxs.c
	$(CC) $(CFLAGS) -o sfxs.rel sfxs.c --codeseg sfxs --constseg sfxs

ladybugc.rel : ladybugc.c
	$(CC) $(CFLAGS) -o ladybugc.rel ladybugC.c --codeseg ladybugc --constseg ladybugc

ladybugp.rel : ladybugp.c
	$(CC) $(CFLAGS) -o ladybugp.rel ladybugP.c --codeseg ladybugp --constseg ladybugp

ladybugscreenc.rel : ladybugscreenc.c
	$(CC) $(CFLAGS) -o ladybugscreenc.rel ladybugscreenc.c --codeseg ladybugscreenc --constseg ladybugscreenc

ladybugscreenp.rel : ladybugscreenp.c
	$(CC) $(CFLAGS) -o ladybugscreenp.rel ladybugscreenp.c --codeseg ladybugscreenp --constseg ladybugscreenp

# libti99 crt0 copied into folder
crt0.rel: crt0.s
	$(AS) $(AFLAGS) crt0.rel crt0.s
