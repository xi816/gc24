#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ptrlen(t) (sizeof(t)/sizeof(t[0]))

int32_t main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Error: expected 1 argument, got %d\n", argc-1);
    exit(1);
  }
  char* color = "\033[32m";
  char* rcolor = "\033[0m";
  char fcom[128];
  if (strcmp(getenv("TERM"), "xterm-256color")) {
    color = "\0";
    rcolor = "\0";
  }
  // Format the disk and create it if it doesn't exist
  printf("Formatting %s%s%s...\n", color, argv[1], rcolor);
  sprintf(fcom, "touch %s", argv[1]); system(fcom);
  sprintf(fcom, "truncate %s -s 16M", argv[1]); system(fcom);
  sprintf(fcom, "./mkfs.govnfs %s", argv[1]); system(fcom);

  // Compile GovnOS
  // printf("Compiling GovnOS...\n");
  system("./kasm -o 700000 -export govnos/govnbios.asm govnos/govnbios.exp");
  system("./kasm -o 700000 govnos/govnbios.asm bios.img");

  system("./kasm -import govnos/govnbios.exp govnos/boot.asm govnos/boot.bin");

  // Load GovnOS
  printf("Loading GovnOS into %s%s%s... ", color, argv[1], rcolor); fflush(stdout);
  sprintf(fcom, "./gboot C00000 %s govnos/boot.bin", argv[1]); system(fcom);
  return 0;
}
