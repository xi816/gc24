  jmp kmain

; write -- Write n characters to the screen
; Arguments:
;   cx -- count
;   si -- string
write:
  push %ax
  dex %cx
.loop:
  lodb %si %ax
  push %ax
  int $02
  loop .loop
  pop %ax
  ret

; puti -- Write a 24-bit unsigned number to the screen
; Arguments:
;   ax -- number
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
  call puti_clr
  ret
puti_clr:
  mov %si puti_buf
  mov %ax $00
  mov %cx 7 ; 8 == floor(log(U24_MAX))+1;
.loop:
  stob %si %ax
  loop .loop
  ret
puti_buf: reserve 8 bytes

; strcmp -- Compare two strings
; Arguments:
;   si -- string 1
;   gi -- string 2
; Returns:
;   ax -- result (0 for equal, 1 for not equal)
strcmp:
  lodb %si %ax
  lodb %gi %bx
  cmp %ax %bx
  jne .fail
  cmp %ax $00
  je .eq
  jmp strcmp
.eq:
  mov %ax $00
  ret
.fail:
  mov %ax $01
  ret

; scans -- Read a string from stdout and save to a buffer
; Arguments:
;   si -- buffer address
scans:
  int 1
  pop %ax
  cmp %ax $7F
  je .back
  cmp %ax $1B
  je scans
  push %ax
  int 2
  cmp %ax $0A
  je .end
  stob %si %ax
  inx @scans_len
  jmp scans
.back:
  mov %gi scans_len
  lodw %gi %ax
  cmp %ax $00
  je scans
.back_strict:
  push %si
  mov %si krnl_bksp_seq
  int $81
  pop %si
  dex %si
  dex @scans_len
  jmp scans
.end:
  mov %ax $00
  stob %si %ax
  ret
scans_len: reserve 2 bytes

; memsub -- Substitute one character to another in a string
; Arguments:
;   ax -- from character
;   bx -- to character
;   si -- string
memsub:
  dex %cx
.loop:
  lodb %si %ax
  cmp %ax %bx
  je .sub
  loop .loop
  ret
.sub:
  dex %si
  stob %si %dx
  loop .loop
  ret

; dmemcpy -- Load n bytes from disk to memory
; Arguments:
;   si -- from address (disk)
;   gi -- to address (memory)
dmemcpy:
  dex %cx
.loop:
  ldds
  inx %si
  stob %gi %ax
  loop .loop
  trap
  ret

; kmain executes when the kernel loads
kmain:
  ret

; Constants/data
krnl_bksp_seq: bytes "^H ^H^@"
