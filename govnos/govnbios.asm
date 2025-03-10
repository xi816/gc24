; This is just a test, don't expect it to run without
; bugs

  jmp GovnBIOS
puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts
GovnBIOS:
  mov %si CLEARSCR
  call puts
  mov %si WELCOMEMSG
  call puts
  mov %si GOVNOSMSG
  call puts
.key:
  int 1
  pop %ax
  cmp %ax $0A
  je .loadGovnOS
  jmp .key
.loadGovnOS:
  mov %si CLEARSCR_BOOT
  call puts
  jmp $030000

CLEARSCR: bytes "^[[44m^[[2J^[[H^@"
CLEARSCR_BOOT: bytes "^[[0m^[[2J^[[H^@"
WELCOMEMSG: bytes "^[[47m^[[30mGovnBIOS 0.1 by Xi-816^[[K$^[[43m^[[K ^[[103mBoot^[[44m$^@"
GOVNOSMSG: bytes "^[[37mPress Enter to boot.$Well, choose GovnOS because you don't have any choice for now :)$^@"
BiosBSE: bytes $AA $55
