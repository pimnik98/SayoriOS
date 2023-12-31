//
// Created by ndraey on 04.10.23.
//

#include "portability.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "lib/stdio.h"
#include "../include/string.h"

string_t* string_new() {
	string_t* string = calloc(sizeof *string, 1);

	if(!string)
		return 0;

	string->data = calloc(1, 1);
	string->length = 0;

	return string;
}

void string_append_charptr(string_t* string, const char* concatenable) {
	if(!string || !concatenable)
		return;

	size_t len = strlen(concatenable);

	size_t alloc_len = len + string->length;

	char* memory = (char*)realloc(string->data, alloc_len + 1);

	if(!memory)
		return;

	memory[alloc_len] = 0;

	string->data = memory;

	if(string->data[0] == 0) {
		strcpy(string->data, concatenable);
	} else {
		strcat(string->data, concatenable);
	}

	string->length = alloc_len;
}

void string_append(string_t* string, const string_t* concatenable) {
	if(!string || !concatenable)
		return;

	string_append_charptr(string, concatenable->data);
}

void string_crop(string_t* string, size_t start, size_t end) {
	if(!string || start >= end || end > string->length)
		return;

	size_t new_length = end - start;

	char* data = malloc(new_length + 1);

	if(!data)
		return;

	data[new_length] = 0;

	memcpy(data, string->data + start, new_length);

	free(string->data);

	string->data = data;
	string->length = new_length;
}

void string_append_char(string_t* string, char ch) {
	if(!string)
		return;

	char* data = realloc(string->data, string->length + 2);

	if(!data)
		return;

	data[string->length] = ch;
	data[string->length + 1] = 0;

	string->length++;

	string->data = data;
}

string_t* string_from_charptr(const char* chars) {
	string_t* string = calloc(sizeof *string, 1);

	if(!string)
		return 0;

	string->length = strlen(chars);
	string->data = malloc(string->length + 1);

	memcpy(string->data, chars, string->length);
	string->data[string->length] = 0;

	return string;
}

string_t* string_from_sized_charptr(const char* chars, size_t length) {
	string_t* string = calloc(sizeof *string, 1);

	if(!string)
		return 0;

	string->data = malloc(length + 1);
	string->length = length;

	memcpy(string->data, chars, length);
	string->data[string->length] = 0;

	return string;
}

vector_t* string_split(string_t* string, const char* delimiter) {
	if(!string || !delimiter)
		return 0;

	vector_t* vec = vector_new();

	if(!vec)
		return 0;

	char* curptr = string->data;
	size_t delim_len = strlen(delimiter);

	char* el = strstr(curptr, delimiter);

	if(!el) { // If no occurrences, just add whole string and return vector.
		string_t* str = string_new();

		string_append_charptr(str, curptr);

		vector_push_back(vec, (size_t)str);

		return vec;
	}

	while(el) {
		el = strstr(curptr, delimiter);
		size_t len = el - curptr;

		if(el == 0)
			len = strlen(curptr);

		string_t* substring = string_from_sized_charptr(curptr, len);

		vector_push_back(vec, (size_t)substring);

		curptr = el + delim_len;
	}

	return vec;
}

void string_split_free(vector_t* vec) {
	if(!vec)
		return;

	for(size_t i = 0; i < vec->size; i++) {
		string_destroy((string_t*)vec->data[i]);
	}

	vector_destroy(vec);
}

string_t* string_clone(string_t* str) {
	if(!str)
		return 0;

	string_t* str2 = string_new();

	if(!str2)
		return 0;

	string_append(str2, str);

	return str2;
}

void string_reverse(string_t *str) {
    if(!str)
        return;

    size_t len = str->length;

    for(size_t i = 0; i < len / 2; i++) {
        char tmp = str->data[i];
        str->data[i] = str->data[len - i - 1];
        str->data[len - i - 1] = tmp;
    }
}

string_t* string_from_integer(int number) {
    string_t* str = string_new();
    bool is_negative = false;

    if(!str)
        return 0;

    if(number == 0) {
        string_append_char(str, '0');

        return str;
    }

    if(number < 0) {
        number = -number;

        is_negative = true;
    }

    while(number > 0) {
        string_append_char(str, (char)((number % 10) + '0'));
        number /= 10;
    }

    if(is_negative) {
        string_append_char(str, '-');
    }

    string_reverse(str);

    return str;
}

void string_destroy(string_t *string) {
	if(string->data)
		free(string->data);

	free(string);
}