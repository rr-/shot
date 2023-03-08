#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include "region_picker/errors.h"
#include "region_picker/interactive.h"
#include "region_picker/interactive_common.h"

int IP_MOUSE_LEFT   = VK_LBUTTON;
int IP_MOUSE_RIGHT  = VK_RBUTTON;
int IP_KEY_LSHIFT   = VK_SHIFT;
int IP_KEY_RSHIFT   = VK_SHIFT;
int IP_KEY_LCONTROL = VK_CONTROL;
int IP_KEY_RCONTROL = VK_CONTROL;
int IP_KEY_ESCAPE   = VK_ESCAPE;
int IP_KEY_Q        = 'Q';
int IP_KEY_H        = 'H';
int IP_KEY_J        = 'J';
int IP_KEY_K        = 'K';
int IP_KEY_L        = 'L';
int IP_KEY_LEFT     = VK_LEFT;
int IP_KEY_DOWN     = VK_DOWN;
int IP_KEY_UP       = VK_UP;
int IP_KEY_RIGHT    = VK_RIGHT;
int IP_KEY_RETURN   = VK_RETURN;

struct private
{
    int mouse_captured;
    HWND hwnd;
    ShotInteractivePicker ip;
    unsigned int border_size;
};

void ip_pull_window_rect(ShotInteractivePicker *ip)
{
    assert(ip);

    struct private *priv = ip->priv;
    RECT rect;
    GetWindowRect(priv->hwnd, &rect);
    ip->rect.pos[0] = rect.left + priv->border_size;
    ip->rect.pos[1] = rect.top + priv->border_size;
    ip->rect.size[0] = rect.right - rect.left - 2 * priv->border_size;
    ip->rect.size[1] = rect.bottom - rect.top - 2 * priv->border_size;
}

void ip_sync_window_rect(ShotInteractivePicker *ip)
{
    assert(ip);
    struct private *priv = ip->priv;
    MoveWindow(
        priv->hwnd,
        ip->rect.pos[0] - priv->border_size,
        ip->rect.pos[1] - priv->border_size,
        ip->rect.size[0] + 2 * priv->border_size,
        ip->rect.size[1] + 2 * priv->border_size,
        TRUE);
}

void ip_update_text(ShotInteractivePicker *ip)
{
    struct private *priv = ip->priv;
    RECT rect;
    GetClientRect(priv->hwnd, &rect);
    InvalidateRect(priv->hwnd, &rect, TRUE);
}

static void draw_text(struct private *p)
{
    char msg[100];
    sprintf(msg,
        "%dx%d+%d+%d",
        p->ip.rect.size[0], p->ip.rect.size[1],
        p->ip.rect.pos[0], p->ip.rect.pos[1]);

    RECT rect;
    GetClientRect(p->hwnd, &rect);
    HDC wdc = GetWindowDC(p->hwnd);
    SetTextColor(wdc, 0x00000000);
    SetBkMode(wdc,TRANSPARENT);
    DrawText(wdc, msg, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    DeleteDC(wdc);
}

static LRESULT CALLBACK wnd_proc(
    HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    struct private *p = (struct private *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
            ip_handle_key_down(&p->ip, wparam);
            break;

        case WM_KEYUP:
            ip_handle_key_up(&p->ip, wparam);
            break;

        case WM_LBUTTONDOWN:
            ip_handle_mouse_down(
                &p->ip, IP_MOUSE_LEFT, LOWORD(lparam), HIWORD(lparam));
            p->mouse_captured = 1;
            SetCapture(p->hwnd);
            break;

        case WM_RBUTTONDOWN:
            ip_handle_mouse_down(
                &p->ip, IP_MOUSE_RIGHT, LOWORD(lparam), HIWORD(lparam));
            p->mouse_captured = 1;
            SetCapture(p->hwnd);
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            ip_handle_mouse_up(&p->ip);
            if (p->mouse_captured)
            {
                ReleaseCapture();
                p->mouse_captured = 0;
            }
            break;

        case WM_MOUSEMOVE:
            ip_handle_mouse_move(
                &p->ip,
                (short)LOWORD(lparam),
                (short)HIWORD(lparam));
            break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    if (p->ip.canceled)
    {
        ShowWindow(p->hwnd, SW_HIDE);
        PostQuitMessage(0);
    }

    return 0;
}

static int register_class(
    const char *class_name,
    LRESULT CALLBACK (*wnd_proc)(HWND, UINT, WPARAM, LPARAM))
{
    assert(class_name);
    assert(wnd_proc);
    WNDCLASSEX wc = {
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
        WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_APPWINDOW,
        class_name,
        title,
        WS_POPUPWINDOW,
        p->ip.rect.pos[0] - p->border_size,
        p->ip.rect.pos[1] - p->border_size,
        p->ip.rect.size[0] + 2 * p->border_size,
        p->ip.rect.size[1] + 2 * p->border_size,
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
    while (!p->ip.canceled && GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}

int update_region_interactively(ShotRegion *region, const ShotRegion *workarea)
{
    const char *class_name = "shot";
    if (register_class(class_name, &wnd_proc))
        return ERR_OTHER;

    struct private p = {
        .mouse_captured = 0,
        .border_size = 1,
    };
    ip_init(&p.ip, region, workarea);
    p.ip.priv = &p;

    if (init_window(class_name, "shot", &p))
        return ERR_OTHER;

    run_event_loop(&p);

    if (p.ip.canceled == 1)
        return ERR_CANCELED;

    //wait for window close, vsync and other blows and whistles
    Sleep(100);

    region->x = p.ip.rect.pos[0];
    region->y = p.ip.rect.pos[1];
    region->width = p.ip.rect.size[0];
    region->height = p.ip.rect.size[1];
    return 0;
}
