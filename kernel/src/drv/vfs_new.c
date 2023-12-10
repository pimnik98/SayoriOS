/**
 * @file drv/vfs_new.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Драйвер виртуальной файловой системы
 * @version 0.3.3
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <io/ports.h>
#include <drv/vfs_new.h>
#include <fs/milla.h>

fs_node_t *fs_root; 			///< Точка монтирования виртуального корня
fs_node_t *vfs_mount[128];		///< Точки монтирования
size_t countMount = 0;			///< Кол-во смонтированных устройств

/**
 * @brief [VFS] Получить реальный путь на устройстве
 *
 * @param int node - Адрес монтирования
 * @param char* path - Путь к файлу (виртуальный)
 */
void vfs_getPath(int node, const char* path, char* buf){
    // strcpy(buf, path);
    substr(buf, path, strlen(vfs_mount[node]->name), strlen(path));
    
    // qemu_log("Before: %s; After: %s; Path in node: %s", path, buf, vfs_mount[node]->name);
}

/**
 * @brief [VFS] Получить адрес монтирования у файла
 *
 * @param char* path - Путь к файлу (виртуальный)
 * 
 * @warning РАБОТАЕТ НЕМНОГО КОСТЫЛЬНО! ПЕРЕДЕЛАТЬ!!!!!
 *
 * @todo АХТУНГ
 *
 * @return int - Индекс, или отрицательное значение если точка монтирования не найдена
 */
int vfs_foundMount(const char* path){
    for (int i=countMount-1; i >= 0; i--){
        if (strlen(vfs_mount[i]->name) > strlen(path)){
            /// Нет смысла чекать, ибо длина монтирования, длинее имени пути.
            continue;
        }
        for(size_t a = 0; a < strlen(vfs_mount[i]->name);a++){
            if ((int) vfs_mount[i]->name[a] != (int) path[a]){
                break;
            }
            if(strlen(vfs_mount[i]->name)-1 == a){
                /// О, кажись совпало.
                return i;
            }
        }
    }
    // qemu_log("IM HERE BLYAT");
    return 0;
}

/**
 * @brief [VFS] Монтирование устройства
 *
 * @param int location - Адрес монтирования
 * @param int type - Тип файловой системы
 */
fs_node_t *NatSuki_initrd(uint32_t location);
bool isInitNatSuki();
void vfs_reg(size_t location, size_t end, size_t type){
    if (type == VFS_TYPE_MOUNT_SEFS){
        //qemu_log("[VFS] [REG] [%d] Sayori Easy File System | Location: %x",countMount,location);
        // vfs_mount[countMount] = (fs_node_t*) kmalloc(sizeof(fs_node_t*));
        vfs_mount[countMount++] = (fs_node_t*) sefs_initrd(location, end);
        // countMount++;
    } else if (type == VFS_TYPE_MOUNT_NATSUKI){
        qemu_log("[VFS] [REG] NatSuki %x | Location: %x", type, location);
        // vfs_mount[countMount] = (fs_node_t*) kmalloc(sizeof(fs_node_t*));
        vfs_mount[countMount] = (fs_node_t*)NatSuki_initrd(location);
        if (isInitNatSuki()){
            countMount++;
        }
    } else {
        qemu_log("[VFS] [REG] Unknown | Location: %x",location);
    }
}

/**
 * @brief [VFS] Запись в файл
 *
 * @param int node - Адрес монтирования
 * @param int elem - Индекс файла
 * @param size_t offset - С какой позиции читать файл
 * @param size_t size - Длина читаемого файла
 * @param void* buf - Что пишем
 *
 */
int vfs_write(int node, int elem, size_t offset, size_t size, const void *buf){
    if (vfs_mount[node]->write != 0){
        uint32_t ret = vfs_mount[node]->write(elem,offset,size,buf);
        return ret;
    } else {
        return -1;
    }
}

/**
 * @brief [VFS] Получить индекс файла, который находиться на устройстве
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return int - Индекс файла, или отрицательное значение при ошибке
 */
