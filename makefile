all:
		gcc -m32 -c boot.s -o boot.o
		gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
		ld -m elf_i386 -T linker.ld -o mykernel.bin boot.o kernel.o
		qemu-system-i386 -kernel mykernel.bin -display curses