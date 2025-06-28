#pragma once

#include <string>
#include <cstdint>

namespace pid {

namespace detail {

constexpr std::uint64_t fnv1a_initial_value = 0xcbf29ce484222325;
constexpr std::uint64_t fnv1a_prime = 0x100000001b3;

template <typename CharT = char>
inline constexpr std::uint64_t
hashed_string(std::basic_string<CharT> str, size_t index = 0,
              const std::uint64_t value = detail::fnv1a_initial_value) noexcept {
    return index == str.size()
               ? value
               : hashed_string<CharT>(str, index + 1,
                                      (value ^ std::uint64_t(str[index])) *
                                          detail::fnv1a_prime);
}

template <typename CharT = char>
inline constexpr std::uint64_t
hashed_string(const CharT* str,
              const std::uint64_t value = detail::fnv1a_initial_value) noexcept {
    return str[0] == '\0' ? value
                          : hashed_string<CharT>(
                                &(str[1]), (value ^ std::uint64_t(str[0])) *
                                               detail::fnv1a_prime);
}

} // namespace detail

} // namespace pid