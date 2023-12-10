# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

include config.mk

# Правило сборки
############################################################
# Cтандартное действие при вызове Make

all:
	@-mkdir -p $(OBJ_DIRECTORY) $(DIRECTORIES)
	@$(MAKE) $(KERNEL)

$(OBJ_DIRECTORY)/%.o : %.s | $(OBJ_DIRECTORY)
	@echo -e '\x1b[32mASM  \x1b[0m' $@
	@$(AS) $< $(ASFLAGS) -o $@

$(OBJ_DIRECTORY)/%.o : %.c | $(OBJ_DIRECTORY)
	@echo -e '\x1b[32mC    \x1b[0m' $@
	@$(CC) $(CFLAGS) -O0 -c -o $@ $<

$(OBJ_DIRECTORY)/%.o : %.cpp | $(OBJ_DIRECTORY)
	@echo -e '\x1b[32mCPP  \x1b[0m' $@
	@$(CXX) $(CPP_FLAGS) -c -o $@ $<

build_rust:
	@echo -e '\x1b[32mRUST  \x1b[0mBuild rust kernel'
	cd $(RUST_DIR) && cargo build --release && cd ..

# Сборка ядра
build: $(SOURCES)

# Запуск
run:
	$(QEMU) -serial file:Qemu.log $(QEMU_FLAGS)

lite:
	$(QEMU) -serial file:Qemu.log -serial telnet:sayorios.piminoff.ru:10000 $(QEMU_FLAGS)

run_milla:
	$(QEMU) -serial file:Qemu.log -serial telnet:sayorios.piminoff.ru:10000 $(QEMU_FLAGS)

# Запуск Live COM2
runLocalMode:
	$(QEMU) -serial file:Qemu.log -serial telnet:localhost:10000 $(QEMU_FLAGS)

run_remote_mon:
	$(QEMU) $(QEMU_FLAGS) -serial mon:stdio -monitor tcp:127.0.0.1:1234,server

run_ahci_sata:
	$(QEMU) $(QEMU_FLAGS) -serial mon:stdio \
	-drive id=disk,file=disk.img,if=none \
	-device ahci,id=ahci \
	-device ide-hd,drive=disk,bus=ahci.0

# Запуск Milla
milla:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -serial tcp:127.0.0.1:64552,server,nowait -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -soundhw pcspk

floppy:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -m 128M -name "SayoriOS v0.3.x (Dev)" -d guest_errors -rtc base=localtime -fda floppy.img -boot order=dc

# Запуск с логами в консоль
runlive:
	$(QEMU) -serial mon:stdio $(QEMU_FLAGS)

# Запуск в режиме UEFI с логами в файл
uefi:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial file:Qemu.log -accel kvm \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime

# Запуск в режиме UEFI с логами в консоль
uefilive:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial mon:stdio -accel kvm \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime
# Генерация ISO-файла
geniso: $(KERNEL)
	grub-mkrescue -o "kernel.iso" iso/ -V kernel

# Генерация ISO-файла с поддержкой UEFI
genuefi:
	grub-mkrescue -d /usr/lib/grub/x86_64-efi -o SayoriOS_UEFI.iso iso/ --locale-directory=/usr/share/locale/ -V "SayoriOS Soul"

# Удаление оригинального файла и *.о файлов
clean:
	-rm -f $(KERNEL)
	-rm -f $(KERNEL_NEED)
	-rm -f $(DEPS)
	-rm -f $(RUST_OBJ_DEBUG)
	-rm -f $(RUST_OBJ_RELEASE)

# Линковка файлов
$(KERNEL): $(KERNEL_NEED) $(RUST_SOURCES) rust/Cargo.toml
	@$(MAKE) build_rust
	@echo -e '\x1b[32mLINK \x1b[0m' $(KERNEL)
	@rm -f $(KERNEL)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(KERNEL_NEED) $(RUST_OBJ_RELEASE)
	@bash tools/genmap.sh
	@bash tools/insertmap.sh
	@-rm kernel.map

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
	@$(MAKE) geniso
	@$(MAKE) lite

bf:
	@$(MAKE) clean
	@$(MAKE) geniso
	@$(MAKE) floppy

cppcheck:
	cppcheck --enable=warning,performance,portability .


debug: geniso
	$(QEMU) $(QEMU_FLAGS) -s -S &
	gdb -ex "target remote localhost:1234" -ex "break kernel" -ex "continue"

ensure_tools:
	@echo "C:" $(CC)
	@echo "C++:" $(CXX)

release:
	ADDCFLAGS="-DRELEASE" $(MAKE)

deploy: $(KERNEL)
	sudo cp iso/boot/kernel.elf /boot/sayorios_kernel.elf
	sudo cp iso/boot/sayori_sefs.img /boot/
	sudo cp other/41_sayori /etc/grub.d/

	sudo update-grub

clangd:
	$(MAKE) clean
	bear -- $(MAKE) -j2

-include $(DEPS)
