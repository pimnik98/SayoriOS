# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

# Исходные объектные модули
ASM=kernel/asm/init.o \
	kernel/asm/interrupt.o \
	kernel/asm/switch_task.o \
	kernel/asm/sys_calls.o \
	kernel/asm/usr.o \
	kernel/asm/gdt.o \
	kernel/asm/regs.o \

SOURCES=kernel/src/sys/bootscreen.o \
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
	kernel/src/lib/list.o \
	kernel/src/lib/stdio.o \
	kernel/src/lib/string.o \
	kernel/src/lib/rand.o \
	kernel/src/lib/math/exp.o \
	kernel/src/lib/math/log.o \
	kernel/src/lib/math/pow.o \
	kernel/src/lib/math/acos.o \
	kernel/src/lib/math/asin.o \
	kernel/src/lib/math/atan.o \
	kernel/src/lib/math/integral.o \
	kernel/src/lib/math/sin.o \
	kernel/src/lib/math/cos.o \
	kernel/src/lib/math/tan.o \
	kernel/src/lib/math/sqrt.o \
	kernel/src/lib/math/cbrt.o \
	kernel/src/lib/math/math.o \
	kernel/src/lib/split.o \
	kernel/src/io/tty.o \
	kernel/src/io/screen.o \
	kernel/src/drv/ata.o \
	kernel/src/drv/cmos.o \
	kernel/src/drv/vfs_new.o \
	kernel/src/drv/input/keyboard.o \
	kernel/src/drv/input/mouse.o \
	kernel/src/drv/beeper.o \
	kernel/src/drv/fpu.o \
	kernel/src/drv/pci.o \
	kernel/src/io/ports.o \
	kernel/src/io/shell.o \
	kernel/src/fs/sefs.o \
	kernel/src/fs/milla.o \
	kernel/src/fs/lucario/fs.o \
	kernel/src/fs/NatSuki.o \
	kernel/src/user/env.o \
	kernel/src/sys/logo.o \
	kernel/src/io/port_io.o \
	kernel/src/gui/pointutils.o \
	kernel/src/gui/circle.o \
	kernel/src/gui/eki.o \
	kernel/src/drv/audio/ac97.o \
	kernel/src/toys/piano.o \
	kernel/src/sys/testing.o \
	kernel/src/kernel.o 

OPTIMIZABLE = kernel/src/gui/basics.o \
	kernel/src/gui/render.o \
	kernel/src/drv/psf.o \
	kernel/src/gui/window.o \
	kernel/src/gui/widget.o \
	kernel/src/gui/sayori_font_file.o \
	kernel/src/gui/widget_button.o \
	kernel/src/gui/widget_label.o \
	kernel/src/gui/widget_image.o \
	kernel/src/gui/parallel_desktop.o \
	kernel/src/lib/base64.o \
	kernel/src/io/imaging.o	

CPP_CODE = kernel/cpp/src/lib/tty.o \
		   kernel/cpp/src/lib/math.o \
		   kernel/cpp/src/lib/memory.o \
		   kernel/cpp/src/lib/string.o \
		   kernel/cpp/src/lib/file.o \
		   kernel/cpp/src/lib/conv.o \
		   kernel/cpp/src/lib/log.o \
		   kernel/cpp/src/lib/display.o \
		   kernel/cpp/src/audio/machinist.o \
		   kernel/cpp/src/audio/machinist_server.o \
		   kernel/cpp/src/audio/machinist_client.o \
		   kernel/cpp/src/audio/machinist_ac97.o \
		   kernel/cpp/src/gui/window.o \
		   kernel/cpp/src/gui/window_manager.o \
		   kernel/cpp/src/test.o

KERNEL = iso/boot/kernel.elf

DEBUG=-ggdb3 # -Werror

