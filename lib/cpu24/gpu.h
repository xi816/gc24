// GPU identificator: GovnGraphics 6970
#include <cpu24/gpuh.h>

ggrgb rgbv[] = {
  (ggrgb){.r = 0x00, .g = 0x00, .b = 0x00},
  (ggrgb){.r = 0xAA, .g = 0x00, .b = 0x00},
  (ggrgb){.r = 0x00, .g = 0xAA, .b = 0x00},
  (ggrgb){.r = 0xAA, .g = 0x55, .b = 0x00},
  (ggrgb){.r = 0x00, .g = 0x00, .b = 0xAA},
  (ggrgb){.r = 0xAA, .g = 0x00, .b = 0xAA},
  (ggrgb){.r = 0x00, .g = 0xAA, .b = 0xAA},
  (ggrgb){.r = 0xAA, .g = 0xAA, .b = 0xAA},

  (ggrgb){.r = 0x55, .g = 0x55, .b = 0x55},
  (ggrgb){.r = 0xFF, .g = 0x55, .b = 0x55},
  (ggrgb){.r = 0x55, .g = 0xFF, .b = 0x55},
  (ggrgb){.r = 0xFF, .g = 0xFF, .b = 0x55},
  (ggrgb){.r = 0x55, .g = 0x55, .b = 0xFF},
  (ggrgb){.r = 0xFF, .g = 0x55, .b = 0xFF},
  (ggrgb){.r = 0x55, .g = 0xFF, .b = 0xFF},
  (ggrgb){.r = 0xFF, .g = 0xFF, .b = 0xFF},
};

enum ggcolors {
  BLACK    = 0, // Standard 8 colors
  RED      = 1,
  GREEN    = 2,
  YELLOW   = 3,
  BLUE     = 4,
  MAGENTA  = 5,
  CYAN     = 6,
  WHITE    = 7,

  EBLACK   = 8, // Bright 8 colors
  ERED     = 9,
  EGREEN   = 10,
  EYELLOW  = 11,
  EBLUE    = 12,
  EMAGENTA = 13,
  ECYAN    = 14,
  EWHITE   = 15,
};

U0 GGinit(gc_gg16* gg, SDL_Renderer* r) {
  gg->status = 0b00000000;
  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  SDL_RenderClear(r);
}

U0 GGflush(GC* gc) {
  U8 byte;
  byte = gc->mem[0x450000];
  SDL_SetRenderDrawColor(gc->renderer, rgbv[byte%16].r, rgbv[byte%16].g, rgbv[byte%16].b, 0xFF);
  memset(gc->mem+0x400000, byte, VGASIZE);
  SDL_RenderPresent(gc->renderer);
}

U0 GGpage_CGA16(GC* gc) {
  U8 byte;
  for (U32 i = 0; i < VGASIZE; i++) {
    byte = gc->mem[0x400000+i];
    SDL_SetRenderDrawColor(gc->renderer, rgbv[byte%16].r, rgbv[byte%16].g, rgbv[byte%16].b, 0xFF);
    SDL_RenderDrawPoint(gc->renderer, i%WINW, i/WINW);
    SDL_RenderDrawPoint(gc->renderer, (i+1)%WINW, (i+1)/WINW);
  }
  SDL_RenderPresent(gc->renderer);
}

U0 GGpage_RGB555LE(GC* gc) {
  printf("OMG IT's THE RGB555LE!\n");
  for (U32 i = 0x4A0000; i < 0x4A0010; i++) {
    printf("%02X ", gc->mem[i]);
  }
  puts("\b");
  U8 byte;
  for (U32 i = 0; i < VGASIZE; i++) {
    byte = (gc->mem[0x400000+i]) + (gc->mem[0x400001+i] << 8);
    SDL_SetRenderDrawColor(gc->renderer,
      (((0x4A00000+byte*2)&0b0111110000000000)>>10), // R
      (((0x4A00000+byte*2)&0b0000001111100000)>>5),  // G
      (((0x4A00000+byte*2)&0b0000000000011111)),     // B
      0xFF);
    SDL_RenderDrawPoint(gc->renderer, i%WINW, i/WINW);
    SDL_RenderDrawPoint(gc->renderer, (i+1)%WINW, (i+1)/WINW);
  }
  SDL_RenderPresent(gc->renderer);
}

U0 (*GGPAGE[2])(GC*) = {&GGpage_CGA16, &GGpage_RGB555LE};
U0 GGpage(GC* gc) {
  printf("yeah its %02X\n", gc->mem[0x49FF00]);
  GGPAGE[gc->mem[0x49FF00]%2](gc);
}
