#pragma once

#include "stdint.hpp"
#include "log.hpp"

namespace std {
    template <typename T>
    class vector {
        public:
            vector() {}

            void push_back(const T& element) {
                if (size >= capacity) {
                    size_t new_capacity = capacity == 0 ? 1 : 2 * capacity;
                    
                    T* new_data = new T[new_capacity];
                    
                    for (size_t i = 0; i < size; ++i) {
                        new_data[i] = data[i];
                    }
                    
                    delete[] data;
                    
                    data = new_data;
                    capacity = new_capacity;
                }

                // std::log << "Data at: " << (void*)data << endl;

                data[size++] = element;
            }

            void remove(size_t index) {
                for (size_t i = index; i < size - 1; ++i) {
                    data[i] = data[i + 1];
                }
                --size;
            }

            size_t get_length() const {
                return size;
            }

            T* begin() {
                return data;
            }

            T* end() {
                return data + size;
            }

            ~vector() {
                delete[] data;
            }

            T operator [] (size_t index) const {
                return data[index];
            }
        
        private:
            size_t size = 0;
            size_t capacity = 0;
            T* data = nullptr;
    };
}