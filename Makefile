# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team

# Исходные объектные модули
SOURCES=kernel/asm/init.o \
	kernel/asm/interrupt.o \
	kernel/asm/switch_task.o \
	kernel/asm/sys_calls.o \
	kernel/asm/usr.o \
	kernel/asm/gdt.o \
	kernel/asm/regs.o \
	kernel/src/sys/bootscreen.o \
	kernel/src/sys/cpu_isr.o \
	kernel/src/sys/cpuinfo.o \
	kernel/src/sys/gdt.o \
	kernel/src/sys/isr.o \
	kernel/src/sys/memory.o \
	kernel/src/sys/scheduler.o \
	kernel/src/sys/sync.o \
	kernel/src/sys/syscalls.o \
	kernel/src/sys/system.o \
	kernel/src/sys/timer.o \
	kernel/src/sys/io_disp.o \
	kernel/src/sys/elf.o \
	kernel/src/sys/fpu.o \
	kernel/src/lib/base64.o \
	kernel/src/lib/list.o \
	kernel/src/lib/stdio.o \
	kernel/src/lib/fonts.o \
	kernel/src/lib/string.o \
	kernel/src/lib/math.o \
	kernel/src/lib/split.o \
	kernel/src/drv/cmos.o \
	kernel/src/drv/vfs_new.o \
	kernel/src/drv/input/keyboard.o \
	kernel/src/drv/input/mouse.o \
	kernel/src/drv/beeper.o \
	kernel/src/io/imaging.o \
	kernel/src/io/ports.o \
	kernel/src/io/tty.o \
	kernel/src/io/shell.o \
	kernel/src/fs/sefs.o \
	kernel/src/user/env.o \
	kernel/src/sys/float.o \
	kernel/src/sys/logo.o \
	kernel/src/kernel.o 
	
# Флаги компилятора языка C
CFLAGS=-nostdlib -fno-builtin -fno-stack-protector -m32 -Ikernel/include/ -O0 -ffreestanding -w

# Флаги компоновщика
LDFLAGS=-T kernel/asm/link.ld -m elf_i386

# Флаги ассемблера
ASFLAGS=--32

# Правило сборки
############################################################
# Cтандартное действие при вызове Make
all:$(SOURCES) link

# Сборка ядра
build:
	$(SOURCES) link

# Запуск
run:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -soundhw pcspk

# Запуск с логами в консоль
runlive:
	qemu-system-i386 -cdrom kernel.iso -serial mon:stdio -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime #-soundhw pcspk

# Запуск в режиме UEFI с логами в файл
uefi:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial file:Qemu.log -accel kvm -soundhw pcspk \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime

# Запуск в режиме UEFI с логами в консоль
uefilive:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial mon:stdio -accel kvm -soundhw pcspk \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime
# Генерация ISO-файла
geniso:
	grub-mkrescue -o "kernel.iso" iso/ -V kernel

# Генерация ISO-файла с поддержкой UEFI
genuefi:
	grub-mkrescue -d /usr/lib/grub/x86_64-efi -o SayoriOS_UEFI.iso iso/ --locale-directory=/usr/share/locale/ -V "SayoriOS Soul"

# Удаление оригинального файла
clean:
	rm kernel.iso || true

# Удаление *.о файлов
clean-objs:
	rm $(SOURCES) || true

# Линковка файлов
link: $(SOURCES)
	ld $(LDFLAGS) -o iso/boot/kernel.elf $(SOURCES)

# Быстрая линковка, генерация ISO, запуск
bir: link geniso run

# Отчистка лишних файлов, линковка, генерация ISO, запуск
сir: clean clean-objs all link geniso run


#debug:
#	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -S -s -soundhw pcspk &
#	sleep 1 && gdb iso/boot/kernel.elf -ex "target remote tcp::1234"
