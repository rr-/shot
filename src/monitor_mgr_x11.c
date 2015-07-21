#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "monitor_mgr.h"

MonitorManager *monitor_mgr_create()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "Cannot open display.\n");
        return NULL;
    }

    MonitorManager *mgr = malloc(sizeof(MonitorManager*));
    mgr->monitor_count = 0;
    mgr->monitors = NULL;

    Window root = DefaultRootWindow(display);
    RROutput primary_output = XRRGetOutputPrimary(display, root);

    XRRScreenResources *screen = XRRGetScreenResources(display, root);
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

            int primary = 0;
            for (int j = 0; j < crtc_info->noutput; j++)
                if (crtc_info->outputs[j] == primary_output)
                    primary = 1;

            Monitor *monitor = monitor_create(
                primary,
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
