#pragma once

#include "mem/vmm.h"
#include "lib/stdio.h"

#define		EI_NIDENT	16

/*-----------------------------------------------------------------------------
 *		Типы сейфы
 *---------------------------------------------------------------------------*/
#define		SHT_NULL		0
#define		SHT_PROGBITS	1
#define		SHT_SYMTAB		2
#define		SHT_STRTAB		3
#define		SHT_RELA		4
#define		SHT_HASH		5
#define		SHT_DYNAMIC		6
#define		SHT_NOTE		7
#define		SHT_NOBITS		8
#define		SHT_REL			9
#define		SHT_SHLIB		10
#define		SHT_DYNSYM		11
#define		SHT_LOPROC		0x70000000
#define		SHT_HIPROC		0x7FFFFFFF
#define		SHT_LOUSER		0x80000000
#define		SHT_HIUSER		0xFFFFFFFF
#define		SHF_WRITE		0x1
#define		SHF_ALLOC		0x2
#define		SHF_EXECINSTR	0x4
#define		SHF_MASKPROC	0xF0000000

/*-----------------------------------------------------------------------------
 *		Типы файлов
 *---------------------------------------------------------------------------*/
#define		ET_NONE			0			///< Нет типа файла
#define		ET_REL			1			///< Перемещаемый файл (Relocatable file)
#define		ET_EXEC			2			///< Исполняемый файл (Executable file)
#define		ET_DYN			3			///< Общий объектный файл (Shared object file)
#define		ET_CORE			4			///< Файлы ядра
#define		ET_LOPROC		0xFF00		///< Зависит от процессора
#define		ET_HIPROC		0xFFFF		///< Зависит от процессора

/*-----------------------------------------------------------------------------
 * 		Типы архитектур
 *---------------------------------------------------------------------------*/
#define		EM_NONE			0
#define		EM_M32			1
#define		EM_SPARC		2
#define		EM_386			3
#define		EM_68K			4
#define		EM_88K			5
#define		EM_860			7
#define		EM_MIPS			8

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
#define		EV_NONE			0
#define		EV_CURRENT		1

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
#define		PT_NULL			0
#define		PT_LOAD			1
#define		PT_DYNAMIC		2
#define		PT_INTERP		3
#define		PT_NOTE			4
#define		PT_SHLIB		5
#define		PT_PHDR			6
#define		PT_LOPROC		0x70000000
#define		PT_HIPROC		0x7FFFFFFF

/*-----------------------------------------------------------------------------
 * 		Типы данных
 *---------------------------------------------------------------------------*/
typedef		unsigned int		Elf32_Addr;
typedef		unsigned short int	Elf32_Half;
typedef		unsigned int		Elf32_Off;
typedef		int					Elf32_Sword;
typedef		unsigned int		Elf32_Word;

/*-----------------------------------------------------------------------------
 * 		Заголовки ELF
 *---------------------------------------------------------------------------*/
typedef	struct
{
	unsigned char	e_ident[EI_NIDENT];	///< Идентификационные данные ELF
	Elf32_Half		e_type;				///< Тип объектного файла
	Elf32_Half		e_mashine;			///< Тип архитектуры
	Elf32_Word		e_version;			///< Версия объектного файла
	Elf32_Addr		e_entry;			///< Точка входа в процесс
	Elf32_Off		e_phoff;			///< Смещение заголовка программы
	Elf32_Off		e_shoff;			///< Смещение заголовка таблицы разделов
	Elf32_Word		e_flags;			///< Флаги, специфичные для процессора
	Elf32_Half		e_ehsize;			///< Размер заголовка
	Elf32_Half		e_phentsize;		///< Размер записи заголовка программы
	Elf32_Half		e_phnum;			///< Количество записей заголовка программы
	Elf32_Half		e_shentsize;		///< Размер записи заголовка раздела
	Elf32_Half		e_shnum;			///< Количество записей в заголовке раздела
	Elf32_Half		e_shstrndx;			///< ...
} Elf32_Ehdr;

/*-----------------------------------------------------------------------------
 * 		Заголовки секций
 *---------------------------------------------------------------------------*/
typedef	struct
{
	Elf32_Word	sh_name;		///< Указатель в таблице перемешивания заголовка раздела
	Elf32_Word	sh_type;		///< Тип раздела
	Elf32_Word	sh_flags;		///< Флаги атрибутов раздела
	Elf32_Addr	sh_addr;		///< Виртуальный адрес раздела в образе процесса
	Elf32_Off	sh_offset;		///< Смещение раздела в файле
	Elf32_Word	sh_size;		///< Размер раздела
	Elf32_Word	sh_link;		///< ...
	Elf32_Word	sh_info;		///< Дополнительная информация
	Elf32_Word	sh_addralign;	///< Добавление адреса раздела
	Elf32_Word	sh_entsize;		///< ...

} Elf32_Shdr;

/*-----------------------------------------------------------------------------
 * 		Программный заголовок
 *---------------------------------------------------------------------------*/
typedef struct
{
	Elf32_Word	p_type;		///< Тип заголовка программы
	Elf32_Off	p_offset;	///< Смещение заголовка программы
	Elf32_Addr	p_vaddr;	///< Виртуальный адрес заголовка программы
	Elf32_Addr	p_paddr;	///< Физический адрес заголовка программы
	Elf32_Word	p_filesz;	///< Число байтов в сегменте файла
	Elf32_Word	p_memsz;	///< ...
	Elf32_Word	p_flags;	///< Флаги
	Elf32_Word	p_align;	///< Отступ
} Elf32_Phdr;

typedef struct elf_sections
{
	Elf32_Ehdr		elf_header;	///< ELF заголовок
	Elf32_Shdr*		section;	///< Секции
	Elf32_Phdr*		p_header;	///< Программный заголовок

	FILE*			file;		///< Ссылка на файл
} elf_t;

elf_t* load_elf(const char* name);
void unload_elf(elf_t* elf);
int32_t run_elf_file(const char *name, int argc, char* eargv[]);

static inline bool is_elf_file(FILE* fp) {
	char* temp = kmalloc(4);
	size_t orig = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	// 7F 45 4C 46

	fread(fp, 4, 1, temp);

	fseek(fp, orig, SEEK_SET);

	bool result = (temp[0] == 0x7F && temp[1] == 0x45 && temp[2] == 0x4C && temp[3] == 0x46);

	kfree(temp);

	return result;
}
