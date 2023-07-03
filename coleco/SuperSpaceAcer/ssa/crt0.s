; crt0.s for Colecovision cart
	;; Ordering of segments for the linker - copied from sdcc crt0.s
	;; The only purpose of this is so that the linker has seen all the
	;; sections in the order that we want them applied. Addresses are
	;; set by the makefile, or continue on from the last one.
	;; Not sure why header doesn't need to be in here... but doesn't matter
	.area _HOME
	.area _CODE
		.ascii "LinkTag:Fixed\0"	; also to ensure there is data BEFORE the banking LinkTags
		.area _ssa					;   to work around a bug that drops the first AREA: tag in the ihx
		.area _tramp
		.area _music
        .area _sfxs

	.area _INITIALIZER
	.area _GSINIT
	.area _GSFINAL

	;; banking
	; Trying to keep the banks (except fixed) under 8k, in hopes of simplifying
	; the port to the TI. The fixed bank we can copy into RAM, hopefully, so it
	; will always be available just like here.
	.area _bank1
		.ascii "LinkTag:Bank1\0"
		.area _enemy

	.area _bank2
		.ascii "LinkTag:Bank2\0"
		.area _human

	.area _bank3
		.ascii "LinkTag:Bank3\0"
		.area _outfile1
		.area _selenac
		.area _selenap

	.area _bank4
		.ascii "LinkTag:Bank4\0"
		.area _difficulty
		.area _outfile0

	.area _bank5
		.ascii "LinkTag:Bank5\0"
		.area _boss1
		.area _boss2
		.area _boss3
		.area _boss4
		.area _boss5
		.area _ssd

	.area _bank6
		.ascii "LinkTag:Bank6\0"
		.area _bg
		.area _outfile2
		.area _outfile3

	.area _bank7
		.ascii "LinkTag:Bank7\0"
		.area _boss
		.area _font

	.area _bank8
		.ascii "LinkTag:Bank8\0"
		.area _wineasy
		.area _winmed
		.area _winhard

	.area _bank9
		.ascii "LinkTag:Bank9\0"
		.area _titlepage
		.area _titlec
		.area _titlep
		.area _ship1c
		.area _ship1p

	.area _bank10
		.ascii "LinkTag:Bank10\0"
		.area _ship2c
		.area _ship2p
		.area _ship3c
		.area _ship3p
		.area _ship4c
		.area _ship4p

	.area _bank11
		.ascii "LinkTag:Bank11\0"
		.area _ladybugc
		.area _ladybugp
		.area _ladybugscreenc
		.area _ladybugscreenp

	.area _bank12
		.ascii "LinkTag:Bank12\0"
		.area _qwertianc
		.area _qwertianp
		.area _attract
		.area _gnat1rlec
		.area _gnat1rlep
		.area _gnat2rlec
		.area _gnat2rlep

	.area _bank13
		.ascii "LinkTag:Bank13\0"
		.area _gnat3rlec
		.area _gnat3rlep
		.area _gnat4rlec
		.area _gnat4rlep
		.area _gnat5rlec
		.area _gnat5rlep

	.area _bank14
		.ascii "LinkTag:Bank14\0"

	.area _bank15
		.ascii "LinkTag:Bank15\0"

	;; end of list - needed for makemegacart. Must go before RAM areas.
	; This isn't used by anything else and should not contain data
	.area _ENDOFMAP

	;; RAM locations        
	.area _DATA
	.area _BSS
	.area _HEAP

	; actual end of mapping order - now the actual code begins. Could this have
	; gone in a separate file before crt0?
	; Anything you don't define will end up here, so gets put into RAM.
	; MakeMegacart should complain loudly if this happens.

	.module crt0
	.globl _main
	.globl _vdpinit
	.globl _my_nmi
	.globl _vdpLimi

	.area _HEADER(ABS)
	.org 0x8000

	.db 0x55, 0xaa			; Title screen and 12 second delay - swap 0x55 and 0xaa not to skip it.
	.dw 0								; sprite table stuff? - rarely used by the BIOS as a pointer
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw 0								; unknown - frequently used in BIOS routines as a pointer to a memory location (data is both written to and read from there, at least 124 bytes are used - maybe this is where the bios routines store most of their data, though with the common value of 0 this would be in the BIOS ROM itself - strange).
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw _startprog						; where to start execution of program.
	ei		; RST 0x08
	reti					
	ei		; RST 0x10
	reti
	ei		; RST 0x18
	reti
	ei		; RST 0x20
	reti
	ei		; RST 0x28
	reti
	ei		; RST 0x30
	reti
	ei		; RST 0x38 - spinner interrupt
	reti
	jp nmi		; NMI

	.ascii " / / NOT"

_startprog:
	; clear interrupt flag right off
	ld hl,#_vdpLimi
	ld (hl),#0

	; clear RAM before starting (except last 6 bytes)
	ld hl,#0x7000			; set copy source
	ld de,#0x7001			; set copy dest
	ld bc,#0x03f9			; set bytes to copy (1 less than size)
	ld (hl),#0				; set initial value (this gets copied through)
	ldir					; do it

	; reserved stack space:
	; 73FA - saved score lsb
	; 73FB - saved score MSB
	; 73FC - inverted score lsb
	; 73FD - inverted score msb
	; 73FE - saved score mode
	; 73FF - attract mode flag

	ld  sp, #0x73FA			; Set stack pointer directly above top of memory, reserving 6 bytes.
	ld	bc,#0xFFFE			; switch in code bank
   	ld	a,(bc)				; note that this does NOT set the local pBank variable, user code still must do that!
	;call gsinit			; Initialize global variables (linker is not setting this up?)
	call _vdpinit			; Initialize video system (also mutes SN)
	call _main
	rst 0x0					; Restart when main() returns.

	.area _BSS
_vdpLimi:					; 0x80 - interrupt set, 0x01 - enabled, other bits used by library
	.ds 1

    .area _CODE
nmi:
; all we do is set the MSB of _vdpLimi, and then check
; if the LSB is set. If so, we call user code now. if
; not, the library will deal with it when enabled.
	push af					; save flags (none affected, but play safe!)
	push hl

; increment score for timing
;	ld hl,#_score
;	inc (hl)
;	jp nz,skip
;	inc hl
;	inc (hl)
;skip: 
	
	ld hl,#_vdpLimi
	bit 0,(hl)				; check LSb (enable)
	jp z,notokay
	
; okay, full on call, save off the (other) regs
	push bc
	push de
	;push ix ; saved by callee
	push iy
	call _my_nmi			; call the lib version
	pop iy
	;pop ix
	pop de
	pop bc	
	jp clrup				

notokay:
	set 7,(hl)				; set MSb (flag)

clrup:
	pop hl					
	pop af
	retn

	.area _GSINIT
gsinit::
	.area _GSFINAL
	ret

