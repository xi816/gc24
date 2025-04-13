info_main:
  mov %si govnos_info
  call kprint
  ret

govnos_info: bytes "^[[96m"
             bytes "GovnOS version: 0.2.1-24$"
             bytes "Release date: 253*8+1-04-13$"
             bytes "(c) Xi816, 253*8+1$"
             bytes "^[[0m^@"
