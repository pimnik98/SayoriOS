/**
 * @file drv/fs/fsm.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief File System Manager (Менеджер файловых систем)
 * @version 0.3.5
 * @date 2023-10-16
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <io/ports.h>
#include <fs/fsm.h>
#include <lib/php/pathinfo.h>
#include "lib/php/str_contains.h"
#include "lib/php/explode.h"
#include "lib/sprintf.h"
#include "mem/vmm.h"

#include "drv/disk/dpm.h"

FSM G_FSM[255] = {0};
int C_FSM = 0;
bool fsm_debug = false;

size_t fsm_DateConvertToUnix(FSM_TIME time) {
    uint32_t seconds_per_day = 24 * 60 * 60;
    size_t unix_time = 0;

    // Подсчет количества дней с начала Unix эпохи
    for (uint32_t year = 1970; year < time.year; year++) {
        uint32_t days_in_year = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 366 : 365;
        unix_time += days_in_year * seconds_per_day;
    }

    int8_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (time.year % 4 == 0 && (time.year % 100 != 0 || time.year % 400 == 0)) {
        month_days[1] = 29;
    }

    // Добавление количества дней в текущем году
    for (uint32_t month = 0; month < time.month - 1; month++) {
        unix_time += month_days[month] * seconds_per_day;
    }

    // Добавление количества дней в текущем месяце
    unix_time += (time.day - 1) * seconds_per_day;

    // Добавление компонентов времени
    unix_time += time.hour * 3600 + time.minute * 60 + time.second;

    return unix_time;
}


void fsm_convertUnix(uint32_t unix_time, FSM_TIME* time) {
	if (fsm_debug) qemu_log("[FSM] Convert unix: %d",unix_time);
    uint32_t seconds_per_day = 24 * 60 * 60;
    uint32_t days = unix_time / seconds_per_day;
    
    uint32_t years = 1970;
    uint32_t month, day;
    while (1) {
        uint32_t days_in_year = (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0)) ? 366 : 365;
        if (days < days_in_year) {
            break;
        }
        days -= days_in_year;
        years++;
    }
    
    int8_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0)) {
        month_days[1] = 29;
    }
    
    uint32_t year_day = days;
    for (month = 0; month < 12; month++) {
        if (year_day < month_days[month]) {
            break;
        }
        year_day -= month_days[month];
    }
    day = year_day + 1;
    
    // Вычисляем компоненты времени
    uint32_t seconds = unix_time % seconds_per_day;
    uint32_t hour = seconds / 3600;
    uint32_t minute = (seconds % 3600) / 60;
    uint32_t second = seconds % 60;
	//qemu_log("%d.%d.%d %d:%d:%d",years, month, day, hour, minute, second);

	time->year = years;
	time->month = month;
	time->day = day;
	time->hour = hour;
	time->minute = minute;
	time->second = second;
	//memcpy();
	//qemu_log("%d.%d.%d %d:%d:%d", time->year, time->month, time->day, time->hour, time->minute, time->second);
}

char* fsm_timePrintable(FSM_TIME time){
	char* btime = 0;

	asprintf(&btime, "%04d.%02d.%02d %02d:%02d:%02d",
		time.year,
		time.month,
		time.day,
		time.hour,
		time.minute,
		time.second);

	return btime;
}

int fsm_isPathToFile(const char* Path,const char* Name){
	char* zpath = pathinfo(Name, PATHINFO_DIRNAME);					///< Получаем родительскую папку элемента
	char* bpath = pathinfo(Name, PATHINFO_BASENAME);				///< Получаем имя файла (или пустоту если папка)
	bool   isCheck1 = strcmpn(zpath,Path);				///< Проверяем совпадение путей
	bool   isCheck2 = strlen(bpath) == 0;				///< Проверяем, что путе нет ничего лишнего (будет 0, если просто папка)
	bool   isCheck3 = str_contains(Name, Path);	///< Проверяем наличие, вхождения путя
	size_t c1 = str_cdsp2(Path,'\\');
	size_t c2 = str_cdsp2(Name,'\\');
	size_t c3 = str_cdsp2(Path,'/');
	size_t c4 = str_cdsp2(Name,'/');
	
	bool   isCheck4 = ((c2 - c1) == 1) && (c4 == c3);
	bool   isCheck5 = ((c4 - c3) == 1) && (c2 == c1);
/*	
	qemu_log("[%d] [%d] [%d] [%d] [%d] %s", isCheck1, isCheck2, isCheck3, isCheck4, isCheck5, Name);
	qemu_log(" |--- %d == %d",c1,c2);
	qemu_log(" |--- %d == %d",c3,c4);*/
	bool isPassed = ((isCheck1 && !isCheck2 && isCheck3) || (!isCheck1 && isCheck2 && isCheck3 && (isCheck4 || isCheck5)));

	kfree(zpath);
	kfree(bpath);

	return isPassed;
}

int fsm_getIDbyName(const char* Name){
	for (int i = 0; i < C_FSM; i++){
		if (!strcmpn(G_FSM[i].Name,Name)) continue;
		return i;
	}
	return -1;
}

void fsm_dump(FSM_FILE file){
	qemu_log("  |--- Ready  : %d",file.Ready);
	qemu_log("  |--- Name   : %s",file.Name);
	qemu_log("  |--- Path   : %s",file.Path);
	qemu_log("  |--- Mode   : %d",file.Mode);
	qemu_log("  |--- Size   : %d",file.Size);
	qemu_log("  |--- Type   : %d",file.Type);
	qemu_log("  |--- Date   : %d",file.LastTime.year);
}

