#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "region_picker/errors.h"
#include "region_picker/monitor.h"

int update_region_from_all_monitors(
    ShotRegion *region, const MonitorManager *mgr)
{
    assert(region);
    assert(mgr);
    int min_x = mgr->monitors[0]->x;
    int min_y = mgr->monitors[0]->y;
    for (unsigned int i = 1; i < mgr->monitor_count; i++)
    {
        if (!i || mgr->monitors[i]->x < min_x) min_x = mgr->monitors[i]->x;
        if (!i || mgr->monitors[i]->y < min_y) min_y = mgr->monitors[i]->y;
    }

    for (unsigned int i = 0; i < mgr->monitor_count; i++)
    {
        ShotRegion pr = {
            .x = mgr->monitors[i]->x,
            .y = mgr->monitors[i]->y,
            .width  = mgr->monitors[i]->width  + mgr->monitors[i]->x - min_x,
            .height = mgr->monitors[i]->height + mgr->monitors[i]->y - min_y,
        };
        if (!i || pr.x      < region->x)      region->x      = pr.x;
        if (!i || pr.y      < region->y)      region->y      = pr.y;
        if (!i || pr.width  > region->width)  region->width  = pr.width;
        if (!i || pr.height > region->height) region->height = pr.height;
    }
    return 0;
}

int update_region_from_monitor(ShotRegion *region, const Monitor *monitor)
{
    assert(region);
    assert(monitor);
    region->x = monitor->x;
    region->y = monitor->y;
    region->width = monitor->width;
    region->height = monitor->height;
    return 0;
}
