/* GovnBIOS -- a basic input/output system for Govno Core 24.
   BIOS is loaded into $700000 in memory.
*/

; This is just a test, don't expect it to run with no bugs

GovnBIOSReset: jmp GovnBIOS
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

  mov %si CLEARSCR
  int $81
  mov %si WELCOMEMSG
  int $81
  mov %si GOVNOSMSG
  int $81
.key:
  int 1
  pop %ax
  cmp %ax $0A
  je .loadGovnOS
  jmp .key
.loadGovnOS:
  mov %si CLEARSCR_BOOT
  int $81
  jmp $030000

CLEARSCR:      bytes "^[[44m^[[2J^[[H^@"
CLEARSCR_BOOT: bytes "^[[0m^[[2J^[[H^@"
WELCOMEMSG:    bytes "^[[47m^[[30mGovnBIOS 0.1 by Xi-816^[[K$^[[43m^[[K ^[[103mBoot^[[44m$^@"
GOVNOSMSG:     bytes "^[[37mPress Enter to boot.$Well, choose GovnOS because you don't have any choice for now :)$^@"
biosBSE:       bytes $AA $55
