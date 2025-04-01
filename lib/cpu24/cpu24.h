// CPU identificator: GC24
#include <cpu24/proc/std.h>
#include <cpu24/proc/interrupts.h>
#include <cpu24/gpu.h>
#include <cpu24/spu.h>

#define BIOSNOBNK 16
#define BANKSIZE 65536

/*
  CPU info:
  Speed: 5THz
  State: Holy 2.0
*/

#define AX 0x00
#define BX 0x01
#define CX 0x02
#define DX 0x03
#define SI 0x04
#define GI 0x05
#define SP 0x06
#define BP 0x07

#define IF(ps) (ps & 0b01000000)
#define ZF(ps) (ps & 0b00000100)
#define NF(ps) (ps & 0b00000010)
#define CF(ps) (ps & 0b00000001)

#define SET_IF(ps) (ps |= 0b01000000)
#define SET_ZF(ps) (ps |= 0b00000100)
#define SET_NF(ps) (ps |= 0b00000010)
#define SET_CF(ps) (ps |= 0b00000001)

#define RESET_IF(ps) (ps &= 0b10111111)
#define RESET_ZF(ps) (ps &= 0b11111011)
#define RESET_NF(ps) (ps &= 0b11111101)
#define RESET_CF(ps) (ps &= 0b11111110)

#define printh(c, s) printf("%02X" s, c)

U8 gc_errno;

U0 Reset(GC* gc);
U0 PageDump(GC* gc, U8 page);
U0 StackDump(GC* gc, U16 c);
U0 RegDump(GC* gc);

// NOTE: every multibyte reads and writes are done in Little Endian

/* ReadWord -- Read a 16-bit value from memory */
gcword ReadWord(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8);
}

/* Read24 -- Read a 24-bit value from memory */
U32 Read24(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8) + (gc->mem[addr+2] << 16);
}

/* WriteWord -- Write a 16-bit value to memory */
U0 WriteWord(GC* gc, U32 addr, U16 val) {
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = (val >> 8);
}

/* Write24 -- Write a 24-bit value to memory */
U0 Write24(GC* gc, U32 addr, U32 val) {
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = ((val >> 8) % 256);
  gc->mem[addr+2] = ((val >> 16) % 256);
}

/* StackPush -- Push a 24-bit value onto the stack */
gcbyte StackPush(GC* gc, U32 val) {
  gc->mem[gc->reg[SP].word] = (val >> 16);
  gc->mem[gc->reg[SP].word-1] = ((val >> 8) % 256);
  gc->mem[gc->reg[SP].word-2] = (val % 256);
  gc->reg[SP].word -= 3;
  return 0;
}

/* StackPop -- Pop a 24-bit value and return it */
U32 StackPop(GC* gc) {
  gc->reg[SP].word += 3;
  return Read24(gc, gc->reg[SP].word-2);
}

/* ReadRegClust -- A function to read the register cluster
  A register cluster is a byte that consists of 2 register pointers:
  It is divided into 3 parts like this:
    00 001 100
  The first 3-byte value is a first operand, and the last 3-byte value
  is a second operand. The first 2 bytes of a byte are unused.
  The function returns a structure with two members as register pointers.
*/
gcrc_t ReadRegClust(U8 clust) {
  gcrc_t rc = {((clust&0b00111000)>>3), (clust&0b00000111)};
    return rc;
}

// The UNK function is ran when an illegal opcode is encountered
U8 UNK(GC* gc) {
  fprintf(stderr, "\033[31mIllegal\033[0m instruction \033[33m%02X\033[0m\nAt position %06X\n", gc->mem[gc->PC], gc->PC);
  old_st_legacy;
  gc_errno = 1;
  return 1;
}

/* Instructions implementation start */
// 00           hlt
U8 HLT(GC* gc) {
  return 1;
}

// 01           trap
U8 TRAP(GC* gc) {
  old_st_legacy;
  ExecD(gc, 1);
  gc->PC++;
  return 0;
}

// 03           sti
U8 STI(GC* gc) {
  Write24(gc, (((gc->mem[gc->PC+1]-0x80)*3)+0xFF0000), gc->reg[SI].word);
  gc->PC += 2;
  return 0;
}

