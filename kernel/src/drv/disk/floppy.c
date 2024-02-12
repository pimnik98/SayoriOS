/**
 * @file drv/disk/floppy.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер Floppy
 * @version 0.3.5
 * @date 2023-07-25
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/


#include <drv/disk/floppy.h>
#include <io/ports.h>
#include "sys/timer.h"
#include "mem/vmm.h"
#include "drv/disk/dpm.h"
#include "io/tty.h"
#include "sys/isr.h"

bool _FloppyDebug = false;

floppy_t floppy_data[2] = {0};
// standard IRQ number for floppy controllers
//static const int floppy_irq = 6;
static const char * drive_types[8] = {
    "none",
    "360kB 5.25\"",
    "1.2MB 5.25\"",
    "720kB 3.5\"",

    "1.44MB 3.5\"",
    "2.88MB 3.5\"",
    "unknown type",
    "unknown type"
};
static const char FLOPPY_DMABUFA[floppy_dmalen] __attribute__((aligned(0x8000)));	///< Буфер дискеты 1
//static const char FLOPPY_DMABUFB[floppy_dmalen] __attribute__((aligned(0x8000)));	///< Буфер дискеты 2
volatile bool interrupted = false;

floppy_t Floppy(int device){
	return floppy_data[device];
}

void irq_waitFloppy() {
	while(interrupted){
		sleep_ms(10);
	}
}

static void irqFloppy(__attribute__((unused)) registers_t regs){
	interrupted = 0;
}

int _FloppyError(int Device,int Error){
	if (_FloppyDebug) qemu_log("FD%c ERROR! Code: %d",(Device==FDB?'B':'A'),Error);
	floppy_data[Device].LastErr = Error;
	return (Error*(-1));
}

static void _FloppyROOT(int device,int offset,int cmd){
	size_t addr = Floppy(device).Addr + offset;

	if (_FloppyDebug)
		qemu_log("   [FD%c] ROOT | Addr: %x | Offset: %x | Input: %x | Cmd: %x",(device?'B':'A'),Floppy(device).Addr,offset,addr,cmd);

	outb(addr, cmd);

	kfree((void*)addr);
}

static void _FloppyDMA(FloppyMode dir) {

    union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

    a.l = (unsigned) &FLOPPY_DMABUFA;
  	c.l = (unsigned) FLOPPY_144_CYLINDER_SIZE - 1;  // -1 because of DMA counting

    if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)) {
        qemu_log("_FloppyDMA: static buffer problem\n");
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case FLOPPY_READ:  mode = 0x46; break;
        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case FLOPPY_WRITE: mode = 0x4a; break;
        default: qemu_log("_FloppyDMA: invalid direction");
                 return; // not reached, please "mode user uninitialized"
    }
	outb(0x0a, 0x06);		///< Выдаем маску для тян
    outb(0x0c, 0xff);		///< Сброс триггера
    outb(0x04, a.b[0]);		///<   - Младший байт адреса
    outb(0x04, a.b[1]);		///<   - Старший байт адреса
    outb(0x81, a.b[2]);		///<  Регистрация внешней страницы
    outb(0x0c, 0xff);		///<  Сброс триггера
    outb(0x05, c.b[0]);		///<   - Считать младший байт адреса
    outb(0x05, c.b[1]);		///<   - Считать старший байт адреса
    outb(0x0b, mode);		///<  Установить новый режим
    outb(0x0a, 0x02);		///<  Забираем маску у тян
}

void _FloppyMotor(int device, int status){
	if (status == 2){
		// Заглушить мотор
		_FloppyROOT(device,FLOPPY_DIGITAL_OUTPUT_REGISTER,0x0C);
		//outb(Floppy(device).Addr + FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0c);
    	floppy_data[device].Motor = 0;
		if (_FloppyDebug) qemu_log(" [FD%c] Motor Offline",(device?'B':'A'));
	} else if (status == 1){
		if (_FloppyDebug) qemu_log(" [FD%c] Motor Online",(device?'B':'A'));
		// Включить мотор
		if (Floppy(device).Motor == 0){
			//qemu_log("[FD%c] Motor | Addr: %x | DOR: %x ",(device?'B':'A'),Floppy(device).Addr,adrcmd);
			if (device == 0) _FloppyROOT(device,FLOPPY_DIGITAL_OUTPUT_REGISTER,0x1C);
			if (device == 1) _FloppyROOT(device,FLOPPY_DIGITAL_OUTPUT_REGISTER,0x2D);
			sleep_ms(500);
		}
		floppy_data[device].Motor = 1;
	} else {
		// Перевести мотор в режим ожидания
		if (_FloppyDebug) qemu_log(" [FD%c] Motor Waiting",(device?'B':'A'));
		if (Floppy(device).Motor == 2){
			if (_FloppyDebug) qemu_log(" [FD%c] Strange, fd motor-state already waiting..",(device?'B':'A'));
		}
		floppy_data[device].Motor = 2;		
		floppy_data[device].Ticks = 300;
	}
}

int _FloppyCMD(int device, char cmd){
	if (_FloppyDebug) qemu_log("   [FD%c] Addr: %x | Command: %x",(device?'B':'A'),Floppy(device).Addr,cmd);
	for(int i = 0; i < 600; i++) {
        sleep_ms(1); // sleep 10 ms
        if(0x80 & inb(Floppy(device).Addr + FLOPPY_MAIN_STATUS_REGISTER)) {
			_FloppyROOT(device,FLOPPY_DATA_FIFO,cmd);
			//outb(Floppy(device).Addr + FLOPPY_DATA_FIFO, cmd);
			return 1;
        }
    }
	return _FloppyError(device,FLOPPY_ERROR_CMD);
}

unsigned char _FloppyData(int device){
    for(int i = 0; i < 600; i++) {
        sleep_ms(1); // sleep 10 ms
        if(0x80 & inb(Floppy(device).Addr + FLOPPY_MAIN_STATUS_REGISTER)) {
            return inb(Floppy(device).Addr + FLOPPY_DATA_FIFO);
        }
    }
    qemu_log("floppy_read_data: timeout");
    return 0; // not reached
}


void _FloppyCI(int device,int *st0, int *cyl){
	_FloppyCMD(device, CMD_SENSE_INTERRUPT);

    *st0 = _FloppyData(device);
    *cyl = _FloppyData(device);
}


int _FloppyCalibrate(int device){
    int i, st0, cyl = -1; // set to bogus cylinder
	_FloppyMotor(device,1);
	for(i = 0; i < 10; i++) {
		if (_FloppyDebug) qemu_log(" [FD%c] Attempt Calibrate %d",(device?'B':'A'),i);
		interrupted = false;
        _FloppyCMD(device, CMD_RECALIBRATE);
        _FloppyCMD(device, 0); // argument is drive, we only support 0
		irq_waitFloppy();
        _FloppyCI(device, &st0, &cyl);
        if(st0 & 0xC0) continue;
        if(!cyl) { // found cylinder 0 ?
            _FloppyMotor(device, 0);
            return 0;
        }
    }
    if (_FloppyDebug) qemu_log("floppy_calibrate: 10 retries exhausted\n");
    _FloppyMotor(device, 0);
    return _FloppyError(device,FLOPPY_ERROR_CALIBRATE);
}

int _FloppyReset(int device){
	interrupted = false;
	_FloppyROOT(device,FLOPPY_DIGITAL_OUTPUT_REGISTER,0x00);
	_FloppyROOT(device,FLOPPY_DIGITAL_OUTPUT_REGISTER,0x0C);
	irq_waitFloppy(); // sleep until interrupt occurs
	{
    	int st0, cyl; // ignore these here...
    	_FloppyCI(device, &st0, &cyl);
  	}	
	_FloppyROOT(device,FLOPPY_CONFIGURATION_CONTROL_REGISTER,0x00);
	_FloppyCMD(device, CMD_SPECIFY);
    _FloppyCMD(device, 0xdf); /* steprate = 3ms, unload time = 240ms */
    _FloppyCMD(device, 0x02); /* load time = 16ms, no-DMA = 0 */

    // it could fail...
    if(_FloppyCalibrate(device) != 0)
		return _FloppyError(device,FLOPPY_ERROR_RESET);
	else
		return 0;
}



