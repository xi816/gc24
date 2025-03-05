jmp _porth_start
write:
  dex %cx
.loop:
  lodb %si %ax
  push %ax
  int $02
  loop .loop
  ret

puti:
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
  push $0A
  int 2
  ret
puti_buf: reserve 8 bytes
_porth_start:
  ; push 34
  push 34
  ; push 35
  push 35
  ; plus
  pop %ax
  pop %bx
  add %ax %bx
  push %ax
  ; dump
  pop %ax
  call puti
  hlt
