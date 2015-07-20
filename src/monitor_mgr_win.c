#include <assert.h>
#include <Windows.h>
#include "monitor_mgr.h"

#define UNUSED(x) (void)(x)

static BOOL CALLBACK callback(
    HMONITOR hmonitor, HDC hdc, LPRECT rect, LPARAM data)
{
    UNUSED(hmonitor);
    UNUSED(hdc);

    MonitorManager *mgr = (MonitorManager*)data;
    assert(mgr);

    Monitor *monitor = monitor_create(
        rect->left,
        rect->top,
        rect->right - rect->left,
        rect->bottom - rect->top);
    assert(monitor);

    monitor_mgr_add(mgr, monitor);
    return TRUE;
}

MonitorManager *monitor_mgr_create()
{
    MonitorManager *mgr = malloc(sizeof(MonitorManager*));
    mgr->monitor_count = 0;
    mgr->monitors = NULL;

    EnumDisplayMonitors(0, NULL, callback, (LPARAM)mgr);

    return mgr;
}
