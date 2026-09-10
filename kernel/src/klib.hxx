#pragma once
// IWYU pragma: always_keep
#include "klib.h" // IWYU pragma: export

namespace std {

template <typename T> struct remove_reference      { using type = T; };
template <typename T> struct remove_reference<T&>  { using type = T; };
template <typename T> struct remove_reference<T&&> { using type = T; };
template <typename T> using remove_reference_t = remove_reference<T>::type;

template <typename T>
static inline remove_reference_t<T>&&
move(T&& v)
{
    return static_cast<remove_reference_t<T>&&>(v);
}

template <typename T>
static inline T&&
forward(std::remove_reference_t<T>& v)
{
    return static_cast<T&&>(v);
}

template <typename T>
static inline T&&
forward(std::remove_reference_t<T>&& v)
{
    return static_cast<T&&>(v);
}

template <typename T, typename U = T>
static inline T
exchange(T& obj, U&& new_val)
{
    T old_val = move(obj);
    obj = forward<U>(new_val);
    return old_val;
}

template <typename T>
static inline T
min(T a, T b)
{
    return b < a ? b : a;
}

template <typename T>
static inline T
max(T a, T b)
{
    return a < b ? b : a;
}

template <typename T>
static inline T
clamp(T val, T min, T max)
{
    return val < min ? min : (val > max ? max : val);
}

template <typename T, size_t N>
struct array {
    T data[N];
    constexpr auto& operator[](this auto&& self, size_t idx) { return self.data[idx]; }
    constexpr size_t count() const { return N; }
};

} // namespace std

struct result_err {
    constexpr result_err()              : errc(error_code(0)) {}
    constexpr result_err(error_code e)  : errc(e) { assert(is_err()); }

    constexpr error_code err() {
        assert(is_err());
        return errc;
    }

    constexpr bool is_ok() { return errc == 0; }
    constexpr bool is_err() { return errc != 0; }
    constexpr explicit operator bool() { return is_ok(); }
protected:
    error_code errc;
};

template <typename T>
struct [[nodiscard]] result : result_err {
    constexpr result(T&& t)         : t(std::move(t)) {}
    constexpr result(error_code e)  : result_err(e) {}

    constexpr ~result() {
        if (is_ok())
            t.~T();
    }

    constexpr T&& value() {
        assert(is_ok());
        return std::move(t);
    }
private:
    union {
        T t;
    };
};

template <>
struct [[nodiscard]] result<void> : result_err {
    struct unit {};

    constexpr result()              : result_err() {}
    constexpr result(unit)          : result_err() {}
    constexpr result(error_code e)  : result_err(e) {}

    constexpr unit value() {
        assert(is_ok());
        return {};
    }
};

template <typename T>
requires __is_trivially_destructible(T)
struct [[nodiscard]] result<T> : result_err {
    constexpr result(T t)           : t(t) {}
    constexpr result(error_code e)  : result_err(e) {}

    constexpr T value() {
        assert(is_ok());
        return t;
    }
private:
    T t;
};

#define TRY(expr) ({                        \
    auto __r = (expr);                      \
    if (__r.is_err()) return __r.err();     \
    __r.value();                            \
})

struct pinned {
    pinned(const pinned&) = delete;
    pinned(pinned&&) = delete;
    pinned& operator=(const pinned&) = delete;
    pinned& operator=(pinned&&) = delete;
    ~pinned() = default;
};
