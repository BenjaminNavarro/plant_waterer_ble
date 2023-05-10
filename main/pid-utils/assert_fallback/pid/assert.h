#pragma once

// In case the full fledged assert library cannot be compiled on the target
// platform we simply fall back to standard assert  behavior.
// Might need some additonal wrappers if more specific assert macros used in 
// user code 

#ifdef NDEBUG
#define ASSERT(expr, ...) ((void)0)
#elif defined(_MSC_VER)
#define ASSERT(expr, ...) do { if(!(expr)) __assume(false); } while(0)
#else // assume GCC/Clang or compatible
#define ASSERT(expr, ...) do { if(!(expr)) __builtin_unreachable(); } while(0)
#endif

#define assert(...) ASSERT(__VA_ARGS__)