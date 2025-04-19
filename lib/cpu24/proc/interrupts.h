// General & I/O
#define INT_EXIT         0x00 // exit code from stack
#define INT_READ         0x01 // push char to stack
#define INT_WRITE        0x02 // write char from stack
#define INT_DATE         0x03 // get date to dx register
#define INT_RESET        0x04 // reset the CPU

// Videobuffer
#define INT_VIDEO_FLUSH  0x11 // flush the videobuffer
#define INT_VIDEO_CLEAR  0x12 // clear the videobuffer
#define INT_PPU_DRAW     0x13 // draw a 256-color palette sprite to videobuffer
#define INT_PPU_DRAWM    0x14 // draw a 2-color palette sprite to videobuffer

#define INT_RAND         0x21 // get random number to dx
#define INT_WAIT         0x22 // wait dx milliseconds

// SPU
#define INT_BEEP         0x23 // play beep sound with frequency from stack
