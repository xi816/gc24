  jmp kmain

memsub:
  dex %cx
.loop:
  lodb %si %ax
  cmp %ax %bx
  je .sub
  loop .loop
  ret
.sub:
  dex %si
  stob %si %dx
  loop .loop
  ret

dmemcpy:
  dex %cx
.loop:
  ldds
  inx %si
  stob %gi %ax
  loop .loop
  ret

kprint:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp kprint

ktest:
  mov %si hk
  call kprint
  int 1
  pop %dx
  ret

kmain:
  ret

hk: bytes "Hello From Kernel!$^@"
