#include <windows.h>

int main()
{
    HDC hDC=NULL;
    RECT clientRect;
    DPtoLP(hDC, (LPPOINT)&clientRect, 2);
}