int _FloppySeek(int device, unsigned cyli, int head){
	int i, st0, cyl = -1;
	_FloppyMotor(device, 1);
	for(i = 0; i < 10; i++) {
		if (_FloppyDebug) qemu_log("  [FD%c] Attempt Seek %d | Cyli: %c | Head: %d",(device?'B':'A'),i,cyli,head);
		interrupted = false;
        _FloppyCMD(device, CMD_SEEK);
        _FloppyCMD(device, head<<2);
        _FloppyCMD(device, cyli & 0xFF);
		irq_waitFloppy();
        _FloppyCI(device, &st0, &cyl);
        if(st0 & 0xC0) continue;
        if(cyl == cyli) {
            _FloppyMotor(device, 0);
            return 0;
        }
	}
	qemu_log("floppy_seek: 10 retries exhausted\n");
    _FloppyMotor(device, 0);
	_FloppyError(device,FLOPPY_ERROR_SEEK);
    return -1;
}

int _FloppyTrack(int device, unsigned cyl, FloppyMode dir) {
    unsigned char cmd;
    static const int flags = 0xC0;
    switch(dir) {
        case FLOPPY_READ:
            cmd = CMD_READ_DATA | flags;
            break;
        case FLOPPY_WRITE:
            cmd = CMD_WRITE_DATA | flags;
            break;
        default:
			
            qemu_log("floppy_do_track: invalid direction");
            return 0; // not reached, but pleases "cmd used uninitialized"
    }
	/// FDB не проходит проверку SEEK
    if(_FloppySeek(device, cyl, 0)) return -1;
    if(_FloppySeek(device, cyl, 1)) return -1;

    int i;
    for(i = 0; i < 20; i++) {
        _FloppyMotor(device, 1);
        _FloppyDMA(dir);
        sleep_ms(100); // give some time (100ms) to settle after the seeks
        _FloppyCMD(device, cmd);  // set above for current direction
        _FloppyCMD(device, 0);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        _FloppyCMD(device, cyl);  // cylinder
        _FloppyCMD(device, 0);    // first head (should match with above)
        _FloppyCMD(device, 1);    // first sector, strangely counts from 1
        _FloppyCMD(device, 2);    // bytes/sector, 128*2^x (x=2 -> 512)
        _FloppyCMD(device, 18);   // number of tracks to operate on
        _FloppyCMD(device, 0x1b); // GAP3 length, 27 is default for 3.5"
        _FloppyCMD(device, 0xff); // data length (0xff if B/S != 0)

		irq_waitFloppy(); // don't SENSE_INTERRUPT here!

        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = _FloppyData(device);
        st1 = _FloppyData(device);
        st2 = _FloppyData(device);
        rcy = _FloppyData(device);
        rhe = _FloppyData(device);
        rse = _FloppyData(device);
        bps = _FloppyData(device);

		(void)rse;
    	(void)rhe;
    	(void)rcy;

		if (st0 & 0xC0)

        if(st0 & 0xC0) {
			if ((st0 >> 6) == 0) return _FloppyError(device,FLOPPY_ERROR_NOSUPPORT);
			if ((st0 >> 6) == 1) return _FloppyError(device,FLOPPY_ERROR_NOSUPPORT);
			if ((st0 >> 6) == 2) return _FloppyError(device,FLOPPY_ERROR_CMD_UNK);
			if ((st0 >> 6) == 3) return _FloppyError(device,FLOPPY_ERROR_NOREADY);
        }
        if(st1 & 0x80) return _FloppyError(device,FLOPPY_ERROR_EOL);
        if(st0 & 0x08) return _FloppyError(device,FLOPPY_ERROR_NOREADY);
        if(st1 & 0x20) return _FloppyError(device,FLOPPY_ERROR_CRC);
        if(st1 & 0x10) return _FloppyError(device,FLOPPY_ERROR_CONTROLLER);
        if(st1 & 0x04) return _FloppyError(device,FLOPPY_ERROR_NODATA);
        if((st1|st2) & 0x01) return _FloppyError(device,FLOPPY_ERROR_NOADDR);
        if(st2 & 0x40) return _FloppyError(device,FLOPPY_ERROR_DELADDR);
        if(st2 & 0x20) return _FloppyError(device,FLOPPY_ERROR_CRC);
        if(st2 & 0x10) return _FloppyError(device,FLOPPY_ERROR_CYLINDER);
        if(st2 & 0x04) return _FloppyError(device,FLOPPY_ERROR_UPD765);
        if(st2 & 0x02) return _FloppyError(device,FLOPPY_ERROR_CYLINDER);
        if(bps != 0x2) return _FloppyError(device,FLOPPY_ERROR_512B);
        if(st1 & 0x02) return _FloppyError(device,FLOPPY_ERROR_WRITE);
		_FloppyMotor(device, 0);

		// What a hell?
        return 0;
    }

    qemu_log("floppy_do_sector: 20 retries exhausted\n");
    _FloppyMotor(device, 0);
	return _FloppyError(device,FLOPPY_ERROR_TRACK);

}

