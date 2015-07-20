#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "region_picker/errors.h"
#include "region_picker/interactive.h"
#include "region_picker/interactive_common.h"

int IP_MOUSE_LEFT   = Button1;
int IP_MOUSE_RIGHT  = Button3;
int IP_KEY_LSHIFT   = XK_Shift_L;
int IP_KEY_RSHIFT   = XK_Shift_R;
int IP_KEY_LCONTROL = XK_Control_L;
int IP_KEY_RCONTROL = XK_Control_R;
int IP_KEY_ESCAPE   = XK_Escape;
int IP_KEY_Q        = XK_q;
int IP_KEY_H        = XK_h;
int IP_KEY_J        = XK_j;
int IP_KEY_K        = XK_k;
int IP_KEY_L        = XK_l;
int IP_KEY_LEFT     = XK_Left;
int IP_KEY_DOWN     = XK_Down;
int IP_KEY_UP       = XK_Up;
int IP_KEY_RIGHT    = XK_Right;
int IP_KEY_RETURN   = XK_Return;

struct private
{
    Display *display;
    Window window;
    Window root;
    GC gc;
    Colormap colormap;

    ShotInteractivePicker ip;
    unsigned int border_size;
};

void ip_pull_window_rect(ShotInteractivePicker *ip)
{
    assert(ip);

    unsigned int depth;
    struct private *priv = ip->priv;

    XGetGeometry(
        priv->display, priv->window, &priv->root,
        &ip->rect.pos[0],
        &ip->rect.pos[1],
        (unsigned int*)&ip->rect.size[0],
        (unsigned int*)&ip->rect.size[1],
        &priv->border_size, &depth);

    Window child;
    XTranslateCoordinates(
        priv->display, priv->window, priv->root,
        0, 0, &ip->rect.pos[0], &ip->rect.pos[1], &child);
}

void ip_sync_window_rect(ShotInteractivePicker *ip)
{
    assert(ip);

    struct private *priv = ip->priv;
    XMoveWindow(
        priv->display,
        priv->window,
        ip->rect.pos[0] - priv->border_size,
        ip->rect.pos[1] - priv->border_size);
    XResizeWindow(
        priv->display,
        priv->window,
        ip->rect.size[0],
        ip->rect.size[1]);
}

void ip_update_text(ShotInteractivePicker *ip)
{
    assert(ip);

    struct private *priv = ip->priv;
    XClearWindow(priv->display, priv->window);
    XFlush(priv->display);

    char msg[100];
    sprintf(msg,
        "%dx%d+%d+%d",
        ip->rect.size[0], ip->rect.size[1],
        ip->rect.pos[0], ip->rect.pos[1]);

    const int char_width = 6;
    const int char_height = 9;
    int text_x = (ip->rect.size[0] - strlen(msg) * char_width) / 2;
    int text_y = (ip->rect.size[1] - char_height) / 2 + char_height;
    XSetForeground(priv->display, priv->gc, 0xff000000);
    XDrawString(priv->display, priv->window, priv->gc,
        text_x, text_y, msg, strlen(msg));
}

static void run_event_loop(struct private *p)
{
    assert(p);
    XSelectInput(p->display, p->window, ExposureMask
        | KeyPressMask | KeyReleaseMask
        | ButtonPress | ButtonReleaseMask | PointerMotionMask);

    static int border_fixed = 0;

    while (!p->ip.canceled)
    {
        XEvent e;
        XNextEvent(p->display, &e);

        switch (e.type)
        {
            case Expose:
                //fix offset due to own border on first draw
                if (!border_fixed)
                {
                    XWindowAttributes attrs;
                    XGetWindowAttributes(
                        p->display, p->window, &attrs);
                    p->border_size = attrs.border_width;
                    ip_sync_window_rect(&p->ip);
                    border_fixed = 1;
                }
                ip_update_text(&p->ip);
                break;

            case KeyRelease:
                ip_handle_key_down(&p->ip, XLookupKeysym(&e.xkey, 0));
                break;

            case KeyPress:
                ip_handle_key_up(&p->ip, XLookupKeysym(&e.xkey, 0));
                break;

            case ButtonPress:
                ip_handle_mouse_down(
                    &p->ip, e.xbutton.button, e.xbutton.x, e.xbutton.y);
                break;

            case ButtonRelease:
                ip_handle_mouse_up(&p->ip);
                break;

            case MotionNotify:
                ip_handle_mouse_move(&p->ip, e.xbutton.x, e.xbutton.y);
                break;
        }
    }
}

static int init_window(struct private *p)
{
    assert(p);
    int screen = DefaultScreen(p->display);
    XVisualInfo visual;
    if (!XMatchVisualInfo(p->display, screen, 32, TrueColor, &visual))
    {
        fprintf(stderr, "Couldn\'t find matching visual\n");
        return -1;
    }

    p->root = RootWindow(p->display, screen);

    p->colormap = XCreateColormap(
        p->display, p->root, visual.visual, AllocNone);
    XSetWindowAttributes attrs;
    attrs.colormap = p->colormap;
    attrs.override_redirect = True;
    attrs.background_pixel = 0x80ff8000;
    attrs.border_pixel = 0;

    p->window = XCreateWindow(
        p->display,
        p->root,
        p->ip.rect.pos[0],
        p->ip.rect.pos[1],
        p->ip.rect.size[0],
        p->ip.rect.size[1],
        0, visual.depth, InputOutput, visual.visual,
        CWBackPixel | CWColormap | CWBorderPixel, &attrs);

    // make floating
    {
        Atom type = XInternAtom(p->display, "_NET_WM_WINDOW_TYPE", False);
        Atom value = XInternAtom(
            p->display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        XChangeProperty(
            p->display, p->window, type, XA_ATOM, 32,
            PropModeReplace, (unsigned char*)&value, 1);
    }

    // disable borders
    {
        Atom type = XInternAtom(p->display, "_MOTIF_WM_HINTS", False);
        struct MotifHints
        {
            unsigned long   flags;
            unsigned long   functions;
            unsigned long   decorations;
            long            inputMode;
            unsigned long   status;
        };
        struct MotifHints value = {
            .flags = 2,
            .decorations = 0,
        };
        XChangeProperty(p->display, p->window, type, type, 32,
            PropModeReplace, (unsigned char*)&value, 5);
    }

    p->gc = XCreateGC(p->display, p->window, 0L, NULL);

    XSetWindowColormap(p->display, p->window, attrs.colormap);
    XMapWindow(p->display, p->window);
    return 0;
}

static void destroy_window(struct private *p)
{
    assert(p);
    XFreeColormap(p->display, p->colormap);
    XFreeGC(p->display, p->gc);
    XDestroyWindow(p->display, p->window);
    XSync(p->display, True);
    XCloseDisplay(p->display);
}

int update_region_interactively(ShotRegion *region, const ShotRegion *workarea)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

    struct private p = {
        .display = display,
    };
    ip_init(&p.ip, region, workarea);
    p.ip.priv = &p;

    if (init_window(&p))
        return ERR_OTHER;
    run_event_loop(&p);
    destroy_window(&p);

    if (p.ip.canceled == 1)
        return ERR_CANCELED;

    // wait til window is actually hidden, vsync redraws things, and
    // hypothetical compiz or other eyecandy draw their fadeout effects
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 5e8;
    nanosleep(&tim , &tim2);

    region->x = p.ip.rect.pos[0];
    region->y = p.ip.rect.pos[1];
    region->width = p.ip.rect.size[0];
    region->height = p.ip.rect.size[1];
    return 0;
}
