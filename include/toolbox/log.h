#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#define INFO(...) (printf("[INFO] %20s -- %40s:\n\t", __FILE__, __func__), printf(__VA_ARGS__))
#define UZU_ERROR(...) (printf("[ERROR] %20s -- %40s:\n\t", __FILE__, __func__), printf(__VA_ARGS__))
#endif // LOG_H
