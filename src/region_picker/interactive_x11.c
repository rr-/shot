#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "region_picker/interactive.h"

void make_floating(Display *display, Window win)
{
    Atom type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    XChangeProperty(display, win, type, XA_ATOM, 32, PropModeReplace, \
        (unsigned char*)&value, 1);
}

void disable_borders(Display *display, Window win)
{
    Atom type = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    struct MotifHints
    {
        unsigned long   flags;
        unsigned long   functions;
        unsigned long   decorations;
        long            inputMode;
        unsigned long   status;
    };
    struct MotifHints value =
    {
        .flags = 2,
        .decorations = 0,
    };
    XChangeProperty(display, win, type, type, 32, PropModeReplace,
        (unsigned char*)&value, 5);
}

void get_geometry(
    Display *display, Window win, Window root,
    int *x, int *y,
    unsigned int *width, unsigned int *height,
    int with_border)
{
    unsigned int border_size, depth;
    XGetGeometry(display, win, &root, x, y, width, height,
        &border_size, &depth);
    Window child;
    if (with_border)
    {
        x -= border_size;
        y -= border_size;
        width += 2 * border_size;
        height += 2 * border_size;
    }
    XTranslateCoordinates(display, win, root, 0, 0, x, y, &child);
}

void update_text(Display *display, Window win, Window root, GC gc)
{
    XClearWindow(display, win);

    int x, y;
    unsigned int width, height;
    get_geometry(display, win, root,  &x, &y, &width, &height, 0);

    char msg[100];
    sprintf(msg, "%dx%d+%d+%d", width, height, x, y);

    const int char_width = 6;
    const int char_height = 9;
    int text_x = (width - strlen(msg) * char_width) / 2;
    int text_y = (height - char_height) / 2 + char_height;
    XSetForeground(display, gc, 0xff000000);
    XDrawString(display, win, gc, text_x, text_y, msg, strlen(msg));
}

