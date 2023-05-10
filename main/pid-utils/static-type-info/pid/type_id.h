/**
 * @file type_id.h
 * @author Benjamin Navarro
 * @brief header for type_id function
 * @version 0.1
 * @date 2022-06-10
 * @ingroup static-type-info
 */

#pragma once

#include <pid/type_name.h>
#include <pid/hashed_string.h>

#include <cstdint>

namespace pid {

struct TypeId {

    // TODO make explicit for next major
    [[nodiscard]] constexpr operator const std::uint64_t&() const noexcept {
        return id_;
    }

    [[nodiscard]] constexpr bool operator==(TypeId other) const noexcept {
        return id_ == other.id_;
    }

    [[nodiscard]] constexpr bool operator!=(TypeId other) const noexcept {
        return id_ != other.id_;
    }

    [[nodiscard]] constexpr bool operator<(TypeId other) const noexcept {
        return id_ < other.id_;
    }

    [[nodiscard]] constexpr bool operator<=(TypeId other) const noexcept {
        return id_ <= other.id_;
    }

    [[nodiscard]] constexpr bool operator>(TypeId other) const noexcept {
        return id_ > other.id_;
    }

    [[nodiscard]] constexpr bool operator>=(TypeId other) const noexcept {
        return id_ >= other.id_;
    }

private:
    template <typename T>
    friend constexpr TypeId type_id();

    constexpr explicit TypeId(std::uint64_t id) noexcept : id_{id} {
    }

    std::uint64_t id_{};
};

//! \brief Produce a numeric identifier for a given type based on its name
//!
//! See pid::hashed_string() for how the identifier is generated
//!
//! \tparam T The type to create an identifier for
//! \return constexpr uint64_t The identifier
template <typename T>
constexpr TypeId type_id() {
    return TypeId{hashed_string(type_name<T>())};
}

//! \deprecated use type_id()
template <typename T>
[[deprecated("use type_id() instead")]] constexpr TypeId
typeId() // NOLINT(readability-identifier-naming)
{
    return type_id<T>();
}

} // namespace pid