// 04           iret
U8 IRET(GC* gc) {
  gc->PS = StackPop(gc);
  gc->PC = StackPop(gc);
  return 0;
}

// 08-0F        mul reg imm24
U8 MULri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x08) % 8].word *= Read24(gc, gc->PC+1);
  gc->PC += 4;
  return 0;
}

// 10-17        sub reg imm24
U8 SUBri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x10) % 8].word -= Read24(gc, gc->PC+1);
  gc->PC += 4;
  return 0;
}

// 18-1F        sub reg byte[imm24]
U8 SUBrb(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x18) % 8].word -= gc->mem[Read24(gc, gc->PC+1)];
  gc->PC += 4;
  return 0;
}

// 20-27        inx reg
U8 INXr(GC* gc) {
  gc->reg[gc->mem[gc->PC]-0x20].word++;
  gc->PC++;
  return 0;
}

// 28-2F        inx reg
U8 DEXr(GC* gc) {
  gc->reg[gc->mem[gc->PC]-0x28].word--;
  gc->PC++;
  return 0;
}

// 30           inx #imm24
U8 INXb(GC* gc) {
  U32 addr = Read24(gc, gc->PC+1);
  gc->mem[addr]++;
  gc->PC += 4;
  return 0;
}

// 32           dex #imm24
U8 DEXb(GC* gc) {
  U32 addr = Read24(gc, gc->PC+1);
  gc->mem[addr]--;
  gc->PC += 4;
  return 0;
}

// 40           inx @imm24
U8 INXw(GC* gc) {
  U32 addr = Read24(gc, gc->PC+1);
  U16 a = ReadWord(gc, addr);
  WriteWord(gc, addr, a+1);
  gc->PC += 4;
  return 0;
}

// 42           dex @imm24
U8 DEXw(GC* gc) {
  U32 addr = Read24(gc, gc->PC+1);
  U16 a = ReadWord(gc, addr);
  WriteWord(gc, addr, a-1);
  gc->PC += 4;
  return 0;
}

// 41           int imm8
U8 INT(GC* gc) {
  if (!IF(gc->PS)) goto intend;
  if (gc->mem[gc->PC+1] >= 0x80) { // Custom interrupt
    StackPush(gc, gc->PC+2); // Return address
    StackPush(gc, gc->PS); // Flags
    gc->PC = Read24(gc, ((gc->mem[gc->PC+1]-0x80)*3)+0xFF0000);
    return 0;
  }
  switch (gc->mem[gc->PC+1]) {
  case INT_EXIT:
    gc_errno = StackPop(gc);
    return 1;
  case INT_READ:
    char a = getchar();
    StackPush(gc, a);
    break;
  case INT_WRITE:
    putchar(StackPop(gc));
    fflush(stdout);
    break;
  case INT_RESET:
    Reset(gc);
    return 0;
  case INT_VIDEO_FLUSH:
    GGpage(gc);
    break;
  case INT_VIDEO_CLEAR:
    GGflush(gc);
    break;
  case INT_RAND:
    gc->reg[DX].word = rand() % 65536;
    break;
  case INT_DATE:
    gc->reg[DX].word = GC_GOVNODATE();
    break;
  case INT_WAIT:
    usleep((U32)(gc->reg[DX].word)*1000); // the maximum is about 65.5 seconds
    break;
  case INT_BEEP:
    double freq = (double)StackPop(gc);
    PlayBeep(freq);
    break;
  default:
    fprintf(stderr, "gc24: \033[91mIllegal\033[0m hardware interrupt: $%02X\n", gc->mem[gc->PC+1]);
    return 1;
  }
  intend: gc->PC += 2;
  return 0;
}

// 37           cmp rc
U8 CMPrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  I16 val0 = gc->reg[rc.x].word;
  I16 val1 = gc->reg[rc.y].word;

  if (!(val0 - val1))    SET_ZF(gc->PS);
  else                   RESET_ZF(gc->PS);
  if ((val0 - val1) < 0) SET_NF(gc->PS);
  else                   RESET_NF(gc->PS);

  gc->PC += 2;
  return 0;
}

