#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int8_t status(uint8_t stat) {
  if (stat) {
    puts("gboot: bootable binary written succesfully");
    return 0;
  }
  else {
    puts("gboot: bootable binary was not written");
    return 1;
  }
}

int main(int argc, char** argv) {
  uint32_t BootSectorStart = 0xC00000;
  if (argc == 1) {
    fprintf(stderr, "gboot: \033[91mfatal error:\033[0m no files given\n");
    return status(0);
  }
  else if (argc == 2) {
    fprintf(stderr, "gboot: \033[91mfatal error:\033[0m no bootable code given\n");
    return status(0);
  }
  FILE* drvfile = fopen(argv[1], "r+b");
  FILE* bsfile = fopen(argv[2], "rb");
  if (!drvfile) {
    fprintf(stderr, "gboot: \033[91mfatal error:\033[0m drive `%s` not found\n", argv[1]);
    return status(0);
  }
  if (!bsfile) {
    fprintf(stderr, "gboot: \033[91mfatal error:\033[0m bootable binary `%s` not found\n", argv[2]);
    return status(0);
  }
  fseek(drvfile, 0, SEEK_END);
  uint32_t drvlen = ftell(drvfile);
  uint8_t* drvbuf = (uint8_t*)malloc(drvlen);
  fseek(drvfile, 0, SEEK_SET);
  fread(drvbuf, 1, drvlen, drvfile);

  fseek(bsfile, 0, SEEK_END);
  uint32_t bslen = ftell(bsfile)+1;
  uint8_t* bsbuf = (uint8_t*)malloc(bslen);
  bsbuf[bslen-1] = 0;
  fseek(bsfile, 0, SEEK_SET);
  fread(bsbuf, 1, bslen, bsfile);

  printf("gboot: writing to disk (%d bytes ROM)\n", drvlen);
  memcpy(drvbuf+BootSectorStart, bsbuf, bslen);
  fseek(drvfile, 0, SEEK_SET);
  fwrite(drvbuf, 1, drvlen, drvfile);

  fclose(drvfile);
  fclose(bsfile);

  return status(1);
}
