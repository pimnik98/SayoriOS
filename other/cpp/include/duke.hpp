#pragma once

#include "string.hpp"

namespace graphics {
    class Duke {
        typedef struct DukeImageMeta {
            uint8_t  magic[4];
            uint16_t width;
            uint16_t height;
            uint32_t data_length;
            uint8_t  alpha;
        } __attribute__((packed)) DukeImageMeta_t;

        public:
            Duke(const std::string) {}
            ~Duke() {}

        private:

        std::string filename;
    };
}