// 38           and rc
U8 ANDrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word &= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// 39           ora rc
U8 ORArc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word |= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// 3A           xor rc
U8 XORrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word ^= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// 47           add rc
U8 ADDrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word += gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// 48-4F        add reg imm24
U8 ADDri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x48) % 8].word += Read24(gc, gc->PC+1);
  gc->PC += 4;
  return 0;
}

// 50-57        add reg byte[imm24]
U8 ADDrb(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x50) % 8].word += gc->mem[Read24(gc, gc->PC+1)];
  gc->PC += 4;
  return 0;
}

// 58-5F        mov reg word[imm24]
U8 ADDrw(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x58) % 8].word += ReadWord(gc, Read24(gc, gc->PC+1));
  gc->PC += 4;
  return 0;
}

// 60-67        mov byte[imm24] reg
U8 ADDbr(GC* gc) {
  gc->mem[Read24(gc, gc->PC+1)] += gc->reg[(gc->mem[gc->PC]-0x60) % 8].word;
  gc->PC += 4;
  return 0;
}

// 68-6F        add word[imm24] reg
U8 ADDwr(GC* gc) {
  U16 addr = Read24(gc, gc->PC+1);
  U16 w = ReadWord(gc, addr);
  WriteWord(gc, addr, w+gc->reg[(gc->mem[gc->PC]-0x60) % 8].word);
  gc->PC += 4;
  return 0;
}

// 70-77        cmp reg imm24
U8 CMPri(GC* gc) {
  I16 val0 = gc->reg[(gc->mem[gc->PC]-0x70) % 8].word;
  I16 val1 = Read24(gc, gc->PC+1);

  if (!(val0 - val1))    SET_ZF(gc->PS);
  else                   RESET_ZF(gc->PS);
  if ((val0 - val1) < 0) SET_NF(gc->PS);
  else                   RESET_NF(gc->PS);

  gc->PC += 4;
  return 0;
}

// 78           call imm24
U8 CALLa(GC* gc) {
  StackPush(gc, gc->PC+4);
  gc->PC = Read24(gc, gc->PC+1);
  return 0;
}

// 79           ret
U8 RET(GC* gc) {
  gc->PC = StackPop(gc);
  return 0;
}

// 7E           stob rc
U8 STOBc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->mem[gc->reg[rc.x].word] = gc->reg[rc.y].word;
  gc->reg[rc.x].word++;
  gc->PC += 2;
  return 0;
}

// 7F           lodb rc
U8 LODBc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.y].word = gc->mem[gc->reg[rc.x].word];
  gc->reg[rc.x].word++;
  gc->PC += 2;
  return 0;
}

// 8E           stow rc
U8 STOWc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  WriteWord(gc, gc->reg[rc.x].word, gc->reg[rc.y].word);
  gc->reg[rc.x].word += 2;
  gc->PC += 2;
  return 0;
}

// 8F           lodw rc
U8 LODWc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.y].word = ReadWord(gc, gc->reg[rc.x].word);
  gc->reg[rc.x].word += 2;
  gc->PC += 2;
  return 0;
}

// 9E           stoh rc
U8 STOHc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  Write24(gc, gc->reg[rc.x].word, gc->reg[rc.y].word);
  gc->reg[rc.x].word += 3;
  gc->PC += 2;
  return 0;
}

// 9F           lodh rc
U8 LODHc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.y].word = Read24(gc, gc->reg[rc.x].word);
  gc->reg[rc.x].word += 3;
  gc->PC += 2;
  return 0;
}

// 80-83        mov reg imm24
U8 DIVri(GC* gc) {
  U32 a = Read24(gc, gc->PC+1);
  gc->reg[DX].word = (gc->reg[(gc->mem[gc->PC]-0x80) % 4].word % a);
  gc->reg[(gc->mem[gc->PC]-0x80) % 4].word /= a;
  gc->PC += 4;
  return 0;
}

