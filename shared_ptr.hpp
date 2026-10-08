#pragma once
#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>
#include "deleters.hpp"

namespace smart_pointers {

class ControlBlockBase {
public:
    std::atomic<std::size_t> strong_count;
    virtual ~ControlBlockBase() = default;
    virtual void destroy_object() = 0;
protected:
    ControlBlockBase() : strong_count(1) {}
};

template<typename T, typename Deleter>
class ControlBlock : public ControlBlockBase {
private:-0
    T* ptr;
    Deleter deleter;
public:
    explicit ControlBlock(T* p, Deleter d = default_delete<T>())
        : ptr(p), deleter(std::move(d)) {}
    
    void destroy_object() override {
        if (ptr) deleter(ptr);
        delete this;
    }
};

template<typename T, typename Deleter>
class ControlBlock<T[], Deleter> : public ControlBlockBase {
private:
    T* ptr;
    Deleter deleter;
public:
    explicit ControlBlock(T* p, Deleter d)
        : ptr(p), deleter(std::move(d)) {}
    
    void destroy_object() override {
        if (ptr) deleter(ptr);
        delete this;
    }
};

template<typename T>
class ShrdPtr {
private:
    T* ptr;
    ControlBlockBase* ctrl;
    
    template<typename> friend class ShrdPtr;
    
public:
    using element_type = T;

    
    constexpr ShrdPtr() noexcept : ptr(nullptr), ctrl(nullptr) {}
    constexpr ShrdPtr(std::nullptr_t) noexcept : ptr(nullptr), ctrl(nullptr) {}
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    explicit ShrdPtr(U* p) 
        : ptr(p), ctrl(new ControlBlock<U, default_delete<U>>(p, default_delete<U>{})) {}
    
    template<typename U, typename E>
    requires std::is_convertible_v<U*, T*>
    ShrdPtr(U* p, E&& d) 
        : ptr(p), ctrl(new ControlBlock<U, std::decay_t<E>>(p, std::forward<E>(d))) {}
    
    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    ShrdPtr(ShrdPtr<U>&& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }
    
    
    ~ShrdPtr() {
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
    }
    
    
    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(other);
            swap(tmp);
        }
        return *this;
    }
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(other);
            swap(tmp);
        }
        return *this;
    }
    
    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    ShrdPtr& operator=(ShrdPtr<U>&& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }
    
    ShrdPtr& operator=(std::nullptr_t) noexcept { 
        reset(); 
        return *this; 
    }
    
    
    void reset() noexcept {
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
        ptr = nullptr;
        ctrl = nullptr;
    }
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    void reset(U* p) {
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
        if (p) {
            ptr = p;
            ctrl = new ControlBlock<U, default_delete<U>>(p, default_delete<U>{});
        } else {
            ptr = nullptr;
            ctrl = nullptr;
        }
    }
    
    void swap(ShrdPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(ctrl, other.ctrl);
    }
    
    T* get() const noexcept { return ptr; }
    
    std::add_lvalue_reference_t<T> operator*() const noexcept { return *ptr; }
    T* operator->() const noexcept { return ptr; }
    
    long use_count() const noexcept {
        return ctrl ? static_cast<long>(ctrl->strong_count.load(std::memory_order_acquire)) : 0;
    }
    
    bool unique() const noexcept { return use_count() == 1; }
    
    explicit operator bool() const noexcept { return ptr != nullptr; }
};



template<typename T>
class ShrdPtr<T[]> {
private:
    T* ptr;
    ControlBlockBase* ctrl;
    
    // template<typename> friend class ShrdPtr;
    
public:
    using element_type = T;
    
    constexpr ShrdPtr() noexcept : ptr(nullptr), ctrl(nullptr) {}
    constexpr ShrdPtr(std::nullptr_t) noexcept : ptr(nullptr), ctrl(nullptr) {}
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    explicit ShrdPtr(U* p) 
        : ptr(p), ctrl(new ControlBlock<U[], default_delete<U[]>>(p, default_delete<U[]>{})) {}
    
    template<typename U, typename E>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    ShrdPtr(U* p, E&& d) 
        : ptr(p), ctrl(new ControlBlock<U[], std::decay_t<E>>(p, std::forward<E>(d))) {}
    
    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    ShrdPtr<T> Copy(ShrdPtr<U>& other) noexcept{
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
        this.ptr = other.ptr;
        this.ctrl = other.ctrl;
        other.ptr = nullptr;
        other.ctrl = nullptr;
        return this;
    }

    ~ShrdPtr() {
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
    }
    
    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(other);
            swap(tmp);
        }
        return *this;
    }
    
    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this->ctrl != other.ctrl) {
            ShrdPtr tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }
    
    ShrdPtr& operator=(std::nullptr_t) noexcept { 
        reset(); 
        return *this; 
    }
    
    void reset() noexcept {
        if (ctrl && ctrl->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->destroy_object();
        }
        ptr = nullptr;
        ctrl = nullptr;
    }
    
    void swap(ShrdPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(ctrl, other.ctrl);
    }
    
    T* get() const noexcept { return ptr; }
    T& operator[](std::size_t i) const { return ptr[i]; }
    
    long use_count() const noexcept {
        return ctrl ? static_cast<long>(ctrl->strong_count.load(std::memory_order_acquire)) : 0;
    }
    
    bool unique() const noexcept { return use_count() == 1; }
    
    explicit operator bool() const noexcept { return ptr != nullptr; }
};

template<typename T>
bool operator==(const ShrdPtr<T>& lhs, std::nullptr_t) noexcept { return !lhs; }
template<typename T>
bool operator==(std::nullptr_t, const ShrdPtr<T>& rhs) noexcept { return !rhs; }
template<typename T>
bool operator!=(const ShrdPtr<T>& lhs, std::nullptr_t) noexcept { return static_cast<bool>(lhs); }
template<typename T>
bool operator!=(std::nullptr_t, const ShrdPtr<T>& rhs) noexcept { return static_cast<bool>(rhs); }


template<typename T, typename... Args>
ShrdPtr<T> make_shrd(Args&&... args) {
    return ShrdPtr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
ShrdPtr<T[]> make_shrd_arr(std::size_t size) {
    return ShrdPtr<T[]>(new std::remove_extent_t<T>[size]());
}

} // namespace smart_pointers