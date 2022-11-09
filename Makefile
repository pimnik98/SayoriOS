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
	kernel/src/lib/base64.o \
	kernel/src/lib/list.o \
	kernel/src/lib/stdio.o \
	kernel/src/lib/fonts.o \
	kernel/src/lib/string.o \
	kernel/src/lib/split.o \
	kernel/src/drv/cmos.o \
	kernel/src/drv/vfs_new.o \
	kernel/src/drv/input/keyboard.o \
	kernel/src/drv/beeper.o \
	kernel/src/io/imaging.o \
	kernel/src/io/ports.o \
	kernel/src/io/tty.o \
	kernel/src/io/shell.o \
	kernel/src/fs/sefs.o \
	kernel/src/user/env.o \
	kernel/src/kernel.o

# kernel/src/io/imaging.o \
# Флаги компилятора языка C
CFLAGS=-nostdlib -fno-builtin -fno-stack-protector -m32 -g -Ikernel/include/ -Wimplicit-function-declaration -O0 -ffreestanding -w
# Флаги компоновщика
LDFLAGS=-T kernel/asm/link.ld -m elf_i386
# Флаги ассемблера
ASFLAGS=--32
# Правило сборки
all:$(SOURCES) link
build:
	$(SOURCES) link
run:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -soundhw pcspk
runlive:
	qemu-system-i386 -cdrom kernel.iso -serial mon:stdio -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime #-soundhw pcspk
uefi:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial file:Qemu.log -accel kvm -soundhw pcspk \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime
geniso:
	grub-mkrescue -o "kernel.iso" iso/ -V kernel
genuefi:
	grub-mkrescue -d /usr/lib/grub/x86_64-efi -o SayoriOS_UEFI.iso iso/ --locale-directory=/usr/share/locale/ -V "SayoriOS Soul"
clean:
	rm kernel.iso || true
clean-objs:
	rm $(SOURCES) || true
link:
	ld $(LDFLAGS) -o iso/boot/kernel.elf $(SOURCES)
bir:	geniso run
