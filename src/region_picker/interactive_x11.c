#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "region_picker/errors.h"
#include "region_picker/interactive.h"

struct private
{
    struct
    {
        int ctrl;
        int shift;
    } keyboard_state;

    struct
    {
        int moving;
        int resizing_x;
        int resizing_y;
    } window_state;

    struct
    {
        int x, y;
    } last_mouse_pos;

    struct
    {
        Display *display;
        Window window;
        Window root;
        GC gc;
        Colormap colormap;
    } xlib;

    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned int border_size;

    int canceled;
};

static void pull_window_rect(struct private *p)
{
    assert(p);
    unsigned int depth;
    XGetGeometry(p->xlib.display, p->xlib.window, &p->xlib.root,
        &p->x, &p->y, &p->width, &p->height, &p->border_size, &depth);
    Window child;
    XTranslateCoordinates(p->xlib.display, p->xlib.window, p->xlib.root,
        0, 0, &p->x, &p->y, &child);
    p->x -= p->border_size;
    p->y -= p->border_size;
}

static void sync_window_rect(const struct private *p)
{
    assert(p);
    XMoveWindow(p->xlib.display, p->xlib.window, p->x, p->y);
    XResizeWindow(p->xlib.display, p->xlib.window,
        p->width, p->height);
}

static void update_text(struct private *p)
{
    XClearWindow(p->xlib.display, p->xlib.window);

    char msg[100];
    sprintf(msg, "%dx%d+%d+%d", p->width, p->height, p->x, p->y);

    const int char_width = 6;
    const int char_height = 9;
    int text_x = (p->width - strlen(msg) * char_width) / 2;
    int text_y = (p->height - char_height) / 2 + char_height;
    XSetForeground(p->xlib.display, p->xlib.gc, 0xff000000);
    XDrawString(p->xlib.display, p->xlib.window, p->xlib.gc,
        text_x, text_y, msg, strlen(msg));
}

static void handle_key_down(struct private *p, KeySym keysym)
{
    switch (keysym)
    {
        case XK_Shift_L:
        case XK_Shift_R:
            p->keyboard_state.shift = 0;
            break;

        case XK_Control_L:
        case XK_Control_R:
            p->keyboard_state.ctrl = 0;
            break;
    }
}

static void handle_key_up(struct private *p, KeySym keysym)
{
    int x = 0, y = 0;

    switch (keysym)
    {
        case XK_Shift_L:
        case XK_Shift_R:
            p->keyboard_state.shift = 1;
            break;

        case XK_Control_L:
        case XK_Control_R:
            p->keyboard_state.ctrl = 1;
            break;

        case XK_Escape:
        case XK_q:
            p->canceled = 1;
            break;

        case XK_Return:
            pull_window_rect(p);
            p->canceled = -1;
            break;

        case XK_Left:
        case XK_h:
            x = -1;
            break;

        case XK_Right:
        case XK_l:
            x = 1;
            break;

        case XK_Down:
        case XK_j:
            y = 1;
            break;

        case XK_Up:
        case XK_k:
            y = -1;
            break;
    }

    if (x || y)
    {
        pull_window_rect(p);

        int delta = p->keyboard_state.shift ? 1 : 25;
        if (p->keyboard_state.ctrl)
        {
            p->width  += x * delta;
            p->height += y * delta;
        }
        else
        {
            p->x += x * delta;
            p->y += y * delta;
        }

        sync_window_rect(p);
        update_text(p);
    }
}

static void handle_mouse_down(
    struct private *p, unsigned int button, int mouse_x, int mouse_y)
{
    pull_window_rect(p);
    p->last_mouse_pos.x = mouse_x;
    p->last_mouse_pos.y = mouse_y;
    if (button == Button1)
    {
        p->window_state.resizing_x = 0;
        p->window_state.resizing_y = 0;
        p->window_state.moving = 1;
    }
    else
    {
        p->window_state.resizing_x = 0;
        p->window_state.resizing_y = 0;
        p->window_state.moving = 0;
        if (mouse_x < (int)p->width / 3)
            p->window_state.resizing_x = -1;
        else if (mouse_x > (int)p->width * 2/3)
            p->window_state.resizing_x = 1;
        if (mouse_y < (int)p->height / 3)
            p->window_state.resizing_y = -1;
        else if (mouse_y > (int)p->height * 2/3)
            p->window_state.resizing_y = 1;
    }
}

static void handle_mouse_up(struct private *p)
{
    p->window_state.moving = 0;
    p->window_state.resizing_x = 0;
    p->window_state.resizing_y = 0;
}

