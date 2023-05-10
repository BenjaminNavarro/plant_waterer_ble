#pragma once

/**
 * @defgroup bitmask bitmask: an utility to manage bitmasks
 *
 */

/**
 * @file bitmask.h
 * @author Robin Passama
 * @brief global header file for pid bitmask
 * @ingroup bitmask
 * @ingroup pid-utils
 * @example bitmask_example.cpp
 *
 */

#pragma once

/*
    Bitmask
    =======
    A generic implementation of the BitmaskType C++ concept
    http://en.cppreference.com/w/cpp/concept/BitmaskType
    Version: 1.1.2
    Latest version and documentation:
        https://github.com/oliora/bitmask
    Copyright (c) 2016-2017 Andrey Upadyshev (oliora@gmail.com)
    Distributed under the Boost Software License, Version 1.0.
    See http://www.boost.org/LICENSE_1_0.txt
    Changes history
    ---------------
    v1.1.2:
        - Fix: Can not define bitmask for a class local enum
   (https://github.com/oliora/bitmask/issues/3) v1.1.1:
        - Added missed `<limits>` header
        - README and code comments updated
    v1.1:
        - Change namespace from `boost` to `bitmask`
        - Add CMake package
          https://github.com/oliora/bitmask/issues/1
    v1.0:
        - Initial release
 */

#include <type_traits>
#include <functional> // for std::hash
#include <limits>     // for std::numeric_limits
#include <cassert>

namespace pid {

namespace bitmask_detail {
// Let's use std::void_t. It's introduced in C++17 but we can easily add it by
// ourself See more at http://en.cppreference.com/w/cpp/types/void_t
template <typename... Ts>
struct MakeVoid {
    using type = void;
};
template <typename... Ts>
using void_t = typename MakeVoid<Ts...>::type;

template <class T>
struct UnderlyingType {
    static_assert(std::is_enum<T>::value, "T is not a enum type");

    using type = typename std::make_unsigned<
        typename std::underlying_type<T>::type>::type;
};

template <class T>
using underlying_type_t = typename UnderlyingType<T>::type;

template <class T, T MaxElement = T::BITMASK_MAX_ELEMENT_>
struct MaskFromMaxElement {
    static constexpr underlying_type_t<T> MAX_ELEMENT_VALUE_ =
        static_cast<underlying_type_t<T>>(MaxElement);

    static_assert(MAX_ELEMENT_VALUE_ >= 0, "Max element is negative");

    // If you really have to define a bitmask that uses the highest bit of
    // signed type (i.e. the sign bit) then define the value mask rather than
    // the max element.
    static_assert(
        MAX_ELEMENT_VALUE_ <=
            (std::numeric_limits<
                 typename std::underlying_type<T>::type>::max() >>
             1) +
                1,
        "Max element is greater than the underlying type's highest bit");

