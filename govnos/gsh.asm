; gsh.asm -- a basic shell implementation, that originally was at boot.asm
; but now the RO GovnFS 2.0 driver is complete, so i seprated it to a file that
; loads at startup

  jmp gshMain
gshMain:
  mov %si gshWelcome
  int $81
gshShell:
  mov %si gshPS1
  int $81

  mov %si scans_len
  mov %ax $0000
  stow %si %ax

  mov %si gshCommand
  call scans

  mov %si gshCommand
  mov %gi gshCommExit
  call strcmp
  cmp %ax $00
  je gshExit

  mov %si gshCommand
  mov %gi gshCommHelp
  call strcmp
  cmp %ax $00
  je gshHelp

  mov %si gshCommand
  mov %gi gshCommHi
  call strcmp
  cmp %ax $00
  je gshHi

  jmp gshBadComm
gshHi:
  mov %si gshHw
  int $81
  jmp gshAftexec
gshHelp:
  mov %si gshHelpMsg
  int $81
  jmp gshAftexec
gshExit:
  ret
gshBadComm:
  push '"'
  int $02
  mov %si gshCommand
  int $81
  mov %si gshBadCommMsg
  int $81
  jmp gshAftexec

gshAftexec:
  jmp gshShell
  ret

gshHw:      bytes "Hiiiii :3$^@"
gshWelcome: bytes "Welcome to gsh, a simple GovnOS shell$^@"
gshPS1:     bytes "^$ ^@"
gshCommand: reserve 128 bytes

gshCommHi:     bytes "hi^@"
gshCommHelp:   bytes "help^@"
gshCommExit:   bytes "exit^@"
gshBadCommMsg: bytes "\": bad command or file name$^@"

gshHelpMsg:    bytes "+-------------------------------+$"
               bytes "| GovnOS help page 1/1          |$"
               bytes "|   exit    Exit from the shell |$"
               bytes "|   help    Show help           |$"
               bytes "+-------------------------------+$^@"

