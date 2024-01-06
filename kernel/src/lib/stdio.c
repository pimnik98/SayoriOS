/**
 * @file lib/stdio.c
 * @authors Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Функции для работы с файлами
 * @version 0.3.4
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include "io/ports.h"
#include "mem/vmm.h"
#include <lib/stdio.h>
#include <fs/fsm.h>
#include <fs/nvfs.h>
#include <io/tty.h>

bool stdio_debug = false;


/**
 * @brief Получение режима работы (маски файла)
 *
 * @param mode - Режии
 *
 * @todo Оптимизировать работу с битовыми флагами
 * @return Режим работы (маска)
 */
uint32_t fmodecheck(const char* mode){
	ON_NULLPTR(mode, {
		qemu_log("Mode is nullptr!");
		return 0;
	});

	uint32_t fmode = 0;
	if (mode[0] == 'w') {
		if (mode[1] == 'x') { // wx
			fmode = O_WRITE | O_CREATE;
		} else if (mode[1] == 'b') {
			if (mode[2] == 'x') { // wbx
				fmode = O_WRITE | O_CREATE;
			} else if (mode[2] == '+') {
				if (mode[3] == 'x') { // wb+x
					fmode = O_WRITE | O_READ | O_CREATE;
				} else { // wb+
					fmode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
				}
			} else { // wb
				fmode = O_WRITE | O_CREATE | O_TRUNC;
			}
		} else if (mode[1] == '+') {
			if (mode[2] == 'x') { // w+x
				fmode = O_WRITE | O_READ | O_CREATE;
			} else if (mode[2] == 'b') {
				if (mode[3] == 'x') { // w+bx
					fmode = O_WRITE | O_READ | O_CREATE;
				} else { // w+b
					fmode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
				}
			} else { // w+
				fmode = O_WRITE | O_READ | O_CREATE | O_TRUNC;
			}
		} else { // w
			fmode = O_WRITE | O_CREATE | O_TRUNC;
		}
	} else if (mode[0] == 'r') {
		if (mode[1] == 'b') {
			if (mode[2] == '+') { // rb+
				fmode = O_READ | O_WRITE;
			} else { // rb
				fmode = O_READ;
			}
		} else if (mode[1] == '+') { // r+ r+b
			fmode = O_READ | O_WRITE;
		} else { // r
			fmode = O_READ;
		}
	} else if (mode[0] == 'a') {
		if (mode[1] == 'b') {
			if (mode[2] == '+') { // ab+
				fmode = O_WRITE | O_READ | O_APPEND | O_CREATE;
			} else { // ab
				fmode = O_WRITE | O_APPEND | O_CREATE;
			}
		} else if (mode[1] == '+') { // a+ a+b
			fmode = O_WRITE | O_READ | O_APPEND | O_CREATE;
		} else { // a
			fmode = O_WRITE | O_APPEND | O_CREATE;
		}
	}
	return fmode;
}

/**
 * @brief Проверка файла на наличие ошибок при работе
 *
 * @param stream Поток (файл)
 */
void fcheckerror(FILE* stream){
	ON_NULLPTR(stream, {
		qemu_log("stream is nullptr!");
		return;
	});
	
	FSM_FILE finfo = nvfs_info(stream->path);
	if (finfo.Ready == 0){
		stream->err = STDIO_ERR_NO_FOUND;
	} else if (stream->fmode == 0){
		stream->err = STDIO_ERR_MODE_ERROR;
	} else if (stream->size <= 0){
		stream->err = STDIO_ERR_SIZE;
	} else if (stream->open == 0){
		stream->err = STDIO_ERR_NO_OPEN;
	}
}

/**
 * @brief Получение кода ошибки
 *
 * @param stream Поток (файл)
 *
 * @return Если возращает 0, значит все в порядке
 */
uint32_t ferror(FILE* stream){
	return stream->err;
}

/**
 * @brief Выводит на экран ошибку с пользовательским сообщением
 *
 * @param stream - Поток (файл)
 * @param s - Пользовательская строка
 */
void perror(FILE* stream,char* s){
	switch(stream->err){
		case STDIO_ERR_NO_FOUND:{
			tty_printf("%s: %s\n",s, "File no found");
			break;
		}
		case STDIO_ERR_MODE_ERROR:{
			tty_printf("%s: %s\n",s, "Unknown operating mode");
			break;
		}
		case STDIO_ERR_SIZE:{
			tty_printf("%s: %s\n",s, "The file size has a non-standard value.");
			break;
		}
		case STDIO_ERR_NO_OPEN:{
			tty_printf("%s: %s\n",s, "The file has not been opened for work.");
			break;
		}
		default: {
			tty_printf("%s: %s\n",s, "Unknown");
			break;
		}
	}
}

/**
 * @brief Открывает файл
 *
 * @param filename Путь к файлу
 * @param mode Режим работы
 *
 * @return Структура FILE*
 */
