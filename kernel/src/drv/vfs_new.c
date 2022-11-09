/**
 * @file drv/vfs_new.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер виртуальной файловой системы
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>
#include <io/ports.h>
#include <drv/vfs_new.h>

fs_node_t *fs_root; 			///< Точка монтирования виртуального корня
fs_node_t *vfs_mount[128];		///< Точки монтирования
size_t countMount = 0;			///< Кол-во смонтированных устройств

/**
 * @brief [VFS] Получить реальный путь на устройстве
 *
 * @param int node - Адрес монтирования
 * @param char* path - Путь к файлу (виртуальный)
 * 
 * @return char* - Реальный путь
 */
char* vfs_getPath(int node,char* path){
    char* file = kmalloc(sizeof(char)*strlen(path));
    strcpy(file,path);
    substr(file,path,strlen(vfs_mount[node]->name),strlen(path));
    //qemu_log("[gP] %s | %s",file,path);
    return file;
}

/**
 * @brief [VFS] Получить адрес монтирования у файла
 *
 * @param char* path - Путь к файлу (виртуальный)
 * 
 * @return int - Индекс, или отрицательное значение если точка монтирования не найдена
 */
int vfs_foundMount(char* path){
    for (int i=0;i < countMount;i++){
        if (strcmp(path,vfs_mount[i]->name) >= 0){
            //qemu_log("[%d] File (%s) found mount: %s",i,path,vfs_mount[i]->name);
            return i;
        }
    }
    return -1;
}

/**
 * @brief [VFS] Монтирование устройства
 *
 * @param int location - Адрес монтирования
 * @param int type - Тип файловой системы
 */
void vfs_reg(int location,int type){
    if (type == VFS_TYPE_MOUNT_SEFS){
        //qemu_log("[VFS] [REG] [%d] Sayori Easy File System | Location: %x",countMount,location);
        vfs_mount[countMount] = (fs_node_t*) kmalloc(sizeof(fs_node_t*));
        vfs_mount[countMount] = (fs_node_t*) sefs_initrd(location);
        countMount++;
    } else {
        //qemu_log("[VFS] [REG] Unknown | Location: %x",location);
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
int vfs_write(int node,int elem, size_t offset, size_t size, void *buf){
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
int vfs_findFile(char* filename){
    //qemu_log("[VFS] [FindFile] File: `%s`",filename);
    int node = vfs_foundMount(filename);
    if (vfs_mount[node]->findFile != 0){
        char* path = vfs_getPath(node,filename);
        uint32_t elem = vfs_mount[node]->findFile(path);
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
bool vfs_exists(char* filename){
  if (vfs_findFile(filename) != -1){
    return true;
  }
  return false;
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
uint32_t vfs_read(int node, int elem, size_t offset, size_t size, void *buf){
    //qemu_log("[VFS] [Read] Node:%d | Elem: %d | Off: %d | Size: %d", node, elem, offset, size);
    if (vfs_mount[node]->read != 0){
        uint32_t ret = vfs_mount[node]->read(elem, offset, size, buf);
		//qemu_log("Buf now: %s", buf);
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
        char* c_path = vfs_getPath(node,path);
        uint32_t elem = vfs_mount[node]->findDir(c_path);
        return elem;
    }
    return -1;
}

/**
 * @brief [VFS] Получить длину файла (поиск по виртуальному пути)
 *
 * @param char* filename - Путь к файлу (виртуальный)
 * 
 * @return uint64_t - Размер файла или отрицательное значение при ошибке
 */
uint64_t vfs_getLengthFilePath(char* filename){
  int node = vfs_foundMount(filename);
  if (vfs_mount[node]->findFile != 0 && vfs_mount[node]->getLengthFile != 0){
        char* path = vfs_getPath(node,filename);
        uint32_t elem = vfs_mount[node]->findFile(path);
        uint64_t size = vfs_mount[node]->getLengthFile(elem);
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
 * @return uint64_t - Размер файла или отрицательное значение при ошибке
 */
uint64_t vfs_getLengthFile(int node,int elem){
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
 * @return uint64_t - Размер диска
 */
uint64_t vfs_getDiskSize(int node){
    return vfs_mount[node]->diskSize(node);
}

/**
 * @brief [VFS] Получить свободное место устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return uint64_t - Размер свободного пространства диска
 */
uint64_t vfs_getDiskSpace(int node){
    return vfs_mount[node]->diskSpace(node);

}

/**
 * @brief [VFS] Получить занятое место устройства
 *
 * @param int node - Адрес монтирования
 *
 * @return uint64_t - Размер занятого пространства диска
 */
uint64_t vfs_getDiskUsed(int node){
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
    if (vfs_mount[node]->getCountElemFolder != 0){
        char* c_path = vfs_getPath(node,path);
        return vfs_mount[node]->getCountElemFolder(c_path);
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
struct direct* vfs_getListFolder(char* path){
    int node = vfs_foundMount(path);
    struct direct* elem;
    if (vfs_mount[node]->getListElem != 0){
        char* c_path = vfs_getPath(node,path);
        elem = vfs_mount[node]->getListElem(c_path);
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
    vfs_mount[0]->list();
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
