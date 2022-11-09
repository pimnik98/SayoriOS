/**
 * @file fs/sefs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система SEFS (Sayori Easy File System)
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>
#include <io/ports.h>
#include <drv/vfs_new.h>
#include <fs/sefs.h>

sefs_header_t *sefs_header;			///< ...
sefs_file_header_t *file_headers;	///< ...
fs_node_t *sefs_root;				///< ...
fs_node_t *sefs_dev;				///< ...
fs_node_t *root_nodes;				///< ...
int nroot_nodes;                    ///< Количество файлов.
struct dirent dirent;				///< ...
int64_t dirName[2048];              ///< Ссылка на названия папок по индексу
int64_t diskUsed = 0;               ///< Количество используемого пространства
int64_t diskSize = 0;               ///< Общие кол-во дисков
int64_t dirCount = 0;               ///< Количество папок

/**
 * @brief [SEFS] Полное чтение файла
 *
 * @param int node - Индекс файла
 * 
 * @return char* - Содержимое файла
 */
char* sefs_readChar(uint32_t node){
    sefs_file_header_t header = file_headers[node];
    //qemu_log("[SEFS] [readChar] Elem: %d",node);
    char* buf = kmalloc(header.length);
    memcpy(buf, header.offset, header.length);
    return buf;
}

/**
 * @brief [SEFS] Чтение файла
 *
 * @param int node - Индекс файла
 * @param int offset - С какой позиции читать файл
 * @param int size - Длина читаемого файла
 * @param void* buf - Буфер
 * 
 * @return uint32_t - Размер файла или отрицательное значение при ошибке
 */
uint32_t sefs_read(uint32_t node, size_t offset, size_t size, void *buffer){
    sefs_file_header_t header = file_headers[node];
    //qemu_log("[SEFS] [Read] Elem: %d | Off: %d | Size: %d",node,offset,size);
    if (offset > size){
        return -2;
    } else if (header.length < size){
        size = header.length;
    }
    memcpy(buffer, header.offset+offset, size);
    return size;
}

/**
 * @brief [SEFS] запись в файл
 *
 * @param int node - Индекс файла
 * @param int offset - С какой позиции писать файл
 * @param int size - Сколько пишем
 * @param void* buf - Буфер
 *
 * @return uint32_t - Размер записаных байтов или отрицательное значение при ошибке
 */
uint32_t sefs_write(uint32_t node, size_t offset, size_t size, void *buffer){
    sefs_file_header_t header = file_headers[node];
    //qemu_log("[SEFS] [Write] Elem: %d | Off: %d | Size: %d",node,offset,size);
    if (offset > size){
        return -2;
    }
    void* newfile = kmalloc((size + offset > header.length)?(size + offset):header.length);
    int w_tmp1 = 0;     ///< Сколько скопировано с начала
    int w_tmp2 = 0;     ///< Сколько вставлено с буфера
    int w_tmp3 = 0;     ///< Сколько вставлено с остатка
    // Сохраняем первую часть данных
    if (offset > 0){
        void* tmp1 = kmalloc(offset);
        w_tmp1 = sefs_read(node,0,offset,tmp1);
        memcpy(newfile, tmp1, offset);
    }
    // Сохраняем буфер
    strcat(newfile, buffer);
    // если остались остатки, то копируем
    if ((size + offset < header.length)){
        w_tmp3 = header.length - (size + offset);   // Откуда копируем
        void* tmp3 = kmalloc(offset);
        w_tmp3 = sefs_read(node,0,w_tmp3,tmp3);
        strcat(newfile, tmp3);
    }
    return w_tmp1+size+w_tmp3;
}

/**
 * @brief [SEFS] Получить размер файла (поиск по индексу)
 *
 * @param int node - Индекс файла
 * 
 * @return uint64_t - Размер файла или 0
 */
uint64_t sefs_getLengthFile(int node){
    //qemu_log("[SEFS] [gLF] Node: %d | Size: %d",node,root_nodes[node].length);

    return root_nodes[node].length;
}

/**
 * @brief [SEFS] Получить отступ в файловой системе у файла
 *
 * @param int node - Индекс файла
 * 
 * @return int - Позиция файла или отрицательное значение при ошибке
 */
int sefs_getOffsetFile(int node){
    return file_headers[node].offset;
}

/**
 * @brief [SEFS] Поиск файла на устройстве
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return int - Индекс файла, или отрицательное значение при ошибке
 */
uint32_t sefs_findFile(char* filename){
    char* file = kmalloc(sizeof(char)*256);
    char* sl = "/";
    strcpy(file, sl);
    strcat(file,filename);

    //qemu_log("[SeFS] `%s` | `%s`",filename,file);
    for (size_t i = 0; i < sefs_header->nfiles; i++){
        if (strcmpn(root_nodes[i].path,file)){
            kfree(file);
            return i;
        }
    }
    kfree(file);
    return -1;
}

/**
 * @brief [SEFS] Поиск папки на устройстве
 *
 * @param char* filename - Путь к папке (виртуальный)
 *
 * @return int - Индекс папки, или отрицательное значение при ошибке
 */
uint32_t sefs_findDir(char* path){
    char* file = kmalloc(sizeof(char)*256);
    char* sl = "/";
    strcpy(file, sl);
    strcat(file,path);

    qemu_log("[SeFS] `%s` | `%s`",path,file);

    for (size_t i = 0, a = 0; i < sefs_header->nfiles; i++){
        if (root_nodes[i].flags != FS_DIRECTORY) continue;
        if (strcmpn(root_nodes[i].name,file)){
            kfree(file);
            return a;
        }
        a++;
    }
    kfree(file);
    return -1;
}

