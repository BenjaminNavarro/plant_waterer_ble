/**
 * @defgroup unreachable unreachable: a utility to flag a part of code as unreachable
 * 
 */

/**
 * @file unreachable.h
 * @author Benjamin Navarro
 * @brief header file for unreachable utility
 * @date 2022-06-10
 * @ingroup unreachable
 * @ingroup pid-utils
 * 
 */
#pragma once

namespace pid {

// from https://stackoverflow.com/a/65258501
#if defined(__GNUC__) // GCC 4.8+, Clang, Intel and other compilers compatible
                      // with GCC (-std=c++0x or above)
[[noreturn]] inline __attribute__((always_inline)) void unreachable() {
    __builtin_unreachable();
}
#elif defined(_MSC_VER) // MSVC
[[noreturn]] __forceinline void unreachable() {
    __assume(false);
}
#else                   // ???
inline void unreachable() {
}
#endif

} // namespace pid