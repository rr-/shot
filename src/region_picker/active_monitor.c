#include <assert.h>
#include "region_picker/errors.h"
#include "region_picker/active_monitor.h"
#include "region_picker/active_window.h"
#include "region_picker/monitor.h"

static int contains(const Monitor *monitor, const int x, const int y)
{
    return x >= monitor->x
        && y >= monitor->y
        && x < monitor->x + (int)monitor->width
        && y < monitor->y + (int)monitor->height;
}

int update_region_from_active_monitor(
    ShotRegion *region, const MonitorManager *mgr)
{
    ShotRegion window_region;
    assert(!update_region_from_active_window(&window_region));
    const int gravity_x = window_region.x + window_region.width / 2;
    const int gravity_y = window_region.y + window_region.height / 2;
    for (size_t i = 0; i < mgr->monitor_count; i++)
    {
        if (contains(mgr->monitors[i], gravity_x, gravity_y))
            return update_region_from_monitor(region, mgr->monitors[i]);
    }
    return ERR_OTHER;
}
