/**
 * @brief Драйвер ATAPI PIO
 * @author NDRAEY >_
 * @date 2023-07-21
 * @version 0.3.5
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

// Super-duper original ATAPI driver by NDRAEY (c) 2023
// for SayoriOS

#include "drv/atapi.h"
#include "net/endianess.h"
#include "debug/hexview.h"

// FIXME: Add REQUEST_SENSE command to handle errors.

/**
 * @brief Ждёт пока освободится порт ATA
 * @param bus Шина (PRIMARY или SECONDARY)
 * @return true - если есть ошибка, false - всё ок
 */
bool ata_scsi_status_wait(uint8_t bus) {
	while (1) {
		uint8_t status = inb(ATA_PORT(bus) + ATA_REG_COMMAND);
		
		if ((status & 0x01) == 1)
			return true; // error
		
		if (!(status & 0x80) && (status & 0x08))
			break;
		
		ide_400ns_delay(ATA_PORT(bus));
	}

	return false;
}

/**
 * @brief Отправляет SCSI команду на ATA дисковод
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param slave Является ли устройство SLAVE
 * @param lba_mid_hi Метаданные
 * @param command Команда размером 12 байт
 * @return
 */
bool ata_scsi_send(uint16_t bus, bool slave, uint16_t lba_mid_hi, uint8_t command[12]) {
	// qemu_log("ATAPI SCSI send [%s %s], LBA (MID AND HI): %d",
				// PRIM_SEC(bus), MAST_SLV(slave), lba_mid_hi);

    ide_select_drive(bus, slave);
	
    ide_400ns_delay(ATA_PORT(bus));

	outb(ATA_PORT(bus) + ATA_REG_ERROR, 0x00); 
	outb(ATA_PORT(bus) + ATA_REG_LBA1, lba_mid_hi & 0xFF);
	outb(ATA_PORT(bus) + ATA_REG_LBA2, (lba_mid_hi >> 8) & 0xFF);
	outb(ATA_PORT(bus) + ATA_REG_COMMAND, ATA_CMD_PACKET);

	ide_400ns_delay(ATA_PORT(bus));

	bool error = ata_scsi_status_wait(bus);

	if(error) {
		qemu_log("Error");
		return error;
	}

	outsw(ATA_PORT(bus) + ATA_REG_DATA, (uint16_t *)command, 6);

	// That's all folks!

	return false;
}

/**
 * @brief Считывает размер передачи данных
 * @param bus Шина (PRIMARY или SECONDARY)
 * @return Размер передачи в байтах
 */
size_t ata_scsi_receive_size_of_transfer(uint16_t bus) {
	bool error = ata_scsi_status_wait(bus);

	if(error) {
		qemu_log("ATAPI size receive error");
		return 0;
	}
	
	return inb(ATA_PORT(bus) + ATA_REG_LBA2) << 8
			| inb(ATA_PORT(bus) + ATA_REG_LBA1);
}

/**
 * @brief Считывает данные с ATA дисковода
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param size Размер в байтах
 * @param buffer Буффер для хранения данных
 */
void ata_scsi_read_result(uint16_t bus, size_t size, uint16_t* buffer) {
	insw(ATA_PORT(bus) + ATA_REG_DATA, (uint16_t*)((uint8_t *)buffer), size);
}

/**
 * @brief Считывает размер диска
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param slave Является ли устройство SLAVE
 * @return Размер диска в секторах
 */
