#include <kernel.h>
#include <fs/lucario/structures.h>

LucarioDescriptor_t* lucario_fs_build_descriptor() {
    return kcalloc(1, sizeof(LucarioDescriptor_t));
}

bool lucario_fs_init(LucarioDescriptor_t* descr, uint8_t ata_drive) {
    ata_drive_t* ata_drives = ata_get_drives();
    
    if(!ata_drives[ata_drive].online) {
        qemu_log("Drive %x not online", ata_drive);
        return false;
    }
    
    descr->ata_drive = ata_drive;
    descr->disk_capacity = ata_drives[ata_drive].capacity;
    
    ata_read(ata_drive, (uint8_t*)&(descr->header), 0, 512);  // Read header

    if(memcmp(descr->header.magic, LUCARIOFS_MAGIC, 7)) {
        qemu_log("Invalid magic for drive %x!", ata_drive);
        return false;
    }

    qemu_log("Initialized successfully for drive: %x", ata_drive);

    qemu_log("Info:");
    qemu_log("\t Magic: %s", descr->header.magic);
    qemu_log("\t Version: %d.%d.%d",
                descr->header.version_major,
                descr->header.version_minor,
                descr->header.version_patch);
    
    descr->max_entries = descr->disk_capacity / LUCARIO_DIVISOR;
    
    qemu_log("\t Max possible entries in this disk: %d", descr->max_entries);

    descr->ok = true;

    return true;
}

void lucario_fs_destroy_descriptor(LucarioDescriptor_t* descr) {
    kfree(descr);
}

void lucario_fs_read_file_entry(LucarioDescriptor_t* descr, size_t index, LucarioFileEntry_t* out) {
    if(index > descr->max_entries)
        return;

    ata_read(
        descr->ata_drive,
        (uint8_t*)out,
        512 + (sizeof(LucarioFileEntry_t) * index),
        sizeof(LucarioFileEntry_t)
    );
}

void lucario_fs_get_file_entry(LucarioDescriptor_t* descr, char name[], size_t folder_id, LucarioFileEntry_t* out) {
    for(size_t i = 0; i < descr->max_entries; i++) {
        lucario_fs_read_file_entry(descr, i, out);

        if(strcmp(out->name, name) == 0 && out->folder_id == folder_id) {
            return;
        }
    }

    out->type = E_NONE;
}

bool lucario_fs_file_exists(LucarioDescriptor_t* descr, char name[], size_t folder_id) {
    for(size_t i = 0; i < descr->max_entries; i++) {
        LucarioFileEntry_t* entry = kcalloc(1, sizeof *entry);

        lucario_fs_read_file_entry(descr, i, entry);

        if(strcmp(entry->name, name) == 0 && entry->folder_id == folder_id) {
            kfree(entry);
            return true;
        }

        kfree(entry);
    }

    return false;
}

size_t lucario_fs_file_size(LucarioDescriptor_t* descr, char name[], size_t folder_id) {
    size_t size = 0;

    LucarioFileEntry_t* entry = kcalloc(1, sizeof *entry);

    lucario_fs_get_file_entry(descr, name, folder_id, entry);

    if(entry->type == E_FILE) {
        size = entry->file_size;
    }

    kfree(entry);

    return size;
}

// Offset in sectors!!!
void lucario_fs_read_sectors_to(LucarioDescriptor_t* descr, size_t sector_list_addr, size_t sector_list_size, size_t offset, char* out) {
    uint32_t cursector;
    
    for (size_t i = 0; i < sector_list_size; i++) {
        ata_read(descr->ata_drive, (uint8_t*)&cursector, sector_list_addr + (i * sizeof(uint32_t)), sizeof(uint32_t));

        // qemu_log("Sector number at addr %x: %d", sector_list_addr + (i * sizeof(uint32_t)), cursector);

        ata_read_sector(descr->ata_drive, (uint8_t*)(out + (i * 512)), cursector);
    }
}

// Offset in bytes!!!
bool lucario_fs_read_file(LucarioDescriptor_t* descr, char name[], size_t folder_id, size_t offset, size_t length, char* out) {
    LucarioFileEntry_t* entry = kcalloc(1, sizeof *entry);

    lucario_fs_get_file_entry(descr, name, folder_id, entry);

    if(!entry->type) {
        kfree(entry);

        return false;
    }

    char* temp = kcalloc(1, ALIGN(entry->file_size, 512));

    lucario_fs_read_sectors_to(
    	descr,
    	entry->sector_list_lba * 512,
    	entry->sector_list_size,
    	ALIGN(offset, 512) / 512,
    	temp
    );
    qemu_log("Temp buffer becames: %s", temp);

    memcpy(out, temp + offset, length);

    kfree(temp);
    
    return true;
}
