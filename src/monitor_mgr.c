#include <assert.h>
#include <stdlib.h>
#include "monitor_mgr.h"

void monitor_mgr_destroy(MonitorManager *mgr)
{
    if (!mgr)
        return;
    if (mgr->monitors)
    {
        for (size_t i = 0; i < mgr->monitor_count; i++)
            monitor_destroy(mgr->monitors[i]);
        free(mgr->monitors);
    }
    free(mgr);
}

void monitor_mgr_add(MonitorManager *mgr, Monitor *monitor)
{
    assert(monitor);
    Monitor **new_monitors = realloc(
        mgr->monitors, mgr->monitor_count * sizeof(Monitor**));
    assert(new_monitors);
    mgr->monitors = new_monitors;
    mgr->monitors[mgr->monitor_count] = monitor;
    ++mgr->monitor_count;
}
