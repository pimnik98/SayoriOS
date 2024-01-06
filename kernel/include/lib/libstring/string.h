//
// Created by ndraey on 04.10.23.
//

#pragma once

#include "common.h"
#include "../../../src/lib/libvector/include/vector.h"

typedef struct string {
	char* data;

	size_t length;
} string_t;

#define ADDR2STRING(addr) ((string_t*)(addr))

string_t* string_new();
void string_append_charptr(string_t* string, const char* concatenable);
void string_append(string_t* string, const string_t* concatenable);
void string_crop(string_t* string, size_t start, size_t end);
void string_append_char(string_t* string, char ch);
string_t* string_from_charptr(const char* chars);
string_t* string_from_sized_charptr(const char* chars, size_t length);
vector_t* string_split(string_t* string, const char* delimiter);
void string_split_free(vector_t* vec);
string_t* string_clone(string_t* str);
void string_reverse(string_t* str);
string_t* string_from_integer(int number);
void string_destroy(string_t *string);