// 86           jmp imm24
U8 JMPa(GC* gc) {
  gc->PC = Read24(gc, gc->PC+1);
  return 0;
}

// 90-97        sub reg word[imm24]
U8 SUBrw(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x90) % 8].word -= ReadWord(gc, Read24(gc, gc->PC+1));
  gc->PC += 4;
  return 0;
}

// A0           je imm24
U8 JEa(GC* gc) {
  if (ZF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
    RESET_ZF(gc->PS);
  }
  else gc->PC += 4;
  return 0;
}

// A1           jne imm24
U8 JNEa(GC* gc) {
  if (!ZF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
  }
  else gc->PC += 4;
  return 0;
}

// A2           jc imm24
U8 JCa(GC* gc) {
  if (CF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
    RESET_CF(gc->PS);
  }
  else gc->PC += 4;
  return 0;
}

// A3           jnc imm24
U8 JNCa(GC* gc) {
  if (!CF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
  }
  else gc->PC += 4;
  return 0;
}

// A4           js imm24
U8 JSa(GC* gc) {
  if (!NF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
  }
  else gc->PC += 4;
  return 0;
}

// A5           jn imm24
U8 JNa(GC* gc) {
  if (NF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
    RESET_NF(gc->PS);
  }
  else gc->PC += 4;
  return 0;
}

// A6           ji imm24
U8 JIa(GC* gc) {
  if (IF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
  }
  else gc->PC += 4;
  return 0;
}

// A7           jni imm24
U8 JNIa(GC* gc) {
  if (!IF(gc->PS)) {
    gc->PC = Read24(gc, gc->PC+1);
  }
  else gc->PC += 4;
  return 0;
}

// A8           re imm24
U8 RE(GC* gc) {
  if (ZF(gc->PS)) {
    gc->PC = StackPop(gc);
    RESET_ZF(gc->PS);
  }
  else gc->PC++;
  return 0;
}

// A9           rne imm24
U8 RNE(GC* gc) {
  if (!ZF(gc->PS)) {
    gc->PC = StackPop(gc);
  }
  else gc->PC++;
  return 0;
}

// AA           rc imm24
U8 RC(GC* gc) {
  if (CF(gc->PS)) {
    gc->PC = StackPop(gc);
    RESET_CF(gc->PS);
  }
  else gc->PC++;
  return 0;
}

// AB           rnc imm24
U8 RNC(GC* gc) {
  if (!CF(gc->PS)) {
    gc->PC = StackPop(gc);
  }
  else gc->PC++;
  return 0;
}

// AC           rs imm24
U8 RS(GC* gc) {
  if (!NF(gc->PS)) {
    gc->PC = StackPop(gc);
  }
  else gc->PC++;
  return 0;
}

// AD           rn imm24
U8 RN(GC* gc) {
  if (NF(gc->PS)) {
    gc->PC = StackPop(gc);
    RESET_NF(gc->PS);
  }
  else gc->PC++;
  return 0;
}

// AE           ri imm24
U8 RI(GC* gc) {
  if (IF(gc->PS)) {
    gc->PC = StackPop(gc);
    RESET_IF(gc->PS);
  }
  else gc->PC++;
  return 0;
}

// AF           rni imm24
U8 RNI(GC* gc) {
  if (!IF(gc->PS)) {
    gc->PC = StackPop(gc);
  }
  else gc->PC++;
  return 0;
}

// B0           push imm24
U8 PUSHi(GC* gc) {
  StackPush(gc, Read24(gc, gc->PC+1));
  gc->PC += 4;
  return 0;
}

// B5           push reg
U8 PUSHr(GC* gc) {
  StackPush(gc, gc->reg[gc->mem[gc->PC+1]].word);
  gc->PC += 2;
  return 0;
}

// B6           pop reg
U8 POPr(GC* gc) {
  gc->reg[gc->mem[gc->PC+1]].word = StackPop(gc);
  gc->PC += 2;
  return 0;
}

// B8           loop imm24
U8 LOOPa(GC* gc) {
  if (gc->reg[CX].word) {
    gc->reg[CX].word--;
    gc->PC = Read24(gc, gc->PC+1);
  }
  else {
    gc->PC += 4;
  }
  return 0;
}

