#ifndef MONITOR_H
#define MONITOR_H
#include <stddef.h>

typedef struct
{
    int x, y;
    unsigned int width, height;
} Monitor;

Monitor *monitor_create(int x, int y, unsigned int width, unsigned int height);

void monitor_destroy(Monitor *monitor);

Monitor *monitor_get(unsigned int n);
unsigned int monitor_count();

#endif
