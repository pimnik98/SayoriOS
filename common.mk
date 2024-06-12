# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

KERNEL = iso/boot/kernel.elf
DEBUG?=# -ggdb3 #-Werror
MEMORY_SIZE?=128M
USE_SSE?=true
OPTIMIZATION_LEVEL?=0
BUILD_PREFIX=./

COMPILER_DETECTOR_FLAGS = ""

ifeq ($(X64),true)
	COMPILER_DETECTOR_FLAGS := "--64"
endif

CC=$(shell bash $(BUILD_PREFIX)tools/compiler.sh $(COMPILER_DETECTOR_FLAGS))

OBJ_DIRECTORY = objects
DIRECTORIES = $(BUILD_PREFIX)objects/kernel/src \
				$(BUILD_PREFIX)objects/kernel/asm \
				$(BUILD_PREFIX)objects/kernel/src/lib \
				$(BUILD_PREFIX)objects/kernel/src/lib/libstring/src \
				$(BUILD_PREFIX)objects/kernel/src/lib/libvector/src \
				$(BUILD_PREFIX)objects/kernel/src/lib/php \
				$(BUILD_PREFIX)objects/kernel/src/lib/elk \
				$(BUILD_PREFIX)objects/kernel/src/lib/elk/ext \
				$(BUILD_PREFIX)objects/kernel/src/extra \
				$(BUILD_PREFIX)objects/kernel/src/sys \
				$(BUILD_PREFIX)objects/kernel/src/io \
				$(BUILD_PREFIX)objects/kernel/src/net \
				$(BUILD_PREFIX)objects/kernel/src/drv \
				$(BUILD_PREFIX)objects/kernel/src/drv/disk \
				$(BUILD_PREFIX)objects/kernel/src/debug \
				$(BUILD_PREFIX)objects/kernel/src/fmt \
				$(BUILD_PREFIX)objects/kernel/src/drv/audio \
				$(BUILD_PREFIX)objects/kernel/src/drv/video \
				$(BUILD_PREFIX)objects/kernel/src/drv/input \
				$(BUILD_PREFIX)objects/kernel/src/drv/network \
				$(BUILD_PREFIX)objects/kernel/src/fs \
				$(BUILD_PREFIX)objects/kernel/src/lib/math \
				$(BUILD_PREFIX)objects/kernel/src/toys \
				$(BUILD_PREFIX)objects/kernel/src/gfx \
				$(BUILD_PREFIX)objects/kernel/src/gui \
				$(BUILD_PREFIX)objects/kernel/src/desktop \
				$(BUILD_PREFIX)objects/kernel/src/user \
				$(BUILD_PREFIX)objects/kernel/src/ports/gameboy \
				$(BUILD_PREFIX)objects/kernel/src/mem \
				$(BUILD_PREFIX)objects/kernel/src/arch/x86 \
				# objects/kernel/src/compress/zlib \

# Исходные объектные модули
ASM_SRC=$(BUILD_PREFIX)kernel/asm/init.s \
	$(BUILD_PREFIX)kernel/asm/interrupt.s \
	$(BUILD_PREFIX)kernel/asm/sys_calls.s \
	$(BUILD_PREFIX)kernel/asm/usr.s \
	$(BUILD_PREFIX)kernel/asm/sse.s \
	$(BUILD_PREFIX)kernel/asm/gdt.s \
	$(BUILD_PREFIX)kernel/asm/paging.s \
	$(BUILD_PREFIX)kernel/asm/regs.s \
	$(BUILD_PREFIX)kernel/src/lib/setjmp.s \
	$(BUILD_PREFIX)kernel/asm/switch_task.s \
	$(BUILD_PREFIX)kernel/asm/64bit_on_32bit.s \

ASM=$(ASM_SRC:%.s=$(OBJ_DIRECTORY)/%.o)

