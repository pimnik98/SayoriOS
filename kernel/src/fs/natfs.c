/**
 * @file drv/fs/tarfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система NatFS - NatSuki File System
 * @version 0.3.6
 * @date 2023-12-08
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <kernel.h>
#include <io/ports.h>
#include <fs/fsm.h>
#include <fs/natfs.h>

#include <lib/php/pathinfo.h>

size_t fs_natfs_read(const char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer){
    char* buf = __milla_getFile(Path);
    qemu_log("NatSuki:\n\t Path: %s\n\tSize: %d | %d\n\tContent: %s", Path, Size, strlen(buf), buf);

    memcpy(Buffer, buf, Size);
    //substr(Buffer, (void*)__milla_getFile(Path), Offset, Size);
    kfree(buf);
    return Size;
}

size_t fs_natfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer){
    return 0;
}

FSM_FILE fs_natfs_info(const char Disk,const char* Path){
    FSM_FILE file = {};
    file.Size = __milla_getSizeFile(Path);
    if (__milla_isFile(__milla_findID(Path,"file")) != 1){
        qemu_warn("[NatFS] File no found???");
        return file;
    }
    file.Ready = 1;
    //fsm_convertUnix(atoi(_m_s[3]), &file.LastTime);

    char* zpath = pathinfo(Path, PATHINFO_DIRNAME);
    char* zname = pathinfo(Path, PATHINFO_BASENAME);

    file.Mode = 'r';
    //file.Type = ((strcmpn(_m_s[0], "file")?0:5));
    file.Type = 0;
    file.Ready = 1;

    memcpy(file.Path, Path, strlen(Path));
    memcpy(file.Name, zname, strlen(zname));
    return file;
}

FSM_DIR* fs_natfs_dir(const char Disk,const char* Path){
    size_t inxDir = __milla_findID(Path,"DIR");
    if (inxDir < 0){
        qemu_warn("[NatSuki] When searching for a folder, the error %d was returned, perhaps the folder was not found or the device is not ready to work.", inxDir);
        return 0;
    }

    qemu_log("[NatSuki] Get path: %s",Path);
    FSM_DIR *Dir = kcalloc(sizeof(FSM_DIR), 1);
    size_t CA = 0, CF = 0, CD = 0, CO = 0;

    char* listt = __milla_getList(Path);
    char* _m_d[256] = {0};
    char* _m_s[256] = {0};
    uint32_t _m_d2 = 0;

    uint32_t _m_d1 = str_cdsp(listt,"\n");

    str_split(listt,_m_d,"\n");

    //tty_printf("Найдено файлов и папок: %d\n",_m_d1-1);


    FSM_FILE *Files = kcalloc(sizeof(FSM_FILE), _m_d1);
    //struct dirent* testFS = kcalloc(_m_d1, sizeof(struct dirent));
    if (_m_d1 == 0){
        Dir->Ready = 1;
        Dir->Count = CA;
        Dir->CountFiles = CF;
        Dir->CountDir = CD;
        Dir->CountOther = CO;
        Dir->Files = Files;

        return Dir;
    }
    for(size_t ind = 0; ind < _m_d1; ind++){
        _m_d2 = str_cdsp(_m_d[ind],"::");
        if (_m_d2 < 1) continue;
        memset(_m_s,0,256);
        str_split(_m_d[ind],_m_s,"::");

        fsm_convertUnix(atoi(_m_s[3]), &Files[CA].LastTime);

//        Files[CA].Mode = 'rw';  // INVALID!
        Files[CA].Mode = 'r';
        Files[CA].Size = (atoi(_m_s[4]));
        Files[CA].Type = ((strcmpn(_m_s[0], "file")?0:5));
        Files[CA].Ready = 1;

        memcpy(Files[CA].Path, Path, strlen(Path));
        memcpy(Files[CA].Name, _m_s[1], strlen(_m_s[1]));


        if (Files[CA].Type == 0) {
            /// Это файл
            CF++;
        } else if (Files[CA].Type == 5){
            /// Это папка
            CD++;
        } else {
            /// Рандомный элемент
            CO++;
        }
        CA++;

        //testFS[inxFile].type = ((strcmpn(_m_s[0], "file")?FS_FILE:FS_DIRECTORY));
        //testFS[inxFile].ino = inxFile;
        //testFS[inxFile].next = inxFile+1;
        //testFS[inxFile].length = (atoi(_m_s[4]));
        //strcpy(testFS[inxFile].name, _m_s[1]);
        qemu_log("[Milla] [%s] %s (%d b.) Date: %s Owner: %s",_m_s[0],_m_s[1],(atoi(_m_s[4])),_m_s[3],_m_s[5]);
    }
    kfree(listt);

    Dir->Ready = 1;
    Dir->Count = CA;
    Dir->CountFiles = CF;
    Dir->CountDir = CD;
    Dir->CountOther = CO;
    Dir->Files = Files;

    return Dir;
}

int fs_natfs_create(const char Disk,const char* Path,int Mode){
    return 0;
}

int fs_natfs_delete(const char Disk,const char* Path,int Mode){
    return 0;
}

void fs_natfs_label(const char Disk, char* Label){
    memcpy(Label,"NatFS",strlen("NatFS"));
}

int fs_natfs_detect(const char Disk){
    return (0);
}

char* __milla_getDiskInfo_Name();
int __milla_getDiskInfo_All();

int fs_natfs_init(){
    int _m = __milla_init();
    if (_m != 0){
        qemu_warn("[NatFS] An error occurred during initialization, the server returned a response: %d",_m);
        return 0;
    }
    int dsize = __milla_getDiskInfo_All();
    char* dname = __milla_getDiskInfo_Name();
    int dpm = dpm_reg('N', dname, "NatFS", 2, dsize, 0, 0, 3, "NAT0-SUKI", (void *) _m);
    if (dpm != 1){
        qemu_warn("[NatFS] An error occurred while initializing the disk in the operating system! DPM returned the code: %d", dpm);
        return 0;
    }
    return 1;
}
