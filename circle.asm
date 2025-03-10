main:
  mov %ax $A
  mov #450000 %ax
  int $12

  mov %ax $0
  mov %si $44AD80
  mov %cx 480
.loop:
  stob %si %ax
  dex %si
  sub %si 639
  loop .loop
end:
  int $11
  int 1
  pop %ax
  hlt
