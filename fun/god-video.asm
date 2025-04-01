main:
mainloop:
  mov %si $400000
  mov %cx 307199
.loop:
  int $21
  stob %si %dx
  loop .loop

  int $11
  jmp mainloop
term:
  hlt
