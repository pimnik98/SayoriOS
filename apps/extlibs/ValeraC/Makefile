CC = clang
CFLAGS = -target i386-pc-none-elf -Wno-unused-command-line-argument \
		 -mno-sse -mno-avx -O0 -ffreestanding -I ../../include/ -c

all: libvalera.so

libvalera.so:
	@echo "Compiling Valera shared library..."
	$(CC) valera.c $(CFLAGS) -o libvalera.lib
