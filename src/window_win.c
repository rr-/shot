#include <windows.h>
#include "window.h"

static HWND hWnd = NULL;
static BOOL bUnattached = FALSE;

void hide_window()
{
    hWnd = GetConsoleWindow();
    bUnattached = GetWindowThreadProcessId(hWnd, NULL) == GetCurrentThreadId();
    if (bUnattached)
    {
        //change it to GUI mode
        FreeConsole();
        //activates another window, while `SW_HIDE` doesn't
        ShowWindow(hWnd, SW_MINIMIZE);
    }
}

void activate_window()
{
    if (bUnattached && hWnd != NULL)
    {
        //catch focus when process is launched in hidden mode
        SetForegroundWindow(hWnd);
    }
}
