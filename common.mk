# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

include config.mk

KERNEL = iso/boot/kernel.elf
BUILD_PREFIX=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

COMPILER_DETECTOR_FLAGS = ""

ifeq ($(X64),true)
	COMPILER_DETECTOR_FLAGS := "--64"
endif

CC=$(shell bash $(BUILD_PREFIX)tools/compiler.sh $(COMPILER_DETECTOR_FLAGS))

OBJ_DIRECTORY = objects

# Исходные объектные модули
ASM_SRC=kernel/asm/init.s \
	kernel/asm/interrupt.s \
	kernel/asm/sys_calls.s \
	kernel/asm/usr.s \
	kernel/asm/sse.s \
	kernel/asm/gdt.s \
	kernel/asm/paging.s \
	kernel/asm/regs.s \
	kernel/src/lib/setjmp.s \
	kernel/asm/switch_task.s \
	kernel/asm/64bit_on_32bit.s \

ASM_SRC:=$(ASM_SRC:%.s=$(BUILD_PREFIX)/%.s)

ASM=$(ASM_SRC:%.s=$(OBJ_DIRECTORY)/%.o)

ELK=\
	kernel/src/lib/elk/elk.c \
	kernel/src/lib/elk/elk_engine.c \
	kernel/src/lib/elk/elk_libs.c \
	kernel/src/lib/elk/jse_func.c \

