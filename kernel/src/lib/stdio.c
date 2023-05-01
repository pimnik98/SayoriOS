/**
 * @file lib/stdio.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функции для работы с файлами
 * @version 0.3.2
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <lib/stdio.h>
/**
 * @brief Получение режима работы (маски файла)
 *
 * @param const char* restrict mode - Режии
 *
 * @return uint32_t - Режим работы (маска)
 */
uint32_t fmodecheck(const char* restrict mode){
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
 * @param FILE* stream - Поток (файл)
 */
void fcheckerror(FILE* stream){
	if (!vfs_exists(stream->path)){
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
 * @param FILE* stream - Поток (файл)
 *
 * @return Если возращает 0, значит все в порядке
 */
uint32_t ferror(FILE* stream){
	return stream->err;
}

/**
 * @brief Выводит на экран ошибку с пользовательским сообщением
 *
 * @param FILE* stream - Поток (файл)
 * @param char* s - Пользовательская строка
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
 * @brief Открыть файл для работы с ним
 *
 * @param char* filename - Путь к файлу
 * @param char* mode - Режим работы
 *
 * @return FILE* - структура
 */
FILE* fopen(const char* filename, const char* _mode){
	qemu_log("Open file: %s (%x) with mode %s (%x)", filename, _mode);

	FILE* file = kmalloc(sizeof(FILE));
	// Получаем тип открытого файла
	int32_t freal_mode = fmodecheck(_mode);
	if (!vfs_exists(filename) || freal_mode == 0) {
		qemu_log("Failed to open file: %s (Exists: %d; FMODE: %d)",
			filename,
			vfs_exists(filename),
			freal_mode);
		return 0;
	}

	file->open = 1;										// Файл успешно открыт
	file->fmode = freal_mode;								// Режим работы с файлом
	file->size = vfs_getLengthFilePath(filename);		// Размер файла
	file->path = (char*)filename;						// Полный путь к файлу
	file->pos = 0;										// Установка указателя в самое начало
	file->err = 0;										// Ошибок в работе нет
	
	return file;
}

/**
 * @brief Закончить работу с файлом
 *
 * @param FILE* stream - Поток (файл)
 */
void fclose(FILE* stream){
	if(stream)
		kfree(stream);
}

/**
 * @brief Получение размера файла в байтах
 *
 * @param FILE* stream - Поток (файл)
 *
 * @return Размер файла в противном случаи -1
 */
ssize_t fsize(FILE* stream){
	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		return -1;
	} else {
		return stream->size;
	}
}

/**
 * @brief Получение содержимого файла (детальная настройка)
 *
 * @param FILE* stream - Поток (файл)
 * @param size_t count - Количество элементов размера size
 * @param size_t size - Сколько читаем таких элементов?
 * @param void* buffer - Буфер
 *
 * @return int - Размер прочитаных байтов или -1 при ошибке
 */
int fread_c(FILE* stream, size_t count, size_t size, void* buffer){
	if (!stream->open || !vfs_exists(stream->path) || stream->size <= 0 || stream->fmode == 0){
		// Удалось ли открыть файл, существует ли файл, размер файла больше нуля и указан правильный режим для работы с файлом
		fcheckerror(stream);
		return -1;
	}

	qemu_log("Params: count=%d, size=%d, toread=%d, seek=%d", count, size, count*size, stream->pos);
	
	size_t node = vfs_foundMount(stream->path);
    int elem = vfs_findFile(stream->path);

	ssize_t res = vfs_read(node, elem, stream->pos, size*count, buffer);

	if(res > 0)
		stream->pos += size*count;
	
	return res;
}

/**
 * @brief Текущая позиция считывания в файле
 *
 * @param FILE* stream - Поток (файл)
 *
 * @return Возращает позицию или отрицательное значение при ошибке
 */
ssize_t ftell(FILE* stream) {
	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		return -1;
	}
	return stream->pos;
}

/**
 * @brief Установка позиции в потоке данных относительно текущей позиции
 *
 * @param FILE* stream - Поток (файл)
 * @param size_t offset - Смещение позиции
 * @param uint32_t whence - Точка отсчета смещения
 *
 * @return Если возращает 0, значит все в порядке
 */
ssize_t fseek(FILE* stream, ssize_t offset, uint8_t whence){
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
 * @brief Установка позиции в потоке данных
 *
 * @param FILE* stream - Поток (файл)
 * @param size_t offset - Смещение позиции
 */
void fsetpos(FILE* stream, ssize_t pos){
	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		stream->pos = 0;
	} else if (pos > 0 && stream->size >= pos){
		stream->pos = pos;
	}
	stream->pos = 0;
}

/**
 * @brief Установка позиции потока в самое начало
 *
 * @param FILE* stream - Поток (файл)
 */
void rewind(FILE* stream){
	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
	}
	stream->pos = 0;
}

/**
 * @brief Получение позиции в потоке данных
 *
 * @param FILE* stream - Поток (файл)
 *
 * @return Возращает позицию или отрицательное значение при ошибке
 */
ssize_t fgetpos(FILE* stream){
	if (!stream->open || stream->size <= 0 || stream->fmode == 0){
		fcheckerror(stream);
		return -1;
	}
	return stream->pos;
}
/**
 * @brief Печатает на экран откладочную информацию
 *
 * @param FILE* stream - Поток (файл)
 */
void fdebuginfo(FILE* stream){
	tty_printf("[fDebugInfo] Path: %s\n\tIsOpen: %d\n\tMode: %d\n\tSize: %d\n\tBuffer: %d\n\tPosition: %d\n\tError code: %d\n",stream->path,stream->open,stream->fmode,stream->size, stream->pos,stream->err);
}

/**
 * @brief Установка позиции в потоке данных
 *
 * @return Количество записаных байт или отрицательное число при ошибке
 */
size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ){
	// Реализуем как будет готовы ATA-драйвера и FS
	return -1;
}


