#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace smart_pointers {

template<typename T>
struct default_delete {
    constexpr default_delete() noexcept = default;
    
    template<typename U>
    requires std::is_convertible_v<U*, T*>
    constexpr default_delete(const default_delete<U>&) noexcept {}
    
    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
        delete ptr;
    }
};

template<typename T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    constexpr default_delete(const default_delete<U[]>&) noexcept {}
    
    template<typename U>
    requires std::is_convertible_v<U(*)[], T(*)[]>
    void operator()(U* ptr) const {
        static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
        delete[] ptr;
    }
};

} // namespace smart_pointers