size_t fsm_read(int FIndex, char DIndex, const char* Name, size_t Offset, size_t Count, void* Buffer){
    if (fsm_debug) qemu_log("[FSM] [READ] F:%d | D:%d | N:%d | O:%d | C:%d",FIndex,DIndex,Name,Offset,Count);
	if (G_FSM[FIndex].Ready == 0) return 0;
    if (fsm_debug) qemu_log("[FSM] [READ] GO TO DRIVER");
	return G_FSM[FIndex].Read(DIndex,Name,Offset, Count, Buffer);
}


int fsm_create(int FIndex, char DIndex, const char* Name, int Mode){
	if (G_FSM[FIndex].Ready == 0) return 0;
	return G_FSM[FIndex].Create(DIndex,Name,Mode);
}


int fsm_delete(int FIndex, const char DIndex, const char* Name, int Mode){
	if (G_FSM[FIndex].Ready == 0)
		return 0;

	return G_FSM[FIndex].Delete(DIndex,Name,Mode);
}

size_t fsm_write(int FIndex, const char DIndex, const char* Name, size_t Offset, size_t Count, void* Buffer){
	if (G_FSM[FIndex].Ready == 0)
		return 0;

	return G_FSM[FIndex].Write(DIndex,Name,Offset, Count, Buffer);
}

FSM_FILE fsm_info(int FIndex,const char DIndex, const char* Name){
    if (fsm_debug) qemu_log("[FSM] [INFO] F:%d | D:%d | N:%s",FIndex,DIndex,Name);
	if (G_FSM[FIndex].Ready == 0){
        if (fsm_debug) qemu_log("[FSM] [INFO] READY == 0");
		return (FSM_FILE){};
	}
    if (fsm_debug) qemu_log("[FSM] [INFO] GO TO GFSM");
	return G_FSM[FIndex].Info(DIndex,Name);
}

FSM_DIR* fsm_dir(int FIndex,const char DIndex, const char* Name){
    if (fsm_debug) qemu_log("[FSM] [DIR] F:%d | D:%d | N:%s",FIndex,DIndex,Name);

	if (G_FSM[FIndex].Ready == 0){
        if (fsm_debug) qemu_log("[FSM] [INFO] READY == 0");
		FSM_DIR* dir = kmalloc(sizeof(FSM_DIR));
		return dir;
	}

	return G_FSM[FIndex].Dir(DIndex, Name);
}

void fsm_reg(const char* Name,int Splash,fsm_cmd_read_t Read, fsm_cmd_write_t Write, fsm_cmd_info_t Info, fsm_cmd_create_t Create, fsm_cmd_delete_t Delete, fsm_cmd_dir_t Dir, fsm_cmd_label_t Label, fsm_cmd_detect_t Detect){
	G_FSM[C_FSM].Ready = 1;
	G_FSM[C_FSM].Splash = Splash;
	G_FSM[C_FSM].Read = Read;
	G_FSM[C_FSM].Write = Write;
	G_FSM[C_FSM].Info = Info;
	G_FSM[C_FSM].Create = Create;
	G_FSM[C_FSM].Delete = Delete;
	G_FSM[C_FSM].Dir = Dir;
	G_FSM[C_FSM].Label = Label;
	G_FSM[C_FSM].Detect = Detect;
	memcpy(G_FSM[C_FSM].Name,Name,strlen(Name));
	qemu_log("[FSM] Registration of the '%s' file system driver is complete.",Name);
	C_FSM++;
}

int fsm_getMode(int FIndex){
	if (G_FSM[FIndex].Ready == 0) return 0;
	
	return G_FSM[FIndex].Splash;
}


void fsm_dpm_update(char Letter){
    char BLANK[128] = {'U','n','k','n','o','w','n',0};
    if (Letter == -1){
        // Global update
        for(int i = 0; i < 26; i++){
            int DISKID = i + 65;
            dpm_LabelUpdate(DISKID, BLANK);
            dpm_FileSystemUpdate(DISKID, BLANK);
            DPM_Disk dpm = dpm_info(DISKID);
            if (dpm.Ready != 1) continue;
            for(int f = 0; f < C_FSM; f++){
                qemu_note("[FSM] [DPM] >>> Disk %c | Test %s", DISKID, G_FSM[f].Name);
                int detect = G_FSM[f].Detect(DISKID);
                if (detect != 1) continue;
                char* lab_test = kcalloc(1,129);
                G_FSM[f].Label(DISKID,lab_test);
                dpm_LabelUpdate(DISKID, lab_test);
                dpm_FileSystemUpdate(DISKID, G_FSM[f].Name);
                qemu_note("                       | Label: %s", lab_test);
                kfree(lab_test);
                break;
            }
        }
    } else {
        // Personal update
        int DISKID  = Letter;
        dpm_LabelUpdate(DISKID, BLANK);
        dpm_FileSystemUpdate(DISKID, BLANK);
        for(int f = 0; f < C_FSM; f++){
            qemu_note("[FSM] [DPM] >>> Disk %c | Test %s", DISKID, G_FSM[f].Name);
            int detect = G_FSM[f].Detect(DISKID);

            if (detect != 1)
                continue;

            char* lab_test = kcalloc(1,129);
            G_FSM[f].Label(DISKID, lab_test);
            dpm_LabelUpdate(DISKID, lab_test);
            dpm_FileSystemUpdate(DISKID, G_FSM[f].Name);
            qemu_note("[FSM] [DPM] ^^^ Disk %c | Label: %s", DISKID, lab_test);
            kfree(lab_test);
            break;
        }
    }
}
