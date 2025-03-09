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

boot:
  mov %si welcome_msg
  call puts
.prompt:
  mov %si env_PS
  call puts

  mov %si command
  call scans
  mov %si command
  call puts
  push $0A
  int 2
  jmp .prompt

  hlt

welcome_msg: bytes "Welcome to ^[[92mGovnOS^[[0m$^@"

command:     reserve 64 bytes
clen:        reserve 2 bytes

bs_seq:      bytes "^H ^H^@"
env_PS:      bytes "# ^@"

bse:         bytes $AA $55
