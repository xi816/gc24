main:
  ; Set up RGB555LE mode
  mov %ax $01
  mov #49FF00 %ax

  ; Load the pixels to color palette
  mov %si $4A0000
  mov %gi test
  mov %cx 7      ; 4 colors (8-1 bcuz 0 is included)
mainloop:
.loop:
  lodb %gi %ax
  stob %si %ax
  loop .loop
  ; Load the colors to VGA
  mov %si $400000
  mov %cx 102399
  mov %ax $01
.loop2:
  stob %si %ax
  loop .loop2

  mov %cx 102399
  mov %ax $02
.loop3:
  stob %si %ax
  loop .loop3

  mov %cx 102399
  mov %ax $03
.loop4:
  stob %si %ax
  loop .loop4

  ; Flush the VGA
  int $11
  ; Wait for a keypress and exit
  int 1
  pop %ax
  cmp %ax 'q'
  je term

  jmp mainloop
term:
  hlt

test:  bytes $10 $42 ; Gray
       bytes $FF $7F ; White
       bytes $1F $00 ; Blue
       bytes $03 $E0 ; Red