void addr_2_coff(uint32_t addr, uint16_t* cyl, uint32_t* offset, uint32_t* size) {
  if (offset) *offset = addr % FLOPPY_144_CYLINDER_SIZE;
  if (size) *size = FLOPPY_144_CYLINDER_SIZE - (addr % FLOPPY_144_CYLINDER_SIZE);
  if (cyl) *cyl = addr / FLOPPY_144_CYLINDER_SIZE;
}



int _FloppyCache(int device,FloppyMode mode,unsigned int addr,unsigned int* offset,unsigned int* size){
	//if (Floppy(device).Status != 1) return _FloppyError(device,FLOPPY_ERROR_NOREADY);
	//qemu_log("[FD%c] Cache | Addr: %d | Off: %d | Size: %d",(device==FDB?'B':'A'),addr,offset,size);
	uint16_t cyl;
	addr_2_coff(addr, &cyl, offset, size);
	if (mode == FLOPPY_READ && cyl == floppy_data[device].Cyr) return 0;
	int ret = _FloppyTrack(device, cyl, mode);
	if (ret >= 0) floppy_data[device].Cyr = cyl;
	return ret;
}

/** 
  * @brief [Floppy] Чтение данных на устройство
  *
  * @param dst - Данные, откуда выполнить чтение
  * @param addr - Адрес, откуда читать (0 - в начало)
  * @param size - Сколько считать данных
  *
  * @return int Количество записанных байт
  */
