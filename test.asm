main:
  mov %ax 3
  mov %bx 5
  ora %ax %bx
  trap
  hlt
