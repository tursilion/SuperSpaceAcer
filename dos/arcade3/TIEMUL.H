/* TI Emulator header */

#define Byte unsigned char
#define Word unsigned short

#define FormatI { Td=(in&0x0c00)>>10; Ts=(in&0x0030)>>4; D=(in&0x03c0)>>6; S=(in&0x000f); B=(in&0x1000)>>12; fixDS(); }
#define FormatII { D=(in&0x00ff); }
#define FormatIII { Td=0; Ts=(in&0x0030)>>4; D=(in&0x03c0)>>6; S=(in&0x000f); B=0; fixDS(); }
#define FormatIV { Td=0; D=0; Ts=(in&0x0030)>>4; S=(in&0x000f); B=0; fixDS(); D=(in&0x03c0)>>6; }
#define FormatV { D=(in&0x00f0)>>4; S=(in&0x000f); S=WP+(S<<1); }
#define FormatVI { Td=0; D=0; Ts=(in&0x0030)>>4; S=in&0x000f; fixDS(); }
#define FormatVII {}
#define FormatVIII_0 { D=(in&0x000f); D=WP+(D<<1); }
#define FormatVIII_1 { D=(in&0x000f); D=WP+(D<<1); S=romword(PC); PC=PC+2; }
#define FormatIX  { D=0; Td=0; Ts=(in&0x0030)>>4; S=(in&0x000f); fixDS(); D=(in&0x03c0)>>6; }

void VDPmain(void);
void VDPdisplay(void);
void gettables(void);
void draw_debug(void);
void restore_video(void);
void VDPgraphics(void);
void VDPtext(void);
void VDPbitmap(void);
void save(Word);

int main(int,char*[]);
void startvdp(void);
void startsound(void);
void warn(char[]);
void fail(char[]);
Word romword(Word);
void wrword(Word,Word);
void emulti(void);
void readroms(void);
Word hex2dec(char[]);
void do1(void);
void opcode0(Word);
void opcode02(Word);
void opcode03(Word);
void opcode04(Word);
void opcode05(Word);
void opcode06(Word);
void opcode07(Word);
void opcode1(Word);
void opcode2(Word);
void opcode3(Word);
Byte rcpubyte(Word);
void wcpubyte(Word,Byte);
Byte rvdpbyte(Word);
void wvdpbyte(Word,Byte);
void wVDPreg(Byte,Byte);
void wsndbyte(Byte);
Byte rgrmbyte(Word);
void wgrmbyte(Word,Byte);
void wcru(Word,int);
int rcru(Word);
void fixDS(void);
void parity(Byte);
void op_a(void);
void op_ab(void);
void op_abs(void);
void op_ai(void);
void op_dec(void);
void op_dect(void);
void op_div(void);
void op_inc(void);
void op_inct(void);
void op_mpy(void);
void op_neg(void);
void op_s(void);
void op_sb(void);
void op_b(void);
void op_bl(void);
void op_blwp(void);
void op_jeq(void);
void op_jgt(void);
void op_jhe(void);
void op_jh(void);
void op_jl(void);
void op_jle(void);
void op_jlt(void);
void op_jmp(void);
void op_jnc(void);
void op_jne(void);
void op_jno(void);
void op_jop(void);
void op_joc(void);
void op_rtwp(void);
void op_x(void);
void op_xop(void);
void op_c(void);
void op_cb(void);
void op_ci(void);
void op_coc(void);
void op_czc(void);
void op_ldcr(void);
void op_sbo(void);
void op_sbz(void);
void op_stcr(void);
void op_tb(void);
void op_ckof(void);
void op_ckon(void);
void op_idle(void);
void op_rset(void);
void op_lrex(void);
void op_li(void);
void op_limi(void);
void op_lwpi(void);
void op_mov(void);
void op_movb(void);
void op_stst(void);
void op_stwp(void);
void op_swpb(void);
void op_andi(void);
void op_ori(void);
void op_xor(void);
void op_inv(void);
void op_clr(void);
void op_seto(void);
void op_soc(void);
void op_socb(void);
void op_szc(void);
void op_szcb(void);
void op_sra(void);
void op_srl(void);
void op_sla(void);
void op_src(void);
void buildcpu(void);
void op_bad(void);

void do_dsrlnk(void);
int keycall(int key);
char *GetPtr(char *szName);
void setlpt(int lpt_number);
void setbase(int port_number);
void readpad(int pad_number);
char *RAMfgets(char *buffer, int length, char *fp);