# Флаги компилятора языка C
CFLAGS=$(DEBUG) -nostdlib -fno-builtin -fno-stack-protector -msse -msse2 -m32 -Ikernel/include/ -ffreestanding -Wall -Wno-div-by-zero -Wno-address-of-packed-member -Wno-implicit-function-declaration
CPP_FLAGS=$(DEBUG) -nostdinc -fno-use-cxa-atexit -fno-exceptions -nostdlib -fno-builtin -fno-stack-protector -msse -msse2 -m32 -Ikernel/include/ -Ikernel/cpp/include/ -ffreestanding -w -Wall -Werror

# Флаги компоновщика
LDFLAGS=-T kernel/asm/link.ld -m elf_i386

# Флаги ассемблера
ASFLAGS=--32

QEMU = qemu-system-i386
# QEMU = ../qemu-5.2.0/build/qemu-system-x86_64

QEMU_FLAGS = -cdrom kernel.iso -m 128M \
			 -name "SayoriOS Soul" -d guest_errors \
			 -rtc base=localtime  \
			 -audiodev pa,id=pa0 \
			 -M pcspk-audiodev=pa0 \
			 -device AC97 -hda disk.img

# Правило сборки
############################################################
# Cтандартное действие при вызове Make

all: $(KERNEL)

$(ASM): %.o : %.s
	@echo -e '\x1b[32mASM  \x1b[0m' $@
	@$(AS) $< $(ASFLAGS) -o $@

$(SOURCES): %.o : %.c
	@echo -e '\x1b[32mC    \x1b[0m' $@
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OPTIMIZABLE): %.o : %.c
	@echo -e '\x1b[32mC-OPT\x1b[0m' $@
	@$(CC) $(CFLAGS) -Ofast -c -o $@ $<

$(CPP_CODE): %.o : %.cpp
	@echo -e '\x1b[32mCPP  \x1b[0m' $@
	@$(CXX) $(CPP_FLAGS) -c -o $@ $<

# Сборка ядра
build: $(SOURCES)

# Запуск
run:
	$(QEMU) -serial file:Qemu.log -accel kvm $(QEMU_FLAGS)
# Запуск
lite:
	$(QEMU) -serial file:Qemu.log $(QEMU_FLAGS)

run_milla:
	$(QEMU) -serial file:Qemu.log -serial telnet:sayorios.piminoff.ru:10000 $(QEMU_FLAGS)

# Запуск Live COM2
runLocalMode:
	$(QEMU) -serial file:Qemu.log -serial telnet:localhost:10000 $(QEMU_FLAGS)

run_remote_mon:
	$(QEMU) $(QEMU_FLAGS) -serial mon:stdio -monitor tcp:127.0.0.1:1234,server

# Запуск Milla
milla:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -serial tcp:127.0.0.1:64552,server,nowait -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -soundhw pcspk

# Запуск с логами в консоль
runlive:
	$(QEMU) -serial mon:stdio $(QEMU_FLAGS)

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

# Удаление оригинального файла и *.о файлов
clean:
	rm kernel.iso || true
	rm $(ASM) || true
	rm $(SOURCES) || true
	rm $(OPTIMIZABLE) || true
	rm $(CPP_CODE) || true

# Линковка файлов
$(KERNEL): $(ASM) $(SOURCES) $(OPTIMIZABLE) $(CPP_CODE)
	@echo -e '\x1b[32mLINK \x1b[0m' $(KERNEL)
	@ld $(LDFLAGS) -o $(KERNEL) $(ASM) $(SOURCES) $(OPTIMIZABLE) $(CPP_CODE)

# Быстрая линковка, генерация ISO, запуск
bir:
	@$(MAKE)
	@$(MAKE) geniso
	@$(MAKE) run

# Быстрая линковка, генерация ISO, запуск
birl:
	@$(MAKE)
	@$(MAKE) geniso
	@$(MAKE) runlive

# Очистка лишних файлов, линковка, генерация ISO, запуск
cir:
	@$(MAKE) clean
	@$(MAKE) bir

cirl:
	@$(MAKE) clean
	@$(MAKE) birl

cppcheck:
	cppcheck --enable=warning,performance,portability .

#debug:
#	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -S -s -soundhw pcspk &
#	sleep 1 && gdb iso/boot/kernel.elf -ex "target remote tcp::1234"