int vfs_findFile(const char* filename){
    //qemu_log("[VFS] [FindFile] File: `%s`",filename);
    int node = vfs_foundMount(filename);
    if (vfs_mount[node]->findFile) {
        char* path = (char*)kmalloc(sizeof(char)*strlen(filename));
        vfs_getPath(node, filename, path);
        uint32_t elem = vfs_mount[node]->findFile(path);

        kfree(path);
        //qemu_log("\n[VFS] Node: %d | Elem:%d \n",node,elem);
        return elem;
    }
    return -1;
}

/**
 * @brief [VFS] Проверить существует ли файл
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return bool - true если файл найден
 */
bool vfs_exists(const char* filename){
  if (vfs_findFile(filename) != -1){
    return true;
  }
  return false;

  // NDRAEY says:
  // return vfs_findFile(filename) != -1;
}

/**
 * @brief [VFS] Чтение файла
 *
 * @param int node - Адрес монтирования
 * @param int elem - Индекс файла
 * @param size_t offset - С какой позиции читать файл
 * @param size_t size - Длина читаемого файла
 * @param void* buf - Буфер
 * 
 * @return uint32_t - Размер файла или отрицательное значение при ошибке
 */
int vfs_read(int node, int elem, size_t offset, size_t size, void *buf){
    //qemu_log("[VFS] [Read] Node:%d | Elem: %d | Off: %d | Size: %d", node, elem, offset, size);
    if (vfs_mount[node]->read != 0){
        uint32_t ret = vfs_mount[node]->read(elem, offset, size, buf);
		// qemu_log("Buf now: %s", buf);
        return ret;
    }
    //qemu_log("[VFS] [Read] Error!!!");
    return -1;
}

/**
 * @brief [VFS] Полное чтение файла
 *
 * @param int node - Адрес монтирования
 * @param int elem - Индекс файла
 * 
 * @return char* - Содержимое файла
 */
char* vfs_readChar(int node,int elem){
    //qemu_log("[VFS] [readChar] Node:%d | Elem: %d",node,elem);
    if (vfs_mount[node]->readChar != 0){
        char* buffer = vfs_mount[node]->readChar(elem);
        return buffer;
    }
    return 0;
}

/**
 * @brief [VFS] Поиск папки
 *
 * @param char* filename - Путь к папке (виртуальный)
 *
 * @return uint32_t - Индекс папки, или отрицательное значение при ошибке
 */
uint32_t vfs_findDir(char* path){
    qemu_log("`%s`",path);

    int node = vfs_foundMount(path);

    if (vfs_mount[node]->findDir != 0){
        char* c_path = (char*)kmalloc(sizeof(char)*(strlen(path)+1));

        vfs_getPath(node, path, c_path);

		qemu_log("Got path: %s", c_path);

        uint32_t elem = vfs_mount[node]->findDir(c_path);

        kfree(c_path);
        return elem;
    }

    return -1;
}

/**
 * @brief [VFS] Получить длину файла (поиск по виртуальному пути)
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return size_t - Размер файла или отрицательное значение при ошибке
 */
ssize_t vfs_getLengthFilePath(const char* filename){
  int node = vfs_foundMount(filename);
  if (vfs_mount[node]->findFile != 0 && vfs_mount[node]->getLengthFile != 0){
        char* path = (char*)kmalloc(sizeof(char)*strlen(filename));
        vfs_getPath(node, filename, path);
        uint32_t elem = vfs_mount[node]->findFile(path);
        size_t size = vfs_mount[node]->getLengthFile(elem);

        kfree(path);
        //qemu_log("\n\n\t * [VFS] [gLFP] Node: %d | Elem: %d | Size: %d",node,elem,size);
        return size;
    }
    return -2;
}

/**
 * @brief [VFS] Получить размер файла (поиск по индексу)
 *
 * @param int node - Адрес монтирования
 * @param int elem - Индекс файла
 * 
 * @return size_t - Размер файла или отрицательное значение при ошибке
 */
