//! \file index_fmt.h
//! \author Benjamin Navarro
//! \brief Provides the {fmt} specialization for Index formatting
//! \date 12-2022

#pragma once

#include <pid/index.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<pid::Index> : fmt::formatter<pid::Index::type> {};