size_t _FloppyRead(int device,char* dst,uint32_t addr,uint32_t size){
	if (Floppy(device).Status != 1) return _FloppyError(FDA,FLOPPY_ERROR_NOREADY);
	//qemu_log("[FD%c] Read | Addr: %d | Size: %d",(device==FDB?'B':'A'),addr,size);
	floppy_data[device].LastErr = 0;
	uint32_t offset=0, ws=0;
	size_t ds = 0;
	int ret;
	while (size > ds){
		ret = _FloppyCache(device,FLOPPY_READ,addr+ds,&offset,&ws);
		if (ret < 0) return ret;
		if (ws > size - ds) ws = size - ds;
		memcpy(dst + ds, (void*) &FLOPPY_DMABUFA[offset], ws);
		ds += ws;
	}
	return ds;
}

size_t _FloppyWrite(int device,const char* dst,uint32_t addr,uint32_t size){
	if (Floppy(device).Status != 1) return _FloppyError(FDA,FLOPPY_ERROR_NOREADY);
	//qemu_log("[FD%c] Write | Addr: %d | Size: %d",(device==FDB?'B':'A'),addr,size);
	
	uint32_t offset=0, ws=0;
	size_t ds = 0;
	int ret;
	while (size > ds){
		ret = _FloppyCache(device,FLOPPY_READ,addr+ds, &offset, &ws);
		if (ret < 0) return ret;
		if (ws > size - ds) ws = size - ds;
		memcpy((void*) &FLOPPY_DMABUFA[offset],dst+ds,ws);
		ret = _FloppyCache(device,FLOPPY_WRITE,addr+ds,nullptr,nullptr);
		if (ret < 0) return ret;
		ds += ws;
	}
	return ds;
}

void _FloppyServiceA(){
	outb(0x70, 0x10);
	unsigned drives = inb(0x71);
	int DiskA = drives >> 4;
	if (DiskA != floppy_data[FDA].Type){
		// Выполняем обслуживание Диска А, так как диск извлечен.
		floppy_data[FDA].Type = DiskA;
		floppy_data[FDA].Status = (floppy_data[FDA].Type == 4?1:0);
		dpm_unmount('A', false);
		dpm_reg('A',"Disk A","Unknown",floppy_data[FDA].Status,0,0,0,3,"FLOP-PYDA",0);
	}
}

/**
  * @brief Автоматическое обновление данных о FD
  */
