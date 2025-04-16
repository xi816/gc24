; Bootloader
reboot: jmp boot

scans:
  int 1
  pop %ax
  cmp %ax $7F
  je .back
  push %ax
  int 2
  cmp %ax $0A
  je .end
  stob %si %ax
  inx @clen
  jmp scans
.back:
  mov %gi clen
  lodw %gi %ax
  cmp %ax $00
  je scans
.back_strict:
  push %si
  mov %si bs_seq
  int $81
  pop %si
  dex %si
  dex @clen
  jmp scans
.end:
  mov %ax $00
  stob %si %ax
  ret

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

pstrcmp:
  lodb %si %ax
  lodb %gi %bx
  cmp %ax %bx
  jne .fail
  cmp %ax %cx
  je .eq
  jmp pstrcmp
.eq:
  mov %ax $00
  ret
.fail:
  mov %ax $01
  ret

dmemcmp:
  dex %cx
.loop:
  ldds
  inx %si
  lodb %gi %bx
  cmp %ax %bx
  jne .fail
  loop .loop
.eq:
  mov %ax $00
  ret
.fail:
  mov %ax $01
  ret

strtok:
  lodb %si %ax
  cmp %ax %cx
  re
  jmp strtok

strnul:
  lodb %si %ax
  cmp %ax $00
  je .nul
  mov %ax $01
  ret
.nul:
  mov %ax $00
  ret

strcpy:
  lodb %si %ax
  cmp %ax $00
  re
  stob %gi %ax
  jmp strcpy

memcpy:
  dex %cx
.loop:
  lodb %si %ax
  stob %gi %ax
  loop .loop
  ret

memset:
  dex %cx
  mov %ax $00
.loop:
  stob %si %ax
  loop .loop
  ret

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
  mov %cx 7 ; 8
.loop:
  stob %si %ax
  loop .loop
  ret
puti_buf: reserve 8 bytes

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
  mov %si bs_seq
  push %ax
  int $81
  pop %ax
  div %ax 10
  jmp .loop

; GovnFS 2.0 driver stores frequently used information
; to a config field at bank $1F
; Entries:
;   $1F0000 -- Drive letter
gfs2_configure:
  mov %si $000010
  mov %gi $1F0000
  ldds
  stob %gi %ax
  ret

gfs2_read_file:
  mov %dx $0001 ; Starting search at sector 1 (sector 0 is for header data)
  mov %si $0200 ; Precomputed starting address (sector*512)
.loop:
  ldds          ; Read the first byte from a sector to get its type
                ;   00 -- Empty sector
                ;   01 -- File start
                ;   02 -- File middle/end
                ;   F7 -- Disk end
  cmp %ax $01
  je .flcheck   ; Check filename and tag
  cmp %ax $00
  je .skip      ; Skip
  cmp %ax $F7
  je .fail
.flcheck:
  inx %si       ; Get to the start of filename
  mov %cx 15    ; Check 15 bytes
  push %gi
  push %bx
  mov %gi %bx
  inx %gi
  call dmemcmp
  pop %bx
  pop %gi
  cmp %ax $00
  je flcpy
  ; this code is unreachable if %ax == $00

  inx %dx
  mov %si %dx
  mul %si $0200
  jmp .loop
.skip:
  add %si $0200
  jmp .loop
.fail:
  mov %ax $01
  ret

flcpy: ; %dx should already contain file's start sector
  ; mov %gi $200000 ; should be configured by the caller
.read:
  cmp %dx $0000
  je .end
  mov %si %dx
  mul %si $0200
  add %si 16  ; Skip the processed header
  mov %cx 494 ; Copy 494 bytes (sectorSize(512) - sectorHeader(16) - sectorFooter(2))
  dex %cx
.loop:
  ldds
  inx %si
  stob %gi %ax
  loop .loop
.end:
  mov %ax $00
  ret

fre_sectors:
  mov %bx 0
  mov %si $000200
.loop:
  ldds
  add %si $200
  cmp %ax $01
  je .inc
  cmp %ax $F7
  je .end
  cmp %si $800000
.inc: ; RIP GovnDate 2.025e3-04-13-2.025e3-04-13
  inx %bx
  jmp .loop
.end:
  mov %ax %bx
  ret

boot:
  ; Load the kernel libraries
  call gfs2_configure
  mov %si welcome_msg
  int $81
  mov %si krnl_load_msg
  int $81
  mov %si emp_sec_msg00
  int $81
  call fre_sectors
  call puti
  mov %si emp_sec_msg01
  int $81

  mov %bx krnl_file_header
  mov %gi $A00000
  call gfs2_read_file
  cmp %ax $00
  jne shell
  call $A00000
shell:
.prompt:
  mov %si env_PS
  int $81

  mov %si clen
  mov %ax $0000
  stow %si %ax
  mov %si command
  call scans
