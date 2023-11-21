#pragma once

#if defined(_WIN32)
#define DEBUG_BREAK __debugbreak()
#elif defined(_LINUX)

#include <csignal>
#define DEBUG_BREAK raise(SIGTRAP)

#else
#error "Build platform not defined"
#endif
