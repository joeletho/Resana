#pragma once

#if defined(__clang__)
#define DEBUG_BREAK __builtin_debugtrap()
#elif defined(_MSC_VER)
#define DEBUG_BREAK __debugbreak()
#endif

#if defined(RS_ENABLE_ASSERTS)
#define RS_ASSERT(x, ...) { if(!x) { RS_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
#define RS_CORE_ASSERT(x, ...) { if(!x) { RS_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
#else
#define RS_ASSERT(x, ...)
#define RS_CORE_ASSERT(x, ...)
#endif