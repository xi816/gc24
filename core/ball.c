#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ptrlen(t) (sizeof(t)/sizeof(t[0]))

int32_t main(int argc, char** argv) {
  char* color = "\033[32m";
  char* rcolor = "\033[0m";
  if (strcmp(getenv("TERM"), "xterm-256color")) {
    color = "\0";
    rcolor = "\0";
  }
  char* targets[]  = {"gc24", "gboot", "mkfs.govnfs", "ugovnfs", "prepare-disk"};
  char* itargets[] = {"gc24", "gboot", "mkfs.govnfs", "ugovnfs", "prepare-disk", "kasm"};
  char* build_commands[] = {
    "gcc core/main.c -Ilib/ -lm -lSDL2 -o gc24",
    "gcc core/gboot/main.c -o gboot",
    "gcc core/mkfs.govnfs/main.c -o mkfs.govnfs",
    "gcc core/ugovnfs/main.c -lm -o ugovnfs",
    "gcc core/prepare-disk.c -o prepare-disk"
  };
  char* install_commands[] = {
    "install -m 755 gc24 /usr/local/bin/gc24",
    "install -m 755 gboot /usr/local/bin/gboot",
    "install -m 755 mkfs.govnfs /usr/local/bin/mkfs.govnfs",
    "install -m 755 ugovnfs /usr/local/bin/ugovnfs",
    "install -m 755 prepare-disk /usr/local/bin/prepare-disk",
    "install -m 755 asm/kasm /usr/local/bin/kasm",
  };

  if (argc == 1) {
    printf("rebuilding %sball%s\n", color, rcolor);
    system("gcc core/ball.c -o ball");
    for (uint16_t i = 0; i < ptrlen(targets); i++) {
      printf("building %s%s%s\n", color, targets[i], rcolor);
      fflush(stdout);
      system(build_commands[i]);
    }
    return 0;
  }
  else if ((argc == 2) && (!strcmp(argv[1], "install"))) {
    for (uint16_t i = 0; i < ptrlen(itargets); i++) {
      printf("installing %s%s%s", color, itargets[i], rcolor);
      system(install_commands[i]);
    }
    return 0;
  }
  else {
    printf("ball: unknown argument `%s`\n", argv[1]);
  }
  return 0;
}
