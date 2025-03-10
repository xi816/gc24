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
  call puts
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
  call puts
shell:
.prompt:
  mov %si env_PS
  call puts

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
  mov %gi com_exit
  call strcmp
  cmp %ax $00
  je govnos_exit

  mov %si bad_command
  call puts
.aftexec:
  jmp .prompt
govnos_hi:
  mov %si hai_world
  call puts
  jmp shell.aftexec
govnos_exit:
  hlt
  jmp shell.aftexec

welcome_msg: bytes "Welcome to ^[[92mGovnOS^[[0m$^@"
bad_command: bytes "Bad command.$^@"

com_hi:      bytes "hi^@"
com_exit:    bytes "exit^@"
hai_world:   bytes "hai world :3$^@"

command:     reserve 64 bytes
clen:        reserve 2 bytes

bs_seq:      bytes "^H ^H^@"
env_PS:      bytes "# ^@"

bse:         bytes $AA $55
