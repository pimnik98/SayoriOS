# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

KERNEL = iso/boot/kernel.elf
DEBUG =# -ggdb3 #-Werror
MEMORY_SIZE=128M
USE_SSE=true

COMPILER_DETECTOR_FLAGS = ""

ifeq ($(X64),true)
	COMPILER_DETECTOR_FLAGS := "--64"
endif

CC=$(shell bash tools/compiler.sh $(COMPILER_DETECTOR_FLAGS))

GAMEBOY = $(wildcard kernel/src/ports/gameboy/*.c)
GAMEBOY_OBJS = $(GAMEBOY:.c=.o)

OBJ_DIRECTORY = objects
DIRECTORIES = objects/kernel/src \
				objects/kernel/asm \
				objects/kernel/src/lib \
				objects/kernel/src/lib/libstring/src \
				objects/kernel/src/lib/libvector/src \
				objects/kernel/src/lib/php \
				objects/kernel/src/lib/elk \
				objects/kernel/src/lib/elk/ext \
				objects/kernel/src/extra \
				objects/kernel/src/sys \
				objects/kernel/src/io \
				objects/kernel/src/net \
				objects/kernel/src/drv \
				objects/kernel/src/drv/disk \
				objects/kernel/src/debug \
				objects/kernel/src/fmt \
				objects/kernel/src/drv/audio \
				objects/kernel/src/drv/video \
				objects/kernel/src/drv/input \
				objects/kernel/src/drv/network \
				objects/kernel/src/fs \
				objects/kernel/src/lib/math \
				objects/kernel/src/toys \
				objects/kernel/src/gui \
				objects/kernel/src/desktop \
				objects/kernel/src/user \
				objects/kernel/src/ports/gameboy \
				objects/kernel/src/mem \
				objects/kernel/src/arch/x86 \
				# objects/kernel/src/compress/zlib \

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
#	kernel/src/sys/v8086.s \

ASM=$(ASM_SRC:%.s=$(OBJ_DIRECTORY)/%.o)

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
	$(wildcard kernel/src/lib/libvector/src/*.c) \
	$(wildcard kernel/src/lib/libstring/src/*.c) \
	kernel/src/lib/stdio.c \
	kernel/src/lib/split.c \
	kernel/src/io/screen.c \
	kernel/src/io/tty.c \
	kernel/src/fs/tarfs.c \
	kernel/src/fs/fsm.c \
	kernel/src/fs/nvfs.c \
	kernel/src/fs/natfs.c \
	kernel/src/fs/iso9660.c \
	kernel/src/lib/php/str_replace.c \
	kernel/src/sys/scheduler.c \
	kernel/src/lib/php/explode.c \
	kernel/src/lib/php/pathinfo.c \
	kernel/src/lib/elk/elk.c \
	kernel/src/lib/elk/elk_engine.c \
	kernel/src/lib/elk/elk_libs.c \
	kernel/src/lib/elk/jse_func.c \
	$(wildcard kernel/src/lib/elk/ext/*.c) \
	kernel/src/drv/psf.c \
	kernel/src/sys/unwind.c \
	kernel/src/fs/NatSuki.c \
	kernel/src/drv/disk/initrd.c \
	kernel/src/drv/disk/dpm.c \
	kernel/src/lib/list.c \
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
	$(wildcard kernel/src/lib/libstring/*.c) \
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
	kernel/src/drv/rtl8139.c \
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
	kernel/src/net/stack.c \
	kernel/src/toys/pavi.c \
	kernel/src/drv/audio/hda.c \
	kernel/src/sys/grub_modules.c \
	kernel/src/drv/disk/mbr.c \
	$(GAMEBOY) \
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

RUST_DIR = rust/
RUST_TARGET = i686-unknown-none
RUST_OBJ_DEBUG = rust/target/$(RUST_TARGET)/debug/librust.a
RUST_OBJ_RELEASE = rust/target/$(RUST_TARGET)/release/librust.a
RUST_SOURCES = $(shell find rust/src/ -type f -name '*.rs')

OBJS = $(SOURCES:%.c=$(OBJ_DIRECTORY)/%.o)
DEPS = $(OBJS:%.o=%.d)

KERNEL_NEED = $(ASM) $(OBJS) $(CPP_CODE)

COMMON_FLAGS = -O0 -nostdlib -fno-stack-protector -fno-builtin -Ikernel/include/ -ffreestanding \
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

# NOTE: -d int works only when using tcg accelerator (no KVM)
QEMU_FLAGS = -cdrom kernel.iso -m $(MEMORY_SIZE) \
			 -name "SayoriOS Soul v0.3.5 (Dev)" \
			 -rtc base=localtime \
			 -d guest_errors,cpu_reset,int \
			 -audiodev pa,id=pa0 \
			 -smp 2 \
			 -netdev user,id=net0,net=192.168.111.0,dhcpstart=192.168.111.128,hostfwd=tcp::9999-:8888 \
			 -device rtl8139,netdev=net0,id=mydev0 \
			 -M pcspk-audiodev=pa0 \
			 -device ich9-intel-hda \
			 -trace "hda*" \
			 -boot d \
			 -cpu core2duo-v1 \
			 -object filter-dump,id=dump0,netdev=net0,file=netdump.pcap \
			 $(KVM_QEMU_FLAGS) \
			 $(ADDITIONAL_QEMU_FLAGS)
 			 # -device AC97 \


# NOTE: -d int works only when using tcg accelerator (no KVM)
QEMU_FLAGS_WSL = -m $(MEMORY_SIZE) \
			 -name "SayoriOS Soul v0.3.5 (Dev) [WSL]" \
			 -rtc base=localtime \
			 -smp 1 \
			 -netdev user,id=net0,net=192.168.111.0,dhcpstart=192.168.111.128 \
			 -device rtl8139,netdev=net0,id=mydev0 \
			 -device AC97 \
			 -boot d \
			 -cpu core2duo-v1
