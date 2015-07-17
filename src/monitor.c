#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "monitor.h"

Monitor *monitor_get_impl(unsigned int n);

Monitor *monitor_create(int x, int y, unsigned int width, unsigned int height)
{
    Monitor *monitor = malloc(sizeof(Monitor));
    assert(monitor);
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

Monitor *monitor_get(unsigned int n)
{
    if (n >= monitor_count())
    {
        fprintf(
            stderr,
            "Invalid monitor number. Valid monitor numbers = 0..%d\n",
            monitor_count() - 1);
        return NULL;
    }

    return monitor_get_impl(n);
}
