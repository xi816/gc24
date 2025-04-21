info_main:
  mov %si govnos_info
  int $81
  ret

govnos_info: bytes "^[[96m"
             bytes "GovnOS version: 0.3.1-24$"
             bytes "Release date: 04.15.E9 (2025-04-21)$"
             bytes "(c) Xi816, 253*8+1$"
             bytes "^[[0m^@"
