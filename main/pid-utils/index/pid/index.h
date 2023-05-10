//! \file index.h
//! \author Benjamin Navarro
//! \brief Define the Index class and all its related operators
//! \date 12-2022

#pragma once

#include <pid/unreachable.h>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <stdexcept>

#ifndef PID_UTILS_INDEX_FORCE_CHECKS
#define PID_UTILS_INDEX_FORCE_CHECKS 0
#endif

#ifndef PID_UTILS_INDEX_CHECKS_THROW
#define PID_UTILS_INDEX_CHECKS_THROW 0
#endif

#if defined(NDEBUG) and PID_UTILS_INDEX_FORCE_CHECKS == 0
#define PID_UTILS_INDEX_NOEXCEPT noexcept
#define PID_UTILS_INDEX_ASSERT(expr, msg)
#else
#if PID_UTILS_INDEX_CHECKS_THROW == 1
#define PID_UTILS_INDEX_NOEXCEPT
#define PID_UTILS_INDEX_ASSERT(expr, msg)                                      \
    if (not(expr)) {                                                           \
        throw std::domain_error(msg);                                          \
    }
#else
#define PID_UTILS_INDEX_NOEXCEPT noexcept
#define PID_UTILS_INDEX_ASSERT(expr, msg) assert(expr&& msg)
#endif
#endif

namespace pid {

//! \brief Represent an index for a container that can be safely constructed
//! from/casted to any standard integer type. Relational and basic arithmetic
//! operators are provided to cover all the possible signedness/width
//! configurations.
//!
//! If an operation would result in a value not representable by an Index, an
//! error will be triggered. The error will take the form of an assert (default)
//! or of an exception (std::domain_error) depending on the value of
//! PID_UTILS_INDEX_CHECKS_THROW (default is 0, define to 1 to switch behavior).
//! Checks are performed only when NDEBUG is defined or if
//! PID_UTILS_INDEX_FORCE_CHECKS is defined to 1 (default is 0).
//!
//! The index value stored internally is an std::int64_t so that it can store
//! all the possible integer values, except the ones >= 2^63 which can be stored
//! in a std::uint64_t. These values should rarely, if ever, be used as indexes
//! and so shouldn't be missed by anyone. Using an std::uint64_t would prohibit
//! negative indexes, which are more commonly useful.
//!
struct Index {
    using type = std::int64_t;
    using utype = std::make_unsigned_t<type>;

    //! \brief Tell if the given value can be represented by an Index and so
    //! stored without changing its value
    template <typename T>
    [[nodiscard]] static constexpr bool
    is_representable([[maybe_unused]] T idx) PID_UTILS_INDEX_NOEXCEPT {
        static_assert(std::is_integral_v<T>);

        static_assert(sizeof(T) <= sizeof(type),
                      "Only standard integer types are supported");

        if constexpr (std::is_same_v<utype, T>) {
            return idx <= umax();
        }

        return true;
    }

    //! \brief Construct an index with a value of 0
    constexpr Index() noexcept = default;

    //! \brief Construct an Index from any integral value
    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    /* implicit */ constexpr Index(T idx) PID_UTILS_INDEX_NOEXCEPT
        : index{static_cast<type>(idx)} {
        PID_UTILS_INDEX_ASSERT(
            is_representable(idx),
            "The given value is too large to be held as an index");
    }

    //! \brief Convert the current index value to the given type
    template <typename T>
    [[nodiscard]] constexpr T as() const PID_UTILS_INDEX_NOEXCEPT {
        static_assert(std::is_integral_v<T>);

        static_assert(sizeof(T) <= sizeof(type),
                      "Only standard integer types are supported");

        if constexpr (std::is_unsigned_v<T>) {
            PID_UTILS_INDEX_ASSERT(
                index >= 0,
                "Cannot convert a negative index to an unsigned value");
        }

        if constexpr (std::is_same_v<T, utype>) {
            PID_UTILS_INDEX_ASSERT(static_cast<T>(index) <=
                                       std::numeric_limits<T>::max(),
                                   "Index is out of bound for the target type");
        } else {
            PID_UTILS_INDEX_ASSERT(
                index <= static_cast<type>(std::numeric_limits<T>::max()),
                "Index is out of bound for the target type");
            PID_UTILS_INDEX_ASSERT(
                index >= static_cast<type>(std::numeric_limits<T>::min()),
                "Index is out of bound for the target type");
        }

        return static_cast<T>(index);
    }

