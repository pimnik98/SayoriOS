//
// Created by ndraey on 23.12.23.
//

#include <drv/disk/ndpm.h>
#include <io/ports.h>
#include <io/tty.h>

ndpm_drive_t ndpm_drives[255] = {0};
int ndpm_drive_count = 0;

void ndpm_init() {

}

void ndpm_add_drive(void* specific_data, ndpm_read_fn_t reader, ndpm_write_fn_t writer) {
    for(int i = 0; i < 255; i++) {
        if(ndpm_drives[i].drive_specific_data == 0) {
            ndpm_drives[i].drive_specific_data = specific_data;
            ndpm_drives[i].read_fn = reader;
            ndpm_drives[i].write_fn = writer;

            ndpm_drive_count++;

            return;
        }
    }

    qemu_ok("Add drive ok");
}

ndpm_drive_t* ndpm_get_drive(int index) {
    if(index >= 255 || ndpm_drives[index].drive_specific_data == 0) {
        return 0;
    }

    return ndpm_drives + index;
}

void ndpm_list() {
    _tty_printf("Drive count: %d", ndpm_drive_count);

    /*for(int i = 0; i < 255; i++) {
        if(ndpm_drives[i].drive_specific_data != 0) {

        }
    }*/
}