size_t atapi_read_size(uint16_t bus, bool slave) {
//	qemu_log("SIZE REQUEST ON (ints): %d %d", bus, slave);
//	qemu_log("SIZE REQUEST ON: %s %s", PRIM_SEC(bus), MAST_SLV(slave));

	uint8_t command[12] = {
        ATAPI_READ_CAPACITY,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    ata_scsi_send(bus, slave, 0x0008, command);
    
    size_t transf_size = ata_scsi_receive_size_of_transfer(bus);

//	qemu_log("Size of transfer is: %d", transf_size);

	if(!transf_size)
		return 0;

    uint16_t* data = kcalloc(transf_size, 1);

    ata_scsi_read_result(bus, transf_size, data);

    uint32_t* data2 = (uint32_t*)data;

    uint32_t maxlba = data2[0];
    uint32_t blocksize = data2[1];

    blocksize = ntohl(blocksize);
    maxlba = ntohl(maxlba);

//	qemu_log("Blocks: %x; Block size: %x", maxlba, blocksize);

	kfree(data);

	return maxlba;
}

/**
 * @brief Считывает размер блока
 * @param bus Номер привода в системе
 * @param slave Является ли устрйоство SLAVE
 * @return Размер блока в байтах
 */
size_t atapi_read_block_size(uint16_t bus, bool slave) {
	uint8_t command[12] = {
        ATAPI_READ_CAPACITY,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    ata_scsi_send(bus, slave, 0x0008, command);
    
    size_t transf_size = ata_scsi_receive_size_of_transfer(bus);

    uint16_t* data = (uint16_t*)kcalloc(transf_size, 1);

    ata_scsi_read_result(bus, transf_size, data);

    uint32_t blocksize = ntohl(*((uint32_t*)(data) + 1));

	// qemu_log("Block size is: %d", blocksize);

	kfree(data);

	return blocksize;
}

/**
 * @brief Читает сектора с диска
 * @param drive Номер привода в системе
 * @param buf Буффер
 * @param lba Номер сектора на диске
 * @param sector_count Количество секторов которые необходимо прочитать
 * @return true - если есть ошибка, false - если нет
 */
bool atapi_read_sectors(uint16_t drive, uint8_t *buf, uint32_t lba, size_t sector_count) {
	uint8_t bus = (drive >> 1) & 1;
	bool slave = (bool)((drive >> 0) & 1);
	
	uint8_t command[12] = {
        ATAPI_CMD_READ,  // Command
        0, // ?
		(lba >> 0x18) & 0xFF,  // LBA
		(lba >> 0x10) & 0xFF,
		(lba >> 0x08) & 0xFF,
		(lba >> 0x00) & 0xFF,
		(sector_count >> 0x18) & 0xFF,  // Sector count
		(sector_count >> 0x10) & 0xFF,
		(sector_count >> 0x08) & 0xFF,
		(sector_count >> 0x00) & 0xFF,
		0, // ?
		0  // ?
    };

	size_t block_size = atapi_read_block_size(bus, slave);	
	bool error = ata_scsi_send(bus, slave, block_size, command);
    
	if(error) {
		qemu_log("Error");
		return error;
	}

    size_t transf_size = ata_scsi_receive_size_of_transfer(bus);

	if(!transf_size) {
		qemu_log("Error: Transfer size can't be 0!");
		return true;
	}

	for (uint32_t i = 0; i < sector_count; i++) {
		error = ata_scsi_status_wait(bus);
		
		if(error)
			return true;
		
		size_t size = ata_scsi_receive_size_of_transfer(bus);
 
		insw(ATA_PORT(bus) + ATA_REG_DATA, (uint16_t*)((uint8_t*)buf + i * block_size), size / 2); // Read it
	}

	return false;
}

/**
 * @brief Извлекает диск из привода
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param slave Является ли диск SLAVE
 * @return true если есть ошибка, false - если нет
 */
bool atapi_eject(uint8_t bus, bool slave) {
	// Byte 4:
	//   Bit 0: Start
	//   Bit 1: LoEj

	// LoEj      Start      Result
	//  0          0     Stop the Disc
	//  0          1     Start the Disc
	//  1          0     Eject Disc
	//  1          1     Close the Tray (Load the Disc)

	uint8_t command[12] = {
        ATAPI_CMD_START_STOP,  // Command
        0, 0, 0,  // Reserved
        0, // Stop disc
		0, 0, 0, 0, 0, 0, 0  // Reserved
    };

	bool error = ata_scsi_send(bus, slave, 0, command);
    
	if(error) {
		qemu_log("Error");
	}

	command[4] = (1 << 1);  // Byte 4: LoEj: Eject disc if possible

	error = ata_scsi_send(bus, slave, 0, command);
    
	if(error) {
		qemu_log("Error");
	}

	return error;
}

/**
 * @brief Считывает и возвращает ошибку (если таковая имеется)
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param slave Является ли дисковод SLAVE
 * @param out Буффер размером 18 байт
 * @return Структура с кодом ошибки
 */
atapi_error_code atapi_request_sense(uint8_t bus, bool slave, uint8_t out[18]) {
	uint8_t command[12] = {
        ATAPI_CMD_RQ_SENSE, 0, 0, 0,
		18, // Allocation Length: We need only 18 bytes (mininal respose length)
		0, 0, 0, 0, 0, 0, 0
    };

	ata_scsi_send(bus, slave, 18, command);

	ata_scsi_receive_size_of_transfer(bus);

    ata_scsi_read_result(bus, 18, (uint16_t*)out);

	// First byte should be 0xf0

	hexview_advanced(out, 18, 10, false, new_qemu_printf);

	return (atapi_error_code){(out[0] >> 7) & 1, out[2] & 0b00001111, out[12], out[13]};
}

/**
 * @brief Проверяет дисковод на наличие диска внутри
 * @param bus Шина (PRIMARY или SECONDARY)
 * @param slave Является ли дисковод SLAVE
 * @return true если диск вствлен, false если нет
 */
bool atapi_check_media_presence(uint8_t bus, bool slave) {
	uint8_t command[12] = {
        ATAPI_CMD_READY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

	bool error = ata_scsi_send(bus, slave, 0, command);
    
	if(error) {
		qemu_log("Error");
		return false;
	}

	uint8_t errorcode[18];

	atapi_error_code error_code = atapi_request_sense(1, 0, errorcode);

	return !(error_code.valid && error_code.sense_key == 0x02 && error_code.sense_code == 0x3A);
}