size_t vfs_getLengthFile(int node,int elem){
    if (vfs_mount[node]->getLengthFile != 0){
        return vfs_mount[node]->getLengthFile(elem);
    }
    return -1;
}

/**
 * @brief [VFS] Получить отступ в файловой системе у файла
 *
 * @param int node - Адрес монтирования
 * @param int elem - Индекс файла
 * 
 * @return int - Позиция файла или отрицательное значение при ошибке
 */
int vfs_getOffsetFile(int node,int elem){
    if (vfs_mount[node]->getOffsetFile != 0){
        return vfs_mount[node]->getOffsetFile(elem);
    }
    return -1;
}

/**
 * @brief [VFS] Получить размер устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return size_t - Размер диска
 */
size_t vfs_getDiskSize(int node){
    return vfs_mount[node]->diskSize(node);
}

/**
 * @brief [VFS] Получить свободное место устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return size_t - Размер свободного пространства диска
 */
size_t vfs_getDiskSpace(int node){
    return vfs_mount[node]->diskSpace(node);

}

/**
 * @brief [VFS] Получить занятое место устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return size_t - Размер занятого пространства диска
 */
size_t vfs_getDiskUsed(int node){
    return vfs_mount[node]->diskUsed(node);
}

/**
 * @brief [VFS] Получить название устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return char* - Название устройства
 */
char* vfs_getName(int node){
    return vfs_mount[node]->getDevName(node);
}

/**
 * @brief [VFS] Получить количество файлов в папке
 *
 * @param char* path - Путь к файлу (виртуальный)
 *
 * @return size_t - Количество файлов
 */
size_t vfs_getCountElemDir(char* path){
    int node = vfs_foundMount(path);
    if (vfs_mount[node]->getCountElemFolder != 0) {
        char* c_path = (char*)kmalloc(sizeof(char)*strlen(path));
        vfs_getPath(node, path, c_path);

        uint32_t folder_elems = vfs_mount[node]->getCountElemFolder(c_path);

        kfree(c_path);
        return folder_elems;
    }
    return 0;
}

/**
 * @brief [VFS] Поиск папки
 *
 * @param char* filename - Путь к папке (виртуальный)
 *
 * @return int - Индекс папки, или отрицательное значение при ошибке
 */
struct dirent* vfs_getListFolder(char* path){
    int node = vfs_foundMount(path);
    struct dirent* elem = 0;

    if (vfs_mount[node]->getListElem != 0) {
        char* c_path = (char*)kmalloc(sizeof(char)*strlen(path) + 1);
        
        vfs_getPath(node, path, c_path);
        elem = vfs_mount[node]->getListElem(c_path);

        kfree(c_path);
        return elem;
    }

	return elem;
}

/**
 * @brief [VFS] Создание файла
 *
 * @warning Еше не реализовано
 */
void vfs_createFile(){
}

/**
 * @brief [VFS] Создание папки
 *
 * @warning Еше не реализовано
 */
void vfs_createDir(){
}

/**
 * @brief [VFS] Вывод содержимого списка файла и папок
 *
 * @warning Еше не реализовано
 */
void vfs_list(){
    vfs_mount[0]->list("");
}

/**
 * @brief [VFS] Удаление файла
 *
 * @warning Еше не реализовано
 */
void vfs_deleteFile(){
}


/**
 * @brief [VFS] Получить тип устройства
 *
 * @warning Еше не реализовано
 */
void vfs_getType(){
}

/**
 * @brief [VFS] Получить точку монтирования устройства
 *
 * @warning Еше не реализовано
 */
void vfs_getMountPoint(){
}

void vfs_unlistFolder(const char* path, struct dirent* ptr) {
    int node = vfs_foundMount(path);
    if (vfs_mount[node]->unlistElem){
        vfs_mount[node]->unlistElem(ptr);
    }
}

size_t vfs_byteToKByte(size_t bytes){
    return (bytes == 0?0:bytes/1024);
}
