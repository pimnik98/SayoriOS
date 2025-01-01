/**
 * @file drv/fs/fat12.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система FAT12
 * @version 0.3.5
 * @date 2023-07-28
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <kernel.h>
#include <io/ports.h>
#include <fs/fat12.h>
struct volume_t* volume = {0};
FAT_ENTRY ebat[32] = {0};
void _FATDetectType(int clus, int ss){
	if (ss == 0){
		qemu_log("[FAT] exFAT");
		return;
	} else if(clus < 4085){
		qemu_log("[FAT] FAT12");
		return;
	} else if(clus < 65525){
		qemu_log("[FAT] FAT16");
		return;
	} else {
		qemu_log("[FAT] FAT32");
		return;
	}
}

void fatStringPath(char* String, int Count){
	qemu_log("[FAT] SP1: %s",String);
	String[Count-1] = 0;
	qemu_log("[FAT] SP2: %s",String);
}

void fatPrint(FAT_BOOT_SECTOR fbs){
	qemu_log("  |--- jump_code => %d | %d | %d",fbs.jump_code[0],fbs.jump_code[1],fbs.jump_code[2]);
    qemu_log("  |--- oem_name => %s",fbs.oem_name);
    qemu_log("  |--- bytes_per_sector => %d",fbs.bytes_per_sector);
    qemu_log("  |--- sectors_per_cluster => %d",fbs.sectors_per_cluster);
    qemu_log("  |--- reserved_sectors => %d",fbs.reserved_sectors);
    qemu_log("  |--- fat_count => %d",fbs.fat_count);
    qemu_log("  |--- root_dir_capacity => %d",fbs.root_dir_capacity);
    qemu_log("  |--- logical_sectors16 => %d",fbs.logical_sectors16);
    qemu_log("  |--- media_type => %d",fbs.media_type);
    qemu_log("  |--- sectors_per_fat => %d",fbs.sectors_per_fat);
    qemu_log("  |--- chs_sectors_per_track => %d",fbs.chs_sectors_per_track);
    qemu_log("  |--- chs_tracks_per_cylinder => %d",fbs.chs_tracks_per_cylinder);
    qemu_log("  |--- hidden_sectors => %d",fbs.hidden_sectors);
    qemu_log("  |--- logical_sectors32 => %d",fbs.logical_sectors32);
    qemu_log("  |--- media_id => %d",fbs.media_id);
    qemu_log("  |--- chs_head => %d",fbs.chs_head);
    qemu_log("  |--- ext_bpb_signature => %d",fbs.ext_bpb_signature);
    qemu_log("  |--- serial_number => %x",fbs.serial_number);
    qemu_log("  |--- volume_label => %s",fbs.volume_label);
    qemu_log("  |--- fsid => %s",fbs.fsid);
    qemu_log("  |--- boot_code => %d|%d|%d|%d|%d|%d|%d|",fbs.boot_code[0],fbs.boot_code[1],fbs.boot_code[2],fbs.boot_code[3],fbs.boot_code[4],fbs.boot_code[5],fbs.boot_code[6]);
    qemu_log("  |--- magic => %x",fbs.magic);
	
}

void _FATDebugPrintEntity(uint32_t* ent){
	if (ent == 0x0){
		qemu_log("[FAT] Ignore Entity: %d|%x",ent,ent);
		return;
	}
	qemu_log("[FAT] Debug Entity: %d|%x",ent,ent);
	FAT_ENTRY* e = kmalloc((uint32_t)sizeof(FAT_ENTRY));
	memcpy(ent,&e,(uint32_t)sizeof(FAT_ENTRY));
	__com_formatString(0x3f8,"Name => %s\n",e->Name);
	__com_formatString(0x3f8,"Size => %d\n",e->Size);
	
	//__com_formatString(0x3f8,"[%d] %x | %x\n",i,table[i],table[i+1]);
}

uint32_t* _FATTable(const uint8_t* fat, uint32_t clusters_count){
    uint32_t * fat_data = kmalloc(sizeof(uint32_t) * (clusters_count+1));
    if(fat_data == nullptr)	return nullptr;
    for(int i = 0, j = 0; i < (int)clusters_count; i += 2, j+= 3){
        uint8_t b1 = fat[j];
        uint8_t b2 = fat[j + 1];
        uint8_t b3 = fat[j + 2];

        /*uint16_t n1 = ((uint16_t)(b2 & 0x0F) << 8) | b1;
        uint16_t n2 = ((uint16_t)b3 << 4) | ((b2 & 0xF0) >> 4);*/
        uint32_t n1 = ((b2 & 0x0F) << 8) | b1;
        uint32_t n2 = ((b2 & 0xF0) >> 4) | (b3 << 4);

        fat_data[i] = n1;
        fat_data[i + 1] = n2;
    }
    return fat_data;
}

