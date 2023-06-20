// packboss.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlstr.h>

// this simple function reads a BMP file, and outputs the approach BOSS file
// which is RLE packed the same as my TI Artist images, that is:
// Count byte - if high bit is set, it's a run of the next byte, else it's
// a stream of data. Assumes a mono BMP, does not store header or color data

// 32k max - even that's really too much :)
unsigned char buf[32*1024], buf2[32*1024];
int nLineCnt=0;

void outbyte(int byte, FILE *fp) {
	fprintf(fp, "0x%02X,", byte);
	nLineCnt++;
	if ((nLineCnt%16) == 0) fprintf(fp, "\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc<3) {
		printf("packboss <file.bmp> <file.c> <label name>\n");
		printf("Does RLE compression of a small mono BMP file.\n");
		printf("Does not save output size!\n");
		return 2;
	}

	printf("%s -> %s\n", argv[1], argv[2]);

	FILE *fp=fopen(argv[1], "rb");
	if (NULL == fp) {
		printf("can't open input file\n");
		return 5;
	}

	int nMaxSize = fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	if ((buf[0]!='B') || (buf[1]!='M')) {
		printf("Not a BMP file?\n");
		return 6;
	}

	int nOffset;
	nOffset=buf[10] | (buf[11]<<8) | (buf[12]<<16) | (buf[13]<<24);
	if (nOffset != 0x3e) {
		printf("Offset not as expected for mono BMP! Will try anyway.\n");
	}
	nMaxSize-=nOffset;

	int nWidth = buf[0x12] | (buf[0x13]<<8) | (buf[0x14]<<16) | (buf[0x15]<<24);
	int nHeight= buf[0x16] | (buf[0x17]<<8) | (buf[0x18]<<16) | (buf[0x19]<<24);

	int nCurW;
	int nCurH;
	int stride = nWidth/8;
	while (stride%4) stride++;	// account for padding per line

	// scramble the data into cells so we can calculate proper runs
	int nOut=0;
	for (nCurH=0; nCurH<nHeight; nCurH+=8) {
		for (nCurW=0; nCurW<nWidth; nCurW+=8) {
			for (int idx=0; idx<8; idx++) {
				// calculate where to read from - remember the BMP was upside down and has an offset!
				int nIn;
				int nLine = nCurH + idx;	// nCurH already steps by 8
				nLine = nHeight-nLine-1;	// invert for BMP
				nIn = (nLine*stride)+(nCurW/8)+nOffset;	// byte address inside BMP
				buf2[nOut++]=buf[nIn];
			}
		}
	}

	// open output file
	fp=fopen(argv[2], "w");
	if (NULL == fp) {
		printf("Can't open output file\n");
		return 10;
	}
	
	// write a header
	fprintf(fp, "const unsigned char %s[] = {\n", argv[3]);
	nLineCnt=0;

	// now work on buf2, which is formatted appropriately
	nOffset=0;
	while (nOffset < nMaxSize) {
		// decide whether to do a run or data
		if ((buf2[nOffset]==buf2[nOffset+1]) && (buf2[nOffset] == buf2[nOffset+2])) {
			// worth doing an RLE run
			// work out for how far!
			int i=nOffset+3;
			while (buf2[i] == buf2[nOffset]) {
				i++;
				if (i>=nMaxSize) break;
				if (i-nOffset >= 127) break;
			}
			outbyte(0x80|(i-nOffset), fp);
			outbyte(buf2[nOffset], fp);
			nOffset=i;
		} else {
			// not worth doing RLE -- see for how long
			int i=nOffset+1;
			while ((buf2[i]!=buf2[i+1]) || (buf2[i] != buf2[i+2])) {
				i++;
				if (i>=nMaxSize) break;
				if (i-nOffset >= 127) break;
			}
			outbyte(i-nOffset, fp);
			for (int idx=0; idx<i-nOffset; idx++) {
				outbyte(buf2[nOffset+idx], fp);
			}
			nOffset=i;
		}
	}

	// close the file
	if (nLineCnt%16) fprintf(fp, "\n");
	fprintf(fp, "};\n\n");
	fclose(fp);
	
	printf("Wrote %d bytes over %d bytes read\n", nLineCnt, nMaxSize);

	return 0;
}

