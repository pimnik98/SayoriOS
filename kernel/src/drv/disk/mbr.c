#include "drv/disk/mbr.h"
#include "drv/disk/dpm.h"
#include "io/ports.h"
#include "debug/hexview.h"
#include "io/tty.h"

void ebr_recursive_dump(char disk, uint64_t abs_lba, uint64_t lba, int depth) {
    struct mbr_parition w = {};

    dpm_read(disk, 0, ((lba + abs_lba) * 512) + 446,  sizeof(w), (uint8_t *)&w);

    qemu_log("%*s Active: %d; [C: %d; H: %d; S: %d] Type: %d; [C: %d; H: %d; S: %d] Start LBA: %d; Num Sectors: %d\n",
           depth + 3, "|--",
           w.activity,
           w.start_cylinder, w.start_head, w.start_sector,
           w.type,
           w.end_cylinder, w.end_head, w.end_sector,
           w.start_sector_lba, w.num_sectors);

    tty_printf("%*s A: %d; [%d:%d:%d] Type: %d; [%d:%d:%d] SLBA: %d; COUNT: %d\n",
           depth + 3, "|--",
           w.activity,
           w.start_cylinder, w.start_head, w.start_sector,
           w.type,
           w.end_cylinder, w.end_head, w.end_sector,
           w.start_sector_lba, w.num_sectors);

    dpm_read(disk, 0, ((lba + abs_lba) * 512) + 446 + 16,  sizeof(w), (uint8_t *)&w);

    if(w.type == 5) {
        ebr_recursive_dump(disk, abs_lba, w.start_sector_lba, depth);
    }
}

void mbr_dump(char disk, uint64_t i) {
    struct mbr_parition p = {};

    dpm_read(disk, 0, 446 + (i * 16), sizeof(p), (uint8_t *)&p);

    qemu_log("[%d] Active: %d; [C: %d; H: %d; S: %d] Type: %d; [C: %d; H: %d; S: %d] Start LBA: %d; Num Sectors: %d",
           i,
           p.activity,
           p.start_cylinder, p.start_head, p.start_sector,
           p.type,
           p.end_cylinder, p.end_head, p.end_sector,
           p.start_sector_lba, p.num_sectors);

   tty_printf("[%d] A: %d; [%d:%d:%d] Type: %d; [%d:%d:%d] SLBA: %d; COUNT: %d\n",
           i,
           p.activity,
           p.start_cylinder, p.start_head, p.start_sector,
           p.type,
           p.end_cylinder, p.end_head, p.end_sector,
           p.start_sector_lba, p.num_sectors);

    if(p.type == 5 || p.type == 15) {
        ebr_recursive_dump(disk, p.start_sector_lba, 0, 1);
    }
}

void mbr_dump_all(char disk) {
    uint16_t p;

    dpm_read(disk, 0, 510, 2, (uint8_t *)&p);

    if(p == 0xaa55) {
        mbr_dump(disk, 0);
        mbr_dump(disk, 1);
        mbr_dump(disk, 2);
        mbr_dump(disk, 3);
        tty_printf("\n");
    }
}