void _FATTableDebug(uint32_t* table,uint32_t clusters_count){	
	qemu_log("[FAT TABLE] DEBUG");
	for (int i=0; i < (int) clusters_count;i+=2){
		if (table[i] == 0x0) continue;
		__com_formatString(0x3f8,"[%d] %x | %x\n",i,table[i],table[i+1]);
		_FATDebugPrintEntity(table[i]);
		_FATDebugPrintEntity(table[i+1]);
	}
}

void _FATDuck(unsigned char* FAT_table,int active_cluster,int first_fat_sector,int section_size){
	unsigned int fat_offset = active_cluster + (active_cluster / 2);// multiply by 1.5
	unsigned int fat_sector = first_fat_sector + (fat_offset / section_size);
	unsigned int ent_offset = fat_offset % section_size;
	
	//at this point you need to read from sector "fat_sector" on the disk into "FAT_table".
	
	unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];
	qemu_log("[FAT] tv = %d | %x",table_value);
	if(active_cluster & 0x0001){
		table_value = table_value >> 4;
		qemu_log("[FAT] tv2 = %d | %x",table_value);
	} else {
		table_value = table_value & 0x0FFF;
		qemu_log("[FAT] tv3 = %d | %x",table_value);
	}
}



int _FATValide(FAT_BOOT_SECTOR fbs){
	// Step 1 - Сначала проверем ебанный множитель
	// Я чесно не ебу зачем, но это так написано в документации
	// Тип поиск подходящего класстера?
	int loop_err = 0;
	for (int i = 1; i <= 128; i=i*2){
		if (fbs.sectors_per_cluster == i){
			loop_err = 1;
			break;
		}
	}
	if (loop_err != 1) return -1;
	// Step 2 - Чекаем логику секторов
	if(!((fbs.logical_sectors16 == 0 && fbs.logical_sectors32 != 0) || (fbs.logical_sectors16 !=0 && fbs.logical_sectors32 == 0))){
		// Значения не валидны, съебываем нахуй
		return -1;
	}
	// Step 3 - Проверяем доступные методы работы с FAT
	if(!(fbs.fat_count == 1 || fbs.fat_count == 2)){
		// Тоже съебываем, если что-то не так.
		// Мне проблемы не нужны
		return -1;
	}
	if (fbs.fat_count == 2){
		// Ясень хуй, мы только с этим типом работать можем
		uint32_t fat1_pos = fbs.reserved_sectors;
        uint32_t fat2_pos = fat1_pos + fbs.sectors_per_fat;
        uint8_t* fat1 = kmalloc(fbs.sectors_per_fat * fbs.bytes_per_sector);
		if (fat1 == nullptr){
			kfree(fat1_pos);
			kfree(fat2_pos);
			qemu_log("[FAT] [Valid] F1 ERROR Malloc");
			return -1;
		}
		uint8_t* fat2 = kmalloc(fbs.sectors_per_fat * fbs.bytes_per_sector);
        if(fat2 == nullptr){
			qemu_log("[FAT] [Valid] F2 ERROR Malloc");
            kfree(fat1);
			kfree(fat1_pos);
			kfree(fat2_pos);
            return -1;
        }
		int read = 0;
		read = _FloppyRead(0,fat1,fat1_pos,fbs.sectors_per_fat);
		if (read != 9){
			qemu_log("[FAT] [Valid] Result: (%x - ERR) | (%x - ERR)",fat1_pos,fat2_pos);
			qemu_log("[FAT] F1 READING %d!=9 bytes.",read);
			kfree(fat1);
			kfree(fat2);
			kfree(fat1_pos);
			kfree(fat2_pos);
			kfree(read);
			return -1;
		}
		read = _FloppyRead(0,fat2,fat2_pos,fbs.sectors_per_fat);
		if (read != 9){
			qemu_log("[FAT] [Valid] Result: (%x - PASS) | (%x - ERR)",fat1_pos,fat2_pos);
			qemu_log("[FAT] F2 READING %d!=9 bytes.",read);
			kfree(fat1);
			kfree(fat2);
			kfree(fat1_pos);
			kfree(fat2_pos);
			kfree(read);
			return -1;
		}
		qemu_log("[FAT] [Valid] Result: (%x - PASS) | (%x - PASS)",fat1_pos,fat2_pos);
        kfree(fat1);
        kfree(fat2);
        kfree(fat1_pos);
        kfree(fat2_pos);
        kfree(read);
		return 1;
	}
	return -1;
}


