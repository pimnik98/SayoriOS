/**
 * @file drv/disk/floppy.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер Floppy
 * @version 0.3.3
 * @date 2023-07-25
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <drv/disk/floppy.h>
#include <io/ports.h>

floppy_t floppy_data[2] = {0};


// standard base address of the primary floppy controller
static const int floppy_base = 0x03f0;
// standard IRQ number for floppy controllers
static const int floppy_irq = 6;
static volatile int floppy_motor_ticks = 0;
static volatile int floppy_motor_state = 0;
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
static const char FLOPPY_DMABUFB[floppy_dmalen] __attribute__((aligned(0x8000)));	///< Буфер дискеты 2
unsigned FLOPPY_DMACYRA = -1;
unsigned FLOPPY_DMACYRB = -1;

bool interrupted = false;

void addr_2_coff(uint32_t addr, uint16_t* cyl, uint32_t* offset, uint32_t* size) {
  if (offset) *offset = addr % FLOPPY_144_CYLINDER_SIZE;
  if (size) *size = FLOPPY_144_CYLINDER_SIZE - (addr % FLOPPY_144_CYLINDER_SIZE);
  if (cyl) *cyl = addr / FLOPPY_144_CYLINDER_SIZE;
}

floppy_t Floppy(int index){
	return floppy_data[index];
}

void _FloppyWaitIRQ(){
	while(interrupted){
		sleep_ms(10);
	}
}

static void _FloppyIRQ(registers_t regs){
	interrupted = 0;
}


static void _FloppyDMA(int device,FloppyMode mode){
	unsigned char m;
	union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

	a.l = (unsigned) &FLOPPY_DMABUFA;				///< Ссылка на буфер
  	c.l = (unsigned) FLOPPY_144_CYLINDER_SIZE - 1;  ///< Размер буфера

	if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)){
		qemu_log("floppy_dma_init: static buffer problem\n");
	}
	if (mode != FLOPPY_READ && mode != FLOPPY_WRITE){
		_FloppyError(device,FLOPPY_ERROR_DMA);
		return;
	}
	if (mode == FLOPPY_READ)  m = 0x46;	///< 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
	if (mode == FLOPPY_WRITE) m = 0x4a;	///< 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2

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
	return;
}

void _FloppyMotor(int device,int onoff){
	if (onoff == 2){
		// Отключаем мотор
		outb(FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0c);
    	floppy_data[device].Motor = 0;
		qemu_log("[FD%c] Motor Offline",(device==FDB?'B':'A'));
	} else if (onoff == 1){
		// Включаем мотор
		if (Floppy(device).Motor == 0){
			outb(FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x1c);
			sleep_ms(500);
		}
		floppy_data[device].Motor = 1;
		qemu_log("[FD%c] Motor Online",(device==FDB?'B':'A'));
	} else {
		// Мотор в режиме ожидания
		if (Floppy(device).Motor == 2){
			qemu_log("[FD%d] Strange, fd motor-state already waiting",device);
		}
		floppy_data[device].Motor = 2;
		floppy_data[device].Ticks = 300;
		qemu_log("[FD%c] Motor Wait",(device==FDB?'B':'A'));
	}
}

void _FloppyCMD(int device, char cmd) {
	qemu_log("[FD%c] CMD: %x",(device==FDB?'B':'A'),cmd);
    int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        sleep_ms(10); // sleep 10 ms
        if(0x80 & inb(FLOPPY_MAIN_STATUS_REGISTER)) {
			outb(FLOPPY_DATA_FIFO, cmd);
			return;
        }
    }
    qemu_log("[FD%d] CMD: timeout",device);
}

unsigned char _FloppyDataRead(int device){
	qemu_log("[FD%c] Read Data",(device==FDB?'B':'A'));
	int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        sleep_ms(1); // sleep 10 ms
        if(0x80 & inb(FLOPPY_MAIN_STATUS_REGISTER)) {
            return inb(FLOPPY_DATA_FIFO);
        }
    }
	qemu_log("[FD%d] Read Data: timeout",device);
	return 0; // not reached
}


void _FloppyCheckIRQ(int device, int *st0, int *cyl) {
    _FloppyCMD(device, FLOPPY_CMD_SENSE_INTERRUPT);

    *st0 = _FloppyDataRead(device);
    *cyl = _FloppyDataRead(device);
}

int _FloppyCalibrate(int device){
	int st0, cyl = -1;
    _FloppyMotor(device, 1);
    for(int i = 0; i < 10; i++) {
		interrupted = false;
        _FloppyCMD(device, FLOPPY_CMD_RECALIBRATE);
        _FloppyCMD(device, 0);
        _FloppyWaitIRQ();
        _FloppyCheckIRQ(device, &st0, &cyl);
        if(st0 & 0xC0) {
            static const char * status[] = { 0, "error", "invalid", "drive" };
            qemu_log("floppy_calibrate: status = %s\n", status[st0 >> 6]);
            continue;
        }
        if(!cyl) { // found cylinder 0 ?
            _FloppyMotor(device, 0);
            return 0;
        }
    }

    qemu_log("floppy_calibrate: 10 retries exhausted\n");
    _FloppyMotor(device, 0);
    return -1;
}

int _FloppyReset(int device){
	interrupted = false;
    outb(FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x00); 
    outb(FLOPPY_DIGITAL_OUTPUT_REGISTER, 0x0C); 
	_FloppyWaitIRQ(); // sleep until interrupt occurs
	{
    	int st0, cyl; // ignore these here..
    	_FloppyCheckIRQ(device, &st0, &cyl);
  	}	
    outb(FLOPPY_CONFIGURATION_CONTROL_REGISTER, 0x00);
	_FloppyCMD(device, FLOPPY_CMD_SPECIFY);
    _FloppyCMD(device, 0xdf); /* steprate = 3ms, unload time = 240ms */
    _FloppyCMD(device, 0x02); /* load time = 16ms, no-DMA = 0 */

    if(_FloppyCalibrate(device)) return -1;
	return 0;
}

