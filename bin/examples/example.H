#pragma once

#include <cstdio> // printf

#if defined(USE_RUN_CODE)

#define RUN_CODE(CODE) \
{\
    printf("## " \
    "%s:%d, [ %s ]\n" \
    "============================================================\n", \
    __FILE__, __LINE__, __func__ /* __PRETTY_FUNCTION__ */); \
    CODE; \
    printf("\n"); \
 } \

 #else

 #define RUN_CODE(CODE) \

 #endif // defined(USE_RUN_CODE)