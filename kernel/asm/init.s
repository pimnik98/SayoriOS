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

.extern kernel

.section .mboot

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM
.long 0, 0, 0, 0, 0     # Неиспользуется
.long 0                 # 0 - графический режим
.long 800, 600, 32      # Ширина, длина, глубина

.section	.text

.global		init

init:
		cli 

		push	%esp
		push	%ebx

		call	kernel

		hlt

loop:
		jmp	loop
