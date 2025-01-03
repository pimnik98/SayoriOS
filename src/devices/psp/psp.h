#pragma once

#include <stdint.h>
#define SCREEN_W        480
#define SCREEN_H        272

#define debug(M, ...) pspDebugScreenPrintf(M, ##__VA_ARGS__);    ///< При переходе в VBE нет смысла