    // `((value - 1) << 1) + 1` is used rather that simpler `(value << 1) - 1`
    // because latter overflows in case if `value` is the highest bit of the
    // underlying type.
    static constexpr underlying_type_t<T> value =
        MAX_ELEMENT_VALUE_ ? ((MAX_ELEMENT_VALUE_ - 1) << 1) + 1 : 0;
};

template <class, class = void_t<>>
struct has_max_element : std::false_type {};

template <class T>
struct has_max_element<T, void_t<decltype(T::BITMASK_MAX_ELEMENT_)>>
    : std::true_type {};

#if !defined _MSC_VER
template <class, class = void_t<>>
struct has_value_mask : std::false_type {};

template <class T>
struct has_value_mask<T, void_t<decltype(T::_bitmask_value_mask)>>
    : std::true_type {};
#else
// MS Visual Studio 2015 (even Update 3) has weird support for expressions
// SFINAE so I can't get a real check for `has_value_mask` to compile.
template <class T>
struct has_value_mask
    : std::integral_constant<bool, !has_max_element<T>::value> {};
#endif

template <class T>
struct is_valid_enum_definition
    : std::integral_constant<bool, !(has_value_mask<T>::value &&
                                     has_max_element<T>::value)> {};

template <class, class = void>
struct enum_mask;

template <class T>
struct enum_mask<T, typename std::enable_if<has_max_element<T>::value>::type>
    : std::integral_constant<underlying_type_t<T>,
                             MaskFromMaxElement<T>::value> {};

template <class T>
struct enum_mask<T, typename std::enable_if<has_value_mask<T>::value>::type>
    : std::integral_constant<underlying_type_t<T>,
                             static_cast<underlying_type_t<T>>(
                                 T::_bitmask_value_mask)> {};

template <class T>
inline constexpr underlying_type_t<T>
disable_unused_function_warnings() noexcept {
    return (static_cast<T>(0) & static_cast<T>(0)).bits() &
           (static_cast<T>(0) | static_cast<T>(0)).bits() &
           (static_cast<T>(0) ^ static_cast<T>(0)).bits() &
           (~static_cast<T>(0)).bits() & bits(static_cast<T>(0));
}

template <class Assert>
inline void constexpr_assert_failed(Assert&& a) noexcept {
    a();
}

// When evaluated at compile time emits a compilation error if condition is not
// true. Invokes the standard assert at run time.
#define bitmask_constexpr_assert(cond)                                         \
    ((void)((cond) ? 0                                                         \
                   : (pid::bitmask_detail::constexpr_assert_failed(            \
                          [&]() { assert(cond); }),                            \
                      0)))

template <class T>
inline constexpr T checked_value(T value, T mask) {
    return bitmask_constexpr_assert((value & ~mask) == 0), value;
}
} // namespace bitmask_detail

template <class T>
inline constexpr bitmask_detail::underlying_type_t<T>
get_enum_mask(const T&) noexcept {
    static_assert(bitmask_detail::is_valid_enum_definition<T>::value,
                  "Both of BITMASK_MAX_ELEMENT_ and _bitmask_value_mask are "
                  "specified");
    return bitmask_detail::enum_mask<T>::value;
}

template <class T>
class Bitmask {
public:
    using value_type = T;
    using underlying_type = bitmask_detail::underlying_type_t<T>;

    static constexpr underlying_type mask_value =
        get_enum_mask(static_cast<value_type>(0));

    constexpr Bitmask() noexcept = default;
    constexpr Bitmask(std::nullptr_t) noexcept : bits_{0} {
    }

    constexpr Bitmask(const Bitmask& other) noexcept : bits_{other.bits_} {
    }
    constexpr Bitmask(Bitmask&& other) noexcept : bits_{other.bits_} {
    }
    constexpr Bitmask& operator=(const Bitmask& other) noexcept {
        if (this != &other) {
            bits_ = other.bits_;
        }
        return *this;
    }

    constexpr Bitmask& operator=(Bitmask&& other) noexcept {
        bits_ = other.bits_;
        return *this;
    }

    constexpr Bitmask(value_type value) noexcept
        : bits_{bitmask_detail::checked_value(
              static_cast<underlying_type>(value), mask_value)} {
    }

    constexpr underlying_type bits() const noexcept {
        return bits_;
    }

    constexpr explicit operator bool() const noexcept {
        return bits();
    }

    constexpr Bitmask operator~() const noexcept {
        return Bitmask{std::true_type{}, ~bits_ & mask_value};
    }

    constexpr Bitmask operator&(const Bitmask& r) const noexcept {
        return Bitmask{std::true_type{}, bits_ & r.bits_};
    }

    constexpr Bitmask operator|(const Bitmask& r) const noexcept {
        return Bitmask{std::true_type{}, bits_ | r.bits_};
    }

    constexpr Bitmask operator^(const Bitmask& r) const noexcept {
        return Bitmask{std::true_type{}, bits_ ^ r.bits_};
    }

