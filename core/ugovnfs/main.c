#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

// The header is the first 32 bytes of the disk
uint8_t readHeader(uint8_t* disk) {
  puts("Disk info:");
  printf("  Filesystem:\tGovnFS 2.0\n", disk[0x00]);
  printf("  Serial:\t%02X%02X%02X%02X\n", disk[0x0C], disk[0x0D], disk[0x0E], disk[0x0F]);
  printf("  Disk letter:\t%c/\n", disk[0x10]);
  return 0;
}

// The header is the first 32 bytes of the disk
uint8_t readFilenames(uint8_t* disk, char c, char* tag) {
  puts("Listing %c/");
  return 0;
}

// CLI tool to make GovnFS partitions
int main(int argc, char** argv) {
  if (argc == 1) {
    puts("ugovnfs: no arguments given");
    return 1;
  }
  if (argc == 2) {
    puts("ugovnfs: no disk/flag given");
    return 1;
  }
  FILE* fl = fopen(argv[2], "rb");
  if (fl == NULL) {
    printf("ugovnfs: \033[91mfatal error:\033[0m file `%s` not found\n", argv[1]);
    return 1;
  }
  fseek(fl, 0, SEEK_END);
  uint32_t flsize = ftell(fl);
  uint8_t* disk = malloc(flsize);
  fseek(fl, 0, SEEK_SET);
  fread(disk, 1, flsize, fl);
  fclose(fl);

  // Check the disk
  if (disk[0x000000] != 0x42) {
    printf("ugovnfs: \033[91mdisk corrupted:\033[0m unknown disk header magic byte `$%02X`\n", disk[0x000000]);
    free(disk);
    return 1;
  }

  if (!strcmp(argv[1], "-i")) {
    return readHeader(disk);
  }
  else {
    printf("ugovnfs: \033[91mfatal error:\033[0m unknown argument: `%s`\n", argv[1]);
    free(disk);
    return 1;
  }
  puts("ugovnfs: no arguments given");
  return 0;
}
