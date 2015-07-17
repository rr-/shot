#include <assert.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "monitor_mgr.h"

MonitorManager *monitor_mgr_create()
{
    MonitorManager *mgr = malloc(sizeof(MonitorManager*));
    mgr->monitor_count = 0;
    mgr->monitors = NULL;

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
        {
            XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(
                display, screen, output_info->crtc);
            assert(crtc_info);

            Monitor *monitor = monitor_create(
                crtc_info->x,
                crtc_info->y,
                crtc_info->width,
                crtc_info->height);
            assert(monitor);

            monitor_mgr_add(mgr, monitor);

            XRRFreeCrtcInfo(crtc_info);
        }

        XRRFreeOutputInfo(output_info);
    }

    XRRFreeScreenResources(screen);
    XCloseDisplay(display);
    return mgr;
}
