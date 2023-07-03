//
// Data file LADYBUGSCREEN.TIAC - Jul 02, 2023
//

#ifndef BIN2INC_HEADER_ONLY
const unsigned char LADYSCREENC[] = {
	 0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,	// 00000000 ................ //
	 0xFF,0x11,0xAC,0x11,0x01,0xF1,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,	// 00000010 ................ //
	 0xF4,0x11,0x01,0xF1,0xFF,0x11,0x9F,0x11,0x01,0xF1,0xBA,0x11,0x01,0xF1,0xFF,0x11,	// 00000020 ................ //
	 0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,	// 00000030 ................ //
	 0x9D,0x11,0x01,0xF1,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,	// 00000040 ................ //
	 0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0x85,0x11,	// 00000050 ................ //
	 0x85,0x81,0x83,0x11,0x85,0x81,0xFF,0x11,0xF1,0x11,0x88,0x18,0x01,0x81,0x87,0x18,	// 00000060 ................ //
	 0xB5,0x11,0x01,0xF1,0xFF,0x11,0xBB,0x11,0x83,0x81,0x85,0x11,0x83,0x81,0xFF,0x11,	// 00000070 ................ //
	 0xFF,0x11,0xFF,0x11,0xFF,0x11,0xFF,0x11,0x02,0x11,0x11                         	// 00000080 ........... //
};
#else
extern const unsigned char LADYSCREENC[];

// Size of data in above array
#define SIZE_OF_LADYSCREENC 139
#endif