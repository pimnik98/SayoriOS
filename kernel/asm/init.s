/*
 * SayoriOS
 * Старт инициализации ядра
 */

.code32
.set ALIGN,		 						1<<0			# Выравнивание загруженных модулей по границам страницы
.set MEMINFO,	 						1<<1			# Просим предоставить карту памяти
.set VBE_MODE,   						1<<2            # VBE mode flag. GRUB will set it for us and provide info about it.
.set INIT_MBOOT_HEADER_MAGIC,           0x1BADB002
.set INIT_MBOOT_HEADER_FLAGS,           ALIGN | MEMINFO | VBE_MODE
.set INIT_MBOOT_CHECKSUM,               0x00000000 - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)

.set STACK_SIZE, 1024 * 64  # 64 KB

.extern kernel

.section .mboot, "a", @progbits

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM
.long 0, 0, 0, 0, 0     # Неиспользуется
.long 0                 # 0 - графический режим
.long 800, 600, 32      # Ширина, длина, глубина

.section .bss
	.align 16
	stack_bottom:
		.skip STACK_SIZE
	stack_top:

.section	.text

.global		__pre_init

__pre_init:
		cli 

		# init FPU
		fninit
		fldcw (conword)

		call sse_enable

		mov $stack_top, %esp

		push	%esp
		push	%ebx

		xor %ebp, %ebp

		call	kernel

		hlt

conword:
		.word 0x37f

loop:
		jmp	loop
