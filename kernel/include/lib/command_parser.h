#pragma once

#include "common.h"

typedef struct {
	char* original_string;
	int argc;
	char** argv;
} command_parser_t;

void command_parser_new(command_parser_t* parser, const char* _s);
void command_parser_destroy(command_parser_t* parser);