    Bitmask& operator|=(const Bitmask& r) noexcept {
        bits_ |= r.bits_;
        return *this;
    }

    Bitmask& operator&=(const Bitmask& r) noexcept {
        bits_ &= r.bits_;
        return *this;
    }

    Bitmask& operator^=(const Bitmask& r) noexcept {
        bits_ ^= r.bits_;
        return *this;
    }

private:
    template <class U>
    constexpr Bitmask(std::true_type, U bits) noexcept
        : bits_(static_cast<underlying_type>(bits)) {
    }

    underlying_type bits_ = 0;
};

template <class T>
inline constexpr Bitmask<T> operator&(T l, const Bitmask<T>& r) noexcept {
    return r & l;
}

template <class T>
inline constexpr Bitmask<T> operator|(T l, const Bitmask<T>& r) noexcept {
    return r | l;
}

template <class T>
inline constexpr Bitmask<T> operator^(T l, const Bitmask<T>& r) noexcept {
    return r ^ l;
}

template <class T>
inline constexpr bool operator!=(const Bitmask<T>& l,
                                 const Bitmask<T>& r) noexcept {
    return l.bits() != r.bits();
}

template <class T>
inline constexpr bool operator==(const Bitmask<T>& l,
                                 const Bitmask<T>& r) noexcept {
    return !operator!=(l, r);
}

template <class T>
inline constexpr bool operator!=(T l, const Bitmask<T>& r) noexcept {
    return static_cast<bitmask_detail::underlying_type_t<T>>(l) != r.bits();
}

template <class T>
inline constexpr bool operator==(T l, const Bitmask<T>& r) noexcept {
    return !operator!=(l, r);
}

template <class T>
inline constexpr bool operator!=(const Bitmask<T>& l, T r) noexcept {
    return l.bits() != static_cast<bitmask_detail::underlying_type_t<T>>(r);
}

template <class T>
inline constexpr bool operator==(const Bitmask<T>& l, T r) noexcept {
    return !operator!=(l, r);
}

template <class T>
inline constexpr bool operator!=(const bitmask_detail::underlying_type_t<T>& l,
                                 const Bitmask<T>& r) noexcept {
    return l != r.bits();
}

template <class T>
inline constexpr bool operator==(const bitmask_detail::underlying_type_t<T>& l,
                                 const Bitmask<T>& r) noexcept {
    return !operator!=(l, r);
}

template <class T>
inline constexpr bool
operator!=(const Bitmask<T>& l,
           const bitmask_detail::underlying_type_t<T>& r) noexcept {
    return l.bits() != r;
}

template <class T>
inline constexpr bool
operator==(const Bitmask<T>& l,
           const bitmask_detail::underlying_type_t<T>& r) noexcept {
    return !operator!=(l, r);
}

// Allow `bitmask` to be be used as a map key
template <class T>
inline constexpr bool operator<(const Bitmask<T>& l,
                                const Bitmask<T>& r) noexcept {
    return l.bits() < r.bits();
}

template <class T>
inline constexpr bitmask_detail::underlying_type_t<T>
bits(const Bitmask<T>& mask) noexcept {
    return mask.bits();
}

// Implementation

template <class T>
constexpr typename Bitmask<T>::underlying_type Bitmask<T>::mask_value;
} // namespace pid

namespace std {
template <class T>
struct hash<pid::Bitmask<T>> {
    constexpr std::size_t operator()(const pid::Bitmask<T>& op) const noexcept {
        using ut = typename pid::Bitmask<T>::underlying_type;
        return std::hash<ut>{}(op.bits());
    }
};
} // namespace std

// Implementation detail macros
#define PID_UTILS_BITMASK_DETAIL_CONCAT_IMPL(a, b) a##b
#define PID_UTILS_BITMASK_DETAIL_CONCAT(a, b)                                  \
    PID_UTILS_BITMASK_DETAIL_CONCAT_IMPL(a, b)

