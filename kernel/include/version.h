#pragma once

#define VERSION_MAJOR   0        /// Версия ядра
#define VERSION_MINOR   3        /// Пре-релиз
#define VERSION_PATCH   4        /// Патч
#ifdef SAYORI64
#define ARCH_TYPE       "x86_64"   /// Архитектура
#else
#define ARCH_TYPE       "i386"   /// Архитектура
#endif
#define VERNAME         "Soul"   /// Имя версии (изменяется вместе с минорной части версии)
#define SUBVERSIONNAME  "Scythe" /// Вторичное имя версии (изменяется вместе с каждым глобальным релизом)
