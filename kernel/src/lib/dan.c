#include "lib/dan.h"

DANDescriptor_t* alloc_dan(const char* filename, bool stream) {
    FILE* file = fopen(filename, "rb");
    if(!file)
        return 0;

    fseek(file, 0, SEEK_END);

    uint32_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    DANDescriptor_t* desc = (DANDescriptor_t*)kcalloc(1, sizeof(DANDescriptor_t));
    
    fseek(file, 0, SEEK_SET);
	fread(file, 1, sizeof(DANHeader_t), &desc->header);

    if(memcmp((char*)desc->header.magic, "DAN", 3) != 0) {
        kfree(desc);
        fclose(file);

        return 0;
    }

    if(!stream) {
        desc->data = (char*)kcalloc(1, filesize - sizeof(DANHeader_t));
		fread(file, filesize - sizeof(DANHeader_t), 1, desc->data);
    }

    desc->stream = stream;
    desc->framesize = desc->header.width * desc->header.height * (desc->header.alpha?4:3);
    desc->fd = file;

    return desc;
}

void read_frame_dan(DANDescriptor_t* dan, size_t index, char* out) {
    if(index >= dan->header.frame_count)
        return;
    
    if(dan->stream) {
        fseek(dan->fd, (ssize_t)sizeof(DANHeader_t) + (ssize_t)(index * dan->framesize), SEEK_SET);
		fread(dan->fd, 1, dan->framesize, out);
    } else {
        memcpy(out, dan->data + (index * dan->framesize), dan->framesize);
    }
}

void free_dan(DANDescriptor_t* dan) {
    kfree(dan->data);
    kfree(dan);
}