void _FloppyCheck(){
	outb(0x70, 0x10);
	unsigned drives = inb(0x71);
	if (Floppy(FDA).Type != drives >> 4){
		floppy_data[FDA].Type = drives >> 4;
		if (_FloppyDebug) qemu_log("[FDA] Change disk type: %d",Floppy(FDA).Type);
	}
	if (Floppy(FDB).Type != (drives & 0xf)){
		floppy_data[FDB].Type = (drives & 0xf);
		if (_FloppyDebug) qemu_log("[FDB] Change disk type: %d",Floppy(FDB).Type);
	}
	if (Floppy(FDA).Type == 0) {
		floppy_data[FDA].Status = _FloppyError(FDA,FLOPPY_ERROR_NOREADY);
	} else if (Floppy(FDA).Type != 4){
		floppy_data[FDA].Status = _FloppyError(FDA,FLOPPY_ERROR_NOSUPPORT);
	} else {
		floppy_data[FDA].Status = 1;
	}

	if (Floppy(FDB).Type == 0) {
		floppy_data[FDB].Status = _FloppyError(FDB,FLOPPY_ERROR_NOREADY);
	} else if (Floppy(FDB).Type != 4){
		floppy_data[FDB].Status = _FloppyError(FDB,FLOPPY_ERROR_NOSUPPORT);
	} else {
		floppy_data[FDB].Status = 1;
	}

	
	// Обновление имени устройства
	
	char fda_fs[6],fdb_fs[6] = {0};
	char fda_label[12],fdb_label[12] = {0};
	char fs_fat12[6] = {'F','A','T','1','2',0};
//	char fs_smfs10[8] = {'S','M','F','S','1','.','0',0};
	int read = -1;
	if (Floppy(FDA).Status == 1){
		//qemu_log("[FD%c] U",(device==FDB?'B':'A'));
		read = Floppy(FDA).Read(FDA,fda_fs,54,5);
		if (_FloppyDebug) qemu_log("[FDA] Read File System => (%d) '%s'",read,fda_fs);
		if (Floppy(FDA).LastErr != 0){
			floppy_data[FDA].Status = FLOPPY_ERROR_NOREADY;
			if (_FloppyDebug) qemu_log("[FDA] No ready drive!");
			floppy_data[FDA].LastErr = 0;
		}
		read = Floppy(FDA).Read(FDA,fda_fs,54,8);
		if (Floppy(FDA).LastErr == 0 && strcmp(fs_fat12, fda_fs) == 0){
			if (_FloppyDebug) qemu_log("[FDA] File System: FAT12");
			Floppy(FDA).Read(FDA, fda_label, 43, 11);
			fda_label[11] = 0;
    		memcpy((void*) floppy_data[FDA].Name, fda_label, 12);
    		memcpy((void*) floppy_data[FDA].FileSystem, fda_fs, 6);
			if (_FloppyDebug) qemu_log("[FDA] Label: %s",floppy_data[FDA].Name);
			
		}
	}
	if (Floppy(FDB).Status == 1){
		read = Floppy(FDB).Read(FDB,fdb_fs,54,5);
		if (_FloppyDebug) qemu_log("[FDB] Read File System => (%d) '%s'",read,fdb_fs);
		if (Floppy(FDB).LastErr != 0){
			floppy_data[FDB].Status = FLOPPY_ERROR_NOREADY;
			if (_FloppyDebug) qemu_log("[FDB] No ready drive!");
			floppy_data[FDB].LastErr = 0;
		} else if (strcmp(fs_fat12,fdb_fs) == 0){
			if (_FloppyDebug) qemu_log("[FDB] File System: FAT12");
			Floppy(FDB).Read(FDB,fdb_label,43,11);
			fdb_label[11] = 0;
			memcpy((void*) floppy_data[FDB].Name, fdb_label, 12);
			if (_FloppyDebug) qemu_log("[FDB] Label: %s",floppy_data[FDB].Name);
    		memcpy((void*) floppy_data[FDB].FileSystem, fdb_fs, 6);
			
		}
	}
	//kfree(fs_fat12);
	
}

size_t _FloppyDPMWriteA(size_t Disk, size_t Offset, size_t Size, void* Buffer){
	qemu_log("[DPM] Floppy A Write! Offset: %d | Size: %d",Offset, Size);
	if (Floppy(0).Status != 1) return 0;
	//qemu_log("[FD%c] Write | Addr: %d | Size: %d",(device==FDB?'B':'A'),addr,size);
	
	uint32_t _offset = 0, ws = 0;
	size_t ds = 0;
	int ret;
	while (Size > ds){
		ret = _FloppyCache(0,FLOPPY_READ, Offset+ds, &_offset, &ws);
		if (ret < 0) return ret;
		if (ws > Size - ds) ws = Size - ds;
		memcpy((void*) &FLOPPY_DMABUFA[_offset], Buffer+ds, ws);
		ret = _FloppyCache(0,FLOPPY_WRITE, Offset+ds, nullptr, nullptr);
		if (ret < 0) return ret;
		ds += ws;
	}
	return ds;
}