.process:
  mov %si command
  call strnul
  cmp %ax $00
  je .aftexec

  mov %si command
  mov %gi com_hi
  call strcmp
  cmp %ax $00
  je govnos_hi

  mov %si command
  mov %gi com_date
  call strcmp
  cmp %ax $00
  je govnos_date

  mov %si command
  mov %gi com_echo
  mov %cx ' '
  call pstrcmp
  cmp %ax $00
  je govnos_echo

  mov %si command
  mov %gi com_exit
  call strcmp
  cmp %ax $00
  je govnos_exit

  mov %si command
  mov %gi com_cls
  call strcmp
  cmp %ax $00
  je govnos_cls

  mov %si command
  mov %gi com_calc
  call strcmp
  cmp %ax $00
  je govnos_calc

  mov %si command
  mov %gi com_help
  call strcmp
  cmp %ax $00
  je govnos_help

  mov %si file_header
  mov %cx 16
  call memset
  mov %si command
  mov %gi file_header
  inx %gi
  call strcpy
  mov %si file_tag
  mov %gi file_header
  add %gi 13
  mov %cx 3
  call memcpy

  mov %bx file_header
  mov %gi $200000
  call gfs2_read_file
  cmp %ax $00
  je .call
  jmp .bad
.call:
  call $200000
  jmp .aftexec
.bad:
  mov %si bad_command
  int $81
.aftexec:
  jmp .prompt
govnos_hi:
  mov %si hai_world
  int $81
  jmp shell.aftexec
govnos_date:
  int 3
  mov %ax %dx
  sar %ax 9
  add %ax 1970
  push %dx
  call puti
  pop %dx
  push '-'
  int 2
  mov %ax %dx
  sar %ax 5
  mov %bx $000F
  and %ax %bx
  inx %ax
  cmp %ax 10
  jg .p0
  push '0'
  int 2
.p0:
  push %dx
  call puti
  pop %dx
  push '-'
  int 2
  mov %ax %dx
  mov %bx $001F
  and %ax %bx
  inx %ax
  cmp %ax 10
  jg .p1
  push '0'
  int 2
.p1:
  call puti
  push '$'
  int 2
  jmp shell.aftexec
govnos_cls:
  mov %si cls_seq
  int $81
  jmp shell.aftexec
govnos_help:
  mov %si help_msg
  int $81
  jmp shell.aftexec
govnos_exit:
  hlt
  jmp shell.aftexec
govnos_echo:
  mov %si command
  mov %cx ' '
  call strtok
  int $81
  push $0A
  int 2
  jmp shell.aftexec
govnos_calc:
  ; First number
  mov %si calc_00
  int $81
  call scani

  ; Second number
  mov %si calc_01
  push %ax
  int $81
  call scani
  mov %bx %ax
  pop %ax

  ; Operation
  mov %si calc_02
  push %ax
  int $81
  pop %ax
  int $01
  pop %cx
  push %cx
  int $02
  push '$'
  int 2

  cmp %cx '+'
  je .add
  cmp %cx '-'
  je .sub
  cmp %cx '*'
  je .mul
  cmp %cx '/'
  je .div
  jmp .unk
.add:
  add %ax %bx
  call puti
  push '$'
  int $02
  jmp shell.aftexec
.sub:
  sub %ax %bx
  call puti
  push '$'
  int $02
  jmp shell.aftexec
.mul:
  mul %ax %bx
  call puti
  push '$'
  int $02
  jmp shell.aftexec
.div:
  cmp %bx $00
  je .div_panic
  div %ax %bx
  call puti
  push '$'
  int $02
  jmp shell.aftexec
.div_panic:
  mov %si calc_04
  int $81
  jmp shell.aftexec
.unk:
  mov %si calc_03
  int $81
  jmp shell.aftexec

welcome_msg:   bytes "Welcome to ^[[92mGovnOS^[[0m$^@"
krnl_load_msg: bytes "Loading ^[[92m:/krnl.bin/com^[[0m...$^@"
emp_sec_msg00: bytes "$Disk sectors used: ^[[92m^@"
emp_sec_msg01: bytes "^[[0m$$^@"
bad_command:   bytes "Bad command.$^@"

help_msg:    bytes "+------------------------------------------+$"
             bytes "|GovnOS help page 1/1                      |$"
             bytes "|  calc        Calculator                  |$"
             bytes "|  cls         Clear the screen            |$"
             bytes "|  echo        Echo text back to output    |$"
             bytes "|  exit        Exit from the shell         |$"
             bytes "|  gsfetch     Show system info            |$"
             bytes "|  help        Show help                   |$"
             bytes "+------------------------------------------+$^@"

com_hi:      bytes "hi^@"
com_cls:     bytes "cls^@"
com_calc:    bytes "calc^@"
com_date:    bytes "date^@"
com_help:    bytes "help^@"
com_echo:    bytes "echo "
com_exit:    bytes "exit^@"
hai_world:   bytes "hai world :3$^@"

calc_00:     bytes "Enter first number: ^@"
calc_01:     bytes "$Enter second number: ^@"
calc_02:     bytes "$Enter operation [+-*/]: ^@"
calc_03:     bytes "Unknown operation. Make sure you typed +, -, *, /$^@"
calc_04:     bytes "Division by 0 has been blocked by Pythagoras$^@"

env_HOST:    bytes "GovnPC 24 Super Edition^@"
env_OS:      bytes "GovnOS 0.2.2 For GovnoCore24^@"
env_CPU:     bytes "Govno Core 24$^@"

; TODO: unhardcode file header TODO: remove this todo
com_predefined_file_header: bytes "^Afile.bin^@^@^@^@com^@"
krnl_file_header:           bytes "^Akrnl.bin^@^@^@^@com^@"
file_header:                reserve 16 bytes
file_tag:                   bytes "com^@"

command:     reserve 64 bytes
clen:        reserve 2 bytes

bs_seq:      bytes "^H ^H^@"
cls_seq:     bytes "^[[H^[[2J^@"
env_PS:      bytes "# ^@"

bse:         bytes $AA $55
