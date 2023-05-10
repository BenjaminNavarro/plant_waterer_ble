//! \file overloaded.h
//! \author Benjamin Navarro
//! \brief Provide the 'overloaded' idiom to simplify the use of std::visit.
//! Code taken from https://en.cppreference.com/w/cpp/utility/variant/visit,
//! consult for details
//! \date 09-2022
#pragma once

namespace pid {

template <class... Ts>
// NOLINTNEXTLINE(readability-identifier-naming)
struct overloaded : Ts... {
    using Ts::operator()...;
};

// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace pid