/**
 * @brief [SEFS] Считает количество элементов в папке
 */
size_t sefs_countElemFolder(char* path){
    uint64_t inxDir = sefs_findDir(path);
    if (inxDir < 0){
        return 0;
    }
    size_t count = 0;
    for (size_t i = 0; i < sefs_header->nfiles; i++){
        if (root_nodes[i].root != inxDir){
            continue;
        }
        count++;
    }
    return count;
}

/**
 * @brief [SEFS] Выводит список файлов
 */
struct dirent* sefs_list(char* path){
    struct dirent* testFS;
    uint64_t inxDir = sefs_findDir(path);
    if (inxDir < 0){
        return testFS;
    }
    qemu_log("[Index Dir] %d",inxDir);
    uint64_t inxFile = 0;
    for (size_t i = 0; i < sefs_header->nfiles; i++){
        if (root_nodes[i].root != inxDir){
            continue;
        }
        testFS[inxFile].type = root_nodes[i].flags;
        testFS[inxFile].ino = i;
        testFS[inxFile].next = i+1;
        testFS[inxFile].length = root_nodes[i].length;
        strcpy(testFS[inxFile].name, root_nodes[i].name);
        inxFile++;
       qemu_log("[SEFS] [Init] I:%d",i);
       qemu_log("\t * Name:%s",root_nodes[i].name);
       qemu_log("\t * mask:%d",root_nodes[i].mask);
       qemu_log("\t * length:%d",root_nodes[i].length);
       qemu_log("\t * flags:%x",root_nodes[i].flags);
       qemu_log("\t * inode:%d",root_nodes[i].inode);
       qemu_log("\t * offset:%d",file_headers[i].offset);
       qemu_log("\t * root:%d",root_nodes[i].root);
   }
   testFS[inxFile].next = 0;
   return testFS;
}

/**
 * @brief [SEFS] Количество используемого места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество используемого места устройства
 */
uint64_t sefs_diskUsed(int node){
    return diskUsed;
}

/**
 * @brief [SEFS] Количество свободного места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество свободного места устройства
 */
uint64_t sefs_diskSpace(int node){
    return 0;
}

/**
 * @brief [SEFS] Количество всего места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество всего места устройства
 */
uint64_t sefs_diskSize(int node){
    return diskSize;
}

/**
 * @brief [SEFS] Получение имени устройства
 *
 * @param int node - Нода
 *
 * @return char* - Имя устройства
 */
char* sefs_getDevName(int node){
    return sefs_root->devName;
}

/**
 * @brief [SEFS] Инициализация Sayori Easy File System
 *
 * @param uint32_t location - Точка монтирования
 * 
 * @return fs_node_t - Структура с файлами
 */
fs_node_t *sefs_initrd(uint32_t location){
    qemu_log("[SEFS] [Init] loc: %x",location);
    // Инициализирует указатели main и заголовке файлов и заполняет корневой директорий.
    sefs_header = (sefs_header_t *)location;
    file_headers = (sefs_file_header_t *) (location+sizeof(sefs_header_t));
    sefs_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    strcpy(sefs_root->name, "/");
    strcpy(sefs_root->devName, "SayoriDisk RDv2");
    sefs_root->mask = sefs_root->uid = sefs_root->gid = sefs_root->inode = sefs_root->length = 0;
    sefs_root->flags = FS_DIRECTORY;
    sefs_root->open = 0;
    sefs_root->close = 0;
    sefs_root->findFile = &sefs_findFile;
    sefs_root->findDir = &sefs_findDir;
    sefs_root->getLengthFile = &sefs_getLengthFile;
    sefs_root->getOffsetFile = &sefs_getOffsetFile;
    sefs_root->list = &sefs_list;
    sefs_root->ptr = 0;
    sefs_root->impl = 0;
    sefs_root->readChar = &sefs_readChar;
    sefs_root->read = &sefs_read;
    sefs_root->write = &sefs_write;
    sefs_root->diskUsed = &sefs_diskUsed;
    sefs_root->diskSpace = &sefs_diskSpace;
    sefs_root->diskSize = &sefs_diskSize;
    sefs_root->getDevName = &sefs_getDevName;
    sefs_root->getCountElemFolder = &sefs_countElemFolder;
    sefs_root->getListElem = &sefs_list;

    root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * sefs_header->nfiles);
    nroot_nodes = sefs_header->nfiles;
    // Для каждого файла...
    for (int i = 0; i < sefs_header->nfiles; i++){
        // Отредактируем заголовок файла — в настоящее время в нем указывается смещение файла
        // относительно ramdisk. Мы хотим, чтобы оно указывалось относительно начала
        // памяти.
        root_nodes[i].root = file_headers[i].parentDir;
        file_headers[i].offset += location;
        // Создаем нод нового файла.
        strcpy(root_nodes[i].name, &file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = (uint64_t) file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = (file_headers[i].types==0?FS_FILE:FS_DIRECTORY);
        if (root_nodes[i].flags == FS_FILE){
            diskUsed += root_nodes[i].length;
            diskSize += root_nodes[i].length;
        }
        if (root_nodes[i].flags == FS_DIRECTORY){
            dirCount++;
        }
    }
    for (int i = 0; i < sefs_header->nfiles; i++){
        if (root_nodes[i].flags != FS_FILE) continue;
        strcpy(root_nodes[i].path, root_nodes[(sefs_header->nfiles - dirCount) + root_nodes[i].root].name);
        strcat(root_nodes[i].path,root_nodes[i].name);
        int fpath_len = strlen(root_nodes[i].path);
        root_nodes[i].path[fpath_len+1] = "\0";
    }
    return sefs_root;
}

