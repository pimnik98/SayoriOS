#pragma once

#include "common.h"

typedef struct stackframe {
  struct stackframe* ebp;
  uint32_t eip;
} stackframe;

void unwind_stack(uint32_t MaxFrames);
