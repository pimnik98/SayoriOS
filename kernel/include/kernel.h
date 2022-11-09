#pragma		once

#define VERSION_MAJOR   0       // Версия ядра
#define VERSION_MINOR   3       // Пре-релиз
#define VERSION_PATCH   0       // Патч
#define ARCH_TYPE       "i386"   // Архитектура

#include	"common.h"
#include	"drv/text_framebuffer.h"
#include	"sys/timer.h"
#include	"sys/memory.h"
#include	"sys/scheduler.h"
#include	"sys/cpu_isr.h"
#include <io/ports.h>
#include <drv/vfs_new.h>
#include <drv/input/keyboard.h>
#include <drv/cmos.h>
#include <fs/sefs.h>