static void handle_mouse_move(struct private *p, int mouse_x, int mouse_y)
{
    if (p->window_state.moving)
    {
        int old_x = p->x, old_y = p->y;
        p->x += mouse_x - p->last_mouse_pos.x;
        p->y += mouse_y - p->last_mouse_pos.y;
        sync_window_rect(p);
        p->last_mouse_pos.x = mouse_x + old_x - p->x;
        p->last_mouse_pos.y = mouse_y + old_y - p->y;
        update_text(p);
    }
    else if (p->window_state.resizing_x || p->window_state.resizing_y)
    {
        int old_x = p->x, old_y = p->y;
        if (p->window_state.resizing_x == -1)
        {
            p->x += mouse_x - p->last_mouse_pos.x;
            p->width -= mouse_x - p->last_mouse_pos.x;
        }
        else if (p->window_state.resizing_x == 1)
            p->width += mouse_x - p->last_mouse_pos.x;
        if (p->window_state.resizing_y == -1)
        {
            p->y += mouse_y - p->last_mouse_pos.y;
            p->height -= mouse_y - p->last_mouse_pos.y;
        }
        else if (p->window_state.resizing_y == 1)
            p->height += mouse_y - p->last_mouse_pos.y;

        sync_window_rect(p);
        update_text(p);
        p->last_mouse_pos.x = mouse_x + old_x - p->x;
        p->last_mouse_pos.y = mouse_y + old_y - p->y;
    }
}

static void run_event_loop(struct private *p)
{
    XSelectInput(p->xlib.display, p->xlib.window, ExposureMask
        | KeyPressMask | KeyReleaseMask
        | ButtonPress | ButtonReleaseMask | PointerMotionMask);

    while (!p->canceled)
    {
        XEvent e;
        XNextEvent(p->xlib.display, &e);

        switch (e.type)
        {
            case Expose:
                update_text(p);
                break;

            case KeyRelease:
                handle_key_down(p, XLookupKeysym(&e.xkey, 0));
                break;

            case KeyPress:
                handle_key_up(p, XLookupKeysym(&e.xkey, 0));
                break;

            case ButtonPress:
                handle_mouse_down(
                    p, e.xbutton.button, e.xbutton.x, e.xbutton.y);
                break;

            case ButtonRelease:
                handle_mouse_up(p);
                break;

            case MotionNotify:
                handle_mouse_move(p, e.xbutton.x, e.xbutton.y);
                break;
        }
    }
}

int init_window(struct private *p)
{
    int screen = DefaultScreen(p->xlib.display);
    XVisualInfo visual;
    if (!XMatchVisualInfo(p->xlib.display, screen, 32, TrueColor, &visual))
    {
        fprintf(stderr, "Couldn\'t find matching visual\n");
        return -1;
    }

    p->xlib.root = RootWindow(p->xlib.display, screen);

    p->xlib.colormap = XCreateColormap(
        p->xlib.display, p->xlib.root, visual.visual, AllocNone);
    XSetWindowAttributes attrs;
    attrs.colormap = p->xlib.colormap;
    attrs.override_redirect = True;
    attrs.background_pixel = 0x80ff8000;
    attrs.border_pixel = 0;

    p->xlib.window = XCreateWindow(
        p->xlib.display, p->xlib.root, p->x, p->y, p->width, p->height,
        0, visual.depth, InputOutput, visual.visual,
        CWBackPixel | CWColormap | CWBorderPixel, &attrs);

    // make floating
    {
        Atom type = XInternAtom(p->xlib.display, "_NET_WM_WINDOW_TYPE", False);
        Atom value = XInternAtom(
            p->xlib.display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        XChangeProperty(
            p->xlib.display, p->xlib.window, type, XA_ATOM, 32,
            PropModeReplace, (unsigned char*)&value, 1);
    }

    // disable borders
    {
        Atom type = XInternAtom(p->xlib.display, "_MOTIF_WM_HINTS", False);
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
        XChangeProperty(p->xlib.display, p->xlib.window, type, type, 32,
            PropModeReplace, (unsigned char*)&value, 5);
    }

    p->xlib.gc = XCreateGC(p->xlib.display, p->xlib.window, 0L, NULL);

    XSetWindowColormap(p->xlib.display, p->xlib.window, attrs.colormap);
    XMapWindow(p->xlib.display, p->xlib.window);
    return 0;
}

static void destroy_window(struct private *p)
{
    XFreeColormap(p->xlib.display, p->xlib.colormap);
    XFreeGC(p->xlib.display, p->xlib.gc);
    XDestroyWindow(p->xlib.display, p->xlib.window);
    XSync(p->xlib.display, True);
    XCloseDisplay(p->xlib.display);
}

int update_region_interactively(ShotRegion *region)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

    struct private p =
    {
        .keyboard_state =
        {
            .ctrl = 0,
            .shift = 0,
        },
        .window_state =
        {
            .resizing_x = 0,
            .resizing_y = 0,
            .moving = 0,
        },
        .xlib =
        {
            .display = display,
        },
        .canceled = 0,
        .width = region->width,
        .height = region->height,
        .x = region->x,
        .y = region->y,
    };

    if (init_window(&p))
        return ERR_OTHER;
    run_event_loop(&p);
    destroy_window(&p);

    if (p.canceled == 1)
        return ERR_CANCELED;

    // wait til window is actually hidden, vsync redraws things, and
    // hypothetical compiz or other eyecandy draw their fadeout effects
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 5e8;
    nanosleep(&tim , &tim2);

    region->x = p.x + p.border_size;
    region->y = p.y + p.border_size;
    region->width = p.width;
    region->height = p.height;
    return 0;
}
