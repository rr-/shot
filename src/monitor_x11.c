#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "monitor.h"

size_t monitor_count()
{
    Display *display = XOpenDisplay(NULL);
    assert(display);
    size_t ret = XScreenCount(display);
    XCloseDisplay(display);
    return ret;
}

Monitor *monitor_get(size_t n)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

    if ((int)n >= XScreenCount(display))
        return NULL;

    XRRScreenResources *screen =
        XRRGetScreenResources(display, DefaultRootWindow(display));
    XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(display, screen, screen->crtcs[0]);

    Monitor *ret = monitor_create(
        crtc_info->x,
        crtc_info->y,
        crtc_info->width,
        crtc_info->height);
    assert(ret);

    XCloseDisplay(display);
    return ret;
}
