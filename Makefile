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
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIRECTORY)/%.o : %.cpp | $(OBJ_DIRECTORY)
	@echo -e '\x1b[32mCPP  \x1b[0m' $@
	@$(CXX) $(CPP_FLAGS) -c -o $@ $<

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
	-device ahci,id=ahci,debug=3 \
	-trace "ahci*" \
	-drive id=thatdisk,file=disk.img,if=none \
	-device ide-hd,drive=thatdisk,bus=ahci.0 \
	-drive id=thatcdrom,file=/dev/cdrom,if=none \
	-device ide-cd,drive=thatcdrom,bus=ahci.1 \
	# -drive id=thatcdrom,file=TEST.iso,if=none \
	# -device ide-cd,drive=thatcdrom,bus=ahci.1 \

run_disks:
	$(QEMU) $(QEMU_FLAGS) -serial mon:stdio -hda disk1.img -hdb disk2.img -hdd disk3.img
	
bra:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@$(MAKE) run_ahci_sata

disks:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@$(MAKE) run_disks

# Запуск Milla
milla:
	qemu-system-i386 -cdrom kernel.iso -serial file:Qemu.log -serial telnet:sayorios.piminoff.ru:10000 -accel kvm -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -soundhw pcspk

floppy:
	qemu-system-i386 -cdrom kernel.iso -serial mon:stdio -m 64M -name "SayoriOS v0.3.x (Dev)" -d guest_errors -rtc base=localtime -fda floppy.img -boot order=dc -accel kvm

# Запуск с логами в консоль
runlive:
	$(QEMU) -serial mon:stdio $(QEMU_FLAGS)

# Запуск в режиме UEFI с логами в файл
uefi:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial file:Qemu.log -accel kvm \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime -device ahci,id=ahci \
						-drive id=thatdisk,file=disk.img,if=none \
						-device ide-hd,drive=thatdisk,bus=ahci.0 \
						-drive id=thatcdrom,file=/dev/cdrom,if=none \
						-device ide-cd,drive=thatcdrom,bus=ahci.1 \


# Запуск в режиме UEFI с логами в консоль
uefilive:
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -cdrom SayoriOS_UEFI.iso -serial mon:stdio -accel kvm \
					   -m 128M -name "SayoriOS Soul" -d guest_errors -rtc base=localtime
# Генерация ISO-файла
geniso: $(KERNEL)
	$(shell bash tools/grub.sh) -o "kernel.iso" iso/ -V kernel

# Генерация ISO-файла с поддержкой UEFI
genuefi:
	$(shell bash tools/grub.sh) -d /usr/lib/grub/x86_64-efi -o SayoriOS_UEFI.iso iso/ --locale-directory=/usr/share/locale/ -V "SayoriOS Soul"

# Удаление оригинального файла и *.о файлов
clean:
	-rm -f $(KERNEL)
	-rm -f $(KERNEL_NEED)
	-rm -f $(DEPS)
	-rm -f iso/boot/ramdisk

# Линковка файлов
$(KERNEL): $(KERNEL_NEED)
	@echo -e '\x1b[32mLINK \x1b[0m' $(KERNEL)
	@rm -f $(KERNEL)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(KERNEL_NEED)
	@bash tools/genmap.sh
	@bash tools/insertmap.sh
	@ls -lh $(KERNEL) kernel.map
	@-rm kernel.map

# Быстрая линковка, генерация ISO, запуск
bir:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@$(MAKE) run

# Быстрая линковка, генерация ISO, запуск
birl:
	@$(MAKE)
	@$(MAKE) RAM
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
	@echo "LD:" $(LD)

release:
	ADDCFLAGS="-DRELEASE" $(MAKE)
	@$(MAKE) RAM

# ВНИМАНИЕ: Данное правило установит SayoriOS на ваш компьютер!
deploy: $(KERNEL)
	sudo cp iso/boot/kernel.elf /boot/sayorios_kernel.elf
	sudo cp iso/boot/ramdisk /boot/sayori_ramdisk
	sudo cp other/41_sayori /etc/grub.d/

	sudo update-grub

