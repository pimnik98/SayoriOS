# MakeFile
# SayoriOS Soul
# (c) SayoriOS Team 2022-2023

# CC=$(shell bash other/compiler.sh cpp)
CC=$(shell bash other/compiler.sh)
CXX=$(shell bash other/compiler.sh cpp)

MEMORY_SIZE=128M
USE_SSE=true

GAMEBOY = $(wildcard kernel/src/ports/gameboy/*.c)
GAMEBOY_OBJS = $(GAMEBOY:.c=.o)

OBJ_DIRECTORY = objects
DIRECTORIES = objects/kernel/src \
				objects/kernel/asm \
				objects/kernel/src/lib \
				objects/kernel/src/lib/php \
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
				objects/kernel/src/fs \
				objects/kernel/src/lib/math \
				objects/kernel/src/toys \
				objects/kernel/src/gui \
				objects/kernel/src/user \
				objects/kernel/src/ports/gameboy \
				objects/kernel/src/compress/zlib \

# Исходные объектные модули
ASM_SRC=kernel/asm/init.s \
	kernel/asm/interrupt.s \
	kernel/asm/switch_task.s \
	kernel/asm/sys_calls.s \
	kernel/asm/usr.s \
	kernel/asm/sse.s \
	kernel/asm/gdt.s \
	kernel/asm/regs.s \
	kernel/src/lib/setjmp.s \
	kernel/src/sys/v8086.s \

ASM=$(ASM_SRC:%.s=$(OBJ_DIRECTORY)/%.o)

SOURCES=kernel/src/sys/bootscreen.c \
	kernel/src/sys/cpu_isr.c \
	$(GAMEBOY) \
	kernel/src/sys/acpi.c \
	kernel/src/sys/cpuinfo.c \
	kernel/src/sys/gdt.c \
	kernel/src/sys/tss.c \
	kernel/src/sys/idt.c \
	kernel/src/sys/isr.c \
	kernel/src/sys/memory.c \
	kernel/src/sys/scheduler.c \
	kernel/src/sys/sync.c \
	kernel/src/sys/syscalls.c \
	kernel/src/sys/system.c \
	kernel/src/sys/timer.c \
	kernel/src/sys/elf.c \
	kernel/src/lib/list.c \
	kernel/src/lib/dan.c \
	kernel/src/lib/stdio.c \
	kernel/src/lib/string.c \
	kernel/src/lib/split.c \
	kernel/src/io/tty.c \
	kernel/src/io/status_sounds.c \
	kernel/src/io/status_loggers.c \
	kernel/src/io/screen.c \
	kernel/src/drv/cmos.c \
	kernel/src/drv/vfs_new.c \
	kernel/src/drv/video/vbe.c \
	kernel/src/drv/input/keyboard.c \
	kernel/src/drv/input/mouse.c \
	kernel/src/drv/beeper.c \
	kernel/src/drv/fpu.c \
	kernel/src/drv/rtl8139.c \
	kernel/src/drv/pci.c \
	kernel/src/io/ports.c \
	kernel/src/fs/sefs.c \
	kernel/src/fs/milla.c \
	kernel/src/fs/NatSuki.c \
	kernel/src/user/env.c \
	kernel/src/sys/logo.c \
	kernel/src/io/port_io.c \
	kernel/src/gui/pointutils.c \
	kernel/src/gui/eki.c \
	kernel/src/gui/line.c \
	kernel/src/gui/circle.c \
	kernel/src/drv/audio/ac97.c \
	kernel/src/toys/piano.c \
	kernel/src/toys/minesweeper.c \
	kernel/src/toys/diskctl.c \
	kernel/src/toys/danview.c \
	kernel/src/toys/mala.c \
	kernel/src/toys/dino.c \
	kernel/src/toys/calendar.c \
	kernel/src/sys/testing.c \
	kernel/src/sys/trigger.c \
	kernel/src/lib/pixel.c \
	kernel/src/lib/tui.c \
	kernel/src/lib/rand.c \
	kernel/src/lib/sprintf.c \
	kernel/src/lib/php/str_replace.c \
	kernel/src/extra/texplorer.c \
	kernel/src/io/shell.c \
	kernel/src/sys/unwind.c \
	kernel/src/sys/fxsave_region.c \
	kernel/src/net/endianess.c \
	kernel/src/net/cards.c \
	kernel/src/net/ethernet.c \
	kernel/src/net/arp.c \
	kernel/src/drv/disk/mbr.c \
	kernel/src/drv/disk/floppy.c \
	kernel/src/drv/disk/ata.c \
	kernel/src/drv/disk/atapi.c \
	kernel/src/fs/fat12.c \
	kernel/src/fs/smfs.c \
	kernel/src/debug/hexview.c \
	kernel/src/debug/ubsan.c \
	kernel/src/fmt/tga.c \
	kernel/src/fmt/tga_extract.c \
	kernel/src/sys/pixfmt.c \
	kernel/src/lib/math/exp.c \
	kernel/src/lib/math/log.c \
	kernel/src/lib/math/pow.c \
	kernel/src/lib/math/acos.c \
	kernel/src/lib/math/asin.c \
	kernel/src/lib/math/atan.c \
	kernel/src/lib/math/integral.c \
	kernel/src/lib/math/sin.c \
	kernel/src/lib/math/cos.c \
	kernel/src/lib/math/tan.c \
	kernel/src/lib/math/sqrt.c \
	kernel/src/lib/math/cbrt.c \
	kernel/src/lib/math/math.c \
	kernel/src/gui/basics.c \
	kernel/src/gui/render.c \
	kernel/src/drv/psf.c \
	kernel/src/gui/window.c \
	kernel/src/gui/widget.c \
	kernel/src/gui/widget_button.c \
	kernel/src/gui/widget_label.c \
	kernel/src/gui/widget_image.c \
	kernel/src/gui/parallel_desktop.c \
	kernel/src/lib/base64.c \
	kernel/src/io/duke_image.c	\
	kernel/src/sys/cpuid.c	\
	kernel/src/sys/cputemp.c	\
	$(wildcard kernel/src/compress/zlib/*.c)	\
	kernel/src/zlibtest.c \
	kernel/src/kernel.c \
	# kernel/src/fs/lucario/fs.c \
	# kernel/src/drv/ata_dma.c \

RUST_DIR = rust/
RUST_TARGET = i686-unknown-none
RUST_OBJ_DEBUG = rust/target/$(RUST_TARGET)/debug/librust.a
RUST_OBJ_RELEASE = rust/target/$(RUST_TARGET)/release/librust.a
RUST_SOURCES = $(shell find rust/src/ -type f -name '*.rs')

OBJS = $(SOURCES:%.c=$(OBJ_DIRECTORY)/%.o)

# NDRAEY: No one except me knows C++, so no one can develop C++ API for kernel.
# NDRAEY: Uncomment it if you want

#CPP_CODE = kernel/cpp/src/start.o \
#		   kernel/cpp/src/lib/tty.o \
#		   kernel/cpp/src/lib/math.o \
#		   kernel/cpp/src/lib/memory.o \
#		   kernel/cpp/src/lib/string.o \
#		   kernel/cpp/src/lib/file.o \
#		   kernel/cpp/src/lib/conv.o \
#		   kernel/cpp/src/lib/log.o \
#		   kernel/cpp/src/lib/display.o \
#		   kernel/cpp/src/audio/machinist.o \
#		   kernel/cpp/src/audio/machinist_server.o \
#		   kernel/cpp/src/audio/machinist_client.o \
#		   kernel/cpp/src/audio/machinist_ac97.o \
#		   kernel/cpp/src/inteleon_ui/rectangle.o \
#		   kernel/cpp/src/inteleon_ui/vertical_layer.o \
#		   kernel/cpp/src/inteleon_ui/horizontal_layer.o \
#		   kernel/cpp/src/inteleon_ui/margin.o \
#		   kernel/cpp/src/inteleon_ui/border.o \
#		   kernel/cpp/src/inteleon_ui/widget.o \
#		   kernel/cpp/src/test.o

DEPS = $(OBJS:%.o=%.d)

KERNEL = iso/boot/kernel.elf

KERNEL_NEED = $(ASM) $(OBJS) $(CPP_CODE)

DEBUG = #-ggdb3 #-Werror

COMMON_FLAGS = -nostdlib -fno-stack-protector -fno-builtin -m32 -Ikernel/include/ -ffreestanding \
			   -Wall -Wno-div-by-zero -Wno-address-of-packed-member -Wno-implicit-function-declaration \
			   -mno-red-zone -march=i386 -MMD -MP -Wno-everything

ifeq ($(USE_SSE),true)
	COMMON_FLAGS := $(COMMON_FLAGS) -msse2 -DUSE_SSE
endif

# Флаги компилятора языка C
CFLAGS=$(DEBUG) $(ADDCFLAGS) $(COMMON_FLAGS)
CPP_FLAGS=$(DEBUG) $(COMMON_FLAGS) -fno-use-cxa-atexit -fno-exceptions -fno-rtti -Werror -Ikernel/cpp/include

LD ?= ld.lld
# Флаги компоновщика
LDFLAGS=-T kernel/asm/link.ld -m elf_i386

# Флаги ассемблера
ASFLAGS=--32

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
			 -name "SayoriOS Soul v0.3.3 (Dev)" \
			 -rtc base=localtime \
			 -d guest_errors,cpu_reset,int \
			 -audiodev pa,id=pa0 \
			 -smp 1 \
			 -netdev user,id=net0 \
			 -device rtl8139,netdev=net0,id=mydev0 \
			 -M pcspk-audiodev=pa0 \
			 -device AC97 \
			 -boot d \
			 -cpu core2duo-v1 \
			 $(KVM_QEMU_FLAGS)
