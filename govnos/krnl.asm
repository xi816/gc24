  jmp kmain

puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts

kmain:
  mov %si hk
  call puts
  int 1
  pop %dx
  ret

hk: bytes "!$^@"
