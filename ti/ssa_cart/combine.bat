@rem * Cartridge layout, keeping to 8k blocks for ease
@rem * Bank0 - >6000
@rem *					0040	boot, unpacker (1k)
@rem *					0440	SSE.pack	to >A000, >13FA bytes
@rem *					14C0	SSD.pack	to VDP >0800, >0F00 bytes
@rem *					1F54	
@rem *	Bank1 - >6002
@rem *					0040	ssa.pack	to >A000, >1FFA bytes
@rem *					14C0	acer_c.pack to VDP >2000, >1800 bytes
@rem *					1840  demq.pack to >2000, >066C bytes
@rem *					1EB2
@rem * Bank2 - >6004
@rem *					0040  ssb.pack	to >BFFA, >1FFA bytes
@rem *					13D0
@rem * Bank3 - >6006
@rem *					0040	ssc.pack	to >DFF4, >1BFA bytes
@rem *					1330	acer_p.pack to VDP >0000, >1800 bytes
@rem *					1F10

@rem ** WARNING:: DEMQ has patched DSRLNK that does not DSRLNK!

\work\setbinsize\release\setbinsize.exe header.bin 64
\work\setbinsize\release\setbinsize.exe unpack.bin 1024
\work\setbinsize\release\setbinsize.exe ssa.pack 5248
\work\setbinsize\release\setbinsize.exe ssc.pack 4848
\work\setbinsize\release\setbinsize.exe sse.pack 4224
\work\setbinsize\release\setbinsize.exe acer_c.pack 896
@rem those are the only files that are stacked

copy /y /b header.bin + /b unpack.bin + /b sse.pack + /b ssd.pack /b bank0.bin
copy /y /b header.bin + /b ssa.pack + /b acer_c.pack + /b demq.pack /b bank1.bin
copy /y /b header.bin + /b ssb.pack bank2.bin
copy /y /b header.bin + /b ssc.pack + /b acer_p.pack /b bank3.bin

\work\setbinsize\release\setbinsize.exe bank0.bin 8192
\work\setbinsize\release\setbinsize.exe bank1.bin 8192
\work\setbinsize\release\setbinsize.exe bank2.bin 8192
\work\setbinsize\release\setbinsize.exe bank3.bin 8192

copy /y /b bank0.bin + /b bank1.bin + /b bank2.bin + /b bank3.bin /b spaceacer8.bin

