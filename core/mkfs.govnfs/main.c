#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define HEADER_SIZE 32

int main(int argc, char** argv) {
  uint8_t HEADER[HEADER_SIZE] = "\x42GOVNFS2.0\0\0\0\0\0\0Z";
  srand(time(NULL));
  for (uint8_t i = 0x0C; i < 0x10; i++) {
    HEADER[i] = rand() % 256;
  }

  puts("mkfs.govnfs 2.0 for GovnFS 2.0");
  if (argc == 1) {
    fprintf(stderr, "mkfs.govnfs: \033[91mfatal error:\033[0m no disk given\n");
    return 1;
  }
  FILE* drv = fopen(argv[1], "r+b");
  if (!drv) {
    fprintf(stderr, "mkfs.govnfs: \033[91mfatal error:\033[0m disk `%s` not found\n", argv[1]);
    return 1;
  }
  fseek(drv, 0, SEEK_END);
  uint32_t drvlen = ftell(drv);
  uint8_t* drvbuf = (uint8_t*)malloc(drvlen);
  fseek(drv, 0, SEEK_SET);
  fread(drvbuf, 1, drvlen, drv);
  fseek(drv, 0, SEEK_SET);

  printf("mkfs.govnfs: erasing the header (%d bytes ROM)\n", drvlen);
  memcpy(drvbuf, HEADER, 32);
  memset(drvbuf+HEADER_SIZE, 0x00, drvlen-HEADER_SIZE);
  drvbuf[0x000200] = 0xF7; // Set first file header byte to $F7, meaning there's no files
  fwrite(drvbuf, 1, drvlen, drv);
  fclose(drv);
  return 0;
}
