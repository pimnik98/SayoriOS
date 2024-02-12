#ifndef SAYORIOS_FILEIO_H
#define SAYORIOS_FILEIO_H
bool is_file(const char* Path);
bool is_dir(const char* Path);
bool file_exists(const char* Path);
size_t filesize(const char* Path);
size_t filemtime(const char* Path);
bool is_readable(const char* Path);
bool is_writable(const char* Path);
bool is_executable(const char* Path);
uint32_t fileperms(const char* Path);
bool touch(const char* Path);
bool mkdir(const char* Path);
bool unlink(const char* Path);
bool rmdir(const char* Path);
#endif //SAYORIOS_FILEIO_H
