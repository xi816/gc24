  jmp main
write:
  dex %cx
.loop:
  lodb %si %ax
  push %ax
  int $02
  loop .loop
  ret
puti:
  push %ax
  mov %gi puti_buf
  add %gi 7
.loop:
  div %ax 10 ; Divide and get the remainder into %dx
  add %dx 48 ; Convert to ASCII
  stob %gi %dx
  sub %gi 2
  cmp %ax $00
  jne .loop
  mov %si puti_buf
  mov %cx 8
  call write
  push '$'
  int 2
  call puti_clr
  pop %ax
  ret
puti_clr:
  mov %si puti_buf
  mov %ax $00
  mov %cx 8
.loop:
  stob %si %ax
  loop .loop
  ret
puti_buf: reserve 8 bytes

; cx dx ax
; a  b  c
; 2  3  5

main:
  mov %cx 1
  mov %bx 1
  mov %ax 1
.loop:
  mov %ax %cx
  add %ax %bx
  call puti
  mov %cx %bx
  mov %bx %ax

  mov %dx 60
  int $22
  jmp .loop

  hlt
