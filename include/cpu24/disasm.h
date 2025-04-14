// Govno Core 16X disassembler

// Config
#define INSTC "\033[33m"
#define IMMC  "\033[35m"
#define REGC  "\033[34m"
#define ADDRC "\033[32m"

U8* regname[16] = {
  "%ax", "%bx", "%cx", "%dx", "%si", "%gi", "%sp", "%bp"
};

U8* jumps[16] = {
  "e", "ne", "c", "nc", "s", "n", "i", "ni"
};

U16 r16(U8* bin, U32 pc) {
  return (U16)((bin[pc] << 8) + (bin[pc+1]));
}

U32 r24(U8* bin, U32 pc) {
  return (U32)((bin[pc+2] << 16) + (bin[pc+1] << 8) + (bin[pc]));
}

U8 disasm_inst(U8* bin, U32* pc, FILE* out) {
  printf("\033[0m  $%06X:\t", *pc);
  switch (bin[*pc]) {
  case 0x00:
    printf(INSTC "hlt\n");
    *pc += 1;
    fputs("\033[0m", stdout);
    return 2;
    break;
  case 0x01:
    printf(INSTC "trap\n");
    *pc += 1;
    break;
  case 0x20 ... 0x27:
    printf(INSTC "inx\t" REGC "%s\n", regname[bin[*pc]-0x20]);
    *pc += 1;
    break;
  case 0x28 ... 0x2F:
    printf(INSTC "dex\t" REGC "%s\n", regname[bin[*pc]-0x28]);
    *pc += 1;
    break;
  case 0x40:
    printf(INSTC "inx\t" ADDRC "#%06X\n", r24(bin, (*pc)+1));
    *pc += 4;
    break;
  case 0x41:
    printf(INSTC "int\t" IMMC "$%02X\n", bin[(*pc)+1]);
    *pc += 2;
    break;
  case 0x70 ... 0x77:
    printf(INSTC "cmp\t" REGC "%s " IMMC "$%06X\n", regname[bin[*pc]-0x70], r24(bin, (*pc)+1));
    *pc += 4;
    break;
  case 0xA0 ... 0xA7:
    printf(INSTC "j%s\t" ADDRC "@%06X\n", jumps[bin[*pc]-0xA0], r24(bin, (*pc)+1));
    *pc += 4;
    break;
  case 0xC0 ... 0xC7:
    printf(INSTC "mov\t" REGC "%s " IMMC "$%06X\n", regname[bin[*pc]-0xC0], r24(bin, (*pc)+1));
    *pc += 4;
    break;
  case 0xE0 ... 0xE7:
    printf(INSTC "mov\t" ADDRC "#%06X " REGC "%s\n", r24(bin, (*pc)+1), regname[bin[*pc]-0xE8]);
    *pc += 4;
    break;
  case 0xE8 ... 0xEF:
    printf(INSTC "mov\t" ADDRC "@%06X " REGC "%s\n", r24(bin, (*pc)+1), regname[bin[*pc]-0xE8]);
    *pc += 4;
    break;
  case 0x9F:
    printf(INSTC "lodh\t" REGC "%s " REGC "%s\n", regname[bin[(*pc)+1] / 8], regname[bin[(*pc)+1] % 8]);
    *pc += 2;
    break;
  default:
    printf("...\n");
    *pc += 1;
  }
  fputs("\033[0m", stdout);
  return 0;
}

U8 disasm(U8* bin, U32 size, FILE* out) {
  U32 pc = 0x030000;
  U8 excode;
  puts("Disassembly of #300000:");
  while (pc < size) {
    excode = disasm_inst(bin, &pc, out);
    if (excode != 0) return (excode == 1) ? 1 : 0;
  }
  return 0;
}