int update_region_interactively(ShotRegion *region)
{
    int canceled = 0;

    Display *display = XOpenDisplay(NULL);
    assert(display);
    int screen = DefaultScreen(display);

    XVisualInfo visual;
    if (!XMatchVisualInfo(display, screen, 32, TrueColor, &visual))
    {
        fprintf(stderr, "Couldn\'t find matching visual\n");
        return -1;
    }

    Window root = RootWindow(display, screen);

    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(display, root, visual.visual, AllocNone);
    attrs.override_redirect = True;
    attrs.background_pixel = 0x80ff8000;
    attrs.border_pixel = 0;

    unsigned int width = 640;
    unsigned int height = 480;
    int x = (DisplayWidth(display, screen) - width) / 2;
    int y = (DisplayHeight(display, screen) - height) / 2;

    Window win = XCreateWindow(
        display, root, x, y, width, height,
        0, visual.depth, InputOutput, visual.visual,
        CWBackPixel | CWColormap | CWBorderPixel, &attrs);

    make_floating(display, win);
    disable_borders(display, win);

    GC gc = XCreateGC(display, win, 0L, NULL);

    XSetWindowColormap(display, win, attrs.colormap);
    XMapWindow(display, win);
    XSelectInput(display, win, ExposureMask
        | KeyPressMask | KeyReleaseMask
        | ButtonPress | ButtonReleaseMask | PointerMotionMask);

    int tmp_x, tmp_y;
    int moving = 0, resizing_x = 0, resizing_y = 0;
    int shift = 0, ctrl = 0;
    while (1) {
        XEvent e;
        XNextEvent(display, &e);

        if (e.type == Expose)
        {
            update_text(display, win, root, gc);
        }

        else if (e.type == KeyRelease)
        {
            if (e.xkey.keycode == XKeysymToKeycode(display, XK_Shift_L)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Shift_R))
            {
                shift = 0;
            }

            if (e.xkey.keycode == XKeysymToKeycode(display, XK_Control_L)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Control_R))
            {
                ctrl = 0;
            }
        }

        else if (e.type == KeyPress)
        {
            if (e.xkey.keycode == XKeysymToKeycode(display, XK_Shift_L)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Shift_R))
            {
                shift = 1;
            }

            else if (e.xkey.keycode == XKeysymToKeycode(display, XK_Control_L)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Control_R))
            {
                ctrl = 1;
            }

            else if (e.xkey.keycode == XKeysymToKeycode(display, XK_Escape)
            || e.xkey.keycode == XKeysymToKeycode(display, XK_Q))
            {
                canceled = 1;
                break;
            }

            if (e.xkey.keycode == XKeysymToKeycode(display, XK_Return))
            {
                canceled = 0;
                get_geometry(display, win, root, &x, &y, &width, &height, 0);
                region->x = x;
                region->y = y;
                region->width = width;
                region->height = height;
                break;
            }

            get_geometry(display, win, root, &x, &y, &width, &height, 1);

            int left = e.xkey.keycode == XKeysymToKeycode(display, XK_H)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Left);
            int right = e.xkey.keycode == XKeysymToKeycode(display, XK_L)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Right);
            int down = e.xkey.keycode == XKeysymToKeycode(display, XK_J)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Down);
            int up = e.xkey.keycode == XKeysymToKeycode(display, XK_K)
                || e.xkey.keycode == XKeysymToKeycode(display, XK_Up);

            int delta = shift ? 1 : 25;

            if (ctrl)
            {
                width  -= left  ? delta : 0;
                width  += right ? delta : 0;
                height -= up    ? delta : 0;
                height += down  ? delta : 0;
            }
            else
            {
                x -= left  ? delta : 0;
                x += right ? delta : 0;
                y -= up    ? delta : 0;
                y += down  ? delta : 0;
            }

            XMoveWindow(display, win, x, y);
            XResizeWindow(display, win, width, height);
            update_text(display, win, root, gc);
        }

        else if (e.type == ButtonPress)
        {
            get_geometry(display, win, root, &x, &y, &width, &height, 0);
            tmp_x = e.xbutton.x;
            tmp_y = e.xbutton.y;
            if (e.xbutton.button == Button1)
            {
                resizing_x = resizing_y = 0;
                moving = 1;
            }
            else
            {
                resizing_x = resizing_y = moving = 0;
                if (e.xbutton.x < (int)width / 3)
                    resizing_x = -1;
                else if (e.xbutton.x > (int)width * 2/3)
                    resizing_x = 1;
                if (e.xbutton.y < (int)height / 3)
                    resizing_y = -1;
                else if (e.xbutton.y > (int)height * 2/3)
                    resizing_y = 1;
            }
        }

        else if (e.type == ButtonRelease)
        {
            moving = resizing_x = resizing_y = 0;
        }

        else if (e.type == MotionNotify && moving)
        {
            int old_x = x, old_y = y;
            x += e.xbutton.x - tmp_x;
            y += e.xbutton.y - tmp_y;
            XMoveWindow(display, win, x, y);
            tmp_x = e.xbutton.x + old_x - x;
            tmp_y = e.xbutton.y + old_y - y;
            update_text(display, win, root, gc);
        }

        else if (e.type == MotionNotify && (resizing_x || resizing_y))
        {
            int old_x = x, old_y = y;
            if (resizing_x == -1)
            {
                x += e.xbutton.x - tmp_x;
                width -= e.xbutton.x - tmp_x;
            }
            else if (resizing_x == 1)
                width += e.xbutton.x - tmp_x;
            if (resizing_y == -1)
            {
                y += e.xbutton.y - tmp_y;
                height -= e.xbutton.y - tmp_y;
            }
            else if (resizing_y == 1)
                height += e.xbutton.y - tmp_y;

            XResizeWindow(display, win, width, height);
            XMoveWindow(display, win, x, y);
            update_text(display, win, root, gc);
            tmp_x = e.xbutton.x + old_x - x;
            tmp_y = e.xbutton.y + old_y - y;
        }
    }

    XFreeColormap(display, attrs.colormap);
    XFreeGC(display, gc);
    XDestroyWindow(display, win);
    XSync(display, True);
    XCloseDisplay(display);

    return canceled ? 1 : 0;
}
