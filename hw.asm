  jmp main
puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts
main:
  mov %si hw
  call puts
  ret
hw: bytes "Hello, World!$^@"
