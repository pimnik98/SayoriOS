#pragma once

#include "string.hpp"

namespace std {
    class type_info {
        public:

        type_info(const char* name) : name_(name) {}

        bool operator==(const type_info& rhs) const {
            return strcmp(name_, rhs.name_) == 0;
        }

        bool operator!=(const type_info& rhs) const {
            return !(*this == rhs);
        }

        const char* name() const {
            return name_;
        }
    
        private:

        const char* name_;
    };
}