// B9           ldds
U8 LDDS(GC* gc) {
  gc->reg[AX].word = gc->rom[gc->reg[SI].word];
  gc->PC++;
  return 0;
}

// BA           lddg
U8 LDDG(GC* gc) {
  gc->reg[AX].word = gc->rom[gc->reg[SI].word];
  gc->PC++;
  return 0;
}

// BB           stds
U8 STDS(GC* gc) {
  gc->rom[gc->reg[SI].word] = gc->reg[gc->mem[gc->PC+1]].word;
  gc->PC++;
  return 0;
}

// BC           stdg
U8 STDG(GC* gc) {
  gc->rom[gc->reg[GI].word] = gc->reg[gc->mem[gc->PC+1]].word;
  gc->PC++;
  return 0;
}

// BF           pow rc
U8 POWrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word = pow(gc->reg[rc.x].word, gc->reg[rc.y].word);
  gc->PC += 2;
  return 0;
}

// C0-C7        mov reg imm24
U8 MOVri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xC0) % 8].word = Read24(gc, gc->PC+1);
  gc->PC += 4;
  return 0;
}

// CF           mov rc
U8 MOVrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word = gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// C8           sub rc
U8 SUBrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word -= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// C9           mul rc
U8 MULrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[rc.x].word *= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// CA           div rc
U8 DIVrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->PC+1]);
  gc->reg[DX].word = gc->reg[rc.x].word % gc->reg[rc.y].word; // Remainder into %dx
  gc->reg[rc.x].word /= gc->reg[rc.y].word;
  gc->PC += 2;
  return 0;
}

// D0-D7        mov reg byte[imm24]
U8 MOVrb(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xD0) % 8].word = gc->mem[Read24(gc, gc->PC+1)];
  gc->PC += 4;
  return 0;
}

// D8-E0        mov reg word[imm24]
U8 MOVrw(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xD8) % 8].word = ReadWord(gc, Read24(gc, gc->PC+1));
  gc->PC += 4;
  return 0;
}

// E0-E7        mov byte[imm24] reg
U8 MOVbr(GC* gc) {
  gc->mem[Read24(gc, gc->PC+1)] = gc->reg[(gc->mem[gc->PC]-0xE0) % 8].word;
  gc->PC += 4;
  return 0;
}

// E8-EF        mov word[imm24] reg
U8 MOVwr(GC* gc) {
  WriteWord(gc, Read24(gc, gc->PC+1), gc->reg[(gc->mem[gc->PC]-0xE8) % 8].word);
  gc->PC += 4;
  return 0;
}

/* Instructions implementation end */

U8 PG0F(GC*); // Page 0F - Additional instructions page

