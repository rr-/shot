#include <stdio.h>
#include <windows.h>
#include "region_picker/errors.h"
#include "region_picker/window.h"

int update_region_from_window(ShotRegion *region, const long int window_id)
{
    HWND hwnd = (HWND)window_id;
    if (!IsWindow(hwnd))
    {
        fprintf(stderr, "Window %ld does not exist?\n", window_id);
        return ERR_OTHER;
    }
    RECT rect;
    GetWindowRect(hwnd, &rect);
    region->x = rect.left;
    region->y = rect.top;
    region->width = rect.right - rect.left;
    region->height = rect.bottom - rect.top;
    return 0;
}
