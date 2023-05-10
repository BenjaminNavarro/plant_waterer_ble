#pragma once

#include <pid/cxx11/detail/hashed_string.h>
#include <string_view>

namespace pid {

namespace detail {

template <typename CharT = char>
inline constexpr std::uint64_t hashed_string(
    std::basic_string_view<CharT> str, std::size_t index = 0,
    const std::uint64_t value = detail::fnv1a_initial_value) noexcept {
    return index == str.size()
               ? value
               : hashed_string<CharT>(str, index + 1,
                                      (value ^ std::uint64_t(str[index])) *
                                          detail::fnv1a_prime);
}

} // namespace detail

} // namespace pid