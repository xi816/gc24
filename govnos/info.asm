info_main:
  mov %si govnos_info
  call kprint
  ret

govnos_info: bytes "^[[96m"
             bytes "GovnOS version: 0.2.2-24$"
             bytes "Release date: 2025-04-16$"
             bytes "(c) Xi816, 253*8+1$"
             bytes "^[[0m^@"
