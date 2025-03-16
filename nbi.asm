main:
  /* mov %dx 10
  int 5 */
.loop:
  int 1
  pop %ax
  cmp %ax 'q'
  je .mt
  push %ax
  int 2
  jmp .loop
.mt:
  push '_'
  int 2
  jmp .loop

