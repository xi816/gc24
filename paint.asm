/* Paint -- an image drawing program written in GovnASM */

  jmp main
draw:
  mov %si x
  lodw %si %ax
  mov %bx y
  lodw %bx %si
  mul %si 640
  add %si %ax
  add %si $400000
  stob %si %dx
  ret
main:
  mov %ax $43
  mov #450000 %ax
  int $12
.l:
  int 1
  pop %ax
  push %ax
  mov %si color
  lodb %si %dx
  call draw
  pop %ax
  cmp %ax 'a'
  je .left
  cmp %ax 'd'
  je .right
  cmp %ax 'w'
  je .up
  cmp %ax 's'
  je .down
  cmp %ax 'x'
  je .colorx
  cmp %ax 'c'
  je .colorc
  cmp %ax 'q'
  je term
  jmp render
.left:
  dex @x
  jmp render
.right:
  inx @x
  jmp render
.up:
  dex @y
  jmp render
.down:
  inx @y
  jmp render
.colorx:
  dex #color
  jmp render.panel
.colorc:
  inx #color
  jmp render.panel

render:
  mov %dx $00
  call draw
  jmp .end
.panel:
  mov %si color
  lodb %si %ax
  mov %si $447E00
  mov %cx 12800
.l1:
  stob %si %ax
  loop .l1
.end:
  int $11
  jmp main.l
term:
  hlt

x:     reserve 2 bytes
y:     reserve 2 bytes
color: reserve 1 bytes
