//
// Created by maractus on 03.01.24.
//

// NOTE: They needed for userspace apps!

#include "sys/file_descriptors.h"
#include "../src/lib/libvector/include/vector.h"
#include "mem/vmm.h"
#include "io/ports.h"

vector_t* descriptors = 0;
int last_descriptor_number = 0;

void file_descriptors_init() {
    descriptors = vector_new();

    qemu_log("File descriptors initialized");
}

void file_descriptor_allocate(const char *filename, const char *mode, int *out) {
    FILE* file = fopen(filename, mode);

    if(!file) {
        *out = -1;
        return;
    }

    struct fd_info* inf = kcalloc(sizeof *inf, 1);

    inf->fd = last_descriptor_number++;
    inf->file = file;

    vector_push_back(descriptors, (size_t) inf);

    *out = inf->fd;

    qemu_ok("Written to: %d", out);
    qemu_log("allocated fd: %d", inf->fd);
}

struct fd_info* file_descriptor_get(int descriptor_number) {
    for(int i = 0; i < descriptors->size; i++) {
        struct fd_info *inf = (struct fd_info *) vector_get(descriptors, i).element;

        if (inf->fd == descriptor_number) {
            return inf;
        }
    }

    return NULL;
}

void file_descriptor_read(int descriptor_number, size_t count, void* buffer) {
	qemu_log("FD: %d; Size: %d; Buffer: %x", descriptor_number, count, buffer);

    if(descriptor_number < 0 || descriptor_number >= last_descriptor_number) {
		qemu_err("Invalid descriptor: %d!", descriptor_number);
        return;
    }

	if(!file_descriptor_get(descriptor_number)) {
        return;
    }

    struct fd_info* inf = file_descriptor_get(descriptor_number);

	qemu_note("Read: %d bytes (buffer at: %x)", count, buffer);

    fread(inf->file, count, 1, buffer);
}

void file_descriptor_close(int descriptor_number) {
    if(descriptor_number < 0 || descriptor_number >= last_descriptor_number)
        return;

    for(int i = 0; i < descriptors->size; i++) {
        struct fd_info *inf = (struct fd_info *) vector_get(descriptors, i).element;

        if (inf->fd == descriptor_number) {
            fclose(inf->file);

            vector_erase_nth(descriptors, i);

            kfree(inf);

		    qemu_ok("Destroyed: %d", descriptor_number);

            return;
        }
    }

    qemu_err("FAILED TO DESTROY fd: %d", descriptor_number);
}

void file_descriptor_seek(int descriptor_number, ssize_t value, size_t whence) {
    if(descriptor_number < 0 || descriptor_number >= last_descriptor_number)
        return;

    for(int i = 0; i < descriptors->size; i++) {
        struct fd_info *inf = (struct fd_info *) vector_get(descriptors, i).element;

        if (inf->fd == descriptor_number) {
            fseek(inf->file, value, whence);

            return;
        }
    }
}

void file_descriptor_tell(int descriptor_number, int* out) {
    if(descriptor_number < 0 || descriptor_number >= last_descriptor_number)
        return;

    for(int i = 0; i < descriptors->size; i++) {
        struct fd_info *inf = (struct fd_info *) vector_get(descriptors, i).element;

        if (inf->fd == descriptor_number) {
            *out = ftell(inf->file);

            return;
        }
    }
}
