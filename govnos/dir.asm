dir_main:
  mov %si $1F0000 ; Drive letter
  lodb %si %bx
  mov %si dir_msg00
  int $81
  push %bx
  int 2
  mov %si dir_msg01
  int $81
  push %bx
  int 2
  mov %si dir_msg02
  int $81

  mov %bx 0
  mov %si $000200
.loop:
  ldds
  add %si $200
  cmp %ax $01
  je .print
  cmp %ax $F7
  re
  cmp %si $800000
  re
.print:
  push ' '
  int 2
  push ' '
  int 2
  mov %gi header
  mov %cx 15
  push %si
  inx %si
  call dmemcpy

  mov %si header
  mov %bx '^@'
  mov %dx ' '
  mov %cx 15
  call memsub
  mov %si header
  mov %cx 15
  call kwrite
  pop %si
  push '$'
  int 2
  jmp .loop

dir_msg00: bytes "Volume in drive ^@"
dir_msg01: bytes " has no label.$Directory of ^@"
dir_msg02: bytes "/$^@"
header:    reserve 15 bytes
