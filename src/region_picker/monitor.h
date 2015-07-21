#ifndef REGION_PICKER_MONITOR_H
#define REGION_PICKER_MONITOR_H
#include "monitor_mgr.h"
#include "region.h"

int update_region_from_all_monitors(
    ShotRegion *region, const MonitorManager *mgr);

int update_region_from_monitor(
    ShotRegion *region, const Monitor *monitor);

#endif
