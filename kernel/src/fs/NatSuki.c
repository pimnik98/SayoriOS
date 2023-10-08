/**
 * @file fs/NatSuki.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief [VFS] [Драйвер] NatSuki - Виртуальная файловая система
 * @version 0.3.3
 * @date 2023-01-27
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h>
fs_node_t *nat_root;                        ///< Ссылка на виртуальную фс
bool    __milla_b_init = false;             ///< Milla готова к работе?
char*   __milla_null = "null";              ///< Ответ, если Milla не готовa
char*   __milla_buffer = 0;                 ///< Буфер
char    __milla_login[256] = "SayoriOS";    ///< Логин для авторизации
char    __milla_passwd[256] = "NatSuki";    ///< Пароль для авторизации
int     __milla_return_code = 0;            ///< Код ошибки

void __milla_setLogin(char* login){
    memset(__milla_login,0,256);
    memcpy((void*)__milla_login, (void*)login, strlen(login));
    qemu_log("[NatSuki] New login: %s",__milla_login);
}

void __milla_setPasswd(char* passwd){
    memset(__milla_passwd,0,256);
    memcpy((void*)__milla_passwd, (void*)passwd, strlen(passwd));
    qemu_log("[NatSuki] New password: %s",__milla_passwd);
}

int __milla_getCode(){
    return __milla_return_code;
}

/*********************************/


/**
 * @brief [Milla] Отправка пакета
*/
void __milla_sendcmd(char* msg){
    if (!__milla_b_init){return;}
    __com_formatString(PORT_COM2,"%s |$MC#|",msg);
}

/**
 * @brief [Milla] Чтение пакета
*/
char* __milla_getcmd(){
    if (!__milla_b_init){return __milla_null;}
    char tmp = 'S';
    size_t inx = 0;
    memset(__milla_buffer,0,256);
    for(;;) {
        if (inx >= 128){
            __milla_buffer[inx+1] = 0;
            break;
        }
        tmp = serial_readchar(PORT_COM2);
        if ((int) tmp == 10 && inx == 0) continue;
        //qemu_log("Write %d / 128",inx);
        //qemu_log("\n (%d = %c) \n",(int) tmp,tmp);
        if ((int) tmp == 10){
            __milla_buffer[inx+1] = 0;
            break;
        }
        //tty_printf("%c", tmp);
        __milla_buffer[inx] = tmp;
        inx++;
    }
    //qemu_log("Milla: %s",__milla_buffer);
    //tty_printf("Milla: %s\n",__milla_buffer);
    return __milla_buffer;
}

int __milla_getSizeFile(char* path){
    if (!__milla_b_init){return 0;}
    __com_formatString(PORT_COM2,"SIZE %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    if (answer < 0){
        qemu_log("[Milla] [getSizeFile] ERR! Return %d:%s",answer,__milla_buffer);
        return 0;
    }
    return answer;
}


char* __milla_getDiskInfo_Name(){
    if (!__milla_b_init){return 0;}
    __com_formatString(PORT_COM2,"DISKNAME |$MC#|");
    __milla_getcmd();

    return __milla_buffer;
}


int __milla_getDiskInfo_Free(){
    if (!__milla_b_init){return 0;}
    __com_formatString(PORT_COM2,"DISKSIZE_FREE |$MC#|");
    int answer = atoi(__milla_getcmd());
    if (answer < 0){
        qemu_log("[Milla] [getDiskInfo_Free] ERR! Return %d:%s",answer,__milla_buffer);
        return 0;
    }
    return answer;
}


int __milla_getDiskInfo_Use(){
    if (!__milla_b_init){return 0;}
    __com_formatString(PORT_COM2,"DISKSIZE_USE |$MC#|");
    int answer = atoi(__milla_getcmd());
    if (answer < 0){
        qemu_log("[Milla] [getDiskInfo_Use] ERR! Return %d:%s",answer,__milla_buffer);
        return 0;
    }
    return answer;
}


int __milla_getDiskInfo_All(){
    if (!__milla_b_init){return 0;}
    __com_formatString(PORT_COM2,"DISKSIZE_ALL |$MC#|");
    int answer = atoi(__milla_getcmd());
    if (answer < 0){
        qemu_log("[Milla] [getDiskInfo_All] ERR! Return %d:%s",answer,__milla_buffer);
        return 0;
    }
    return answer;
}


/**
 * @brief Сбрасывает состояние Milla
 */
