#pragma once
/**
 * @brief Структура кол-ва файлов
 */
typedef struct {
   uint32_t nfiles; // Число файлов в ramdisk.
} sefs_header_t;

/**
 * @brief Структура файлов
 */
typedef struct {
   uint32_t index;             ///< Индекс
   uint32_t magic;            ///< Магическое число для проверки ошибок.
   char name[128];            ///< Имя файла
   uint32_t offset;           ///< Смещение в sefs, указывающее откуда начинается файл.
   uint32_t length;           ///< Длина файла
   uint8_t types;             ///< Тип (0 - файл/1 - Папка)
   uint8_t parentDir;         ///< Родительская папка (0 - значит root)
} sefs_file_header_t;

fs_node_t *sefs_initrd(uint32_t location, uint32_t end);

