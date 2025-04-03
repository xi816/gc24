main:
  mov %si $400000
mainloop:
  mov %cx 639
.loop:
  int $21
  stob %si %dx
  loop .loop

  int $11
  int 1
  pop %ax
  jmp mainloop
term:
  hlt
