#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#if DEBUG_PRINT
extern void fraise_printf(const char* fmt, ...);
#define DEBUG(...) fraise_printf("%s: ", __func__), fraise_printf(__VA_ARGS__), fraise_printf("\n")
#else
#define DEBUG(...)
#endif

#endif
