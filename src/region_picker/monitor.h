#ifndef REGION_PICKER_MONITOR_H
#define REGION_PICKER_MONITOR_H
#include "monitor_mgr.h"
#include "region.h"

void update_region_from_all_monitors(MonitorManager *mgr, ShotRegion *region);

int update_region_from_monitor(
    MonitorManager *mgr, ShotRegion *region, unsigned int n);

#endif
