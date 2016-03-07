#ifndef REGION_PICKER_ACTIVE_MONITOR_H
#define REGION_PICKER_ACTIVE_MONITOR_H
#include "monitor_mgr.h"
#include "region.h"

int update_region_from_active_monitor(
    ShotRegion *region, const MonitorManager *mgr);

#endif
