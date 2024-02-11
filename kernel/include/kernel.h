#pragma		once

#include <version.h>

#include <stdarg.h>

#if USE_SSE
#include <emmintrin.h>
#endif

#include "common.h"
#include "config.h"

#include "lib/string.h"
#include "lib/stdlib.h"
#include "lib/sprintf.h"
#include "lib/stdio.h"
#include "lib/split.h"
#include "lib/math.h"
#include "lib/setjmp.h"
#include "lib/fileio.h"

#include "mem/pmm.h"
#include "mem/vmm.h"

#include "sys/acpi.h"
#include "sys/timer.h"
#include "sys/scheduler.h"
#include "sys/cpu_isr.h"
#include "sys/bootscreen.h"
#include "sys/logo.h"
#include "sys/descriptor_tables.h"
#include "sys/syscalls.h"
#include "sys/v8086.h"
#include "sys/system.h"

#include <io/ports.h>
#include <io/serial_port.h>
#include <io/screen.h>
#include <io/status_loggers.h>
#include <io/tty.h>

#include <drv/vfs_new.h>
#include <drv/input/keyboard.h>
#include <drv/input/mouse.h>

#include <drv/disk/ahci.h>
#include <drv/disk/dpm.h>
#include <drv/disk/ata_dma.h>

#include <drv/cmos.h>
#include <drv/pci.h>
#include <drv/beeper.h>
#include <drv/psf.h>
#include "drv/disk/ata.h"
#include <drv/atapi.h>
#include <drv/rtl8139.h>

#include <fs/fsm.h>

#include <fs/milla.h>
#include <fs/tarfs.h>
#include <fs/fat32.h>
#include <fs/iso9660.h>
#include <fs/nvfs.h>
#include <fs/tempfs.h>

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

#include "sys/cputemp.h"