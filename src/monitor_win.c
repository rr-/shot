#include <assert.h>
#include <Windows.h>
#include "monitor.h"

#define UNUSED(x) (void)(x)

struct FetchData
{
    Monitor *monitor;
    int monitor_to_fetch;
    int current;
};

BOOL CALLBACK count_cb(HMONITOR hmonitor, HDC hdc, LPRECT rect, LPARAM data)
{
    UNUSED(hmonitor);
    UNUSED(hdc);
    UNUSED(rect);

    unsigned int *count = (unsigned int*)data;
    assert(count);
    (*count)++;
    return TRUE;
}

BOOL CALLBACK fetch_cb(HMONITOR hmonitor, HDC hdc, LPRECT rect, LPARAM data)
{
    UNUSED(hmonitor);
    UNUSED(hdc);

    struct FetchData *fetch_data = (struct FetchData*)data;

    if (fetch_data->current == fetch_data->monitor_to_fetch)
    {
        fetch_data->monitor = monitor_create(
            rect->left,
            rect->top,
            rect->right - rect->left,
            rect->bottom - rect->top);
        assert(fetch_data->monitor);
        return FALSE;
    }
    fetch_data->current ++;

    return TRUE;
}

unsigned int monitor_count()
{
    unsigned int count = 0;
    EnumDisplayMonitors(0, NULL, count_cb, (LPARAM)&count);
    return count;
}

Monitor *monitor_get_impl(unsigned int n)
{
    struct FetchData fetch_data =
    {
        .monitor = NULL,
        .monitor_to_fetch = n,
        .current = 0,
    };

    EnumDisplayMonitors(0, NULL, fetch_cb, (LPARAM)&fetch_data);
    assert(fetch_data.monitor);
    return fetch_data.monitor;
}
