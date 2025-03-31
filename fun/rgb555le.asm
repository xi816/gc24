main:
  ; Set background color to $04 ()
  ; Set up RGB555LE mode
  mov %ax $01
  mov #49FF00 %ax

  ; Load the pixels to color palette
  mov %si $4A0000
  mov %gi test
  mov %cx 5      ; 3 colors (6-1 bcuz 0 is included)
.loop:
  lodb %gi %ax
  stob %si %ax
  trap
  loop .loop
  ; Load the colors to color palette
  mov %si $400000
  mov %gi test2
  mov %cx 2      ; 3 pixels
.loop2:
  lodb %gi %ax
  stob %si %ax
  loop .loop2

  ; Flush the VGA
  int $11
  ; Wait for a keypress and exit
  int 1
  pop %ax
  hlt

test:  bytes $7C $00 $7F $E0 $7D $E0
test2: bytes $00 $01 $02
