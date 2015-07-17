#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "monitor.h"

unsigned int monitor_count()
{
    int ret = 0;
    Display *display = XOpenDisplay(NULL);
    assert(display);

    XRRScreenResources *screen = XRRGetScreenResources(
        display, DefaultRootWindow(display));
    assert(screen);

    for (int i = 0; i < screen->noutput; i++)
    {
        XRROutputInfo *output_info = XRRGetOutputInfo(
            display, screen, screen->outputs[i]);
        assert(output_info);
        if (output_info->connection == RR_Connected)
            ++ret;
        XRRFreeOutputInfo(output_info);
    }

    XRRFreeScreenResources(screen);
    XCloseDisplay(display);
    return ret;
}

Monitor *monitor_get_impl(unsigned int n)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

    XRRScreenResources *screen = XRRGetScreenResources(
        display, DefaultRootWindow(display));
    assert(screen);

    unsigned int count = 0;
    XRROutputInfo *output_info = NULL;
    for (int i = 0; i < screen->noutput; i++)
    {
        output_info = XRRGetOutputInfo(display, screen, screen->outputs[i]);
        assert(output_info);
        if (output_info->connection == RR_Connected)
        {
            if (count == n)
                break;
            count++;
        }
        XRRFreeOutputInfo(output_info);
        output_info = NULL;
    }

    assert(output_info);

    XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(display, screen, output_info->crtc);
    assert(crtc_info);

    Monitor *monitor = monitor_create(
        crtc_info->x,
        crtc_info->y,
        crtc_info->width,
        crtc_info->height);
    assert(monitor);

    XRRFreeCrtcInfo(crtc_info);
    XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(screen);
    XCloseDisplay(display);
    return monitor;
}