SOURCES=\
	kernel/src/sys/cpuinfo.c \
	kernel/src/sys/cpu_isr.c \
	kernel/src/sys/gdt.c \
	kernel/src/sys/isr.c \
	kernel/src/io/ports.c \
	kernel/src/io/serial_port.c \
	kernel/src/lib/string.c \
	kernel/src/drv/fpu.c \
	kernel/src/sys/timer.c \
	kernel/src/sys/logo.c \
	kernel/src/lib/math/math.c \
	kernel/src/mem/pmm.c	\
	kernel/src/mem/vmm.c	\
	kernel/src/lib/stdio.c \
	kernel/src/lib/split.c \
	kernel/src/io/screen.c \
	kernel/src/io/tty.c \
	kernel/src/fs/tarfs.c \
	kernel/src/fs/fsm.c \
	kernel/src/fs/nvfs.c \
	kernel/src/fs/natfs.c \
	kernel/src/fs/tempfs.c \
	kernel/src/fs/iso9660.c \
	kernel/src/lib/php/str_replace.c \
	kernel/src/sys/scheduler.c \
	kernel/src/lib/php/explode.c \
	kernel/src/lib/php/pathinfo.c \
	$(ELK) \
	kernel/src/drv/psf.c \
	kernel/src/sys/unwind.c \
	kernel/src/fs/NatSuki.c \
	kernel/src/drv/disk/initrd.c \
	kernel/src/drv/disk/dpm.c \
	kernel/src/lib/list.c \
	kernel/src/lib/fileio.c \
	kernel/src/sys/sync.c \
	kernel/src/gui/basics.c \
	kernel/src/lib/pixel.c \
	kernel/src/sys/bootscreen.c \
	kernel/src/debug/hexview.c \
	kernel/src/drv/video/vbe.c \
	kernel/src/drv/input/keyboard.c \
	kernel/src/drv/input/mouse.c \
	kernel/src/sys/syscalls.c \
	kernel/src/sys/testing.c \
	kernel/src/sys/trigger.c \
	kernel/src/lib/rand.c \
	kernel/src/drv/cmos.c \
	kernel/src/drv/beeper.c \
	kernel/src/user/env.c \
	kernel/src/drv/pci.c \
	kernel/src/gui/pointutils.c \
	kernel/src/gui/line.c \
	kernel/src/gui/circle.c \
	kernel/src/lib/math/exp.c \
	kernel/src/lib/math/log.c \
	kernel/src/lib/math/pow.c \
	kernel/src/lib/math/acos.c \
	kernel/src/lib/math/asin.c \
	kernel/src/lib/math/atan.c \
	kernel/src/lib/math/modf.c \
	kernel/src/lib/math/integral.c \
	kernel/src/lib/math/sin.c \
	kernel/src/lib/math/cos.c \
	kernel/src/lib/math/tan.c \
	kernel/src/lib/math/sqrt.c \
	kernel/src/io/rgb_image.c \
	kernel/src/lib/math/cbrt.c \
	kernel/src/sys/cpuid.c	\
	kernel/src/drv/disk/ata.c \
	kernel/src/drv/disk/atapi.c \
	kernel/src/sys/cputemp.c	\
	kernel/src/net/endianess.c \
	kernel/src/net/cards.c \
	kernel/src/net/ethernet.c \
	kernel/src/net/arp.c \
	kernel/src/net/ipv4.c \
	kernel/src/net/udp.c \
	kernel/src/net/dhcp.c \
	kernel/src/net/icmp.c \
	kernel/src/sys/system.c \
	kernel/src/io/status_sounds.c \
	kernel/src/io/status_loggers.c \
	kernel/src/extra/cli.c \
	kernel/src/sys/variable.c	\
	kernel/src/fs/fat32.c \
	kernel/src/sys/fxsave_region.c \
	kernel/src/toys/gfxbench.c \
	kernel/src/toys/miniplay.c \
	kernel/src/drv/network/rtl8139.c \
	kernel/src/drv/network/virtio_network.c \
	kernel/src/fmt/tga.c \
	kernel/src/lib/sprintf.c \
	kernel/src/debug/ubsan.c \
	kernel/src/drv/disk/floppy.c \
	kernel/src/drv/disk/ata_dma.c \
	kernel/src/drv/audio/ac97.c \
	kernel/src/sys/elf.c \
	kernel/src/sys/acpi.c \
	kernel/src/sys/pixfmt.c \
	kernel/src/desktop/render.c \
	kernel/src/desktop/window.c \
	kernel/src/desktop/widget.c \
	kernel/src/desktop/widget_button.c \
	kernel/src/desktop/widget_progress.c \
	kernel/src/desktop/widget_image.c \
	kernel/src/desktop/widget_label.c \
	kernel/src/desktop/eki.c \
	kernel/src/desktop/parallel_desktop.c \
	kernel/src/sys/mtrr.c \
	kernel/src/net/net_info_cli.c \
	kernel/src/toys/mala.c \
	kernel/src/debug/memmeter.c \
	kernel/src/drv/disk/ahci.c \
	kernel/src/drv/disk/ata_pio.c \
	kernel/src/toys/minesweeper.c \
	kernel/src/toys/calendar.c \
	kernel/src/toys/diskctl.c \
	kernel/src/lib/utf_conversion.c \
	kernel/src/lib/base64.c \
	kernel/src/sys/file_descriptors.c \
	kernel/src/net/tcp.c \
	kernel/src/net/stack.c \
	kernel/src/toys/pavi.c \
	kernel/src/toys/forth.c \
	kernel/src/drv/audio/hda.c \
	kernel/src/sys/grub_modules.c \
	kernel/src/drv/disk/mbr.c \
	kernel/src/sys/lapic.c \
	kernel/src/drv/ps2.c \
	kernel/src/drv/video/intel.c \
	kernel/src/extra/command_parser.c \
	kernel/src/kernel.c \
#	kernel/src/lib/duktape.c \
	kernel/src/toys/piano.c \
	kernel/src/toys/dino.c \
	kernel/src/lib/ttf_font.c \
	kernel/src/extra/texplorer.c \
	kernel/src/drv/disk/mbr.c \
	kernel/src/fs/fat12.c \
	kernel/src/fs/smfs.c \
	kernel/src/lib/base64.c \

