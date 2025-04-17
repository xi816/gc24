govnos_gsfetch:
  mov %si gsfc_000
  int $81

  ; Hostname
  mov %si gsfc_001
  int $81
  mov %si env_HOST
  int $81

  ; OS name
  mov %si gsfc_002
  int $81
  mov %si env_OS
  int $81

  ; CPU name
  mov %si gsfc_003
  int $81
  mov %si env_CPU
  int $81

  ; Memory
  mov %si gsfc_004
  int $81
  mov %ax bse
  sub %ax $030002
  call puti
  mov %si gsfc_005
  int $81

  ; Disk space
  mov %si gsfc_008
  int $81
  call fre_sectors
  mul %ax 512
  call puti
  mov %si gsfc_009
  int $81

  mov %si gsfc_006
  int $81
  mov %cx 7 ; 8 colors
  mov %ax $30
.gsfcL1:
  push '^[' int $2
  push '['  int $2
  push '4'  int $2
  push %ax  int $2
  push 'm'  int $2
  push ' '  int $2
  push ' '  int $2
  inx %ax
  loop .gsfcL1

  mov %si gsfc_007
  int $81
  mov %si gsfc_006
  int $81
  mov %cx 7 ; 8 colors
  mov %ax $30
.gsfcL2:
  push '^[' int $2
  push '['  int $2
  push '1'  int $2
  push '0'  int $2
  push %ax  int $2
  push 'm'  int $2
  push ' '  int $2
  push ' '  int $2
  inx %ax
  loop .gsfcL2
  mov %si gsfc_007
  int $81

  mov %si gsfc_logo
  int $81

  jmp shell.aftexec

gsfc_000:    bytes "             ^[[97mgsfetch$^[[0m             ---------$^@"
gsfc_001:    bytes "             ^[[97mHost: ^[[0m^@"
gsfc_002:    bytes "$             ^[[97mOS: ^[[0m^@"
gsfc_003:    bytes "$             ^[[97mCPU: ^[[0m^@"
gsfc_004:    bytes "             ^[[97mMemory: ^[[0m^@"
gsfc_005:    bytes "B/16MiB$^@"
gsfc_006:    bytes "             ^@"
gsfc_007:    bytes "^[[0m$^@"
gsfc_008:    bytes "             ^[[97mDisk space used: ^[[0m^@"
gsfc_009:    bytes " B/16 MiB$$^@"
gsfc_logo:   bytes "^[[10A^[[33m  .     . .$"
             bytes            "     A     .$"
             bytes            "    (=) .$"
             bytes            "  (=====)$"
             bytes            " (========)^[[0m$$$$$$^@"
