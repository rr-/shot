#ifndef MONITOR_H
#define MONITOR_H
#include <stddef.h>

typedef struct
{
    size_t x, y;
    size_t width, height;
} Monitor;

Monitor *monitor_create(size_t x, size_t y, size_t width, size_t height);

void monitor_destroy(Monitor *monitor);

Monitor *monitor_get(size_t n);
size_t monitor_count();

#endif
