/**
 * @file drv/fs/iso9660.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система ISO 9660
 * @version 0.3.5
 * @date 2023-12-23
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <kernel.h>
#include "portability.h"
#include <io/ports.h>
#include <../src/lib/elk/ext/jse_function.h>

void __out_limit(char* pr, char* str, size_t count){
    char* buf = kcalloc(1, count+1);
    memcpy(buf,str, count);
    printf("%s: '%s'\n",pr, buf);
    kfree(buf);
}


size_t fs_iso9660_read(const char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer){
    return 0;
}

size_t fs_iso9660_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer){
    return 0;
}

FSM_FILE fs_iso9660_info(const char Disk,const char* Path){
    return (FSM_FILE){};
}

FSM_DIR* fs_iso9660_dir(const char Disk,const char* Path){
    return 0;
}

int fs_iso9660_create(const char Disk,const char* Path,int Mode){
    return 0;
}

int fs_iso9660_delete(const char Disk,const char* Path,int Mode){
    return 0;
}

void fs_iso9660_label(const char Disk, char* Label){
    char* l = kcalloc(1, 33);
    int buf_read = dpm_read(Disk, 0, 0x8028, 32, l);
    if (buf_read != 32){
        memcpy(Label,"Unsupported disk",strlen("Unsupported disk"));
    } else {
        l[33] = 0;
        jse_trim(l);
        memcpy(Label,l,strlen(l));
    }
    kfree(l);
}

int fs_iso9660_detect(const char Disk){
    ISO9660_PVD* pvd = kcalloc(1, sizeof(ISO9660_PVD));
    int buf_read = dpm_read(Disk, 0, 0x8000, sizeof(ISO9660_PVD), pvd);
    if (
    pvd->Version != 0x1 ||
    pvd->ID[0]   != 0x43 ||
    pvd->ID[1]   != 0x44 ||
    pvd->ID[2]   != 0x30 ||
    pvd->ID[3]   != 0x30 ||
    pvd->ID[4]   != 0x31 ||
    pvd->FileStructureVersion != 0x1
    ){
        qemu_err("[ISO9660] %c | No passed test!",Disk);

        kfree(pvd);
        return 0;
    }

    qemu_warn("--------------------------------");
    qemu_warn("| Label Disk %c           | READ: %d", Disk, buf_read);
    qemu_warn("| Zero                   | %x |  %d ", pvd->Zero, pvd->Zero);
    __out_limit("ID", pvd->ID, 5);
    qemu_warn("| Version                | %x |  %d ", pvd->Version, pvd->Version);
    __out_limit("SystemName", pvd->SystemName, 32);
    __out_limit("Label", pvd->Label, 32);
    qemu_warn("| VolumeSpaceSize        | %x |  %d ", pvd->VolumeSpaceSize, pvd->VolumeSpaceSize);
    qemu_warn("| VolumeSetSize          | %x |  %d ", pvd->VolumeSetSize, pvd->VolumeSetSize);
    qemu_warn("| VolumeSequenceNumber   | %x |  %d ", pvd->VolumeSequenceNumber, pvd->VolumeSequenceNumber);
    qemu_warn("| LogicalBlockSize       | %x |  %d ", pvd->LogicalBlockSize, pvd->LogicalBlockSize);
    qemu_warn("| PathTableSize          | %x |  %d ", pvd->PathTableSize, pvd->PathTableSize);
    qemu_warn("| LocOfType_L_PathTable  | %x |  %d ", pvd->LocOfType_L_PathTable, pvd->LocOfType_L_PathTable);
    qemu_warn("| LocOfOpti_L_PathTable  | %x |  %d ", pvd->LocOfOpti_L_PathTable, pvd->LocOfOpti_L_PathTable);
    qemu_warn("| LocOfType_M_PathTable  | %x |  %d ", pvd->LocOfType_M_PathTable, pvd->LocOfType_M_PathTable);
    qemu_warn("| LocOfOpti_M_PathTable  | %x |  %d ", pvd->LocOfOpti_M_PathTable, pvd->LocOfOpti_M_PathTable);

    __out_limit("DirectoryEntry", pvd->DirectoryEntry, 34);
    __out_limit("VolumeSetID", pvd->VolumeSetID, 128);
    __out_limit("PublisherID", pvd->PublisherID, 128);
    __out_limit("DataPreparerID", pvd->DataPreparerID, 128);
    __out_limit("ApplicationID", pvd->ApplicationID, 128);
    __out_limit("CopyrightFileID", pvd->CopyrightFileID, 37);
    __out_limit("AbstractFileID", pvd->AbstractFileID, 37);
    __out_limit("BibliographicFileID", pvd->BibliographicFileID, 37);
    __out_limit("VolumeCreationDate", pvd->VolumeCreationDate, 37);
    __out_limit("VolumeModificationDate", pvd->VolumeModificationDate, 17);
    __out_limit("VolumeExpirationDate", pvd->VolumeExpirationDate, 17);
    __out_limit("VolumeEffectiveDate", pvd->VolumeEffectiveDate, 17);

    qemu_warn("| FSVersion          | %x |  %d ", pvd->FileStructureVersion, pvd->FileStructureVersion);
    qemu_warn("--------------------------------");




//    ISO9660_Entity* entity = kcalloc(1, sizeof(ISO9660_Entity));
//    buf_read = dpm_read(Disk, 0x809C - 3, sizeof(ISO9660_Entity), entity);
//
//    int lba = (entity->LBA[0] & 0xFF) | ((entity->LBA[0] & 0xFF00) >> 8) | ((entity->LBA[0] & 0xFF0000) >> 16) | ((entity->LBA[0] & 0xFF000000) >> 24);
//    qemu_warn("LBA: %d | %x",lba,lba);

    kfree(pvd);
    //kfree(entity);
    return (1);
}