; gsh.asm -- a basic shell implementation, that originally was at boot.asm
; but now the RO GovnFS 2.0 driver is complete, so i seprated it to a file that
; loads at startup

  jmp gshMain
gshMain:
  mov %si gshHw
  int $81
gshShell:
  mov %si gshPS1
  int $81

  mov %si clen
  mov %ax $0000
  stow %si %ax

  mov %si gshCommand
  call scans

  jmp gshShell
  ret

gshHw:      bytes "Welcome to gsh, a simple GovnOS shell$^@"
gshPS1:     bytes "$ ^@"
gshCommand: reserve 128 bytes
