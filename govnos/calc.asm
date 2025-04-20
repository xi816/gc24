calc_main:
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
  ret
.sub:
  sub %ax %bx
  call puti
  push '$'
  int $02
  ret
.mul:
  mul %ax %bx
  call puti
  push '$'
  int $02
  ret
.div:
  cmp %bx $00
  je .div_panic
  div %ax %bx
  call puti
  push '$'
  int $02
  ret
.div_panic:
  mov %ax $000000
  jmp krnl_panic
.unk:
  mov %si calc_03
  int $81
  ret

calc_00:     bytes "Enter first number: ^@"
calc_01:     bytes "$Enter second number: ^@"
calc_02:     bytes "$Enter operation [+-*/]: ^@"
calc_03:     bytes "Unknown operation. Make sure you typed +, -, *, /$^@"
