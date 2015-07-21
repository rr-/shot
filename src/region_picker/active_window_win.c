#include <Windows.h>
#include "region_picker/errors.h"
#include "region_picker/active_window.h"

int update_region_from_active_window(ShotRegion *region)
{
    RECT rect;
    HWND hwnd = GetForegroundWindow();
    GetWindowRect(hwnd, &rect);
    region->x = rect.left;
    region->y = rect.top;
    region->width = rect.right - rect.left;
    region->height = rect.bottom - rect.top;
    return 0;
}
