#ifndef MONITOR_MGR_H
#define MONITOR_MGR_H
#include "monitor.h"

typedef struct
{
    unsigned int monitor_count;
    Monitor **monitors;
} MonitorManager;

MonitorManager *monitor_mgr_create();
void monitor_mgr_destroy(MonitorManager *mgr);

void monitor_mgr_add(MonitorManager *mgr, Monitor *monitor);

#endif