uint16_t _FATGetClusterValue(uint32_t cl){
	int root = (volume->super.reserved_sectors + 0 * volume->super.sectors_per_fat) * volume->super.bytes_per_sector;
	uint16_t data;
	int read = 0;
	read = _FloppyRead(0,&data,root + cl * sizeof(data),sizeof(data));
	qemu_log("[FAT] [Get Cluster Value] Reading %d bytes. Value: %d|%x",read,data,data);
	kfree(read);
	return data;
	//_FATDebugPrintEntity(data);
}

addr_t _FATFindEntry(uint32_t clusterIndex, bool free, int32_t skip, int32_t* index) {
	uint16_t rootEntryCount = volume->super.root_dir_capacity;
	uint32_t start = _FATGetClusterValue(clusterIndex);
	for (int32_t offset = skip; offset < rootEntryCount; offset++) {
		struct dir_entry_t *entry = kmalloc(sizeof(struct dir_entry_t));
		_FloppyRead(0,&entry,start + offset * sizeof(struct dir_entry_t) + offsetof(struct dir_entry_t, name),sizeof(struct dir_entry_t));
    	if (free ^ (entry->name[0] != ENTRY_AVAILABLE && entry->name[0] != ENTRY_ERASED)) {
			if (index) *index = offset;
			return start + offset * sizeof(struct dir_entry_t);
		}
	}
}


int _FATReadData(uint32_t clusterIndex, void* data, uint32_t len, size_t skip) {
  uint32_t bytesPerCluster = volume->super.sectors_per_cluster * volume->super.bytes_per_sector;

  // Skip empty files
  if (len == 0) return 0;

  uint16_t endOfChainValue = _FATGetClusterValue(1);
  uint32_t read_size = 0;
  uint8_t *p = (uint8_t *)data;

  // Copy data one cluster at a time.
  while (clusterIndex != endOfChainValue && len > (uint32_t)(p-(uint8_t*)data)) {
    if (read_size + bytesPerCluster > skip) {
      // Determine amount of data to copy
      uint32_t start = read_size > skip ? 0 : skip - read_size;
      uint32_t count = len - (p-(uint8_t*)data);
      if (count > bytesPerCluster) count = bytesPerCluster;

      // Transfer bytes into image at cluster location
      uint32_t offset = _FATGetClusterValue(clusterIndex);
      int read = 0;
		read = _FloppyRead(0,p,offset+start,count);
		qemu_log("[FAT] [READ DATA] Reading %d bytes.",read);
		//disk_read(d, p, offset+start, count);
      p += count;
    }
    read_size += bytesPerCluster;
    clusterIndex = _FATGetClusterValue(clusterIndex);
  }
  return p - (uint8_t *)data;
}

