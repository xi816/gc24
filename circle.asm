main:
  mov %ax $000000
  mov %bx $000000
  mov %si $400000
  mov %cx 307200
.loop:
  ; Get x,y cordinates
  inx %ax
  inx %si
  cmp %ax 640
  je .inc_y
  jmp .next0
.inc_y:
  inx %bx
  mov %ax $000000
.next0:
  mov %dx %ax
  mov %gi %bx
  mul %dx %dx
  mul %gi %gi
  add %dx %gi
  cmp %dx 5
  je .draw_point
  loop .loop
.draw_point:
  mov %gi $06   ; Cyan
  stob %si %gi
  loop .loop
end:
  int $11

  int 1
  pop %ax
  hlt
