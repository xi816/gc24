  jmp main
func1:
  call func2
  ret
func2:
  mov %bx $50
  ret

main:
  push $000069
  push $416941
  pop %ax
  call func1
  hlt
