#pragma		once

#define VERSION_MAJOR   0       // Версия ядра
#define VERSION_MINOR   3       // Пре-релиз
#define VERSION_PATCH   2       // Патч
#define ARCH_TYPE       "i386"  // Архитектура
#define VERNAME "Soul"         // Имя версии

#include "common.h"

#include "lib/string.h"
#include "lib/stdio.h"
#include "/lib/stdlib.h"
#include "lib/split.h"

#include "sys/timer.h"
#include "sys/memory.h"
#include "sys/scheduler.h"
#include "sys/cpu_isr.h"
#include "sys/bootscreen.h"
#include "sys/logo.h"
#include "sys/system.h"

#include <io/imaging.h>
#include <io/ports.h>
#include <io/screen.h>
#include <io/tty.h>

#include <drv/vfs_new.h>
#include <drv/input/keyboard.h>
#include <drv/input/mouse.h>
#include <drv/cmos.h>
#include <drv/sb16.h>
#include <drv/pci.h>
#include <drv/psf.h>
#include <drv/ata.h>

#include <fs/sefs.h>
#include <fs/milla.h>
#include <fs/lucario/fs.h>

#include "elf/elf.h"
