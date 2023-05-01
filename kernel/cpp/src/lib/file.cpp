#include "file.hpp"

using namespace std;

extern "C" {
    FILE* fopen(const char* filename, const char* mode);
    void fclose(FILE* stream);
    int fread_c(FILE* stream, size_t count, size_t size, void* buffer);
    ssize_t fseek(FILE* stream, ssize_t offset, uint8_t whence);
    ssize_t ftell(FILE* stream);
}

File::File(const char* fname, const char* mode) {
    this->filename = (char*)fname;
    this->mode = (char*)mode;
}

File::File(const std::string fname, const char* mode) {
    this->filename = fname.c_str;
    this->mode = (char*)mode;
}

bool File::open() {
    fp = fopen(filename, mode);

    if(!fp)
        return true;

    return false;
}

void File::seek(ssize_t offset, SeekWhence whence) {
    fseek(fp, offset, whence);
}

ssize_t File::tell() {
    return ftell(fp);
}

int File::read(char* buffer, size_t size, size_t count) {
    return fread_c(fp, count, size, buffer);
}

int File::read_all(char* buffer) {
    auto curpos = tell();
    seek(0, SeekWhence::End);

    auto endpos = tell();
    seek(0, SeekWhence::Set);

    auto r = fread_c(fp, endpos, 1, buffer);
    
    seek(curpos, SeekWhence::Set);

    return r;
}

size_t File::size() {
    size_t a = tell();
    seek(0, SeekWhence::End);
    
    size_t sz = tell();
    seek(a, SeekWhence::Set);

    return sz;
}

void File::close() {
    fclose(fp);
}

File::~File() { this->close(); }
