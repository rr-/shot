#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "region_picker/errors.h"
#include "region_picker/active_window.h"

int update_region_from_active_window(ShotRegion *region)
{
    int ret;
    Display *display = XOpenDisplay(NULL);
    assert(display);

    Window window;
    int revert_to;
    XGetInputFocus(display, &window, &revert_to);
    if (window == None)
    {
        fprintf(stderr, "No focused window?\n");
        ret = ERR_OTHER;
        goto end;
    }

    Window root = RootWindow(display, DefaultScreen(display));

    unsigned int border_size, depth;
    XGetGeometry(display, window, &root,
        &region->x, &region->y, &region->width, &region->height,
        &border_size, &depth);

    Window child;
    XTranslateCoordinates(display, window, root,
        0, 0, &region->x, &region->y, &child);

    region->x -= border_size;
    region->y -= border_size;
    region->width += 2 * border_size;
    region->height += 2 * border_size;

    ret = 0;

end:
    XCloseDisplay(display);
    return ret;
}
