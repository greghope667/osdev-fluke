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

template <typename T, T v>
struct integral_constant {
    using value_type = T;
    static constexpr T value = v;
};

template <bool b> using bool_constant = integral_constant<bool, b>;

template<class T>
struct is_integral : std::bool_constant<
requires (T t, T* p, void (*f)(T)) // T* parameter excludes reference types
{
    reinterpret_cast<T>(t); // Exclude class types
    f(0); // Exclude enumeration types
    p + t; // Exclude everything not yet excluded but integral types
}> {};

template <typename T>
constexpr bool is_integral_v = is_integral<T>::value;

} // namespace std

template <typename T>
struct [[nodiscard]] result {
    constexpr result(T&& t)
        : t(std::move(t)), errc(error_code(0)) {}
    constexpr result(error_code e)
        : c(), errc(e) { assert(is_err()); }

    result(const result&) = delete;
    result(result&&) = delete;
    result& operator=(const result&) = delete;
    result& operator=(result&&) = delete;

    constexpr ~result() {
        if (is_ok())
            t.~T();
    }

    constexpr T&& value() {
        assert(is_ok());
        return std::move(t);
    }

    constexpr error_code err() {
        assert(is_err());
        return errc;
    }

    constexpr bool is_ok() { return errc == 0; }
    constexpr bool is_err() { return errc != 0; }
    constexpr explicit operator bool() { return is_ok(); }
private:
    union {
        T t;
        char c;
    };
    error_code errc;
};

template <>
struct [[nodiscard]] result<void> {
    struct unit {};

    constexpr result()
        : errc(error_code(0)) {}
    constexpr result(unit)
        : errc(error_code(0)) {}
    constexpr result(error_code e)
        : errc(e) { assert(is_err()); }

    constexpr unit value() {
        assert(is_ok());
        return {};
    }

    constexpr error_code err() {
        assert(is_err());
        return errc;
    }

    constexpr bool is_ok() { return errc == 0; }
    constexpr bool is_err() { return errc != 0; }
    constexpr explicit operator bool() { return is_ok(); }
private:
    error_code errc;
};

template <typename T>
struct [[nodiscard]] result_trivial {
    constexpr result_trivial(T t)
        : t(t), errc(error_code(0)) {}
    constexpr result_trivial(error_code e)
        : t(), errc(e) { assert(is_err()); }

    constexpr T value() {
        assert(is_ok());
        return t;
    }

    constexpr error_code err() {
        assert(is_err());
        return errc;
    }

    constexpr bool is_ok() { return errc == 0; }
    constexpr bool is_err() { return errc != 0; }
    constexpr explicit operator bool() { return is_ok(); }
private:
    T t;
    error_code errc;
};

template <typename T>
struct [[nodiscard]] result<T*> : public result_trivial<T*> {
    constexpr result(T* t) : result_trivial<T*>(t) {}
    constexpr result(error_code e) : result_trivial<T*>(e) {}
};

template <typename T>
requires std::is_integral_v<T>
struct [[nodiscard]] result<T> : public result_trivial<T> {
    constexpr result(T t) : result_trivial<T>(t) {}
    constexpr result(error_code e) : result_trivial<T>(e) {}
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
