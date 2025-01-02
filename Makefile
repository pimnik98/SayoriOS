### ПРОЧИЕ НАСТРОЙКИ
CURRENT_USER 			:= $(shell whoami)
WSL_ADB 				= /mnt/c/SayoriDev/adb.exe
SRC_MAIN 				= $(wildcard src/os/*.c) $(wildcard src/os/apps/*.c) $(wildcard src/os/libs/*.c) $(wildcard src/os/modules/*.c)

### Настройка сборки PSP
PSP_SRC_MAIN 			= $(wildcard src/devices/psp/*.c) $(wildcard src/devices/psp/drivers/*.c)

TARGET 					= sayori_whisper
OBJS 					= $(SRC_MAIN) $(PSP_SRC_MAIN)

CFLAGS 					=  
CXXFLAGS 				= $(CFLAGS) -std=c++14 -fno-rtti -Wimplicit-function-declaration
ASFLAGS 				= $(CFLAGS)

BUILD_PRX 				= 1
PSP_FW_VERSION 			= 500
PSP_LARGE_MEMORY 		= 1

EXTRA_TARGETS 			= EBOOT.PBP
PSP_EBOOT_NAME 			= SayoriOS Whisper
PSP_EBOOT_TITLE 		= SayoriOS Whisper
PSPSDK=$(shell psp-config --pspsdk-path)
#PSP_EBOOT_ICON 		= ICON0.PNG

### Настройка сборки Android

ANDROID_NDK 			?= android-ndk-r11c
ANDROID_NDK_DIR 		?= /opt/SayoriDev/$(ANDROID_NDK)/
ANDROID_TOOLCHAIN_DIR 	?= $(ANDROID_NDK_DIR)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/
ANDROID_CROSS_CC_PROG 	?= arm-linux-androideabi-gcc
ANDROID_CROSS_CC 		?= $(ANDROID_TOOLCHAIN_DIR)/$(ANDROID_CROSS_CC_PROG)
ANDROID_SYSROOT_DIR 	?= $(ANDROID_NDK_DIR)/platforms/android-17/arch-arm/
ANDROID_PLAT_DIR 		?= $(ANDROID_SYSROOT_DIR)/usr/
ANDROID_LINK_LIBS 		= -lm
ANDROID_OUTPUT_NAME 	= whisper.elf
ANDROID_SRC_MAIN 		= $(wildcard src/devices/android/*.c) $(wildcard src/devices/android/drivers/*.c)

### ПРАВИЛА СБОРКИ
include /home/pimnik98/pspdev/psp/sdk/lib/build.mak

build_android:
	$(ANDROID_CROSS_CC) --sysroot=$(ANDROID_SYSROOT_DIR) -std=c99 -I $(ANDROID_PLAT_DIR)include/ -L $(ANDROID_PLAT_DIR)lib/ -fPIE -static -o $(ANDROID_OUTPUT_NAME) $(ANDROID_SRC_MAIN) -D__ARM -O0 -w $(ANDROID_LINK_LIBS)
	python3 tools/fix_tls_alignment.py $(ANDROID_OUTPUT_NAME)

build_psp:
	@-rm -rf out/psp/*
	make
	@-mkdir out/psp/ -p
	@-mv EBOOT.PBP out/psp/EBOOT.PBP
	@-mv PARAM.SFO out/psp/PARAM.SFO
	@-mv sayori_whisper.prx out/psp/sayori_whisper.prx
	@-mv sayori_whisper.elf out/psp/sayori_whisper.elf

wsl_copy_psp:
	@-mkdir /mnt/c/SayoriDev/Whisper/PSP/ -p
	@-rm -rf /mnt/c/SayoriDev/Whisper/PSP/*
	@-cp -r out/psp/ /mnt/c/SayoriDev/Whisper/
