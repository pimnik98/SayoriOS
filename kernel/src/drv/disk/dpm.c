/**
 * @file drv/disk/dpm.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Disk Partition Manager - Менеджер разметки дисков
 * @version 0.3.5
 * @date 2023-10-16
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <io/ports.h>
#include <drv/disk/dpm.h>
#include "mem/vmm.h"
bool dpm_debug = false;

DPM_Disk DPM_Disks[32] = {0};

int dpm_searchFreeIndex(int Index) {
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	for (int i = Index; i < 32; i++){
		if (DPM_Disks[i].Ready == 1)
			continue;

		return i;
	}

	for (int i = 4; i < 32; i++){
		if (DPM_Disks[i].Ready == 1)
			continue;

		return i;
	}

	return -1;
}

void dpm_fnc_write(char Letter, dpm_disk_rw_cmd Read, dpm_disk_rw_cmd Write) {
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	
	if (DPM_Disks[Index].Ready == 0 || DPM_Disks[Index].Status == 0)
		return;

	DPM_Disks[Index].Read = Read;
	DPM_Disks[Index].Write = Write;
}

void* dpm_metadata_read(char Letter){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	if (DPM_Disks[Index].Ready == 0 || DPM_Disks[Index].Status == 0)
		return 0;

	return DPM_Disks[Index].Reserved;
}

void dpm_metadata_write(char Letter, uint32_t Addr){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	DPM_Disks[Index].Reserved = (void*)Addr;
}

/**
 * @brief [DPM] Считывание данных с диска
 *
 * @param Letter - Буква для считывания
 * @param Offset - Отступ для считывания
 * @param Size - Кол-во байт данных для считывания
 * @param Buffer - Буфер куда будет идти запись
 * 
 * @return Кол-во прочитанных байт
 */
size_t dpm_read(char Letter, size_t Offset, size_t Size, void *Buffer){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	if (DPM_Disks[Index].Ready == 0 || DPM_Disks[Index].Status == 0)
		return DPM_ERROR_NOT_READY;

	if (DPM_Disks[Index].AddrMode == 2){
		// Диск является частью ОЗУ, поэтому мы просто копируем данные оттуда
        if (dpm_debug)qemu_log("[DPM] [2] An attempt to read data in 'Disk %c' from position %x to the number of %d bytes.", Index+65, DPM_Disks[Index].Point+Offset, Size);
		memcpy(Buffer, (void *) (DPM_Disks[Index].Point + Offset), Size);

		return Size;
	} else if (DPM_Disks[Index].AddrMode == 3){
		// Режим 3, предполагает что вы указали функцию для чтения и записи с диска
        if (dpm_debug)qemu_log("[DPM] [3] An attempt to read data in 'Disk %c' from position %x to the number of %d bytes.", Index+65, DPM_Disks[Index].Point+Offset, Size);
		if (DPM_Disks[Index].Read == 0){
            qemu_err("[DPM] [3] Function 404");
            return 0;
        }
		
		return DPM_Disks[Index].Read(Index,Offset,Size,Buffer);
	} else {
        if (dpm_debug)qemu_log("[DPM] This functionality has not been implemented yet.");
	}

	return DPM_ERROR_NO_READ;
}

/**
 * @brief [DPM] Запись данных на диск
 *
 * @param Letter - Буква
 * @param size_t Offset - Отступ
 * @param size_t Size - Кол-во байт данных для записи
 * @param Buffer - Буфер откуда будет идти запись
 * 
 * @return size_t - Кол-во записанных байт
 */
size_t dpm_write(char Letter, size_t Offset, size_t Size, char* Buffer){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	if (DPM_Disks[Index].Ready == 0 || DPM_Disks[Index].Status == 0)
		return DPM_ERROR_NOT_READY;

	if (DPM_Disks[Index].AddrMode == 2){
		// Диск является частью ОЗУ, поэтому мы просто копируем данные туда
		// Опастна! Если не знать, что делать!
        if (dpm_debug)qemu_log("[DPM] [2] An attempt to write data in 'Disk %c' from position %x to the number of %d bytes.", Index+65, DPM_Disks[Index].Point+Offset, Size);
		memcpy((void *) (DPM_Disks[Index].Point + Offset), Buffer, Size);

		return Size;
	} else if (DPM_Disks[Index].AddrMode == 3){
		// Режим 3, предполагает что вы указали функцию для чтения и записи с диска
        if (dpm_debug)qemu_log("[DPM] [3] An attempt to write data in 'Disk %c' from position %x to the number of %d bytes.", Index+65, DPM_Disks[Index].Point+Offset, Size);
        if (DPM_Disks[Index].Write == 0){
            qemu_err("[DPM] [3] Function 404");
            return 0;
        }
		return DPM_Disks[Index].Write(Index,Offset,Size,Buffer);
	} else {
        if (dpm_debug)qemu_log("[DPM] This functionality has not been implemented yet.");
	}

	return DPM_ERROR_NO_READ;
}

