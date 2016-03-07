#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "region_picker/errors.h"
#include "region_picker/window.h"

static int window_found = 1;

static int error_handler(Display *display, XErrorEvent *event)
{
    (void)display;
    if (event->error_code == BadWindow)
        window_found = 0;
    return 0;
}

int update_region_from_window(ShotRegion *region, const long int window_id)
{
    XSetErrorHandler(error_handler);
    window_found = 1;

    int ret;
    Display *display = XOpenDisplay(NULL);
    assert(display);

    Window root = RootWindow(display, DefaultScreen(display));
    Window window = window_id;

    XWindowAttributes xwa;
    XGetWindowAttributes(display, window, &xwa);

    if (!window_found)
    {
        fprintf(stderr, "Window %ld does not exist?\n", window_id);
        ret = ERR_OTHER;
        goto end;
    }

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
    XSetErrorHandler(NULL);
    XCloseDisplay(display);
    return ret;
}
