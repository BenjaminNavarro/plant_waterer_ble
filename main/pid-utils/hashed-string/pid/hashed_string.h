/**
 * @defgroup hashed-string hashed-string: hashing string as integers
 *
 *
 */
/**
 * @file hashed_string.h
 * @author Benjamin Navarro
 * @brief header file for hashed string class
 * @date 2022-06-10
 * @ingroup hashed-string
 * @ingroup pid-utils
 *
 */
#pragma once

#include <pid/cxx11/hashed_string.h>
#if PID_UTILS_CXX17_AVAILABLE
#include <pid/cxx17/hashed_string.h>
#endif

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace pid {

namespace literals {

constexpr std::uint64_t operator"" _hs(const char* str,
                                       std::size_t /*unused*/) {
    return pid::hashed_string(str);
}

constexpr std::uint64_t operator"" _hws(const wchar_t* str,
                                        std::size_t /*unused*/) {
    return pid::hashed_string(str);
}

} // namespace literals

} // namespace pid