int dpm_unmount(char Letter, bool FreeReserved){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	if (DPM_Disks[Index].Ready == 0) return 0;

	DPM_Disks[Index].Ready = 0;

	//memcpy(DPM_Disks[Index].Name, NULL, sizeof(NULL));
	//memcpy(DPM_Disks[Index].Serial, NULL, sizeof(NULL));
	//memcpy(DPM_Disks[Index].FileSystem, NULL, sizeof(NULL));
	DPM_Disks[Index].Status = 0;
	DPM_Disks[Index].Size = 0;
	DPM_Disks[Index].Sectors = 0;
	DPM_Disks[Index].SectorSize = 0;
	DPM_Disks[Index].AddrMode = 0;
	DPM_Disks[Index].Point = 0;

	if (FreeReserved && DPM_Disks[Index].Reserved != 0)	{
		kfree(DPM_Disks[Index].Reserved);
	}
	return 1;
}

/**
 * @brief [DPM] Регистрация дискового раздела
 *
 * @param Letter - Буква для регистрации
 * 
 * @return int - Результат регистрации
 */
int dpm_reg(char Letter, char* Name, char* FS, int Status, size_t Size, size_t Sectors, size_t SectorSize, int AddrMode, char* Serial, void *Point){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	if (DPM_Disks[Index].Ready == 1){
		qemu_warn("[DPM] Warning! This letter is already occupied, and an attempt will be made to search for a free letter.");
		Index = dpm_searchFreeIndex(Index);
		if (Index == DPM_ERROR_NO_MOUNT){
			qemu_warn("[DPM] Sorry, but the disk could not be registered because there is no free letter. Delete the extra devices and try again.");
			return DPM_ERROR_NO_MOUNT;
		}
		qemu_log("[DPM] The drive was assigned the letter '%c'",Index+65);
	}
	
	DPM_Disks[Index].Ready = 1;

	memcpy(DPM_Disks[Index].Name,Name,strlen(Name));
	memcpy(DPM_Disks[Index].Serial,Serial,strlen(Serial));
	memcpy(DPM_Disks[Index].FileSystem,FS,strlen(FS));
	DPM_Disks[Index].Status = Status;
	DPM_Disks[Index].Size = Size;
	DPM_Disks[Index].Sectors = Sectors;
	DPM_Disks[Index].SectorSize = SectorSize;
	DPM_Disks[Index].AddrMode = AddrMode;
	DPM_Disks[Index].Point = Point;

	qemu_log("[DPM] Disk '%c' is registered!",Index+65);
	qemu_log("  |-- Name: %s",DPM_Disks[Index].Name);
	qemu_log("  |-- Serial: %s",DPM_Disks[Index].Serial);
	qemu_log("  |-- FileSystem: %s",DPM_Disks[Index].FileSystem);
	qemu_log("  |-- Status: %d",DPM_Disks[Index].Status);
	qemu_log("  |-- Size: %d",DPM_Disks[Index].Size);
	qemu_log("  |-- Sectors: %d",DPM_Disks[Index].Sectors);
	qemu_log("  |-- SectorSize: %d",DPM_Disks[Index].SectorSize);
	qemu_log("  |-- AddrMode: %d",DPM_Disks[Index].AddrMode);
	qemu_log("  |-- Point: %x",DPM_Disks[Index].Point);

	return Index;
}

void dpm_FileSystemUpdate(char Letter, char* FileSystem){
    Letter -= 65;

    size_t index = (Letter > 32 ? Letter - 32 : Letter);
    index = (Letter < 0 || Letter > 25 ? 0 : Letter);

    size_t c = strlen(FileSystem);
    memset(DPM_Disks[index].FileSystem, 0, 64);   /// Зачищаем данные
    memcpy(DPM_Disks[index].FileSystem, FileSystem, (c > 64 || c == 0?64:c)); /// Пишем данные
}


void dpm_LabelUpdate(char Letter, char* Label){
    Letter -= 65;

    size_t index = (Letter > 32 ? Letter - 32 : Letter);
    index = (Letter < 0 || Letter > 25 ? 0 : Letter);

    size_t c = strlen(Label);
    memset(DPM_Disks[index].Name, 0, 128);   /// Зачищаем данные
    memcpy(DPM_Disks[index].Name, Label, (c > 128 || c == 0?128:c));/// Пишем данные
}

size_t dpm_disk_size(char Letter){
    Letter -= 65;

    size_t index = (Letter > 32 ? Letter - 32 : Letter);
    index = (Letter < 0 || Letter > 25 ? 0 : Letter);

    return (DPM_Disks[index].Size > 0?DPM_Disks[index].Size:0);

}

DPM_Disk dpm_info(char Letter){
	Letter -= 65;

	size_t index = (Letter > 32 ? Letter - 32 : Letter);
	index = (Letter < 0 || Letter > 25 ? 0 : Letter);

	return DPM_Disks[index];
}

