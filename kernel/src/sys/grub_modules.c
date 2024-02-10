//
// Created by ndraey on 2/10/24.
//

#include "sys/grub_modules.h"
#include "multiboot.h"
#include "io/ports.h"
#include "lib/string.h"
#include "drv/disk/initrd.h"

size_t grub_last_module_end = 0;

// Needed to configure physical memory manager
void grub_modules_prescan(multiboot_header_t* hdr) {
    multiboot_module_t *mod = ((multiboot_module_t *)hdr->mods_addr) + hdr->mods_count - 1;

    grub_last_module_end = mod->mod_end;
}


void grub_modules_init(multiboot_header_t* hdr) {
    qemu_log("Initializing kernel modules...");

    if(hdr->mods_count == 0) {
        qemu_err("No modules were connected!");
        return;
    }

    qemu_log("Found %d modules", hdr->mods_count);

    multiboot_module_t* module_list = (multiboot_module_t*)hdr->mods_addr;

    for (size_t i = 0; i < hdr->mods_count; i++) {
        multiboot_module_t *mod = module_list + i;

        size_t mod_size = mod->mod_end - mod->mod_start;

        qemu_log("[kModules] Found module #%d. (Start: %x | End: %x | Size: %d); CMD: %s (at %x)",
                 i,
                 mod->mod_start,
                 mod->mod_end,
                 mod_size,
                 (char*)mod->cmdline,
                 (size_t)mod->cmdline
        );

        if (strcmp((const char *) mod->cmdline, "initrd_tarfs") == 0) {
            initrd_tarfs(mod->mod_start, mod->mod_end);
        }
    }
}