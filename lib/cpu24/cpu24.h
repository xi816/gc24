// CPU identificator: GC24
#include <cpu24/proc/std.h>
#include <cpu24/proc/interrupts.h>
#include <cpu24/gpu.h>

// CPU info:
// Speed: 1THz
// State: Holy

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

U8 gc_errno;

U0 Reset(GC* gc);
U0 PageDump(GC* gc, U8 page);
U0 StackDump(GC* gc, U16 c);
U0 RegDump(GC* gc);

U32 FullSP(GC* gc) {
  return (gc->SPAGE << 16) + gc->reg[SP].word;
}

gcword ReadWord(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8);
}

U32 Read24(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8) + (gc->mem[addr+2] << 16);
}

U0 WriteWord(GC* gc, U32 addr, U16 val) {
  // Least significant byte goes first
  // $1448 -> $48,$14
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = (val >> 8);
}

gcbyte StackPush(GC* gc, U32 val) {
  gc->mem[FullSP(gc)] = (val >> 16);
  gc->mem[FullSP(gc)-1] = ((val >> 8) % 256);
  gc->mem[FullSP(gc)-2] = (val % 256);
  gc->reg[SP].word -= 3;
  return 0;
}

gcword StackPop(GC* gc) {
  gc->reg[SP].word += 3;
  return Read24(gc, FullSP(gc)-2);
}

gcrc_t ReadRegClust(U8 clust) { // Read a register cluster
  // The register address is 4 bytes
  gcrc_t rc = {((clust&0b00111000)>>3), (clust&0b00000111)};
  return rc;
}

U8 UNK(GC* gc) {    // Unknown instruction
  fprintf(stderr, "\033[31mIllegal\033[0m instruction \033[33m%02X\033[0m\nAt position %04X\n", gc->mem[gc->PC], gc->PC);
  old_st_legacy;
  gc_errno = 1;
  return 1;
}

/* Instructions implementation start */
// 00           hlt
U8 HLT(GC* gc) {
  return 1;
}

// 41           int imm8
U8 INT(GC* gc) {
  if (!IF(gc->PS)) goto intend;
  if (gc->mem[gc->PC+1] >= 0x80) { // Custom interrupt
    gc->PC = Read24(gc, ((gc->mem[gc->PC+1]-0x80+1)*3-3)+0xFF0000);
    return 0;
  }
  switch (gc->mem[gc->PC+1]) {
  case INT_EXIT:
    gc_errno = StackPop(gc);
    return 1;
  case INT_READ:
    StackPush(gc, getchar());
    break;
  case INT_WRITE:
    putchar(StackPop(gc));
    fflush(stdout);
    break;
  case INT_RESET:
    Reset(gc);
    return 0;
  case INT_VIDEO_FLUSH:
    GGpage(gc, gc->renderer);
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
  default:
    fprintf(stderr, "gc24: \033[91mIllegal\033[0m hardware interrupt: $%02X\n", gc->mem[gc->PC+1]);
    return 1;
  }
  intend: gc->PC += 2;
  return 0;
}

// 48-4F        add reg imm16
U8 ADDri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0x48) % 8].word += ReadWord(gc, gc->PC+1);
  gc->PC += 3;
  return 0;
}

// 86           jmp imm24
U8 JMPa(GC* gc) {
  gc->PC = Read24(gc, gc->PC+1);
  return 0;
}

// B0           push imm16
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

// C0-C7        mov reg imm16
U8 MOVri(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xC0) % 8].word = ReadWord(gc, gc->PC+1);
  gc->PC += 3;
  return 0;
}

// D0-D7        mov reg byte[imm16]
U8 MOVrb(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xD0) % 8].word = gc->mem[Read24(gc, gc->PC+1)];
  gc->PC += 4;
  return 0;
}

// D8-E0        mov reg word[imm16]
U8 MOVrw(GC* gc) {
  gc->reg[(gc->mem[gc->PC]-0xD8) % 8].word = ReadWord(gc, Read24(gc, gc->PC+1));
  gc->PC += 4;
  return 0;
}

// E0-E7        mov byte[imm16] reg
U8 MOVbr(GC* gc) {
  gc->mem[Read24(gc, gc->PC+1)] = gc->reg[(gc->mem[gc->PC]-0xE0) % 8].byte;
  gc->PC += 4;
  return 0;
}

// E8-EF        mov word[imm16] reg
U8 MOVwr(GC* gc) {
  WriteWord(gc, Read24(gc, gc->PC+1), gc->reg[(gc->mem[gc->PC]-0xE8) % 8].word);
  gc->PC += 4;
  return 0;
}

/* Instructions implementation end */

U8 PG0F(GC*); // Page 0F - Additional instructions page

// Zero page instructions
U8 (*INSTS[256])() = {
  &HLT  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &INT  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &JMPa , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &PUSHi, &UNK  , &UNK  , &UNK  , &UNK  , &PUSHr, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &MOVri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrb, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw, &MOVrw,
  &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVbr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr, &MOVwr,
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
  gc->reg[SP].word = 0x00FFFF;
  gc->reg[BP].word = 0x00FFFF;
  gc->PC = 0x030000;

  gc->PS = 0b01000000;
}

U0 PageDump(GC* gc, U8 page) {
  for (U16 i = (page*256); i < (page*256)+256; i++) {
    if (!(i % 16)) putchar(10);
    printf("%02X ", gc->mem[i]);
  }
}

U0 StackDump(GC* gc, U16 c) {
  for (U32 i = (gc->SPAGE*65536)+0xFFFF; i > (gc->SPAGE*65536)+0xFFFF-c; i--) {
    printf("%04X: %02X\n", i, gc->mem[i]);
  }
}

U0 RegDump(GC* gc) {
  printf("pc: %06X;  a: %04X\n", gc->PC, gc->reg[AX]);
  printf("b: %04X;     c: %04X\n", gc->reg[BX], gc->reg[CX]);
  printf("d: %04X;     s: %04X\n", gc->reg[DX], gc->reg[SI]);
  printf("g: %04X;     spage:sp: %02X%04X\n", gc->reg[GI], gc->SPAGE, gc->reg[SP]);
  printf("ps %08b; spage:bp: %02X%04X\n", gc->PS, gc->SPAGE, gc->reg[BP]);
  printf("   -I---ZNC\033[0m\n");
}

U8 Exec(GC* gc, const U32 memsize) {
  U8 exc = 0;
  execloop:
    exc = (INSTS[gc->mem[gc->PC]])(gc);
    // StackDump(gc, 12);
    // RegDump(gc);
    if (exc != 0) return gc_errno;
    goto execloop;
  return exc;
}
