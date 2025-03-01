main:
  mov %ax $6000
  mov @000000 %ax
.loop:
  dex @000000
  mov %ax @000000
  cmp %ax $50
  jg .loop
  hlt
