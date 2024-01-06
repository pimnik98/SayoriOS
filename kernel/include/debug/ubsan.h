//
// Created by ndraey on 23.09.23.
//

#pragma once

#include "common.h"

#define is_aligned(value, alignment) !((value) & ((alignment) - 1))

struct source_location {
	const char *file;
	uint32_t line;
	uint32_t column;
};

struct type_descriptor {
	uint16_t kind;
	uint16_t info;
	char name[];
};

struct type_mismatch_info {
	struct source_location location;
	struct type_descriptor *type;
	size_t alignment;
	uint8_t type_check_kind;
};

struct out_of_bounds_info {
	struct source_location location;
	struct type_descriptor left_type;
//	struct type_descriptor right_type;
};

static const char *Type_Check_Kinds[] = {
		"load of",
		"store to",
		"reference binding to",
		"member access within",
		"member call on",
		"constructor call on",
		"downcast of",
		"downcast of",
		"upcast of",
		"cast to virtual base of",
};