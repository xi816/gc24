main:
  mov %ax $4149
  mov @000000 %ax
  mov %si $000000
  mov %ax $0000
.loop:
  lodh %si %ax
  trap
  inx %ax
  cmp %ax $05
  jl .loop
  hlt