int __milla_cleanState(){
    if (!__milla_b_init){return 0;}
    __milla_sendcmd("CLEAN");
    int answer = strcmp(__milla_getcmd(),"CLEAN");
    if (answer != 0){
        for (size_t i = 0;i < 5;i++){
            __milla_sendcmd("CLEAN");
            answer = strcmp(__milla_getcmd(),"CLEAN");
            if (answer == 0){
                break;
            }
        }
    }
    if (answer == 0){
        qemu_log("[Milla] [Clean] OK! Return %d:%s",answer,__milla_buffer);
        return 1;
    } else {
        qemu_log("[Milla] [Clean] ERR! Return %d:%s",answer,__milla_buffer);
        return 0;
    }
}

char* __milla_getFile(char* path) {
    if (!__milla_b_init){return __milla_null;}
    __com_formatString(PORT_COM2,"READ %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    if (answer < 0){
        qemu_log("[Milla] [getFile] ERR! Return %d:%s",answer,__milla_buffer);
        return __milla_null;
    } else if (answer == 0){
        qemu_log("[Milla] [getFile] ERR! Return %d:%s",answer,__milla_buffer);
        return __milla_null;
    }
    qemu_log("[Milla] [getFile] OK! Return %d:%s",answer,__milla_buffer);
    __milla_sendcmd("READ GET");
    char tmp = 'S';
    size_t inx = 0;
    char* buf = (char*)kmalloc(sizeof(char*) * answer);
    memset(buf,0,answer);
    for(;;) {
        if (inx >= answer)break;
        tmp = serial_readchar(PORT_COM2);
        buf[inx] = tmp;
        inx++;
    }
    return buf;
}

int __milla_writeFile(char* path, char* data){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"WRITE %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    if (answer != 1){
        qemu_log("[Milla] [writeFile] ERR1! Return %d:%s",answer,__milla_buffer);
        return -1;
    }
    __com_formatString(PORT_COM2,"WRITE %d |$MC#|",strlen(data));
    answer = atoi(__milla_getcmd());
    if (answer != 1){
        qemu_log("[Milla] [writeFile] ERR2! Return %d:%s",answer,__milla_buffer);
        return -2;
    }
    __com_formatString(PORT_COM2,"%s",data);
    answer = atoi(__milla_getcmd());
    if (answer != strlen(data)){
        qemu_log("[Milla] [writeFile] ERR3! Return %d:%s",answer,__milla_buffer);
        return -3;
    }
    qemu_log("[Milla] [writeFile] OK! %d:%s",strlen(data),data);
    return answer;
}

int __milla_delete(char* path){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"DEL %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    
    return answer;
}

int __milla_mkdir(char* path){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"MKDIR %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    
    return answer;
}

size_t __milla_findID(char* path,char* type){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"FIND %s %s |$MC#|",type,path);
    return atoi(__milla_getcmd());
}


size_t __milla_isDir(size_t inx){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"ISDIR %d |$MC#|",inx);
    return atoi(__milla_getcmd());
}

size_t __milla_isFile(size_t inx){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"ISFILE %d |$MC#|",inx);
    return atoi(__milla_getcmd());
}

char* __milla_getRootID(size_t inx){
    if (!__milla_b_init){return __milla_null;}
    __com_formatString(PORT_COM2,"ROOT %d |$MC#|",inx);
    return (__milla_getcmd());
}


int __milla_touch(char* path){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"TOUCH %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());

    return answer;
}




char* __milla_getList(char* path){
    if (!__milla_b_init){return __milla_null;}
    __com_formatString(PORT_COM1,"\n< LIST %s |$MC#|\n",path);
    __com_formatString(PORT_COM2,"LIST %s |$MC#|",path);
    int answer = atoi(__milla_getcmd());
    __com_formatString(PORT_COM1,"> %d\n",answer);
    if (answer < 0){
        qemu_log("[Milla] [getFile] ERR! Return %d:%s",answer,__milla_buffer);
        return __milla_null;
    } else if (answer == 0){
        qemu_log("[Milla] [getFile] OK! Folder EMPTY Return %d:%s",answer,__milla_buffer);
        return __milla_null;
    }

    qemu_log("[Milla] [getFile] OK! Return %d:%s",answer,__milla_buffer);
    __com_formatString(PORT_COM1,"\n> LIST GET\n");
    __com_formatString(PORT_COM2,"LIST GET|$MC#|");
    //__milla_sendcmd("LIST GET");
    char tmp = 'S';
    size_t inx = 0;
    char* buf = (char*)kmalloc(sizeof(char*) * answer);
    memset(buf,0,answer);
    for(;;) {
        if (inx >= answer)break;
        tmp = serial_readchar(PORT_COM2);
        buf[inx] = tmp;
        inx++;
    }
    __com_formatString(PORT_COM1,"< %s\n",buf);
    return buf;
}

size_t __milla_getCountFiles(char* path){
    if (!__milla_b_init){return -9999;}
    __com_formatString(PORT_COM2,"COUNT %s |$MC#|",path);
    return atoi(__milla_getcmd());
}