    //! \brief Conversion operator for integral types. Forward the call to
    //! cast()
    template <typename T, std::enable_if_t<std::is_integral_v<T> and
                                               not std::is_same_v<T, type>,
                                           int> = 0>
    [[nodiscard]] /* implicit */ constexpr
    operator T() const PID_UTILS_INDEX_NOEXCEPT {
        return as<T>();
    }

    //! \brief Conversion operator for integral types. Forward the call to
    //! cast()
    //!
    //! Keep this not non-templated so that compilers resolve it for C-style
    //! array indexing
    [[nodiscard]] /* implicit */ constexpr
    operator type() const PID_UTILS_INDEX_NOEXCEPT {
        return index;
    }

    [[nodiscard]] constexpr bool
    operator==(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index == other.index;
    }

    [[nodiscard]] constexpr bool
    operator!=(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index != other.index;
    }

    [[nodiscard]] constexpr bool
    operator<(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index < other.index;
    }

    [[nodiscard]] constexpr bool
    operator<=(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index <= other.index;
    }

    [[nodiscard]] constexpr bool
    operator>(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index > other.index;
    }

    [[nodiscard]] constexpr bool
    operator>=(Index other) const PID_UTILS_INDEX_NOEXCEPT {
        return index >= other.index;
    }

    constexpr Index& operator++() PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(index != max(),
                               "Incrementing the index would overflow it (UB)");
        ++index;
        return *this;
    }