SOURCES:=$(SOURCES:%.c=$(BUILD_PREFIX)/%.c) \
	$(wildcard $(BUILD_PREFIX)/kernel/src/lib/libvector/src/*.c) \
	$(wildcard $(BUILD_PREFIX)/kernel/src/lib/libstring/src/*.c) \
	$(wildcard $(BUILD_PREFIX)/kernel/src/lib/elk/ext/*.c) \

DIRECTORIES := $(addprefix $(OBJ_DIRECTORY)/,$(sort $(dir $(SOURCES) $(ASM_SRC))))

OBJS = $(SOURCES:%.c=$(OBJ_DIRECTORY)/%.o)
DEPS = $(OBJS:%.o=%.d)

KERNEL_NEED = $(ASM) $(OBJS)

COMMON_FLAGS = -O$(OPTIMIZATION_LEVEL) -nostdlib -fno-stack-protector -fno-builtin -I$(BUILD_PREFIX)kernel/include/ -ffreestanding \
			   -Wall -Wno-div-by-zero -Wno-address-of-packed-member -Wno-implicit-function-declaration \
			   -mno-red-zone -MMD -MP -g 

# Флаги компилятора языка C
CFLAGS=$(DEBUG) $(ADDCFLAGS) $(COMMON_FLAGS)
CPP_FLAGS=$(DEBUG) $(COMMON_FLAGS) -fno-use-cxa-atexit -fno-exceptions -fno-rtti -Werror -Ikernel/cpp/include

LD ?= ld.lld
# Флаги компоновщика
LDFLAGS=-T $(BUILD_PREFIX)kernel/asm/link.ld

# Флаги ассемблера
ASFLAGS=--32

ifeq ($(USE_SSE),true)
	COMMON_FLAGS := $(COMMON_FLAGS) -msse2 -DUSE_SSE
endif

ifeq ($(X64),true)
	COMMON_FLAGS := $(COMMON_FLAGS) -DSAYORI64
	LDFLAGS := $(LDFLAGS) -m elf_x86_64
else
	LDFLAGS := $(LDFLAGS) -m elf_i386
endif


QEMU ?= qemu-system-i386

# Memory minimal: 33 MB

KVM_QEMU_FLAGS := -accel kvm

ifeq ($(KVM),false)
    # Remove -accel kvm when KVM=false
    KVM_QEMU_FLAGS :=
endif

ifeq ($(SANITIZE),true)
    # Remove -accel kvm when KVM=false
    COMMON_FLAGS := $(COMMON_FLAGS) -fsanitize=undefined
endif

QEMU_BASE_FLAGS = -cdrom kernel.iso \
 				 -m $(MEMORY_SIZE) \
				 -name "SayoriOS Soul v0.3.5 (Dev)" \
				 -rtc base=localtime \
				 -d guest_errors,cpu_reset,int \
    			 $(KVM_QEMU_FLAGS) \
	    		 $(ADDITIONAL_QEMU_FLAGS)


# NOTE: -d int works only when using tcg accelerator (no KVM)
QEMU_FLAGS = $(QEMU_BASE_FLAGS) \
			 -audiodev pa,id=pa0 \
			 -netdev user,id=net0,net=192.168.111.0,dhcpstart=192.168.111.128,hostfwd=tcp::9999-:9999 \
			 -device rtl8139,netdev=net0,id=mydev0 \
			 -M pcspk-audiodev=pa0 \
			 -device ac97,audiodev=pa0 \
			 -trace "hda*" \
			 -device ich9-usb-uhci1 \
			 -drive file=disk.img,id=disk0,if=none \
			 -device usb-storage,drive=disk0 \
			 -boot d \
			 -object filter-dump,id=dump0,netdev=net0,file=netdump.pcap \
 			 # -device AC97 \


# NOTE: -d int works only when using tcg accelerator (no KVM)
QEMU_FLAGS_WSL = $(QEMU_BASE_FLAGS) \
			 -netdev user,id=net0,net=192.168.111.0,dhcpstart=192.168.111.128 \
			 -device rtl8139,netdev=net0,id=mydev0 \
			 -device AC97 \
			 -boot d \
			 -cpu core2duo-v1
