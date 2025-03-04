reboot: jmp main

puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts

scans:
  int 1
  pop %ax
  str %si %ax

main:
  mov %si welcome_msg
  call puts

  hlt

welcome_msg: bytes "GovnBoot for Govno Core 24 loading...$^@"
bse: bytes $AA $55
