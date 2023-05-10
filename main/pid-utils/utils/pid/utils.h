/**
 * @defgroup utils utils: various c++11/17 utilities
 *
 */

/**
 * @file utils.h
 * @author Benjamin Navarro
 * @author Robin Passama
 * @brief global header for all pid-utils components
 * @date 2022-06-10
 * @ingroup utils
 */
#pragma once

#if PID_UTILS_CXX17_AVAILABLE
#include <pid/static_type_info.h>
#include <pid/scope_exit.h>
#include <pid/unreachable.h>
#endif

#include <pid/hashed_string.h>
#include <pid/assert.h>
#include <pid/containers.h>
#include <pid/bitmask.h>