reboot: jmp main

puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts

main:
  mov %si msg
  call puts
  hlt

msg: bytes "hai world :3$^@"
