  jmp kmain
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