void fatTest(){
	qemu_log("[FAT] В пизду"); return;
	qemu_log("[FAT] TEST INIT");
	qemu_log("[FAT] Struct\n\tvolume_t: %d\n\tFAT_BOOT_SECTOR: %d",sizeof(struct volume_t),sizeof(FAT_BOOT_SECTOR));
	qemu_log("[FAT] Trying to allocate %d bytes for a struct",sizeof(struct volume_t));

	int read = 0;
	read = _FloppyRead(0,&volume->super,0,sizeof(FAT_BOOT_SECTOR));
	qemu_log("[FAT] READING %d bytes.",read);
	//fatStringPath(volume->super.oem_name,8);	///> Вроде не требуется этот момент патчить
	fatStringPath(volume->super.volume_label,11);
	fatStringPath(volume->super.fsid,8);
	fatPrint(volume->super);
	if (_FATValide(volume->super) != 1) return;
	uint32_t fat1_pos = volume->super.reserved_sectors;
	uint32_t root_dir_pos = fat1_pos + volume->super.fat_count * volume->super.sectors_per_fat;
	uint32_t root_dir_sectors = (volume->super.root_dir_capacity * (uint32_t)sizeof(FAT_ENTRY)) / volume->super.bytes_per_sector;
	if((volume->super.root_dir_capacity * (uint32_t)sizeof(FAT_ENTRY)) % volume->super.bytes_per_sector != 0){
		root_dir_sectors += 1;
	}
	uint32_t clusters2_pos = root_dir_pos + root_dir_sectors;
	uint32_t volume_sectors = volume->super.logical_sectors16 == 0 ? volume->super.logical_sectors32 : volume->super.logical_sectors16;
	uint32_t data_clusters = (volume_sectors - (volume->super.fat_count * volume->super.sectors_per_fat) - root_dir_sectors ) / volume->super.sectors_per_cluster;

	uint8_t* fat1 = kmalloc(sizeof(uint8_t) * volume->super.sectors_per_fat * volume->super.bytes_per_sector);
	if (fat1 == nullptr){
		kfree(volume);
		qemu_log("[FAT] F1 ERROR MALLOC");
		return;
	}
	read = _FloppyRead(0,fat1,fat1_pos,volume->super.sectors_per_fat);
	if (read != 9){
		qemu_log("[FAT] Result: (%x - ERR)",fat1_pos);
		qemu_log("[FAT] F1 READING %d!=9 bytes.",read);
		kfree(fat1);
		kfree(fat1_pos);
		kfree(read);
		return;
	}

	
	int RootTable =  (volume->super.reserved_sectors + 0 * volume->super.sectors_per_fat) * volume->super.bytes_per_sector;
	qemu_log("RootTable: %d|%x",RootTable,RootTable);
	uint32_t * converted_fat = _FATTable(fat1, data_clusters + 2);
    if(converted_fat == nullptr){
		qemu_log("[FAT] Fatal Table");
        kfree(fat1);
        kfree(volume);
        return;
    }
    kfree(fat1);
	_FATTableDebug(converted_fat,data_clusters + 2);
    volume->fat1_pos = fat1_pos;
    volume->cluster2_pos = clusters2_pos;
    volume->root_pos = root_dir_pos;
    volume->root_sectors = root_dir_sectors;
    //volume->disk = pdisk;
    volume->fat_table = converted_fat;
	int total_sectors = volume_sectors;

	uint32_t rds = ((volume->super.root_dir_capacity * 32) + (volume->super.bytes_per_sector - 1)) / volume->super.bytes_per_sector;
	int first_data_sector = volume->super.reserved_sectors + (volume->super.fat_count * volume_sectors) + root_dir_sectors;
	int first_fat_sector = volume->super.reserved_sectors;
	
	int fat_size = volume->super.sectors_per_fat;

	int data_sectors = volume_sectors - (volume->super.reserved_sectors + (volume->super.fat_count * fat_size) + root_dir_sectors);
	int total_clusters = data_sectors / volume->super.sectors_per_cluster;
	qemu_log("[FAT] Init Complete");
	qemu_log("volume_sectors: %d",volume_sectors);	/// Всего секторов по объему (включая VBR):
	qemu_log("data_clusters: %d",data_clusters);	/// Общие количество секторных данных
	qemu_log("root_dir_sectors: %d|%d",root_dir_sectors,rds);	/// Размер корневого каталога (если у вас не FAT32, в этом случае размер будет равен 0):
	qemu_log("first_data_sector: %d",first_data_sector);	/// Первый сектор данных (то есть первый сектор, в котором могут храниться каталоги и файлы):
	qemu_log("first_fat_sector: %d",first_fat_sector);	/// Первый сектор в таблице размещения файлов:
	qemu_log("total_clusters: %d",total_clusters);	/// Общее количество кластеров
	int first_root_dir_sector = first_data_sector - root_dir_sectors ;
	qemu_log("first_root_dir_sector: %d",first_root_dir_sector);	/// Общее количество кластеров
	int first_sector_of_cluster = ((0 - 2) * volume->super.sectors_per_cluster) + first_data_sector;
	qemu_log("first_sector_of_cluster: %d",first_sector_of_cluster);	/// Общее количество кластеров

	//uint16_t frds = _FATGetClusterValue(first_root_dir_sector);
	
	//FAT_ENTRY *e = kmalloc(sizeof(FAT_ENTRY*));
	read = _FloppyRead(0,&ebat[0],first_data_sector,sizeof(FAT_ENTRY));
	qemu_log("[READ] %d",read);
	qemu_log("[READ] Name: %s\nSize: %d",ebat[0].Name,ebat[0].Size);
	//__com_formatString(0x3f8,"Name => %s\n",e->Name);
	//__com_formatString(0x3f8,"Size => %d\n",e->Size);
	uint32_t data = 0;
	/**
	for (int i = 0; i < total_clusters;i++){
		data = _FATGetClusterValue(i);
		qemu_log("[TC] [%d] %d | %x",i,data,data);
	}
	*/
	//_FATDebugPrintEntity(first_sector_of_cluster);
	//_FATGetClusterValue(0);
	//_FATGetClusterValue(first_root_dir_sector);
	//_FATDuck(converted_fat,0,int first_fat_sector,int section_size);
	//_FATDetectType(total_clusters,1);
    return;
	
	//qemu_log("[FAT] volume_label: %s",volume->super.volume_label);
}

