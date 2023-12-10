#pragma once

#include "common.h"

#define EOF (-1)
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIO_ERR_NO_FOUND 		1	// Файл не найден
#define STDIO_ERR_MODE_ERROR	2	// Режим работы не определён
#define STDIO_ERR_SIZE			3	// Размер файла имеет недопустимый размер
#define STDIO_ERR_NO_OPEN		4	// Файл не был открыт


/**
 * @brief Структура файла. Требуется для работы с VFS
 * 
 */
typedef struct FILE {
	char* path;
    int32_t size;
    uint32_t fmode;
	bool open;
	size_t pos;
	uint32_t err;
} FILE;

// Типы открытого файла, тип флагов rw и т.д.
enum FileOpenMode {
	O_READ = 1,
	O_WRITE = 2,
	O_CREATE = 4,
	O_APPEND = 8,
	O_TRUNC = 16,
};

FILE* fopen(const char* filename, const char* mode);
void fclose(FILE *stream);
int32_t fread(FILE* stream, size_t count, size_t size, void* buffer);
int ftell(FILE* stream);
int fsize(FILE *stream);
ssize_t fseek(FILE* stream, ssize_t offset, uint8_t whence);
void rewind(FILE *stream);
void perror(FILE* stream,char* s);
uint32_t ferror(FILE* stream);
void fsetpos(FILE* stream, ssize_t pos);
ssize_t fgetpos(FILE* stream);
void fdebuginfo(FILE* stream);

//size_t fread(FILE* stream, char* ptr, size_t size, size_t nmemb);
size_t fwrite(FILE *stream, size_t size, size_t count, const void *ptr);
