#pragma once

#define FS_FILE         0x00              ///< Файл
#define FS_DIRECTORY    0x01              ///< Папка
#define FS_INVALID      0x02              ///< Невалидный файл
#define FS_CHARDEVICE   0x03              ///< Символьное устройство (Пример: com,tty,speaker) (вид файла устройств обеспечивающий интерфейс к устройству с возможностью посимвольного обмена информацией.)
#define FS_BLOCKDEVICE  0x04              ///< Блочное устройство (вид файла устройств обеспечивающий интерфейс к устройству в виде файла в файловой системе)
#define FS_PIPE         0x05              ///< ???
#define FS_SYMLINK      0x06              ///< Ссылка
#define FS_MOUNTPOINT   0x08              ///< Точка монтирования

#define VFS_TYPE_MOUNT_SEFS     0x0000    ///< Sayori Easy File System
#define VFS_TYPE_MOUNT_NATSUKI  0xCAFE    ///< NatSuki File System

typedef char* (*readChar_type_t)(uint32_t);
typedef uint32_t (*read_type_t)(uint32_t,size_t,size_t,void *);
typedef struct dirent* (*list_type_t)();
typedef uint32_t (*write_type_t)(uint32_t,size_t,size_t,void *);
typedef uint32_t (*findFile_type_t)(char*);
typedef char* (*charData_type_t)(char*);
typedef char* (*charintData_type_t)(int);
typedef size_t (*getLengthFile_type_t)(int);
typedef int (*getOffsetFile_type_t)(int);
typedef size_t (*getDeviceSize_type_t)(int);
typedef struct dirent* (*dirlist_type_t)(char*);
typedef struct fs_node (*getFileInfo_type_t)(struct fs_node*,char*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);

typedef struct dirent * (*readdir_type_t)(struct fs_node*,uint32_t);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*,char *name);

typedef void (*dirfree_type_t)(struct dirent*);

typedef struct fs_node
{
   char name[128];                        ///< Имя файла.
   char path[512];                        ///< Полный путь
   uint32_t root;                         ///< Индекс папки
   uint32_t mask;                         ///< Маска прав доступа.
   uint32_t uid;                          ///< Пользователь, владеющий файлом.
   uint32_t gid;                          ///< Группа, владеющая файлом.
   uint32_t flags;                        ///< Включает тип нода. Смотрите определение #defines, приведенное выше.
   uint32_t inode;                        ///< Зависит от устройства, позволяет файловой системе идентифицировать файлы.
   uint32_t length;                       ///< Размер файла в байтах.
   uint32_t impl;                         ///< Номер, зависящий от реализации.
   readChar_type_t readChar;              ///< Функция FS - Полное чтение файла
   read_type_t read;                      ///< Функция FS - Чтение файла с указанием параметров
   write_type_t write;                    ///< Функция FS - Запись в файл
   open_type_t open;                      ///< Функция FS - Функция для открытия файла (не исп)
   close_type_t close;                    ///< Функция FS - Функция для закрытия файла (не исп)
   readdir_type_t readDir;                ///< Функция FS - Функция для чтение папки (не исп)
   findFile_type_t getCountElemFolder;    ///< Функция FS - Функция для получения количества файлов в папке
   findFile_type_t findFile;              ///< Функция FS - Функция для поиска файла
   findFile_type_t findDir;               ///< Функция FS - Функция для поиска файла
   getFileInfo_type_t getFileInfo;        ///< Функция FS - Функция для получения информации о файле (не исп) (старое)
   getLengthFile_type_t getLengthFile;    ///< Функция FS - Функция для получения размера файла
   getLengthFile_type_t getOffsetFile;    ///< Функция FS - Функция для получения позиции файла (отступ)
   dirlist_type_t getListElem;            ///< Функция FS - Функция для получения списка файлов
   dirfree_type_t unlistElem;
   charintData_type_t getDevName;            ///< Функция для получения имени устройства
   char devName[512];                     ///< Имя устройства
   getDeviceSize_type_t diskUsed;         ///< Сколько использовано места
   getDeviceSize_type_t diskSpace;        ///< Сколько свободно места
   getDeviceSize_type_t diskSize;         ///< Размер диска
   list_type_t list;                      ///< Список
   struct fs_node *ptr;                   ///< Используется для точек монтирования и символических ссылок.
} fs_node_t;

/**
 * @brief Согласно POSIX, один из них возвращается вызовом readdir.
 */
struct dirent
{
  char name[128];                       ///< Имя файла
  uint32_t ino;                         ///< Номер inode. Требеся для POSIX.
  uint8_t type;                         ///< Тип файла
  uint8_t next;                         ///< Следующая позиция
  size_t length;                      ///< Размер файла
};

extern fs_node_t *fs_root; // The root of the filesystem.

// Стандартные функции чтения, записи, открытия, закрытия. Обратите внимание,
//  что у них всех используется суффикс _fs с тем, чтобы отличать от функций чтения,
// записи, открытия и закрытия дескрипторов файлов, а не нодов файлов.
// uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
// uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
// void open_fs(fs_node_t *node, uint8_t read, uint8_t write);
// void close_fs(fs_node_t *node);
// struct dirent *readdir_fs(fs_node_t *node, uint32_t index);
// fs_node_t *finddir_fs(fs_node_t *node, char *name);

void vfs_getPath(int node, const char* path, char* buf);
int vfs_foundMount(const char* path);
void vfs_reg(size_t location, size_t type);
int vfs_write(int node, int elem, size_t offset, size_t size, void *buf);
int vfs_findFile(const char* filename);
bool vfs_exists(const char* filename);
uint32_t vfs_read(int node, int elem, size_t offset, size_t size, void *buf);
char* vfs_readChar(int node,int elem);
size_t vfs_findDir(char* path);
size_t vfs_getLengthFilePath(const char* filename);
size_t vfs_getLengthFile(int node,int elem);
int vfs_getOffsetFile(int node,int elem);
size_t vfs_getDiskSize(int node);
size_t vfs_getDiskSpace(int node);
size_t vfs_getDiskUsed(int node);
char* vfs_getName(int node);
size_t vfs_getCountElemDir(char* path);
struct dirent* vfs_getListFolder(char* path);
void vfs_createFile();
void vfs_createDir();
void vfs_list();
void vfs_deleteFile();
void vfs_getType();
void vfs_getMountPoint();
