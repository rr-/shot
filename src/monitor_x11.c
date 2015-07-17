#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "monitor.h"

unsigned int monitor_count()
{
    Display *display = XOpenDisplay(NULL);
    assert(display);
    unsigned int ret = XScreenCount(display);
    XCloseDisplay(display);
    return ret;
}

Monitor *monitor_get_impl(unsigned int n)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

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
