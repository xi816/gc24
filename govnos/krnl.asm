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

; scani -- Read a 24-bit unsigned integer stdout and save to ax
; Returns:
;   ax -- number
scani:
  mov %ax $00
.loop:
  int $01
  pop %bx

  cmp %bx $0A ; Check for Enter
  re
  cmp %bx $20 ; Check for space
  re
  cmp %bx $7F ; Check for Backspace
  je .back
  cmp %bx $30 ; Check if less than '0'
  jl .loop
  cmp %bx $3A ; Check if greater than '9'+1
  jg .loop

  mul %ax 10
  push %bx
  int $02
  sub %bx 48
  add %ax %bx
  jmp .loop
.back: ; Backspace handler
  cmp %ax 0
  jne .back_strict
  jmp .loop
.back_strict:
  mov %si krnl_bksp_seq
  push %ax
  int $81
  pop %ax
  div %ax 10
  jmp .loop

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
  ret

krnl_panic:
  mov %si krnl_panic00
  int $81
  int 1
  pop %dx
  mov %si krnl_show_cursor
  int $81
  hlt

; kmain executes when the kernel loads
kmain:
  ret

; Constants/data
krnl_bksp_seq: bytes "^H ^H^@"
krnl_panic00:  bytes "^[[44m^[[H^[[2J^[[?25l$"
               bytes "A problem has been detected and GovnOS has been down to prevent damage$"
               bytes "to your computer.$$"
               bytes "UNKNOWN_DETECTED_ERROR$$"
               bytes "If this is the first time you've seen this error screen,$"
               bytes "restart your computer. If this screen appears again, follow$"
               bytes "these steps:$$"
               bytes "Check to make sure any new GovnOS binary files are properly installed.$"
               bytes "If this is a new installation, ask your Govno Core / GovnOS manufacturer$"
               bytes "for any Govno Core / GovnOS updates you might need.$$"
               bytes "If problems continue, disable or remove any newly installed hardware$"
               bytes "or software. Do not change BIOS if you don't know how to do it properly.$$"
               bytes "Press any key to shut down"
               bytes "^@"
krnl_show_cursor: bytes "^[[0m^[[H^[[2J^[[?25h^@"
