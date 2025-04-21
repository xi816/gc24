/* GovnBIOS -- a basic input/output system for Govno Core 24.
   BIOS is loaded into $700000 in memory.
*/

GovnBIOSReset: jmp GovnBIOS
; Interrupt $81 -- Output a string
puts:
  lodb %si %ax
  cmp %ax $00
  je .done
  push %ax
  int 2
  jmp puts
.done:
  iret
GovnBIOS:
  ; Set up the interrupt table
  mov %si puts
  sti $81

  ; Draw the screen
  mov %si CLEARSCR
  int $81
  mov %si WELCOMEMSG
  int $81
  mov %si GOVNOSMSG
  int $81
  mov %si OPTION0
  int $81
.key:
  int 1
  pop %ax
  cmp %ax $0A
  je .loadGovnOS
  cmp %ax '0'
  je .term
  jmp .key
.loadGovnOS:
  mov %si CURSOR_SHOW
  int $81
  mov %si CLEARSCR_BOOT
  int $81
  jmp $030000 ; Jump to the boot sector loaded
              ; from the primary disk
.term:
  mov %si CLEARSCR_BOOT
  int $81
  mov %si CURSOR_SHOW
  int $81
  hlt

; ANSI MAGIC HAPPENS HERE
CLEARSCR:      bytes "^[[48;5;232m^[[2J^[[H^@"
CLEARSCR_BOOT: bytes "^[[0m^[[2J^[[H^@"
CURSOR_SHOW:   bytes "^[[?25h^@"
WELCOMEMSG:    bytes "^[[48;5;253m^[[30m                                                         GovnBIOS Boot Manager^[[K^[[48;5;232m$"
               bytes "^[[s^[[999B^[[48;5;253m^[[K^[[999DENTER=Choose          0=Shutdown^[[48;5;232m^[[u^@"
GOVNOSMSG:     bytes "^[[38;5;253m$Choose an operating system to start, or press TAB to do nothing.$"
               bytes "(Use the arrow keys to highlight your choice, then press ENTER.)$$^@"
OPTION0:       bytes "    ^[[48;5;253m^[[K^[[999C^[[3D^[[48;5;232m    ^[[999D    ^[[48;5;253m^[[38;5;232mGovnOS on disk 0$$"
               bytes "^[[48;5;232m^[[38;5;253mTo shutdown, press 0.^[[?25l$^@"
biosBSE:       bytes $AA $55
