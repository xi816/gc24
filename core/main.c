#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>

#include <sterm-control.h>
new_st;
#include <holyc-types.h>
#include <gc-types.h>
#include <cpu24/bpf.h>
#include <cpu24/cli.h>
#include <cpu24/cpu24.h>
#include <cpu24/disasm.h>

/* usage -- Show usage of the emulator */
U8 usage() {
  puts("gc24: a Govno Core 24 emulator");
  puts("Syntax: gc24 [OPTIONS] <file>");
  puts("  gc24   <file>  Load the file directly to memory");
  puts("Options:");
  puts("  bios   <file>  Load the file as BIOS");
  puts("  cli            Start the emulator in CLI mode");
  puts("  disk   <file>  Load the file as ROM");
  puts("  disasm <file>  Disassemble the binary file instead of running it");
  puts("  help           Show help");
  return 0;
}

/* loadBootSector -- loads a block of data from one pointer to
   another, until it finds an $AA$55 */
U8 loadBootSector(U8* drive, U8* mem, U32 start, U32 to) {
  U32 oto = to;
  while (1) {
    if ((*(drive+start) == 0xAA) && (*(drive+start+1) == 0x55)) break;
    *(mem+to) = *(drive+start);
    to++;
    start++;
  }
  printf("gc24: read %d bytes from ROM\n", to-oto+1);
  return 0;
}

U8 main(I32 argc, I8** argv) {
  set_st;
  srand(time(NULL));
  U8 driveboot;
  U8 climode = 0;
  U8 disasmmode = 0;
  U8 verbosemode = 0;
  U8 argp = 1; // 256 arguments is enough for everyone
  U8* filename = NULL;
  U8* biosfile = NULL;

  driveboot = 0;
  if (argc == 1) {
    old_st;
    usage();
    fprintf(stderr, "gc24: fatal error: no arguments given\n");
    return 1;
  }
  while (argp < argc) {
    // Load from the disk
    if ((!strcmp(argv[argp], "disk")) || (!strcmp(argv[argp], "-d")) || (!strcmp(argv[argp], "--disk"))) {
      driveboot = 1;
      argp++;
    }
    else if ((!strcmp(argv[argp], "bios")) || (!strcmp(argv[argp], "-b")) || (!strcmp(argv[argp], "--bios"))) {
      biosfile = argv[argp+1];
      argp += 2;
    }
    else if ((!strcmp(argv[argp], "verbose")) || (!strcmp(argv[argp], "-v")) || (!strcmp(argv[argp], "--verbose"))) {
      verbosemode = 1;
      argp++;
    }
    else if ((!strcmp(argv[argp], "cli")) || (!strcmp(argv[argp], "-c")) || (!strcmp(argv[argp], "--cli"))) {
      climode = 1;
      argp++;
    }
    else if ((!strcmp(argv[argp], "help")) || (!strcmp(argv[argp], "-h")) || (!strcmp(argv[argp], "--help"))) {
      old_st;
      usage();
      exit(1);
    }
    else if ((!strcmp(argv[argp], "disasm")) || (!strcmp(argv[argp], "-D")) || (!strcmp(argv[argp], "--disasm"))) {
      disasmmode = 1;
      argp++;
    }
    else {
      filename = argv[argp];
      break;
    }
  }

  U8 biosbuf[BIOSNOBNK * BANKSIZE];
  biosbuf[0x000000] = 0xAA;
  biosbuf[0x000001] = 0x55;
  // Create a virtual CPU
  GC gc;
  gc.pin = 0b00000000; // Reset the pin
  InitGC(&gc);
  Reset(&gc);

  if (!driveboot) { // Load a memory dump
    FILE* fl = fopen(filename, "rb");
    if (fl == NULL) {
      fprintf(stderr, "gc24: \033[91mfatal error:\033[0m file `%s` not found\n", filename);
      old_st;
      free(gc.mem);
      free(gc.rom);
      return 1;
    }
    fread(gc.mem+0x030000, 1, MEMSIZE, fl);
    fclose(fl);
    if (disasmmode) {
      if (disasm(gc.mem, MEMSIZE, stdout) == 1)
        puts("unexpected instruction");
      old_st;
      return 1;
    }
    // Disk signaures for GovnFS (without them, fs drivers would not work)
    gc.rom[0x00] = 0x60;
    gc.rom[0x11] = '#';
    gc.rom[0x21] = 0xF7;
    gc.pin &= 0b01111111;
    gc.PC = 0x030000;
  }
  else { // Load a disk
    FILE* fl = fopen(filename, "rb");
    if (fl == NULL) {
      fprintf(stderr, "\033[31mError\033[0m while opening %s\n", filename);
      old_st;
      return 1;
    }
    fread(gc.rom, 1, ROMSIZE, fl);
    fclose(fl);
    // Load the boot sector from $C00000 into RAM ($030000)
    loadBootSector(gc.rom, gc.mem, 0xC00000, 0x030000);
    // Setup the pin bit 7 to 1 (drive)
    gc.pin |= 0b10000000;
  }
  if (biosfile != NULL) { // BIOS provided
    FILE* fl = fopen(biosfile, "rb");
    if (fl == NULL) {
      fprintf(stderr, "\033[31mError\033[0m while opening %s\n", biosfile);
      old_st;
      return 1;
    }
    fread(biosbuf, 1, BIOSNOBNK*BANKSIZE, fl);
    fclose(fl);
    loadBootSector(biosbuf, gc.mem, 0x000000, 0x700000);
  }

  // GPU
  gravno_start;
  gc.renderer = renderer;
  GGinit(&(gc.gg), renderer);

  int runcode = 0xFF;
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  if (climode) runcode = ExecD(&gc, 0);
  if (runcode == 0) {
    return 0;
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  U8 exec_errno = Exec(&gc, MEMSIZE, verbosemode);
  gravno_end;
  old_st;
  if (driveboot) { // Save the modified disk back
    FILE* fl = fopen(filename, "wb");
    if (fl == NULL) {
      fprintf(stderr, "\033[31mError\033[0m while opening %s\n", filename);
      old_st;
      free(gc.rom);
      free(gc.mem);
      return 1;
    }
    fwrite(gc.rom, 1, ROMSIZE, fl);
    fclose(fl);
  }
  free(gc.rom);
  free(gc.mem);
  return exec_errno;
}