// Must be user-defined if compiler doesn't have `__COUNTER__` macro
#ifndef PID_UTILS_BITMASK_MAKE_UNIQUE_NAME
#define PID_UTILS_BITMASK_MAKE_UNIQUE_NAME(prefix)                             \
    PID_UTILS_BITMASK_DETAIL_CONCAT(prefix, __COUNTER__)
#endif

#define PID_UTILS_BITMASK_DETAIL_DEFINE_OPS(value_type)                        \
    inline constexpr pid::Bitmask<value_type> operator&(                       \
        value_type l, value_type r) noexcept {                                 \
        return pid::Bitmask<value_type>{l} & r;                                \
    }                                                                          \
    inline constexpr pid::Bitmask<value_type> operator|(                       \
        value_type l, value_type r) noexcept {                                 \
        return pid::Bitmask<value_type>{l} | r;                                \
    }                                                                          \
    inline constexpr pid::Bitmask<value_type> operator^(                       \
        value_type l, value_type r) noexcept {                                 \
        return pid::Bitmask<value_type>{l} ^ r;                                \
    }                                                                          \
    inline constexpr pid::Bitmask<value_type> operator~(                       \
        value_type op) noexcept {                                              \
        return ~pid::Bitmask<value_type>{op};                                  \
    }                                                                          \
    inline constexpr pid::Bitmask<value_type>::underlying_type bits(           \
        value_type op) noexcept {                                              \
        return pid::Bitmask<value_type>{op}.bits();                            \
    }                                                                          \
    namespace bitmask_definition_detail {                                      \
    class PID_UTILS_BITMASK_MAKE_UNIQUE_NAME(                                  \
        _disable_unused_function_warnings_) {                                  \
        static constexpr int _unused() noexcept {                              \
            return pid::bitmask_detail::disable_unused_function_warnings<      \
                       value_type>(),                                          \
                   0;                                                          \
        }                                                                      \
    };                                                                         \
    }

#define PID_UTILS_BITMASK_DETAIL_DEFINE_VALUE_MASK(value_type, value_mask)     \
    inline constexpr pid::bitmask_detail::underlying_type_t<value_type>        \
    get_enum_mask(value_type) noexcept {                                       \
        return value_mask;                                                     \
    }

#define PID_UTILS_BITMASK_DETAIL_DEFINE_MAX_ELEMENT(value_type, max_element)   \
    inline constexpr pid::bitmask_detail::underlying_type_t<value_type>        \
    get_enum_mask(value_type) noexcept {                                       \
        return pid::bitmask_detail::MaskFromMaxElement<                        \
            value_type, value_type::max_element>::value;                       \
    }

// Public macros

// Defines missing operations for a bit-mask elements enum 'value_type'
// Value mask is taken from 'value_type' definition. One should has either
// '_bitmask_value_mask' or 'BITMASK_MAX_ELEMENT_' element defined.
#define PID_UTILS_BITMASK_DEFINE(value_type)                                   \
    PID_UTILS_BITMASK_DETAIL_DEFINE_OPS(value_type)

// Defines missing operations and a value mask for
// a bit-mask elements enum 'value_type'
#define PID_UTILS_BITMASK_DEFINE_VALUE_MASK(value_type, value_mask)            \
    PID_UTILS_BITMASK_DETAIL_DEFINE_VALUE_MASK(value_type, value_mask)         \
    PID_UTILS_BITMASK_DETAIL_DEFINE_OPS(value_type)

// Defines missing operations and a value mask based on max element for
// a bit-mask elements enum 'value_type'
#define PID_UTILS_BITMASK_DEFINE_MAX_ELEMENT(value_type, max_element)          \
    PID_UTILS_BITMASK_DETAIL_DEFINE_MAX_ELEMENT(value_type, max_element)       \
    PID_UTILS_BITMASK_DETAIL_DEFINE_OPS(value_type)
