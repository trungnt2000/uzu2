#ifndef LOG_H
#define LOG_H
#include "config.h"
#include <stdio.h>
#define INFO(...) (printf("[INFO] %20s -- %40s:\n\t", __FILE__, __func__), printf(__VA_ARGS__))

#if ENABLE_ERROR_LOG
#define UZU_ERROR(...) (printf("[ERROR] %20s -- %20s:\n\t", __FILE__, __func__), printf(__VA_ARGS__))
#else
#define UZU_ERROR(...)                                                                                         \
    do                                                                                                         \
    {                                                                                                          \
    } while (0)
#endif

#define LEAVE_ERROR(code, ...)                                                                                 \
    do                                                                                                         \
    {                                                                                                          \
        UZU_ERROR(__VA_ARGS__);                                                                                \
        return code;                                                                                           \
    } while (0)
#endif // LOG_H