FILE* fopen(const char* filename, const char* _mode){
	ON_NULLPTR(filename, {
		qemu_log("Filename is nullptr!");
		return NULL;
	});

	ON_NULLPTR(_mode, {
		qemu_log("Mode is nullptr!");
		return NULL;
	});

    qemu_log("Open file");
    qemu_log("|- Name: '%s'", filename);
    qemu_log("|- Mode: '%s'", _mode);

	FILE* file = kcalloc(sizeof(FILE), 1);
	// Получаем тип открытого файла
	uint32_t freal_mode = fmodecheck(_mode);
	FSM_FILE finfo = nvfs_info(filename);
	if (finfo.Ready == 0 || freal_mode == 0) {
        //kfree(file);
        qemu_err("Failed to open file: %s (Exists: %d; FMODE: %d)",
			filename,
			finfo.Ready,
			freal_mode);
		return 0;
	}

	file->open = 1;										// Файл успешно открыт
	file->fmode = freal_mode;								// Режим работы с файлом
	file->size = finfo.Size;		// Размер файла
	file->path = (char*)filename;						// Полный путь к файлу
	file->pos = 0;										// Установка указателя в самое начало
	file->err = 0;										// Ошибок в работе нет

    qemu_ok("File opened!");
	return file;
}

/**
 * @brief Закончить работу с файлом
 *
 * @param stream Поток (файл)
 */
void fclose(FILE* stream){
	if(stream)
		kfree(stream);
}

/**
 * @brief Получение размера файла в байтах
 *
 * @param stream Поток (файл)
 *
 * @return Размер файла, в противном случае -1
 */
int fsize(FILE* stream){
	ON_NULLPTR(stream, {
		return -1;
	});

	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		return -1;
	} else {
		return stream->size;
	}
}

/**
 * @brief Чтение файла
 *
 * @param stream - Поток (файл)
 * @param count - Количество элементов размера size
 * @param size - Сколько читаем таких элементов?
 * @param buffer - Буфер
 *
 * @return Размер прочитаных байтов или -1 при ошибке
 */
int fread(FILE* stream, size_t count, size_t size, void* buffer){
	ON_NULLPTR(stream, {
		return -1;
	});

	ON_NULLPTR(buffer, {
		return -1;
	});

	FSM_FILE finfo = nvfs_info(stream->path);
	if (!stream->open || finfo.Ready == 0 || stream->size <= 0 || stream->fmode == 0){
		// Удалось ли открыть файл, существует ли файл, размер файла больше нуля и указан правильный режим для работы с файлом
		fcheckerror(stream);
		return -1;
	}

    if (stdio_debug) qemu_log("Params: count=%d, size=%d, toread=%d, seek=%d", count, size, count*size, stream->pos);

	size_t res = nvfs_read(stream->path, stream->pos, size*count, buffer);

// 	ssize_t res = vfs_read(node, elem, stream->pos, size*count, buffer);

	if(res > 0)
		stream->pos += size*count;
	
	return res;
}

/**
 * @brief Текущая позиция считывания в файле
 *
 * @param stream - Поток (файл)
 *
 * @return Возращает позицию или отрицательное значение при ошибке
 */
int ftell(FILE* stream) {
	ON_NULLPTR(stream, {
		return -1;
	});

	if (!stream->open
		|| stream->size <= 0
		|| stream->fmode == 0
	) {
		fcheckerror(stream);
		return -1;
	}

	return (int)stream->pos;
}

/**
 * @brief Установка позиции в потоке данных относительно текущей позиции
 *
 * @param stream - Поток (файл)
 * @param offset - Смещение позиции
 * @param whence - Точка отсчета смещения
 *
 * @return Если возращает 0, значит все в порядке
 */
ssize_t fseek(FILE* stream, ssize_t offset, uint8_t whence){
	ON_NULLPTR(stream, {
		return -1;
	});

	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		return -1;
	}
	size_t lsk = 0;
	if (whence == SEEK_CUR) {
		lsk = stream->pos;
	} else if (whence == SEEK_END) {
		lsk = stream->size;
	} else if (whence == SEEK_SET) {
		//lsk = 0;
		if(offset >= 0 && offset <= stream->size) {
			stream->pos = offset;
			return 0;
		}
	} else {
		return -1;
	}
	if (lsk+offset > 0 && stream->size >=lsk+offset){
		stream->pos = lsk+offset;
	}
	return 0;
}

/**
 * @brief Установка позиции потока в самое начало
 *
 * @param stream - Поток (файл)
 */
void rewind(FILE* stream){
	ON_NULLPTR(stream, {
		return;
	});

	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
	}
	stream->pos = 0;
}

/**
 * @brief Запись файла
 * @param stream Объект файла
 * @param size Размер в байтах
 * @param count Количество объектов размера 'size'
 * @param ptr Буфер
 * @return Количество записаных байт
 */
size_t fwrite(FILE *stream, size_t size, size_t count, const void *ptr) {
	ON_NULLPTR(stream, {
		qemu_log("stream is nullptr!");
		return 0;
	});
	
//	FSM_FILE finfo = nvfs_info(stream->path);
	
	size_t res = nvfs_write(stream->path, stream->pos, size*count, ptr);

	if(res > 0)
		stream->pos += size*count;

	return -1;
}


