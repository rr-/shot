#include <assert.h>
#include <stdlib.h>
#include "monitor.h"

Monitor *monitor_create(
    int primary,
    int x, int y,
    unsigned int width, unsigned int height)
{
    Monitor *monitor = malloc(sizeof(Monitor));
    assert(monitor);
    monitor->primary = primary;
    monitor->x = x;
    monitor->y = y;
    monitor->width = width;
    monitor->height = height;
    return monitor;
}

void monitor_destroy(Monitor *monitor)
{
    if (monitor)
        free(monitor);
}
