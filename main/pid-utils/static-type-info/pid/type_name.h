/**
 * @file type_name.h
 * @author Benjamin Navarro
 * @brief header for type_name function
 * @date 2022-06-10
 * @ingroup static-type-info
 */

#pragma once

#include <string_view>

namespace pid {

//! \brief Provide an std::string_view representing the name of a given type
//!
//! \tparam T The type to get the name of
//! \return constexpr std::string_view The name of the type
template <typename T>
constexpr auto type_name() noexcept {
    std::string_view name;
    std::string_view prefix;
    std::string_view suffix;
#ifdef __clang__
#if __clang_major__ < 5
#error "Clang 5 or later is required for type_name() to work"
#else
    name = __PRETTY_FUNCTION__;
    prefix = "auto pid::type_name() [T = ";
    suffix = "]";
#endif
#elif defined(__GNUC__) and not defined(__INTEL_COMPILER)
#if __GNUC__ < 8
#error "GCC 8 or later is required for type_name() to work"
#else
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto pid::type_name() [with T = ";
    suffix = "]";
#endif
#elif defined(_MSC_VER)
#if _MSC_VER < 1914
#error "MSVC 19.14 or later is required for type_name() to work"
#else
    name = __FUNCSIG__;
    prefix = "auto __cdecl pid::type_name<";
    suffix = ">(void) noexcept";
#endif
#elif defined(__INTEL_COMPILER)
#if __INTEL_COMPILER < 2021
#error "ICC 2021 or later is required for type_name() to work"
#else
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto pid::type_name() noexcept [with T = ";
    suffix = "]";
#endif
#else
#error "You compiler vendor is not supported"
#endif
    using namespace std::literals;
    // MSVC adds type prefixes but other compilers don't so remove them to be 
    // more consistent
    for(auto type_prefix: {"struct "sv, "class "sv, "enum "sv, "union "sv}) {
        if(auto pos = name.find(type_prefix); pos != std::string_view::npos) {
            name.remove_prefix(type_prefix.size());
        }
    }
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

//! \deprecated use type_name()
template <typename T>
[[deprecated("use type_name() instead")]] constexpr auto
typeName() noexcept // NOLINT(readability-identifier-naming)
{
    return type_name<T>();
}

} // namespace pid