#include "drv/atapi.h"
#include "io/tty.h"

extern ata_drive_t drives[4];

void diskctl_help() {
    tty_printf(
        "DiskCTL - Disk Management Utility by NDRAEY (c) 2023.\n"
        "\n"
        "Commands:\n"
        "- info [disk num] - shows info about selected disk (or all disks if disk num is not present)\n"
        "- eject [disk num] - ejects disc\n"
    );
}

void diskctl_show_disk(size_t drive_n) {
    if(drives[drive_n].is_packet) // Re-Identifying works in ATAPI devices
        ide_identify(drive_n >> 1, drive_n & 1);

    ata_drive_t drive = drives[drive_n];
    
    tty_printf(
        "Drive #%d: %s [%s]\n",
        drive_n + 1,
        drive.is_packet ? "Packet Device" : "Hard Drive",
        drive.online ? "online" : "offline"
    );

    if(!drive.online)
        goto end;

    tty_printf("|--- Capacity: ");

    if(drive.is_chs_addressing) {
        tty_printf(
            "Cylinders: %d; Heads: %d; Sectors: %d\n",
            drive.cylinders,
            drive.heads,
            drive.sectors
        );
    } else {
        tty_printf(
            "%d bytes; %d kilobytes; %d megabytes\n",
            drive.capacity,
            drive.capacity / 1024,
            drive.capacity / (1024 * 1024)
        );
    }

    tty_printf("|--- CHS Addressing: %s\n", drive.is_chs_addressing ? "yes" : "no");
    tty_printf("|--- Block size: %d\n", drive.block_size);

    if(drive.is_packet)
        tty_printf("|--- Media presence: %s\n", atapi_check_media_presence(ATA_SECONDARY, ATA_MASTER) ? "true" : "false");

    end:
    tty_printf("\n");
}

void diskctl_show_all_disks() {
    for(int i = 0; i < 4; i++) {
        diskctl_show_disk(i);
    }
}

void diskctl_info(uint32_t argc, char** argv) {
    if(!argc) {
        diskctl_show_all_disks();
    } else {
        if(!isnumberstr(argv[1])) {
            tty_error("Not a number!\n");
            return;
        }

        size_t disk_n = atoi(argv[1]);

        if(disk_n < 1 || disk_n > 4) {
            tty_error("Invalid disk number. Disk number in range 1 ... 4\n");
            return;
        }

        diskctl_show_disk(disk_n - 1);
    }
}

void diskctl_eject(uint32_t argc, char** argv) {
    if(!argc) {
        tty_error("Choose disc drive in range 1 ... 4");
    } else {
        if(!isnumberstr(argv[1])) {
            tty_error("Not a number!\n");
            return;
        }

        size_t disk_n = atoi(argv[1]);

        if(disk_n < 1 || disk_n > 4) {
            tty_error("Invalid disk number. Disk number in range 1 ... 4\n");
            return;
        }

        disk_n--;

        if(!drives[disk_n].is_packet) {
            tty_error("This drive does not supports eject.\n");
            return;
        }

        atapi_eject(disk_n >> 1, disk_n & 1);
    }
}

void diskctl_test(uint32_t argc, char** argv) {
    if(!argc) {
        tty_error("Choose disc drive in range 1 ... 4");
    } else {
        if(!isnumberstr(argv[1])) {
            tty_error("Not a number!\n");
            return;
        }

        size_t disk_n = atoi(argv[1]);

        if(disk_n < 1 || disk_n > 4) {
            tty_error("Invalid disk number. Disk number in range 1 ... 4\n");
            return;
        }

        disk_n--;

        if(!drives[disk_n].online) {
            tty_printf("Cannot read from that drive: it's offline.\n");
            return;
        }

        tty_printf("Allocating 1 MB buffer...\n");

        uint8_t* buf = kmalloc(1 * 1024 * 1024);

        tty_printf("Reading 1 MB from address 0...\n");

        ata_read(disk_n, buf, 0, 1 * 1024 * 1024);

        tty_printf("Freeing buffer...\n");

        kfree(buf);
    }
}

int shell_diskctl(uint32_t argc, char** argv) {
    if(argc == 0) {
        tty_printf("Not enough arguments.\n\n");

        diskctl_help();
        return 1;
    }

    char* verb = argv[1];

    if(!strcmp(verb, "info")) {
        diskctl_info(argc - 1, argv + 1);
    } else if(!strcmp(verb, "eject")) {
        diskctl_eject(argc - 1, argv + 1);
    } else if(!strcmp(verb, "test")) {
        diskctl_test(argc - 1, argv + 1);
    } else {
        tty_error("Unknown verb: %s", verb);
    }

    return 0;
}
