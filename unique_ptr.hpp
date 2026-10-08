#pragma once

#include "deleters.hpp"
#include <cstddef>
#include <type_traits>
#include <utility>

namespace smart_pointers {
//int f (int, double) { return 5;}
template<typename T, typename Deleter = default_delete<T>>
class UnqPtr {
private:
    T* ptr;
    Deleter deleter;
    
public:
    using pointer = T*;
    
    constexpr UnqPtr() noexcept : ptr(nullptr), deleter() {}
    constexpr UnqPtr(std::nullptr_t) noexcept : ptr(nullptr), deleter() {}
    explicit UnqPtr(pointer p) noexcept : ptr(p), deleter() {}
    UnqPtr(pointer p, const Deleter& d) noexcept : ptr(p), deleter(d) {}
    UnqPtr(pointer p, Deleter&& d) noexcept : ptr(p), deleter(std::move(d)) {}
    UnqPtr(UnqPtr&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }
    
    template<typename U, typename E>
    requires std::is_convertible_v<U*, T*> && std::is_convertible_v<E, Deleter>
    UnqPtr(UnqPtr<U, E>&& other) noexcept
        : ptr(other.release()), deleter(std::move(other.get_deleter())) {}
    
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;
    
    ~UnqPtr() { 
        if (ptr){
            deleter(ptr); 
        }
    }
    
    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this->get() != other.get()) {
            reset(other.release());
            deleter = std::move(other.deleter);
        }
        return *this;
    }
    
    UnqPtr& operator=(std::nullptr_t) noexcept { 
        reset(); 
        return *this; 
    }
    
    pointer release() noexcept {
        pointer tmp = ptr;
        ptr = nullptr;
        return tmp;
    }
    
    void reset(pointer p = pointer()) noexcept { // (T*){} можно написать, но это уже рофл какой-то или мб стоит nullptr просто?
        pointer old = ptr;
        ptr = p;
        if (old){
            deleter(old);
        }
    }
    
    void swap(UnqPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(deleter, other.deleter);
    }
    
    pointer get() const noexcept { return ptr; }
    Deleter& get_deleter() noexcept { return deleter; }
    const Deleter& get_deleter() const noexcept { return deleter; }
    explicit operator bool() const noexcept { return ptr != nullptr; }
    
    decltype(auto) operator*() const { return *ptr; } // можно кнш T& но потом траблы будут
    pointer operator->() const noexcept { return ptr; }
};

template<typename T, typename Deleter>
class UnqPtr<T[], Deleter> {
private:
    T* ptr;
    Deleter deleter;
    
public:
    using pointer = T*;
    
    constexpr UnqPtr() noexcept : ptr(nullptr), deleter() {}
    constexpr UnqPtr(std::nullptr_t) noexcept : ptr(nullptr), deleter() {}
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    explicit UnqPtr(U* p) noexcept : ptr(p), deleter() {}
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    UnqPtr(U* p, const Deleter& d) noexcept : ptr(p), deleter(d) {}
    
    UnqPtr(UnqPtr&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }
    
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;
    
    ~UnqPtr() { 
        if (ptr){
            deleter(ptr);
         }
    }
    
    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this->get() != other.get()) {
            reset(other.release());
            deleter = std::move(other.deleter);
        }
        return *this;
    }
    
    UnqPtr& operator=(std::nullptr_t) noexcept { 
        reset(); 
        return *this; 
    }
    
    pointer release() noexcept {
        pointer tmp = ptr;
        ptr = nullptr;
        return tmp;
    }
    
    void reset(std::nullptr_t = nullptr) noexcept {
        pointer old = ptr;
        ptr = nullptr;
        if (old){
            deleter(old);
        }
    }
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    void reset(U* p) noexcept {
        pointer old = ptr;
        ptr = p;
        if (old) deleter(old);
    }
    
    void swap(UnqPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(deleter, other.deleter);
    }
    
    pointer get() const noexcept { return ptr; }
    Deleter& get_deleter() noexcept { return deleter; }
    const Deleter& get_deleter() const noexcept { return deleter; }
    explicit operator bool() const noexcept { return ptr != nullptr; }
    
    T& operator[](std::size_t i) const { return ptr[i]; }
};

template<typename T, typename D>
bool operator==(const UnqPtr<T, D>& lhs, std::nullptr_t) noexcept { return !lhs; }
template<typename T, typename D>
bool operator==(std::nullptr_t, const UnqPtr<T, D>& rhs) noexcept { return !rhs; }
template<typename T, typename D>
bool operator!=(const UnqPtr<T, D>& lhs, std::nullptr_t) noexcept { return static_cast<bool>(lhs); }
template<typename T, typename D>
bool operator!=(std::nullptr_t, const UnqPtr<T, D>& rhs) noexcept { return static_cast<bool>(rhs); }

template<typename T, typename... Args>
UnqPtr<T> make_unq(Args&&... args) {
    return UnqPtr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
UnqPtr<T[]> make_unq_arr(std::size_t size) {
    return UnqPtr<T[]>(new std::remove_extent_t<T>[size]());
}

} // namespace smart_pointers