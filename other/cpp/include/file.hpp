#pragma once

#include "string.hpp"

namespace std {
    typedef struct {
        char* path;
        int32_t size;
        uint32_t fmode;
        bool open;
        size_t pos;
        uint32_t err;
    } FILE;

    typedef enum : uint8_t {
        Set     = 0,
        Current = 1,
        End     = 2
    } SeekWhence;

    class File {
        public:
            File(const char*,      const char* mode = "r");
            File(const std::string, const char* mode = "r");

            bool open();

            void seek(ssize_t offset, SeekWhence whence = SeekWhence::Set);
            ssize_t tell();
            
            int  read(char* buffer, size_t size, size_t count);
            int  read_all(char* buffer);
            
            // Dangerous for streams.
            size_t size();
            
            void close();

            ~File();
        
        private:
           	char* filename;
            char* mode;
            FILE* fp;
    };
}
