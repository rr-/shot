#include <windows.h>
#include "window.h"

static HWND hWnd = NULL;
static BOOL bGUI = FALSE;

void hide_window()
{
    hWnd = GetConsoleWindow();
    bGUI = GetWindowThreadProcessId(hWnd, NULL) == GetCurrentThreadId();
    if (bGUI)
    {
        ShowWindow(hWnd, SW_HIDE); //doesn't activate another window
        ShowWindow(hWnd, SW_MINIMIZE); //activates another window
        ShowWindow(hWnd, SW_HIDE);
    }
}

void activate_window()
{
    if (bGUI && hWnd != NULL)
    {
        ShowWindow(hWnd, SW_RESTORE); //required by SW_MINIMIZE
        ShowWindow(hWnd, SW_HIDE);
    }
}
