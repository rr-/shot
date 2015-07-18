#include <assert.h>
#include <stdio.h>
#include <Windows.h>
#include "interactive.h"

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

    HWND hwnd;

    int x;
    int y;
    unsigned int width;
    unsigned int height;

    int canceled;
};

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
    MoveWindow(p->hwnd, p->x, p->y, p->width, p->height, TRUE);
}

static void update_text(struct private *p)
{
    assert(p);
    (void)p;
    //TODO...
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

        case VK_ESCAPE:
        case 'Q':
            p->canceled = 1;
            PostQuitMessage(0);
            break;

        case VK_RETURN:
            p->canceled = 0;
            ShowWindow(p->hwnd, SW_HIDE);
            PostQuitMessage(0);
            break;
    }
}

static void handle_button_down(
    struct private *p, int button, int mouse_x, int mouse_y)
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

static void handle_button_up(struct private *p)
{
    assert(p);
    p->window_state.resizing_x = 0;
    p->window_state.resizing_y = 0;
    p->window_state.moving = 0;
}

static void handle_mouse_move(struct private *p, int mouse_x, int mouse_y)
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
            FillRect((HDC)wparam, &rect, CreateSolidBrush(RGB(255, 128, 0)));
            break;
        }

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
            handle_button_down(p, VK_LBUTTON, LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_RBUTTONDOWN:
            handle_button_down(p, VK_RBUTTON, LOWORD(lparam), HIWORD(lparam));
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            handle_button_up(p);
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
    WNDCLASSEX wc;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = wnd_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = 0;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //...what the fuck?
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = class_name;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&wc))
    {
        fprintf(stderr, "Failed to register window class\n");
        return -1;
    }
    return 0;
}

static int create_window(
    const char *class_name,
    const char *title,
    const struct private *p,
    HWND *hwnd_out)
{
    assert(class_name);
    assert(title);
    assert(p);
    assert(hwnd_out);

    *hwnd_out = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        class_name,
        title,
        WS_POPUPWINDOW,
        p->x, p->y, p->width, p->height,
        NULL, NULL, 0, NULL);

    if (!*hwnd_out)
    {
        fprintf(stderr, "Failed to create a window\n");
        return -1;
    }

    SetLayeredWindowAttributes(*hwnd_out,  0, 0.5f * 255, LWA_ALPHA);

    SetWindowLongPtr(*hwnd_out, GWLP_USERDATA, (long)p);
    return 0;
}

int update_region_interactively(ShotRegion *region)
{
    const char *class_name = "shot";
    if (register_class(class_name, &wnd_proc))
        return -1;

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
        .width = 640,
        .height = 480,
    };
    p.x = (GetSystemMetrics(SM_CXSCREEN) - p.width) / 2;
    p.y = (GetSystemMetrics(SM_CYSCREEN) - p.height) / 2;

    if (create_window(class_name, "shot", &p, &p.hwnd))
        return -1;
    ShowWindow(p.hwnd, SW_SHOWNORMAL);
    UpdateWindow(p.hwnd);

    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    if (p.canceled)
        return 1;

    //wait for window close, vsync and other blows and whistles
    Sleep(100);

    const int border_size = 1;
    region->x = p.x + border_size;
    region->y = p.y + border_size;
    region->width = p.width - 2 * border_size;
    region->height = p.height - 2 * border_size;
    return 0;
}