int _FloppySeek(int device, unsigned cyli, int head){
	qemu_log("[FD%c] Seek | Cyli: %d | Int: %d",(device==FDB?'B':'A'),cyli,head);
	int st0,cyl = -1;
	_FloppyMotor(device,1);
	for (int i = 0;i<10;i++){
		interrupted = false;
        _FloppyCMD(device, FLOPPY_CMD_SEEK);
        _FloppyCMD(device, head<<2);
        _FloppyCMD(device, cyli);
        _FloppyWaitIRQ();
        _FloppyCheckIRQ(device, &st0, &cyl);
        if(st0 & 0xC0) {
            static const char * status[] =
            { "normal", "error", "invalid", "drive" };
            qemu_log("floppy_seek: status = %s\n", status[st0 >> 6]);
            continue;
        }
        if(cyl == cyli) {
            _FloppyMotor(device,0);
            return 0;
        }
	}
    _FloppyMotor(device,0);
    return -1;
}



int _FloppyTrack(int device,unsigned cyl,FloppyMode mode){
	qemu_log("[FD%c] Track",(device==FDB?'B':'A'));
	unsigned char cmd;
	static const int flags = 0xC0;
	if (mode == FLOPPY_READ) cmd = FLOPPY_CMD_READ_DATA | flags;
	if (mode == FLOPPY_WRITE) cmd = FLOPPY_CMD_WRITE_DATA | flags;
	if (mode != FLOPPY_READ && mode != FLOPPY_WRITE) return _FloppyError(device,FLOPPY_ERROR_TRACK);
	if (_FloppySeek(device, cyl, 0)) return -1;
    if (_FloppySeek(device, cyl, 1)) return -1;
    for(int i = 0; i < 20; i++) {
        _FloppyMotor(device, 1);
		_FloppyDMA(device,mode);
        sleep_ms(100); 					///< Ждем 0.1 сек, для корректировки настроек
        _FloppyCMD(device, cmd);		///< Отправляем команду
        _FloppyCMD(device, 0);			///< 0:0:0:0:0:HD:US1:US0 = Головка и привод
        _FloppyCMD(device, cyl);		///< Цилиндер
        _FloppyCMD(device, 0);			///< Первый заголовок
        _FloppyCMD(device, 1);			///< Первый сектор (отсчет начинается с 1)
        _FloppyCMD(device, 2);			///< Байты и сектора, 128*2^x (x=2 -> 512)
        _FloppyCMD(device, 18);			///< Количество дорожек для работы
        _FloppyCMD(device, 0x1b);		///< Длина GAP3, 27 по умолчанию для 3,5"
        _FloppyCMD(device, 0xff);		///< Длина данных (0xff если B/S != 0)
        _FloppyWaitIRQ();				///< Не вызывайте SENSE_INTERRUPT здесь!

        // Информация о первом считывании данных
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        int error = 0;
        st0 = _FloppyDataRead(device);
        st1 = _FloppyDataRead(device);
        st2 = _FloppyDataRead(device);
        rcy = _FloppyDataRead(device);
        rhe = _FloppyDataRead(device);
        rse = _FloppyDataRead(device);
        bps = _FloppyDataRead(device);
		
		(void)rse;
    	(void)rhe;
    	(void)rcy;

         if(st0 & 0xC0) {
            static const char * status[] = { 0, "error", "invalid command", "drive not ready" };
            qemu_log("floppy_do_sector: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if(st1 & 0x80) {
            qemu_log("floppy_do_sector: end of cylinder\n");
            error = 1;
        }
        if(st0 & 0x08) {
            qemu_log("floppy_do_sector: drive not ready\n");
            error = 1;
        }
        if(st1 & 0x20) {
            qemu_log("floppy_do_sector: CRC error\n");
            error = 1;
        }
        if(st1 & 0x10) {
            qemu_log("floppy_do_sector: controller timeout\n");
            error = 1;
        }
        if(st1 & 0x04) {
            qemu_log("floppy_do_sector: no data found\n");
            error = 1;
        }
        if((st1|st2) & 0x01) {
            qemu_log("floppy_do_sector: no address mark found\n");
            error = 1;
        }
        if(st2 & 0x40) {
            qemu_log("floppy_do_sector: deleted address mark\n");
            error = 1;
        }
        if(st2 & 0x20) {
            qemu_log("floppy_do_sector: CRC error in data\n");
            error = 1;
        }
        if(st2 & 0x10) {
            qemu_log("floppy_do_sector: wrong cylinder\n");
            error = 1;
        }
        if(st2 & 0x04) {
            qemu_log("floppy_do_sector: uPD765 sector not found\n");
            error = 1;
        }
        if(st2 & 0x02) {
            qemu_log("floppy_do_sector: bad cylinder\n");
            error = 1;
        }
        if(bps != 0x2) {
            qemu_log("floppy_do_sector: wanted 512B/sector, got %d", (1<<(bps+7)));
            error = 1;
        }
        if(st1 & 0x02) {
            qemu_log("floppy_do_sector: not writable\n");
            error = 2;
        }
		if(!error) {
            _FloppyMotor(device, 0);
            return 0;
        }
        if(error > 1) {
            qemu_log("floppy_do_sector: not retrying..\n");
            _FloppyMotor(device, 0);
            return -2;
        }
    } 
	qemu_log("floppy_do_sector: 20 retries exhausted\n");
    _FloppyMotor(device, 0);
    return -1;
}

int _FloppyError(int Device,int Error){
	qemu_log("FD%c ERROR! Code: %d",(Device==FDB?'B':'A'),Error);
	return (Error*(-1));
}

int _FloppyCache(int device,FloppyMode mode,unsigned int addr,unsigned int* offset,unsigned int* size){
	//if (Floppy(device).Status != 1) return _FloppyError(device,FLOPPY_ERROR_NOREADY);
	qemu_log("[FD%c] Cache | Addr: %d | Off: %d | Size: %d",(device==FDB?'B':'A'),addr,offset,size);
	uint16_t cyl;
	addr_2_coff(addr, &cyl, offset, size);
	if (mode == FLOPPY_READ && cyl == floppy_data[device].Cyr) return 0;
	int ret = _FloppyTrack(device,cyl,mode);
	if (ret >= 0) floppy_data[device].Cyr = cyl;
	return ret;
}

/** 
  * @brief [Floppy] Чтение данных на устройство
  *
  * @param dst - Данные, куда выполнить запись
  * @param addr - Адрес,откуда читать (0 - в начало)
  * @param size - Сколько считать данных
  *
  * @return int Количество записанных байт
  */
int _FloppyRead(int device,char* dst,uint32_t addr,uint32_t size){
	qemu_log("[FD%c] Read | Addr: %d | Size: %d",(device==FDB?'B':'A'),addr,size);
	uint32_t offset,ws,ds = 0;
	int ret;
	while (size > ds){
		ret = _FloppyCache(device,FLOPPY_READ,addr+ds,&offset,&ws);
		if (ret < 0) return ret;
		if (ws > size - ds) ws = size - ds;
		memcpy(dst + ds, (void*) &floppy_data[device].Buffer[offset], ws);
		ds += ws;
	}
	return ds;
}

int _FloppyWrite(int device,const char* dst,uint32_t addr,uint32_t size){
	
	qemu_log("[FD%c] Write | Addr: %d | Size: %d",(device==FDB?'B':'A'),addr,size);
	uint32_t offset,ws,ds = 0;
	int ret;
	while (size > ds){
		ret = _FloppyCache(device,FLOPPY_READ,addr+ds, &offset, &ws);
		if (ret < 0) return ret;
		if (ws > size - ds) ws = size - ds;
		memcpy((void*) &floppy_data[device].Buffer[offset],dst+ds,ws);
		ret = _FloppyCache(device,FLOPPY_WRITE,addr+ds,nullptr,nullptr);
		if (ret < 0) return ret;
		ds += ws;
	}
	return ds;
}

//
// THIS SHOULD BE STARTED IN A SEPARATE THREAD.
//
//
void timerFloppy() {
    while(1) {
        // sleep for 500ms at a time, which gives around half
        // a second jitter, but that's hardly relevant here.
        sleep_ms(500);
        if(floppy_motor_state == 2) {
            floppy_motor_ticks -= 50;
            if(floppy_motor_ticks <= 0) {
                _FloppyMotor(0,2);
            }
        }
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
		qemu_log("[FDA] Change disk type: %d",Floppy(FDA).Type);
	}
	if (Floppy(FDB).Type != (drives & 0xf)){
		floppy_data[FDB].Type = (drives & 0xf);
		qemu_log("[FDB] Change disk type: %d",Floppy(FDB).Type);
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
	if (Floppy(FDA).Status == 1){
		
		//qemu_log("[FD%c] U",(device==FDB?'B':'A'));
		Floppy(FDA).Read(FDA,fda_fs,54,5);
		qemu_log("[FDA] Read File System => '%s'",fda_fs);
		if (strcmp(fs_fat12,fda_fs) == 0){
			qemu_log("[FDA] File System: FAT12");
			Floppy(FDA).Read(FDA,fda_label,43,11);
			fda_label[12] = 0;
    		memcpy((void*) floppy_data[FDA].Name, fda_label, 12);
    		memcpy((void*) floppy_data[FDA].FileSystem, fda_fs, 6);
			qemu_log("[FDA] Label: %s",floppy_data[FDA].Name);
			//kfree(fda_label);
			//kfree(fda_fs);
		}
	}
	if (Floppy(FDB).Status == 1){
		Floppy(FDB).Read(FDB,fdb_fs,54,5);
		if (strcmp(fs_fat12,fdb_fs) == 0){
			qemu_log("[FDB] File System: FAT12");
			Floppy(FDB).Read(FDB,fdb_label,43,11);
			fdb_label[12] = 0;
			memcpy((void*) floppy_data[FDB].Name, fdb_label, 12);
			qemu_log("[FDB] Label: %s",floppy_data[FDB].Name);
    		memcpy((void*) floppy_data[FDB].FileSystem, fdb_fs, 6);
			//kfree(fdb_label);
			//kfree(fdb_fs);
		}
	}
	//kfree(fs_fat12);
	qemu_log("FDA FS Check: %s = %s | %d",fda_fs,fs_fat12,strcmp(fs_fat12,fda_fs));
	
}

void _FloppyPrint(){
	_FloppyCheck();
	tty_printf("[-] Floppy Disk A\n");
	tty_printf(" |--- Status: %d\n",Floppy(FDA).Status);
	tty_printf(" |--- Type: %s\n",drive_types[Floppy(FDA).Type]);
	if (Floppy(FDA).Status == 1){
		tty_printf(" |--- File System: %s\n",Floppy(FDA).FileSystem);
		tty_printf(" |--- Label: %s\n",Floppy(FDA).Name);
	}
	
	tty_printf("\n");

	tty_printf("[-] Floppy Disk B\n");
	tty_printf(" |--- Status: %d\n",Floppy(FDB).Status);
	tty_printf(" |--- Type: %s\n",drive_types[Floppy(FDB).Type]);
	if (Floppy(FDB).Status == 1){
		tty_printf(" |--- File System: %s\n",Floppy(FDB).FileSystem);
		tty_printf(" |--- Label: %s\n",Floppy(FDB).Name);
	}
}

// Obviously you'd have this return the data, start drivers or something.
void initFloppy() {
	tty_printf("Поиск дискет...\n");
	
	register_interrupt_handler(32+6,&_FloppyIRQ);

	floppy_data[FDA].Index = FDA;
	floppy_data[FDA].Type = 0;

	floppy_data[FDB].Index = FDB;
	floppy_data[FDB].Type = 0;

	floppy_data[FDA].Cache = &_FloppyCache;
	floppy_data[FDB].Cache = &_FloppyCache;

	floppy_data[FDA].Read = &_FloppyRead;
	floppy_data[FDB].Read = &_FloppyRead;

	floppy_data[FDA].Write = &_FloppyWrite;
	floppy_data[FDB].Write = &_FloppyWrite;

	floppy_data[FDA].Addr = 0;
	floppy_data[FDB].Addr = 0x80;
	
	floppy_data[FDA].Cyr = -1;
	floppy_data[FDB].Cyr = -1;

	floppy_data[FDA].Buffer = &FLOPPY_DMABUFA;
	floppy_data[FDB].Buffer = &FLOPPY_DMABUFB;
	floppy_data[FDA].Status = 0;
	floppy_data[FDB].Status = 0;
	_FloppyReset(FDA);
	_FloppyCheck();
	_FloppyPrint();

}