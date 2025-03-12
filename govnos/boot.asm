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

boot:
  mov %si welcome_msg
  int $81
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
  mov %gi com_help
  call strcmp
  cmp %ax $00
  je govnos_help

  mov %si bad_command
  int $81
.aftexec:
  jmp .prompt
govnos_hi:
  mov %si hai_world
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

welcome_msg: bytes "Welcome to ^[[92mGovnOS^[[0m$^@"
bad_command: bytes "Bad command.$^@"

help_msg:    bytes "GovnOS help page 1/1$"
             bytes "  help        Show help$"
             bytes "  echo        Echo text back to output$^@"
             bytes "  exit        Exit from the shell$^@"

com_hi:      bytes "hi^@"
com_help:    bytes "help^@"
com_echo:    bytes "echo "
com_exit:    bytes "exit^@"
hai_world:   bytes "hai world :3$^@"

command:     reserve 64 bytes
clen:        reserve 2 bytes

bs_seq:      bytes "^H ^H^@"
env_PS:      bytes "# ^@"

bse:         bytes $AA $55
