#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "region_picker/errors.h"
#include "region_picker/interactive.h"

#define MIN_SIZE 30

struct rectangle
{
    int pos[2];
    int size[2];
};

void _min(int *a, int b) { if (*a > b) *a = b; }
void _max(int *a, int b) { if (*a < b) *a = b; }

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
        int resizing[2];
    } window_state;

    int last_mouse_pos[2];

    struct
    {
        Display *display;
        Window window;
        Window root;
        GC gc;
        Colormap colormap;
    } xlib;

    unsigned int border_size;
    struct rectangle rect;
    const struct rectangle workarea;

    int canceled;
};

static void pull_window_rect(struct private *p)
{
    assert(p);
    unsigned int depth;
    XGetGeometry(
        p->xlib.display, p->xlib.window, &p->xlib.root,
        &p->rect.pos[0], &p->rect.pos[1],
        (unsigned int*)&p->rect.size[0], (unsigned int*)&p->rect.size[1],
        &p->border_size, &depth);
    Window child;
    XTranslateCoordinates(
        p->xlib.display, p->xlib.window, p->xlib.root,
        0, 0, &p->rect.pos[0], &p->rect.pos[1], &child);
}

static void sync_window_rect(struct private *p)
{
    assert(p);
    XMoveWindow(
        p->xlib.display, p->xlib.window,
        p->rect.pos[0] - p->border_size,
        p->rect.pos[1] - p->border_size);
    XResizeWindow(
        p->xlib.display, p->xlib.window,
        p->rect.size[0],
        p->rect.size[1]);
}

static void update_text(struct private *p)
{
    XClearWindow(p->xlib.display, p->xlib.window);
    XFlush(p->xlib.display);

    char msg[100];
    sprintf(msg, "%dx%d+%d+%d", p->rect.size[0], p->rect.size[1], p->rect.pos[0], p->rect.pos[1]);

    const int char_width = 6;
    const int char_height = 9;
    int text_x = (p->rect.size[0] - strlen(msg) * char_width) / 2;
    int text_y = (p->rect.size[1] - char_height) / 2 + char_height;
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
    int delta[2] = { 0, 0 };

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
            delta[0] = -1;
            break;

        case XK_Right:
        case XK_l:
            delta[0] = 1;
            break;

        case XK_Down:
        case XK_j:
            delta[1] = 1;
            break;

        case XK_Up:
        case XK_k:
            delta[1] = -1;
            break;
    }

    if (delta[0] || delta[1])
    {
        pull_window_rect(p);

        const struct rectangle *wa = &p->workarea;
        struct rectangle *r = &p->rect;

        if (!p->keyboard_state.shift)
        {
            for (int i = 0; i < 2; i++)
                delta[i] *= 25;
        }

        if (p->keyboard_state.ctrl)
        {
            for (int i = 0; i < 2; i++)
            {
                r->size[i] += delta[i];
                _max(&r->size[i], MIN_SIZE);
                _min(&r->size[i], wa->pos[i] + wa->size[i] - r->pos[i]);
            }
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                r->pos[i] += delta[i];
                _max(&r->pos[i], wa->pos[i]);
                _min(&r->pos[i], wa->pos[i] + wa->size[i] - r->size[i]);
            }
        }

        sync_window_rect(p);
        update_text(p);
    }
}

static void handle_mouse_down(
    struct private *p, unsigned int button, int mouse_x, int mouse_y)
{
    pull_window_rect(p);
    p->last_mouse_pos[0] = mouse_x;
    p->last_mouse_pos[1] = mouse_y;
    if (button == Button1)
    {
        p->window_state.resizing[0] = 0;
        p->window_state.resizing[1] = 0;
        p->window_state.moving = 1;
    }
    else
    {
        p->window_state.moving = 0;
        int mouse_pos[2] = { mouse_x, mouse_y };
        for (int i = 0; i < 2; i++)
        {
            p->window_state.resizing[i] = 0;
            if (mouse_pos[i] < p->rect.size[i] / 3)
                p->window_state.resizing[i] = -1;
            else if (mouse_pos[i] > p->rect.size[i] * 2/3)
                p->window_state.resizing[i] = 1;
        }
    }
}