    constexpr Index operator++(int) PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(index != max(),
                               "Incrementing the index would overflow it (UB)");
        return Index{index++};
    }

    constexpr Index& operator--() PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(
            index != min(), "Decrementing the index would underflow it (UB)");
        --index;
        return *this;
    }

    constexpr Index operator--(int) PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(
            index != min(), "Decrementing the index would underflow it (UB)");
        return Index{index--};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index
    operator+(T other) const PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(
            (other.index >= 0 and (index <= max() - other.index)) or
                (other.index < 0 and (min() - other.index <= index)),
            "Performing the addition would overflow/underflow the index \
                   (UB)");
        return Index{index + other.index};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    constexpr Index& operator+=(T other) PID_UTILS_INDEX_NOEXCEPT {
        *this = *this + other;
        return *this;
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index
    operator-(T other) const PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(
            (other.index >= 0 and (min() + other.index) <= index) or
                (other.index < 0 and (index <= max() + other.index)),
            "Performing the subtraction would overflow/underflow the \
                   index (UB)");
        return Index{index - other.index};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    constexpr Index& operator-=(T other) PID_UTILS_INDEX_NOEXCEPT {
        *this = *this - other;
        return *this;
    }

    [[nodiscard]] constexpr Index operator-() const PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(
            index != min(),
            "Negating the index minimum value would overflow it (UB)");
        return Index{-index};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index
    operator*(T other) const PID_UTILS_INDEX_NOEXCEPT {
        [[maybe_unused]] auto check = [](type a, type b) {
            if (b == 0) {
                return true;
            } else if (b == -1) {
                return a != min();
            } else {
                return (min() / b <= a) and (a <= max() / b);
            }
        };

        PID_UTILS_INDEX_ASSERT(
            check(index, other.index),
            "Performing the multiplication would overflow the index (UB)");

        return Index{index * other.index};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index&
    operator*=(T other) PID_UTILS_INDEX_NOEXCEPT {
        *this = *this * other;
        return *this;
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index
    operator/(T other) const PID_UTILS_INDEX_NOEXCEPT {
        PID_UTILS_INDEX_ASSERT(other.index != 0,
                               "Trying to perform a division by zero (UB)");
        PID_UTILS_INDEX_ASSERT(
            not(index == min() and other.index == -1),
            "Performing the division would overflow the index (UB)");
        return Index{index / other.index};
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, Index>, int> = 0>
    [[nodiscard]] constexpr Index&
    operator/=(T other) PID_UTILS_INDEX_NOEXCEPT {
        *this = *this / other;
        return *this;
    }

    [[nodiscard]] static constexpr type min() {
        return std::numeric_limits<type>::min();
    }

    [[nodiscard]] static constexpr type max() {
        return std::numeric_limits<type>::max();
    }

    [[nodiscard]] static constexpr utype umax() {
        return static_cast<utype>(std::numeric_limits<type>::max());
    }

    type index{};
};

//! \brief type alias pid::Index as pid::index for naming consistency with
//! integer types
using index = pid::Index;

namespace literals {
//! \brief User-defined literal to create a pid::Index
//!
//! ### Example
//! ```cpp
//! using namespace pid::literals;
//! auto idx = 42_index;
//! ```
constexpr pid::Index operator""_index(unsigned long long index) {
    return static_cast<pid::Index::type>(index);
}
} // namespace literals

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator==(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs == Index{rhs};
    } else {
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator==(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return rhs == lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator!=(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return not(lhs == rhs);
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator!=(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return rhs != lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator<(const Index& lhs,
                                       const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs < Index{rhs};
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return true;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator<(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(lhs)) {
        return Index{lhs} < rhs;
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator<=(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs <= Index{rhs};
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return true;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator<=(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(lhs)) {
        return Index{lhs} <= rhs;
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator>(const Index& lhs,
                                       const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs > Index{rhs};
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator>(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(lhs)) {
        return Index{lhs} > rhs;
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool operator>=(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs >= Index{rhs};
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr bool
operator>=(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(lhs)) {
        return Index{lhs} >= rhs;
    } else {
        // The only non-representable values are from an uint64_t and greater
        // than any possible index (msb set)
        return false;
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index operator+(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if constexpr (std::is_same_v<T, Index::type>) {
        return lhs + Index{rhs};
    } else {
        if (Index::is_representable(rhs)) {
            return lhs + Index{rhs};
        } else {
            // There is a chance that the result is itself representable by an
            // index if the lhs is negative (2^63 is not representable but -2^63
            // is)
            PID_UTILS_INDEX_ASSERT(
                lhs <= Index{0},
                "Performing the addition would overflow the index (UB)");
            const auto urhs = static_cast<Index::utype>(rhs);
            if (lhs.index == Index::min()) {
                // -min is not representable so cannot use std::abs but in this
                // case all possible rhs are valid
                const auto abs_lhs = Index::umax() + 1;
                return Index{urhs - abs_lhs};
            } else {
                const auto abs_lhs =
                    static_cast<Index::utype>(std::abs(lhs.index));
                const auto target_value = urhs - abs_lhs;
                PID_UTILS_INDEX_ASSERT(
                    target_value <= Index::umax(),
                    "Performing the addition would overflow the index (UB)");
                return Index{target_value};
            }
        }
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index
operator+(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return rhs + lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr Index& operator+=(Index& lhs, const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs + rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T& operator+=(T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs + rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index operator-(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs - Index{rhs};
    } else {
        // There is a chance that the result is itself representable by an index
        // if the lhs is positive
        PID_UTILS_INDEX_ASSERT(
            lhs >= Index{0},
            "Performing the subtraction would underflow the index (UB)");
        const auto target_value = static_cast<Index::utype>(rhs) -
                                  static_cast<Index::utype>(lhs.index);
        PID_UTILS_INDEX_ASSERT(
            target_value <= Index::umax(),
            "Performing the subtraction would underflow the index (UB)");
        return Index{-static_cast<Index::type>(target_value)};
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index
operator-(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return -rhs + lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr Index& operator-=(Index& lhs, const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs - rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T& operator-=(T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs - rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index operator*(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs * Index{rhs};
    } else {
        PID_UTILS_INDEX_ASSERT(
            lhs == 0 or (lhs == Index{0} and rhs == Index::umax()),
            "Performing the multiplication would underflow the index (UB)");
        if (lhs == 0) {
            return Index{0};
        } else {
            // lhs == Index{0} and rhs == Index::umax()
            return Index::min();
        }
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index
operator*(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    return rhs * lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr Index& operator*=(Index& lhs, const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs * rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T& operator*=(T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs * rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index operator/(const Index& lhs,
                                        const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(rhs)) {
        return lhs / Index{rhs};
    } else {
        if (lhs == Index::min() and rhs == Index::umax()) {
            return Index{-1};
        } else {
            return Index{0};
        }
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
[[nodiscard]] constexpr Index
operator/(const T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    if (Index::is_representable(lhs)) {
        return Index{lhs} / rhs;
    } else {
        PID_UTILS_INDEX_ASSERT(
            false, "Division of a non-representable value is not implemented");
        pid::unreachable();
    }
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr Index& operator/=(Index& lhs, const T& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs / rhs;
    return lhs;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T& operator/=(T& lhs, const Index& rhs) PID_UTILS_INDEX_NOEXCEPT {
    lhs = lhs / rhs;
    return lhs;
}

} // namespace pid