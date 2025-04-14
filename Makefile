CC				:= gcc
ASM				:= nasm
INSTALL		:= linclude

INCLUDE		:= -Iinclude
CFLAGS		:= -O3

BINARY		:= disk.img

help:
	@echo Available targets: \[ all \| prepare \| clean \]

all:
	  gcc core/main.c $(INCLUDE) $(CFLAGS) -lm -lSDL2 -o gc24
		gcc core/gboot/main.c $(CFLAGS) -o gboot
		gcc core/mkfs.govnfs/main.c $(CFLAGS) -o mkfs.govnfs
		gcc core/ugovnfs/main.c $(CFLAGS) -lm -o ugovnfs
		gcc core/prepare-disk.c $(CFLAGS) -o prepare-disk

prepare:
	touch $(BINARY)
	truncate $(BINARY) -s 16M
	./mkfs.govnfs $(BINARY)
	./kasm -o 700000 -e govnos/govnbios.asm govnos/govnbios.exp
	./kasm -o 700000 govnos/govnbios.asm bios.img
	./kasm -i govnos/govnbios.exp govnos/boot.asm govnos/boot.bin
	./kasm -e govnos/boot.asm govnos/boot.exp
	./kasm -o A00000 govnos/krnl.asm govnos/krnl.bin
	./kasm -o A00000 -e govnos/krnl.asm govnos/krnl.exp
	./kasm -o 200000 -i govnos/krnl.exp govnos/info.asm govnos/info.bin
	./kasm -o 200000 -i govnos/boot.exp govnos/gsfetch.asm govnos/gsfetch.bin
	./kasm -o 200000 -i govnos/krnl.exp govnos/dir.asm govnos/dir.bin
	./gboot C00000 $(BINARY) govnos/boot.bin
	./ugovnfs -c $(BINARY) govnos/krnl.bin krnl.bin com
	./ugovnfs -c $(BINARY) govnos/info.bin info com
	./ugovnfs -c $(BINARY) govnos/gsfetch.bin gsfetch com
	./ugovnfs -c $(BINARY) govnos/dir.bin dir com

clean:
	rm gc24 gboot mkfs.govnfs ugovnfs prepare-disk
