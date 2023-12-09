#pragma		once

#define VERSION_MAJOR   0        // Версия ядра
#define VERSION_MINOR   3        // Пре-релиз
#define VERSION_PATCH   3        // Патч
#define ARCH_TYPE       "i386"   // Архитектура
#define VERNAME         "Soul"   // Имя версии (изменяется вместе с минорной части версии)
#define SUBVERSIONNAME  "Flight" // Вторичное имя версии (изменяется вместе с каждым глобальным релизом)

#include <stdarg.h>

#if USE_SSE
#include <emmintrin.h>
#endif

#include "common.h"
#include "config.h"
#include "rust_header.h"

#include "lib/string.h"
#include "lib/stdlib.h"
#include "lib/sprintf.h"
#include "lib/stdio.h"
#include "lib/split.h"
#include "lib/math.h"
#include "lib/setjmp.h"
#include "lib/dan.h"

#include "sys/acpi.h"
#include "sys/timer.h"
#include "sys/memory.h"
#include "sys/scheduler.h"
#include "sys/cpu_isr.h"
#include "sys/bootscreen.h"
#include "sys/logo.h"
#include "sys/descriptor_tables.h"
#include "sys/syscalls.h"
#include "sys/v8086.h"
#include "sys/system.h"

#include <io/duke_image.h>
#include <io/ports.h>
#include <io/port_io.h>
#include <io/screen.h>
#include <io/status_loggers.h>
#include <io/tty.h>

#include <drv/vfs_new.h>
#include <drv/input/keyboard.h>
#include <drv/input/mouse.h>
#include <drv/cmos.h>
#include <drv/sb16.h>
#include <drv/pci.h>
#include <drv/beeper.h>
#include <drv/psf.h>
#include <drv/ata.h>
#include <drv/atapi.h>
#include <drv/rtl8139.h>

#include <fs/sefs.h>
#include <fs/milla.h>
#include <fs/lucario/fs.h>

#include <gui/pointutils.h>
#include <gui/line.h>
#include <gui/circle.h>

#include "elf/elf.h"

#include "net/endianess.h"
#include "net/cards.h"
#include "net/ethernet.h"
#include "net/arp.h"

#include "debug/hexview.h"
#include "debug/ubsan.h"