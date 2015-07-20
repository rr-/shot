#include <assert.h>
#include <stdio.h>
#include <Windows.h>
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
        short x, y;
    } last_mouse_pos;

    HWND hwnd;

    unsigned int border_size;
    int x;
    int y;
    int width;
    int height;

    int canceled;
};

static inline int _max(int a, int b)
{
    return a > b ? a : b;
}

static void pull_window_rect(struct private *p)
{
    assert(p);
    RECT rect;
    GetWindowRect(p->hwnd, &rect);
    p->x = rect.left;
    p->y = rect.top;
    p->width = rect.right - rect.left;
    p->height = rect.bottom - rect.top;
}

static void sync_window_rect(struct private *p)
{
    assert(p);
    int min_width = p->border_size * 2 + 5;
    int min_height = p->border_size * 2 + 5;
    p->width = _max(min_width, p->width);
    p->height = _max(min_height, p->height);
    MoveWindow(p->hwnd, p->x, p->y, p->width, p->height, TRUE);
}

static void update_text(struct private *p)
{
    RECT rect;
    GetClientRect(p->hwnd, &rect);
    InvalidateRect(p->hwnd, &rect, FALSE);
}

static void draw_text(struct private *p)
{
    char msg[100];
    sprintf(msg, "%dx%d+%d+%d", p->width, p->height, p->x, p->y);

    RECT rect;
    GetClientRect(p->hwnd, &rect);
    HDC wdc = GetWindowDC(p->hwnd);
    SetTextColor(wdc, 0x00000000);
    SetBkMode(wdc,TRANSPARENT);
    DrawText(wdc, msg, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    DeleteDC(wdc);
}

static void handle_key_down(struct private *p, WPARAM key)
{
    assert(p);
    int x = 0, y = 0;

    switch (key)
    {
        case VK_SHIFT:
            p->keyboard_state.shift = 1;
            break;

        case VK_CONTROL:
            p->keyboard_state.ctrl = 1;
            break;

        case VK_ESCAPE:
        case 'Q':
            p->canceled = 1;
            PostQuitMessage(0);
            break;

        case VK_RETURN:
            p->canceled = -1;
            ShowWindow(p->hwnd, SW_HIDE);
            PostQuitMessage(0);
            break;

        case VK_LEFT:
        case 'H':
            x = -1;
            break;

        case VK_RIGHT:
        case 'L':
            x = 1;
            break;

        case VK_DOWN:
        case 'J':
            y = 1;
            break;

        case VK_UP:
        case 'K':
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

static void handle_key_up(struct private *p, WPARAM key)
{
    assert(p);
    switch (key)
    {
        case VK_SHIFT:
            p->keyboard_state.shift = 0;
            break;

        case VK_CONTROL:
            p->keyboard_state.ctrl = 0;
            break;
    }
}

static void handle_mouse_down(
    struct private *p, int button, short mouse_x, short mouse_y)
{
    assert(p);
    pull_window_rect(p);
    p->last_mouse_pos.x = mouse_x;
    p->last_mouse_pos.y = mouse_y;
    if (button == VK_LBUTTON)
    {
        p->window_state.moving = 1;
        p->window_state.resizing_x = 0;
        p->window_state.resizing_y = 0;
    }
    else if (button == VK_RBUTTON)
    {
        p->window_state.moving = 0;
        p->window_state.resizing_x = 0;
        p->window_state.resizing_y = 0;
        if (p->last_mouse_pos.x < (int)p->width / 3)
            p->window_state.resizing_x = -1;
        else if (p->last_mouse_pos.x > (int)p->width * 2/3)
            p->window_state.resizing_x = 1;
        if (p->last_mouse_pos.y < (int)p->height / 3)
            p->window_state.resizing_y = -1;
        else if (p->last_mouse_pos.y > (int)p->height * 2/3)
            p->window_state.resizing_y = 1;
    }
}

static void handle_mouse_up(struct private *p)
{
    assert(p);
    p->window_state.resizing_x = 0;
    p->window_state.resizing_y = 0;
    p->window_state.moving = 0;
}

static void handle_mouse_move(struct private *p, short mouse_x, short mouse_y)
{
    assert(p);
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

static LRESULT CALLBACK wnd_proc(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    struct private *p = GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {

        case WM_ERASEBKGND:
        {
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect((HDC)wparam, &rect, CreateSolidBrush(RGB(255, 200, 0)));
            break;
        }

        case WM_PAINT:
            draw_text(p);
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_KEYDOWN:
            handle_key_down(p, wparam);
            break;

        case WM_KEYUP:
            handle_key_up(p, wparam);
            break;

        case WM_LBUTTONDOWN:
            handle_mouse_down(p, VK_LBUTTON, LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_RBUTTONDOWN:
            handle_mouse_down(p, VK_RBUTTON, LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            handle_mouse_up(p);
            break;

        case WM_MOUSEMOVE:
            handle_mouse_move(p, LOWORD(lparam), HIWORD(lparam));
            break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

static int register_class(
    const char *class_name,
    LRESULT CALLBACK (*wnd_proc)(HWND, UINT, WPARAM, LPARAM))
{
    assert(class_name);
    assert(wnd_proc);
    WNDCLASSEX wc =
    {
        .cbSize        = sizeof(WNDCLASSEX),
        .style         = 0,
        .lpfnWndProc   = wnd_proc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = 0,
        .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor       = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = 0,
        .lpszMenuName  = NULL,
        .lpszClassName = class_name,
        .hIconSm       = LoadIcon(NULL, IDI_APPLICATION),
    };
    if (!RegisterClassEx(&wc))
    {
        fprintf(stderr, "Failed to register window class\n");
        return -1;
    }
    return 0;
}

static int init_window(
    const char *class_name, const char *title, struct private *p)
{
    assert(class_name);
    assert(title);
    assert(p);

    p->hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        class_name,
        title,
        WS_POPUPWINDOW,
        p->x, p->y, p->width, p->height,
        NULL, NULL, 0, NULL);

    if (!p->hwnd)
    {
        fprintf(stderr, "Failed to create a window\n");
        return -1;
    }

    SetLayeredWindowAttributes(p->hwnd,  0, 0.5f * 255, LWA_ALPHA);
    SetWindowLongPtr(p->hwnd, GWLP_USERDATA, (long)p);
    ShowWindow(p->hwnd, SW_SHOWNORMAL);
    UpdateWindow(p->hwnd);
    return 0;
}

static void run_event_loop(struct private *p)
{
    MSG Msg;
    while (!p->canceled && GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}

int update_region_interactively(ShotRegion *region)
{
    const char *class_name = "shot";
    if (register_class(class_name, &wnd_proc))
        return ERR_OTHER;

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
        .canceled = 0,
        .border_size = 1,
    };
    p.width = region->width + 2 * p.border_size;
    p.height = region->height + 2 * p.border_size;
    p.x = region->x - p.border_size;
    p.y = region->y - p.border_size;

    if (init_window(class_name, "shot", &p))
        return ERR_OTHER;

    run_event_loop(&p);

    if (p.canceled == 1)
        return ERR_CANCELED;

    //wait for window close, vsync and other blows and whistles
    Sleep(100);

    region->x = p.x + p.border_size;
    region->y = p.y + p.border_size;
    region->width = _max(0, p.width - 2 * p.border_size);
    region->height = _max(0, p.height - 2 * p.border_size);
    return 0;
}
