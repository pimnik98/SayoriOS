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

.set STACK_SIZE, 1024 * 16  # 16 KB

.extern kernel

.section .mboot

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

		# enable SSE
		mov %cr0, %eax
		and $~0x04, %al
		or $0x22, %al
		mov %eax, %cr0
		
		mov %cr4, %eax
		or $0x600, %ax
		mov %eax, %cr4

		mov $stack_top, %esp

		push	%esp
		push	%ebx

		call	kernel

		hlt

conword:
		.word 0x37f

loop:
		jmp	loop