RAM: 
	@-rm ./iso/boot/ramdisk
	@tar -cvf ./iso/boot/ramdisk ./ramdisk/

VBOX_create_vm:
	@echo "TODO"
	@VBoxManage createvm --name=SayoriOS --register
	@VBoxManage storagectl SayoriOS --name=ide --add=ide --bootable=on
	@VBoxManage storageattach SayoriOS --type=dvddrive --medium kernel.iso --storagectl=ide --port=0 --device=0

VBOX:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@VBoxManage startvm "SayoriOS"

WSL_RUN:
	"/mnt/c/Program Files/qemu/qemu-system-i386.exe" -cdrom "C:\\SayoriDev\\SayoriOS_DEV_WSL.iso" -serial mon:stdio -m 128M -name "SayoriOS DEV WSL MODE" -d guest_errors -rtc base=localtime $(QEMU_FLAGS_WSL)


WSL:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@-mkdir /mnt/c/SayoriDev/
	mv kernel.iso /mnt/c/SayoriDev/SayoriOS_DEV_WSL.iso
	"/mnt/c/Program Files/qemu/qemu-system-i386.exe" -cdrom "C:\\SayoriDev\\SayoriOS_DEV_WSL.iso" -serial mon:stdio -m 128M -name "SayoriOS DEV WSL MODE" -d guest_errors -rtc base=localtime -netdev user,id=net1,net=192.168.222.0,dhcpstart=192.168.222.128 -device virtio-net-pci,netdev=net1,id=mydev1,mac=52:54:00:6a:40:f8 $(QEMU_FLAGS_WSL)

WSL_NAT:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@-mkdir /mnt/c/SayoriDev/
	mv kernel.iso /mnt/c/SayoriDev/SayoriOS_DEV_WSL.iso
	"/mnt/c/Program Files/qemu/qemu-system-i386.exe" -cdrom "C:\\SayoriDev\\SayoriOS_DEV_WSL.iso" -serial mon:stdio -serial telnet:sayorios.piminoff.ru:10000 -m 128M -name "SayoriOS DEV WSL MODE" -d guest_errors -rtc base=localtime $(QEMU_FLAGS_WSL)

WSL_DISKS:
	@$(MAKE)
	@$(MAKE) RAM
	@$(MAKE) geniso
	@-mkdir /mnt/c/SayoriDev/
	mv kernel.iso /mnt/c/SayoriDev/SayoriOS_DEV_WSL.iso
	@-mv disk1.img /mnt/c/SayoriDev/disk1.img
	@-mv disk2.img /mnt/c/SayoriDev/disk2.img
	@-mv disk3.img /mnt/c/SayoriDev/disk3.img
	"/mnt/c/Program Files/qemu/qemu-system-i386.exe" -cdrom "C:\\SayoriDev\\SayoriOS_DEV_WSL.iso" -serial mon:stdio -m 128M -name "SayoriOS DEV WSL MODE" -d guest_errors -rtc base=localtime $(QEMU_FLAGS_WSL) -hda "C:\\SayoriDev\\disk1.img" -hdb "C:\\SayoriDev\\disk2.img" -hdd "C:\\SayoriDev\\disk3.img"

create_fat_disk:
	fallocate -l 64M disk1.img
	sudo mkfs.fat -F 32 disk1.img

net_tap_dev: geniso
	sudo $(QEMU) -cdrom kernel.iso -m $(MEMORY_SIZE) \
            			 -name "SayoriOS Soul v0.3.5 (Dev) [NETWORK ON TAP]" \
            			 -rtc base=localtime \
            			 -d guest_errors,cpu_reset,int \
            			 -smp 1 \
            			 -netdev tap,id=net0 \
            			 -device rtl8139,netdev=net0,id=mydev0 \
            			 -boot d \
            			 -cpu core2duo-v1 \
						 -serial mon:stdio \
            			 $(KVM_QEMU_FLAGS)


clangd:
	$(MAKE) clean
	bear -- $(MAKE) -j2

-include $(DEPS)