size_t _FloppyDPMReadA(size_t Disk, size_t Offset, size_t Size, void* Buffer){
	qemu_log("[DPM] Floppy A Read! Offset: %d | Size: %d",Offset, Size);
	if (Floppy(0).Status != 1) return 0;
	floppy_data[0].LastErr = 0;
	uint32_t _offset=0, ws=0;
	size_t ds = 0;
	int ret;
	while (Size > ds){
		ret = _FloppyCache(0, FLOPPY_READ, Offset+ds, &_offset, &ws);
		if (ret < 0) return ret;
		if (ws > Size - ds) ws = Size - ds;
		memcpy(Buffer + ds, (void*) &FLOPPY_DMABUFA[_offset], ws);
		ds += ws;
	}
	return ds;
}

void _FloppyPrint(){
	_FloppyCheck();
	_tty_printf("[-] Floppy Disk A\n");
	_tty_printf(" |--- Status: %d\n",Floppy(FDA).Status);
	_tty_printf(" |--- Type: %s\n",drive_types[Floppy(FDA).Type]);
	if (Floppy(FDA).Status == 1){
		_tty_printf(" |--- File System: %s\n",Floppy(FDA).FileSystem);
		_tty_printf(" |--- Label: %s\n",Floppy(FDA).Name);
	}
	
	tty_printf("\n");

	_tty_printf("[-] Floppy Disk B\n");
	_tty_printf(" |--- Status: %d\n",Floppy(FDB).Status);
	_tty_printf(" |--- Type: %s\n",drive_types[Floppy(FDB).Type]);
	if (Floppy(FDB).Status == 1){
		_tty_printf(" |--- File System: %s\n",Floppy(FDB).FileSystem);
		_tty_printf(" |--- Label: %s\n",Floppy(FDB).Name);
	}
}

// Obviously you'd have this return the data, start drivers or something.
void initFloppy() {
	tty_printf("Поиск дискет...\n");

	floppy_data[FDA].Index = FDA;
	floppy_data[FDA].Type = 0;

	floppy_data[FDA].Cache = &_FloppyCache;
	floppy_data[FDB].Cache = &_FloppyCache;

	floppy_data[FDA].Read = &_FloppyRead;
	floppy_data[FDB].Read = &_FloppyRead;

	floppy_data[FDA].Write = &_FloppyWrite;
	floppy_data[FDB].Write = &_FloppyWrite;

	floppy_data[FDA].Addr = FLOPPY_MAIN;
	floppy_data[FDB].Addr = FLOPPY_SECO;
	
	floppy_data[FDA].Cyr = -1;
	floppy_data[FDB].Cyr = -1;

	floppy_data[FDA].Status = 0;
	floppy_data[FDB].Status = 0;
	floppy_data[FDA].Motor = 0;
	floppy_data[FDB].Motor = 0;
	
	_FloppyReset(FDA);
	_FloppyReset(FDB);
	register_interrupt_handler(32+6,&irqFloppy);

	dpm_reg('A',"Disk A","Unknown",1,0,0,0,3,"FLOP-PYDA",0);
	dpm_metadata_write('A', (uint32_t) &floppy_data[FDA]);
	dpm_fnc_write('A',_FloppyDPMReadA,_FloppyDPMWriteA);
	
	_FloppyServiceA();


	dpm_reg('B',"Disk B","Unknown",0,0,0,0,3,"FLOP-PYDB",0);
	dpm_metadata_write('B', (uint32_t) &floppy_data[FDB]);

	_FloppyCheck();
	_FloppyPrint();
	
// 	fs_tarfs_detect('A');
// 	fs_tarfs_detect('R');
// 
// 	char* fs_fat12 = kmalloc(32);
// 	size_t read = dpm_read('A', 0x1F, 12, fs_fat12);
// 
// 	qemu_log("[DPM] [A] Ready: %d | '%s'",read,fs_fat12);

	// base - 0x03f0
	//int data = 0;ЮПСРВЯАФ1234567890-=
	//char text[32] = {0};
	//int read = Floppy(0).Read(0,text,0xD0,32);
	//qemu_log("FD0:TEST = (%d) '%s'",read,text);
	//char* write = "SAYORIFLOPPY";
	//floppy_write(0,write,strlen(write));
}