/**
 * @brief Инициализация проекта Милла
*/
int __milla_init(){
    __com_setInit(2,1);
    if (!__milla_b_init){
        int cominit = __com_init(PORT_COM2);
        if (cominit == 1){
            __milla_return_code = 1;
            return -1;  // Не удалось выполнить инициализацию
        }
        qemu_log("[Milla] Step 1 PASSED");
        // Подгрузим буфер и отчистим его, если там есть что-то
        __milla_buffer =  (char*)kmalloc(sizeof(char*) * 256);
        memset(__milla_buffer,0,256);
        __milla_b_init = true;
        __com_formatString(PORT_COM2,"LOGIN %s %s|$MC#|",__milla_login,__milla_passwd);
        int answer = atoi(__milla_getcmd());
        if (answer != 1){
            qemu_log("[Milla] ERROR AUTH %d (%s)",answer,__milla_buffer);
            __milla_b_init = false;
            __milla_return_code = 2;
            return -2;  // Не удалось выполнить инициализацию
        }
        cominit = __milla_cleanState();
        qemu_log("[Milla] Step 2 PASSED");
        if (cominit != 1){
            qemu_log("[Milla] Step 3 ERROR");
            __milla_return_code = 3;
            return -3;  // Не удалось отчистить буфер
        }
        qemu_log("[Milla] Step 3 PASSED");
    }
    __milla_b_init = true;
    __milla_return_code = 0;
    return 0;
}


void __milla_destroy(){
    kfree(__milla_buffer);
}

/*********************************/









/**
 * @brief [SEFS] Полное чтение файла
 *
 * @param int node - Индекс файла
 *
 * @return char* - Содержимое файла
 */
