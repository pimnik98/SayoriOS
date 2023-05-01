#pragma once
#include	"kernel.h"

typedef char* (*comReadChar_type_t)(uint64_t);


typedef struct com_control
{
   comReadChar_type_t readChar;
} com_control_t;