static void handle_mouse_up(struct private *p)
{
    p->window_state.moving = 0;
    p->window_state.resizing[0] = 0;
    p->window_state.resizing[1] = 0;
}

static void handle_mouse_move(struct private *p, int mouse_x, int mouse_y)
{
    const struct rectangle *wa = &p->workarea;
    struct rectangle *r = &p->rect;

    int mouse_pos[2] = { mouse_x, mouse_y };
    int old_pos[2] = { r->pos[0], r->pos[1] };

    if (p->window_state.moving)
    {
        for (int i = 0; i < 2; i++)
        {
            r->pos[i] += mouse_pos[i] - p->last_mouse_pos[i];
            if (r->pos[i] < wa->pos[i])
                r->pos[i] = wa->pos[i];
            if (r->pos[i] > wa->pos[i] + wa->size[i] - r->size[i])
                r->pos[i] = wa->pos[i] + wa->size[i] - r->size[i];
        }
        sync_window_rect(p);
        update_text(p);
    }

    else if (p->window_state.resizing[0] || p->window_state.resizing[1])
    {
        int mouse_delta[2] = {
            mouse_x - p->last_mouse_pos[0],
            mouse_y - p->last_mouse_pos[1]
        };

        for (int i = 0; i < 2; i++)
        {
            if (p->window_state.resizing[i] == -1)
            {
                int npos = r->pos[i] + mouse_delta[i];
                _max(&npos, wa->pos[i]);
                _min(&npos, wa->pos[i] + wa->size[i] - r->size[i]);
                int nsize = p->rect.size[i] + old_pos[i] - npos;
                _max(&nsize, MIN_SIZE);
                _min(&nsize, wa->pos[i] + wa->size[i] - r->pos[i]);
                npos = p->rect.size[i] + old_pos[i] - nsize;
                p->rect.size[i] = nsize;
                r->pos[i] = npos;
            }
            else if (p->window_state.resizing[i] == 1)
            {
                p->rect.size[i] += mouse_delta[i];
                _max(&r->size[i], MIN_SIZE);
                _min(&r->size[i], wa->pos[i] + wa->size[i] - r->pos[i]);
            }
        }

        sync_window_rect(p);
        update_text(p);
    }

    for (int i = 0; i < 2; i++)
        p->last_mouse_pos[i] = mouse_pos[i] + old_pos[i] - r->pos[i];
}

static void run_event_loop(struct private *p)
{
    XSelectInput(p->xlib.display, p->xlib.window, ExposureMask
        | KeyPressMask | KeyReleaseMask
        | ButtonPress | ButtonReleaseMask | PointerMotionMask);

    static int border_fixed = 0;

    while (!p->canceled)
    {
        XEvent e;
        XNextEvent(p->xlib.display, &e);

        switch (e.type)
        {
            case Expose:
                //fix offset due to own border on first draw
                if (!border_fixed)
                {
                    XWindowAttributes attrs;
                    XGetWindowAttributes(
                        p->xlib.display, p->xlib.window, &attrs);
                    p->border_size = attrs.border_width;
                    sync_window_rect(p);
                    border_fixed = 1;
                }
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

static int init_window(struct private *p)
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
        p->xlib.display, p->xlib.root,
        p->rect.pos[0], p->rect.pos[1], p->rect.size[0], p->rect.size[1],
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
        struct MotifHints value = {
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

int update_region_interactively(ShotRegion *region, const ShotRegion *workarea)
{
    Display *display = XOpenDisplay(NULL);
    assert(display);

    struct private p = {
        .keyboard_state = {
            .ctrl = 0,
            .shift = 0,
        },
        .window_state = {
            .resizing = { 0, 0 },
            .moving = 0,
        },
        .xlib = {
            .display = display,
        },
        .canceled = 0,
        .workarea = {
            .pos = { workarea->x, workarea->y },
            .size = { workarea->width, workarea->height }
        },
        .rect = {
            .pos = { region->x, region->y },
            .size = { region->width, region->height },
        },
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

    region->x = p.rect.pos[0];
    region->y = p.rect.pos[1];
    region->width = p.rect.size[0];
    region->height = p.rect.size[1];
    return 0;
}
