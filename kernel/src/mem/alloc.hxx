#pragma once

#include "alloc.h"
#include "klib.hxx"

constexpr void* operator new(usize, void* p) noexcept { return p; };

template <typename T>
static inline T*
kalloc_t()
{
    constexpr static auto size_class = alloc_calc_size_class(sizeof(T));
    return (T*)kalloc_class(size_class);
}

template <typename T>
static inline void
kfree_t(T* ptr)
{
    constexpr static auto size_class = alloc_calc_size_class(sizeof(T));
    kfree_class(ptr, size_class);
}

template <typename T>
struct owned {
    owned() {}

    owned(const owned&) = delete;
    owned& operator=(const owned&) = delete;

    constexpr owned(owned&& o) : ptr_(std::exchange(o.ptr_, nullptr)) {}
    constexpr owned& operator=(owned&& o) {
        if (this != &o) {
            ~owned();
            ptr_ = std::exchange(o.ptr_, nullptr);
        }
        return *this;
    }

    constexpr ~owned() {
        if (ptr_) {
            ptr_->~T();
            kfree_class(ptr_, size_class);
            ptr_ = nullptr;
        }
    }

    T* get() { return ptr_; }
    T* operator->() { return ptr_; }
    T& operator*() { return *ptr_; }
    explicit operator bool() { return ptr_; }

    [[nodiscard]] T* release() { return std::exchange(ptr_, nullptr); }

    template <typename... Ts>
    static inline result<owned> make(Ts&&...vs)
    {
        auto ptr = TRY_ALLOC(kalloc_class(size_class));
        return owned(new(ptr) T{std::forward<Ts>(vs)...});
    }
private:
    static const int size_class = alloc_calc_size_class(sizeof(T));
    constexpr explicit owned(T* ptr) : ptr_(ptr) {}
    T* ptr_{};
};
