ANDROID_NDK 			?= android-ndk-r11c
ANDROID_NDK_DIR 		?= /opt/SayoriDev/$(ANDROID_NDK)/
ANDROID_TOOLCHAIN_DIR 	?= $(ANDROID_NDK_DIR)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/
ANDROID_CROSS_CC_PROG 	?= arm-linux-androideabi-gcc
ANDROID_CROSS_CC 		?= $(ANDROID_TOOLCHAIN_DIR)/$(ANDROID_CROSS_CC_PROG)
ANDROID_SYSROOT_DIR 	?= $(ANDROID_NDK_DIR)/platforms/android-17/arch-arm/
ANDROID_PLAT_DIR 		?= $(ANDROID_SYSROOT_DIR)/usr/
ANDROID_LINK_LIBS 		= -lm
ANDROID_OUTPUT_NAME 	= whisper.elf

WSL_ADB 				= /mnt/c/SayoriDev/adb.exe
SRC_MAIN 				= $(wildcard src/*.c) $(wildcard src/lib/*.c) $(wildcard src/wd_mod/*.c) $(wildcard src/apps/*.c) $(wildcard src/gui/*.c)


all:
	$(MAKE) build_android

build_android:
	$(ANDROID_CROSS_CC) --sysroot=$(ANDROID_SYSROOT_DIR) -std=c99 -I $(ANDROID_PLAT_DIR)include/ -L $(ANDROID_PLAT_DIR)lib/ -fPIE -static -o $(ANDROID_OUTPUT_NAME) $(SRC_MAIN) -D__ARM -O0 -w $(ANDROID_LINK_LIBS)
	python3 tools/fix_tls_alignment.py $(ANDROID_OUTPUT_NAME)