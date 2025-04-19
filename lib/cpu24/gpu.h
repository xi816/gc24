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
  }
  SDL_RenderPresent(gc->renderer);
}

U0 GGpage_RGB555LE(GC* gc) {
  U8 byte; U16 palitro;
  for (U32 i = 0; i < VGASIZE; i++) {
    byte = gc->mem[0x400000+i];
    palitro = (gc->mem[0x4A0000+2*byte]) + (gc->mem[0x4A0001+2*byte] << 8);
    SDL_SetRenderDrawColor(gc->renderer,
      ((palitro&0b0111110000000000)>>10)*8, // R
      ((palitro&0b0000001111100000)>>5)*8,  // G
      ((palitro&0b0000000000011111))*8,     // B
      0xFF);
    SDL_RenderDrawPoint(gc->renderer, i%WINW, i/WINW);
  }
  SDL_RenderPresent(gc->renderer);
}

U0 (*GGPAGE[2])(GC*) = {&GGpage_CGA16, &GGpage_RGB555LE};
U0 GGpage(GC* gc) {
  GGPAGE[gc->mem[0x49FF00]%2](gc);
}

// PPU functions
U0 GGsprite_256(GC* gc) {
  U32 to = gc->reg[0x04].word; // SI
  U32 from = gc->reg[0x05].word; // GI
  for (U8 y = 0; y < 8; y++) {
    for (U8 x = 0; x < 8; x++) {
      gc->mem[to+(y*640)+x] = gc->mem[from+(y*8)+x];
    }
  }
}

U0 GGsprite_mono(GC* gc) {
  U32 to = gc->reg[0x04].word; // SI
  U32 from = gc->reg[0x05].word; // GI
  U8 color = gc->reg[0x00].word; // AX
  for (U8 y = 0; y < 8; y++) {
    for (U8 x = 0; x < 8; x++) {
      gc->mem[to+(y*640)+x] = color*((gc->mem[from+y] & (1<<(7-x))) >> (7-x));
    }
  }
}
