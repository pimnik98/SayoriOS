#pragma once

#include "common.h"

typedef struct Coordinates {
    ssize_t x, y;
} Coordinates_t;

bool point_in_rect(ssize_t px, ssize_t py, ssize_t rx, ssize_t ry, ssize_t rw, ssize_t rh);