SOURCES=\
	$(BUILD_PREFIX)kernel/src/sys/cpuinfo.c \
	$(BUILD_PREFIX)kernel/src/sys/cpu_isr.c \
	$(BUILD_PREFIX)kernel/src/sys/gdt.c \
	$(BUILD_PREFIX)kernel/src/sys/isr.c \
	$(BUILD_PREFIX)kernel/src/io/ports.c \
	$(BUILD_PREFIX)kernel/src/io/serial_port.c \
	$(BUILD_PREFIX)kernel/src/lib/string.c \
	$(BUILD_PREFIX)kernel/src/drv/fpu.c \
	$(BUILD_PREFIX)kernel/src/sys/timer.c \
	$(BUILD_PREFIX)kernel/src/sys/logo.c \
	$(BUILD_PREFIX)kernel/src/lib/math/math.c \
	$(BUILD_PREFIX)kernel/src/mem/pmm.c	\
	$(BUILD_PREFIX)kernel/src/mem/vmm.c	\
	$(wildcard kernel/src/lib/libvector/src/*.c) \
	$(wildcard kernel/src/lib/libstring/src/*.c) \
	$(BUILD_PREFIX)kernel/src/lib/stdio.c \
	$(BUILD_PREFIX)kernel/src/lib/split.c \
	$(BUILD_PREFIX)kernel/src/io/screen.c \
	$(BUILD_PREFIX)kernel/src/io/tty.c \
	$(BUILD_PREFIX)kernel/src/fs/tarfs.c \
	$(BUILD_PREFIX)kernel/src/fs/fsm.c \
	$(BUILD_PREFIX)kernel/src/fs/nvfs.c \
	$(BUILD_PREFIX)kernel/src/fs/natfs.c \
	$(BUILD_PREFIX)kernel/src/fs/tempfs.c \
	$(BUILD_PREFIX)kernel/src/fs/iso9660.c \
	$(BUILD_PREFIX)kernel/src/lib/php/str_replace.c \
	$(BUILD_PREFIX)kernel/src/sys/scheduler.c \
	$(BUILD_PREFIX)kernel/src/lib/php/explode.c \
	$(BUILD_PREFIX)kernel/src/lib/php/pathinfo.c \
	$(BUILD_PREFIX)kernel/src/lib/elk/elk.c \
	$(BUILD_PREFIX)kernel/src/lib/elk/elk_engine.c \
	$(BUILD_PREFIX)kernel/src/lib/elk/elk_libs.c \
	$(BUILD_PREFIX)kernel/src/lib/elk/jse_func.c \
	$(wildcard kernel/src/lib/elk/ext/*.c) \
	$(BUILD_PREFIX)kernel/src/drv/psf.c \
	$(BUILD_PREFIX)kernel/src/sys/unwind.c \
	$(BUILD_PREFIX)kernel/src/fs/NatSuki.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/initrd.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/dpm.c \
	$(BUILD_PREFIX)kernel/src/lib/list.c \
	$(BUILD_PREFIX)kernel/src/lib/fileio.c \
	$(BUILD_PREFIX)kernel/src/sys/sync.c \
	$(BUILD_PREFIX)kernel/src/gui/basics.c \
	$(BUILD_PREFIX)kernel/src/lib/pixel.c \
	$(BUILD_PREFIX)kernel/src/sys/bootscreen.c \
	$(BUILD_PREFIX)kernel/src/debug/hexview.c \
	$(BUILD_PREFIX)kernel/src/drv/video/vbe.c \
	$(BUILD_PREFIX)kernel/src/drv/input/keyboard.c \
	$(BUILD_PREFIX)kernel/src/drv/input/mouse.c \
	$(BUILD_PREFIX)kernel/src/sys/syscalls.c \
	$(BUILD_PREFIX)kernel/src/sys/testing.c \
	$(BUILD_PREFIX)kernel/src/sys/trigger.c \
	$(BUILD_PREFIX)kernel/src/lib/rand.c \
	$(BUILD_PREFIX)kernel/src/drv/cmos.c \
	$(BUILD_PREFIX)kernel/src/drv/beeper.c \
	$(BUILD_PREFIX)kernel/src/user/env.c \
	$(BUILD_PREFIX)kernel/src/drv/pci.c \
	$(BUILD_PREFIX)kernel/src/gui/pointutils.c \
	$(BUILD_PREFIX)kernel/src/gui/line.c \
	$(BUILD_PREFIX)kernel/src/gui/circle.c \
	$(BUILD_PREFIX)kernel/src/lib/math/exp.c \
	$(BUILD_PREFIX)kernel/src/lib/math/log.c \
	$(BUILD_PREFIX)kernel/src/lib/math/pow.c \
	$(BUILD_PREFIX)kernel/src/lib/math/acos.c \
	$(BUILD_PREFIX)kernel/src/lib/math/asin.c \
	$(BUILD_PREFIX)kernel/src/lib/math/atan.c \
	$(BUILD_PREFIX)kernel/src/lib/math/modf.c \
	$(BUILD_PREFIX)kernel/src/lib/math/integral.c \
	$(BUILD_PREFIX)kernel/src/lib/math/sin.c \
	$(BUILD_PREFIX)kernel/src/lib/math/cos.c \
	$(BUILD_PREFIX)kernel/src/lib/math/tan.c \
	$(BUILD_PREFIX)kernel/src/lib/math/sqrt.c \
	$(BUILD_PREFIX)kernel/src/io/rgb_image.c \
	$(BUILD_PREFIX)kernel/src/lib/math/cbrt.c \
	$(BUILD_PREFIX)kernel/src/sys/cpuid.c	\
	$(BUILD_PREFIX)kernel/src/drv/disk/ata.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/atapi.c \
	$(BUILD_PREFIX)kernel/src/sys/cputemp.c	\
	$(BUILD_PREFIX)kernel/src/net/endianess.c \
	$(BUILD_PREFIX)kernel/src/net/cards.c \
	$(BUILD_PREFIX)kernel/src/net/ethernet.c \
	$(BUILD_PREFIX)kernel/src/net/arp.c \
	$(BUILD_PREFIX)kernel/src/net/ipv4.c \
	$(BUILD_PREFIX)kernel/src/net/udp.c \
	$(BUILD_PREFIX)kernel/src/net/dhcp.c \
	$(BUILD_PREFIX)kernel/src/net/icmp.c \
	$(BUILD_PREFIX)kernel/src/sys/system.c \
	$(BUILD_PREFIX)kernel/src/io/status_sounds.c \
	$(BUILD_PREFIX)kernel/src/io/status_loggers.c \
	$(BUILD_PREFIX)kernel/src/extra/cli.c \
	$(BUILD_PREFIX)kernel/src/sys/variable.c	\
	$(BUILD_PREFIX)kernel/src/fs/fat32.c \
	$(BUILD_PREFIX)kernel/src/sys/fxsave_region.c \
	$(BUILD_PREFIX)kernel/src/toys/gfxbench.c \
	$(BUILD_PREFIX)kernel/src/toys/miniplay.c \
	$(BUILD_PREFIX)kernel/src/drv/rtl8139.c \
	$(BUILD_PREFIX)kernel/src/drv/network/virtio_network.c \
	$(BUILD_PREFIX)kernel/src/fmt/tga.c \
	$(BUILD_PREFIX)kernel/src/lib/sprintf.c \
	$(BUILD_PREFIX)kernel/src/debug/ubsan.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/floppy.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/ata_dma.c \
	$(BUILD_PREFIX)kernel/src/drv/audio/ac97.c \
	$(BUILD_PREFIX)kernel/src/sys/elf.c \
	$(BUILD_PREFIX)kernel/src/sys/acpi.c \
	$(BUILD_PREFIX)kernel/src/sys/pixfmt.c \
	$(BUILD_PREFIX)kernel/src/desktop/render.c \
	$(BUILD_PREFIX)kernel/src/desktop/window.c \
	$(BUILD_PREFIX)kernel/src/desktop/widget.c \
	$(BUILD_PREFIX)kernel/src/desktop/widget_button.c \
	$(BUILD_PREFIX)kernel/src/desktop/widget_progress.c \
	$(BUILD_PREFIX)kernel/src/desktop/widget_image.c \
	$(BUILD_PREFIX)kernel/src/desktop/widget_label.c \
	$(BUILD_PREFIX)kernel/src/desktop/eki.c \
	$(BUILD_PREFIX)kernel/src/desktop/parallel_desktop.c \
	$(BUILD_PREFIX)kernel/src/sys/mtrr.c \
	$(BUILD_PREFIX)kernel/src/net/net_info_cli.c \
	$(BUILD_PREFIX)kernel/src/toys/mala.c \
	$(BUILD_PREFIX)kernel/src/debug/memmeter.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/ahci.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/ata_pio.c \
	$(BUILD_PREFIX)kernel/src/toys/minesweeper.c \
	$(BUILD_PREFIX)kernel/src/toys/calendar.c \
	$(BUILD_PREFIX)kernel/src/toys/diskctl.c \
	$(BUILD_PREFIX)kernel/src/lib/utf_conversion.c \
	$(BUILD_PREFIX)kernel/src/lib/base64.c \
	$(BUILD_PREFIX)kernel/src/sys/file_descriptors.c \
	$(BUILD_PREFIX)kernel/src/net/tcp.c \
	$(BUILD_PREFIX)kernel/src/net/stack.c \
	$(BUILD_PREFIX)kernel/src/toys/pavi.c \
	$(BUILD_PREFIX)kernel/src/drv/audio/hda.c \
	$(BUILD_PREFIX)kernel/src/sys/grub_modules.c \
	$(BUILD_PREFIX)kernel/src/drv/disk/mbr.c \
	$(BUILD_PREFIX)kernel/src/sys/lapic.c \
	$(BUILD_PREFIX)kernel/src/drv/ps2.c \
	$(BUILD_PREFIX)kernel/src/gfx/intel.c \
	$(BUILD_PREFIX)kernel/src/kernel.c \
#	kernel/src/lib/duktape.c \
	kernel/src/toys/piano.c \
	kernel/src/toys/dino.c \
	kernel/src/lib/ttf_font.c \
	kernel/src/extra/texplorer.c \
	kernel/src/drv/disk/mbr.c \
	kernel/src/fs/fat12.c \
	kernel/src/fs/smfs.c \
	kernel/src/lib/base64.c \

OBJS = $(SOURCES:%.c=$(OBJ_DIRECTORY)/%.o)
DEPS = $(OBJS:%.o=%.d)

KERNEL_NEED = $(ASM) $(OBJS)

COMMON_FLAGS = -O$(OPTIMIZATION_LEVEL) -nostdlib -fno-stack-protector -fno-builtin -Ikernel/include/ -ffreestanding \
			   -Wall -Wno-div-by-zero -Wno-address-of-packed-member -Wno-implicit-function-declaration \
			   -mno-red-zone -MMD -MP 

# Флаги компилятора языка C
CFLAGS=$(DEBUG) $(ADDCFLAGS) $(COMMON_FLAGS)
CPP_FLAGS=$(DEBUG) $(COMMON_FLAGS) -fno-use-cxa-atexit -fno-exceptions -fno-rtti -Werror -Ikernel/cpp/include

LD ?= ld.lld
# Флаги компоновщика
LDFLAGS=-T kernel/asm/link.ld

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
			 -device ich9-intel-hda,debug=0 \
			 -device hda-output,audiodev=pa0 \
			 -trace "hda*" \
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
