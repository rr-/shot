#ifndef MONITOR_H
#define MONITOR_H
#include <stddef.h>

typedef struct
{
    int primary;
    int x, y;
    unsigned int width, height;
} Monitor;

Monitor *monitor_create(
    int primary,
    int x, int y,
    unsigned int width, unsigned int height);

void monitor_destroy(Monitor *monitor);

#endif