// Zero page instructions
U8 (*INSTS[256])() = {
  &HLT  , &TRAP , &UNK  , &STI  , &IRET , &UNK  , &UNK  , &UNK  , &MULri, &MULri, &MULri, &MULri, &MULri, &MULri, &MULri, &MULri,
  &SUBri, &SUBri, &SUBri, &SUBri, &SUBri, &SUBri, &SUBri, &SUBri, &SUBrb, &SUBrb, &SUBrb, &SUBrb, &SUBrb, &SUBrb, &SUBrb, &SUBrb,
  &INXr , &INXr , &INXr , &INXr , &INXr , &INXr , &INXr , &INXr , &DEXr , &DEXr , &DEXr , &DEXr , &DEXr , &DEXr , &DEXr , &DEXr ,
  &INXb , &UNK  , &DEXb , &UNK  , &UNK  , &UNK  , &UNK  , &CMPrc, &ANDrc, &ORArc, &XORrc, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &INXw , &INT  , &DEXw , &UNK  , &UNK  , &UNK  , &UNK  , &ADDrc, &ADDri, &ADDri, &ADDri, &ADDri, &ADDri, &ADDri, &ADDri, &ADDri,
  &ADDrb, &ADDrb, &ADDrb, &ADDrb, &ADDrb, &ADDrb, &ADDrb, &ADDrb, &ADDrw, &ADDrw, &ADDrw, &ADDrw, &ADDrw, &ADDrw, &ADDrw, &ADDrw,
  &ADDbr, &ADDbr, &ADDbr, &ADDbr, &ADDbr, &ADDbr, &ADDbr, &ADDbr, &ADDwr, &ADDwr, &ADDwr, &ADDwr, &ADDwr, &ADDwr, &ADDwr, &ADDwr,
  &CMPri, &CMPri, &CMPri, &CMPri, &CMPri, &CMPri, &CMPri, &CMPri, &CALLa, &RET  , &UNK  , &UNK  , &UNK  , &UNK  , &STOBc, &LODBc,
  &DIVri, &DIVri, &DIVri, &DIVri, &DIVri, &DIVri, &JMPa , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &STOWc, &LODWc,
  &SUBrw, &SUBrw, &SUBrw, &SUBrw, &SUBrw, &SUBrw, &SUBrw, &SUBrw, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &STOHc, &LODHc,
  &JEa  , &JNEa , &JCa  , &JNCa , &JSa  , &JNa  , &JIa  , &JNIa , &RE   , &RNE  , &RC   , &RNC  , &RS   , &RN   , &RI   , &RNI  ,
  &PUSHi, &UNK  , &UNK  , &UNK  , &UNK  , &PUSHr, &POPr , &UNK  , &LOOPa, &LDDS , &LDDG , &STDS , &STDG , &UNK  , &UNK  , &POWrc,
  &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &SUBrc, &MULrc, &DIVrc, &UNK  , &UNK  , &UNK  , &UNK  , &MOVrc,
  &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw,
  &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK
};

U8 (*INSTS_PG0F[256])() = {
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK
};

U8 PG0F(GC* gc) {   // 0FH
  gc->PC++;
  return (INSTS_PG0F[gc->mem[gc->PC]])(gc);
}

U0 Reset(GC* gc) {
  gc->reg[SP].word = 0xFEFFFF;
  gc->reg[BP].word = 0xFEFFFF;
  gc->PC = 0x700000;

  // Reset the general purpose registers
  for (U8 i = 0; i < 6; i++) 
    gc->reg[i].word = 0x000000;

  gc->PS = 0b01000000;
}

U0 PageDump(GC* gc, U8 page) {
  for (U16 i = (page*256); i < (page*256)+256; i++) {
    if (!(i % 16)) putchar(10);
    printf("%02X ", gc->mem[i]);
  }
}

U0 StackDump(GC* gc, U16 c) {
  for (U32 i = 0xFEFFFF; i > 0xFEFFFF-c; i--) {
    if (i != gc->reg[SP].word) printf("%06X: %02X\n", i, gc->mem[i]);
    else                       printf("\033[92m%06X: %02X\033[0m\n", i, gc->mem[i]);
  }
}

U0 MemDump(GC* gc, U32 start, U32 end, U8 newline) {
  for (U32 i = start; i < end; i++) {
    printf("%02X ", gc->mem[i]);
  }
  putchar(8);
  putchar(10*newline);
}

U0 RegDump(GC* gc) {
  printf("pc: %06X;  a: %06X\n", gc->PC, gc->reg[AX]);
  printf("b: %06X;     c: %06X\n", gc->reg[BX], gc->reg[CX]);
  printf("d: %06X;     s: %06X\n", gc->reg[DX], gc->reg[SI]);
  printf("g: %06X;     sp: %06X\n", gc->reg[GI], gc->reg[SP]);
  printf("ps: %08b; bp: %06X\n", gc->PS, gc->reg[BP]);
  printf("   -I---ZNC\033[0m\n");
}

U8 Exec(GC* gc, const U32 memsize) {
  U8 exc = 0;
  U8 step = 0;
  U32 insts = 0;
  execloop:
    exc = (INSTS[gc->mem[gc->PC]])(gc);
    insts++;
    usleep(1000);
    if (exc != 0) {
      printf("gc24: executed 1E%.10lf instructions\n", log10(insts));
      return gc_errno;
    }
    goto execloop;
  return exc;
}
