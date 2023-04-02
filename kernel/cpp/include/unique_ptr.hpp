#pragma once

template <typename T>
class unique_ptr {
    public: 
        unique_ptr(T* t) : ptr(t) {}

        ~unique_ptr() {
            delete ptr;
        }

        T* operator->() noexcept {
            return this->ptr;
        }

        T& operator*() noexcept {
            return *this->ptr;
        }

        T* ptr = nullptr;
};