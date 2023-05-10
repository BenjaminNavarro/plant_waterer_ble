/**
 * @file hashed_string.h
 * @author Benjamin Navarro
 * @brief include file for c++17 hashed string implementation
 * @date 2022-06-10
 * @ingroup hashed-string
 *
 */
#pragma once

#include <pid/cxx17/detail/hashed_string.h>
#include <string_view>

namespace pid {

//! \brief Hash function for std::basic_string_view<T> (a.k.a std::string_view /
//! std::wstring_view)
//!
//! \tparam CharT Character type, automatically deduced
//! \param str The string to hash
//! \return constexpr uint64_t The resulting hash value
template <typename CharT = char>
inline constexpr std::uint64_t
hashed_string(std::basic_string_view<CharT> str) noexcept {
    return detail::hashed_string(str);
}

//! \deprecated use hashed_string()
template <typename CharT = char>
[[deprecated("use hashed_string() instead")]] inline constexpr std::uint64_t
hashedString( // NOLINT(readability-identifier-naming)
    std::basic_string_view<CharT> str) noexcept {
    return hashed_string(str);
}

} // namespace pid