char* nat_readChar(uint32_t node){
    qemu_log("[NatSuki] [nat_readChar] ");
    return 0;
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
uint32_t nat_read(uint32_t node, size_t offset, size_t size, void *buffer){
    if (!__milla_b_init) return 0;

	substr(buffer,(void*)__milla_getFile(__milla_getRootID(node)),offset,size);

    //memcpy(buffer, __milla_getFile(__milla_getRootID(node)), size);
    qemu_log("[NatSuki] [nat_read] [Offset:%d] [Size:%d] Node: %d | Data: %d",offset,size,strlen(buffer));
    return strlen(buffer);
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
uint32_t nat_write(uint32_t node, size_t offset, size_t size, void *buffer){
    if (!__milla_b_init) return 0;
    qemu_log("[NatSuki] [nat_write] ");
    return -2;
}

/**
 * @brief [SEFS] Получить размер файла (поиск по индексу)
 *
 * @param int node - Индекс файла
 *
 * @return size_t - Размер файла или 0
 */
size_t nat_getLengthFile(int node){
    if (!__milla_b_init) return 0;
    //qemu_log("[NatSuki] [nat_getLengthFile] Node: %d",node);

    return __milla_getSizeFile(__milla_getRootID(node));
}

/**
 * @brief [SEFS] Получить отступ в файловой системе у файла
 *
 * @param int node - Индекс файла
 *
 * @return int - Позиция файла или отрицательное значение при ошибке
 */
size_t nat_getOffsetFile(int node){
    qemu_log("[NatSuki] [nat_getOffsetFile] ");
    return 0;
}

/**
 * @brief [SEFS] Поиск файла на устройстве
 *
 * @param char* filename - Путь к файлу (виртуальный)
 *
 * @return int - Индекс файла, или отрицательное значение при ошибке
 */
int32_t nat_findFile(char* filename){
    if (!__milla_b_init) return -1;
    //qemu_log("[NatSuki] [nat_findFile] %s",filename);
    return __milla_findID(filename,"FILE");
}

/**
 * @brief [SEFS] Поиск папки на устройстве
 *
 * @param char* filename - Путь к папке (виртуальный)
 *
 * @return int - Индекс папки, или отрицательное значение при ошибке
 */
int32_t nat_findDir(char* path){
    if (!__milla_b_init) return -1;
    //qemu_log("[NatSuki] [nat_findDir] %s",path);

    return __milla_findID(path,"DIR");
}

/**
 * @brief [SEFS] Считает количество элементов в папке
 */
size_t nat_countElemFolder(char* path){
    if (!__milla_b_init) return 0;
    //qemu_log("[NatSuki] [nat_countElemFolder] %s",path);
    return __milla_getCountFiles(path);
}

/**
 * @brief [SEFS] Выводит список файлов
 */
struct dirent* nat_list(char* path){
    size_t inxDir = nat_findDir(path);
    if (inxDir < 0){
        return 0;
    }



    qemu_log("[NatSuki] [nat_list] %s",path);
    char* listt = __milla_getList(path);
    //tty_printf("\n%s\n",listt);

    uint32_t _m_d1 = str_cdsp(listt,"\n");
    char* _m_d[256] = {0};
    str_split(listt,_m_d,"\n");
    uint32_t _m_d2 = 0;

    char* _m_s[256] = {0};
    size_t inxFile = 0;
    tty_printf("Найдено файлов и папок: %d\n",_m_d1);
    struct dirent* testFS = kcalloc(_m_d1, sizeof(struct dirent));
    if (_m_d1 == 0){
        return testFS;
    }
    for(size_t ind = 0; ind < _m_d1; ind++){
        _m_d2 = str_cdsp(_m_d[ind],"::");
        if (_m_d2 < 1) continue;
        memset(_m_s,0,256);
        str_split(_m_d[ind],_m_s,"::");
        testFS[inxFile].type = ((strcmpn(_m_s[0], "file")?FS_FILE:FS_DIRECTORY));
        testFS[inxFile].ino = inxFile;
        testFS[inxFile].next = inxFile+1;
        testFS[inxFile].length = (atoi(_m_s[4]));
        strcpy(testFS[inxFile].name, _m_s[1]);
        inxFile++;
        qemu_log("[Milla] [%s] %s (%d b.) Date: %s Owner: %s",_m_s[0],_m_s[1],(atoi(_m_s[4])),_m_s[3],_m_s[5]);
    }
    testFS[inxFile].next = 0;
    kfree(listt);
    kfree((void*)_m_d);
    kfree((void*)_m_s);
    kfree((void*)_m_d1);
    kfree((void*)_m_d2);
    return testFS;
}

/**
 * @brief [SEFS] Количество используемого места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество используемого места устройства
 */
size_t nat_diskUsed(int node){
    qemu_log("[NatSuki] [nat_diskUsed] ");
    return 2;
}

/**
 * @brief [SEFS] Количество свободного места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество свободного места устройства
 */
size_t nat_diskSpace(int node){
    qemu_log("[NatSuki] [nat_diskSpace] ");
    return 0;
}

/**
 * @brief [SEFS] Количество всего места устройства
 *
 * @param int node - Нода
 *
 * @return uint64_t - Количество всего места устройства
 */
size_t nat_diskSize(int node){
    qemu_log("[NatSuki] [nat_diskSize] ");
    return 1;
}

/**
 * @brief [SEFS] Получение имени устройства
 *
 * @param int node - Нода
 *
 * @return char* - Имя устройства
 */
char* nat_getDevName(int node){
    qemu_log("[NatSuki] [nat_getDevName] ");
    if (!__milla_b_init) return "0000-0000";
    return nat_root->devName;
}

void nat_dirfree(struct dirent* ptr) {
    qemu_log("[NatSuki] [nat_dirfree] ");
}

bool isInitNatSuki(){
    return __milla_b_init;
}

/**
 * @brief [SEFS] Инициализация Sayori Easy File System
 *
 * @param uint32_t location - Точка монтирования
 *
 * @return fs_node_t - Структура с файлами
 */
fs_node_t *NatSuki_initrd(uint32_t location){
    qemu_log("[NatSuki] [Init] loc: %x | state: %d",location,__milla_b_init);

    nat_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!__milla_b_init){
        int cominit = __milla_init();

        qemu_log("NatSuki -> %d",cominit);
        if (cominit != 0){
            strcpy(nat_root->name, "/nat/");
            strcpy(nat_root->devName, "0000-0000");
            return nat_root;  // Не удалось выполнить инициализацию
        }
        __milla_b_init = true;
    }

    strcpy(nat_root->name, "/nat/");
    strcpy(nat_root->devName, __milla_getDiskInfo_Name());
    nat_root->mask = nat_root->uid = nat_root->gid = nat_root->inode = nat_root->length = 0;
    nat_root->flags = FS_DIRECTORY;
    nat_root->open = 0;
    nat_root->close = 0;
    nat_root->findFile = &nat_findFile;
    nat_root->findDir = &nat_findDir;
    nat_root->getLengthFile = &nat_getLengthFile;
    nat_root->getOffsetFile = &nat_getOffsetFile;
    nat_root->list = &nat_list;
    nat_root->ptr = 0;
    nat_root->impl = 0;
    nat_root->readChar = &nat_readChar;
    nat_root->read = &nat_read;
    nat_root->write = &nat_write;
    nat_root->diskUsed = &nat_diskUsed;
    nat_root->diskSpace = &nat_diskSpace;
    nat_root->diskSize = &nat_diskSize;
    nat_root->getDevName = &nat_getDevName;
    nat_root->getCountElemFolder = &nat_countElemFolder;
    nat_root->getListElem = &nat_list;
    nat_root->unlistElem = &nat_dirfree;

    return nat_root;
}

