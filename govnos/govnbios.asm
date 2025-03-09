; This is just a test, don't expect it to run without
; bugs

GovnBIOS:
puts:
  lodb %si %ax
  cmp %ax $00
  re
  push %ax
  int 2
  jmp puts
BiosBSE: bytes $AA $55
