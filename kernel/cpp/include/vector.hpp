#pragma once

#include "stdint.hpp"
#include "memory.hpp"

namespace std {
    template <typename T>
    class vector {
        public:
            vector() {
                elements = (T*)memory::alloc(sizeof(T));
            }

            void push_back(T element) {
                T* newptr = (T*)memory::alloc(sizeof(T) * (length + 1));

                memory::copy(newptr, elements, length);
                newptr[length] = element;
                length++;


                memory::free(elements);
                elements = newptr;
            }

            void remove(size_t index) {
                for (size_t i = index; i < length; i++) {
                    elements[i] = elements[i+1];
                }

                length--;
            }

            size_t get_length() const {
                return length;
            }

            ~vector() {
                memory::free(elements);
            }

            T operator [] (size_t index) const {
                return elements[index];
            }
        
        private:
            size_t length = 0;
            T* elements = nullptr;
    };
}