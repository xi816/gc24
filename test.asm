main:
  ; Set the background color
  mov %ax $04
  mov @450000 %ax
  int $12

  mov %ax $0006
  mov %cx 12800
  mov %si $400000
.loop:
  stob %si %ax
  loop .loop
  int $11

  int 1
  pop %ax
  hlt
