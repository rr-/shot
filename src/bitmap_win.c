#include <stdio.h>
#include <windows.h>
#include "bitmap.h"

int bitmap_save_to_clipboard(ShotBitmap *bitmap)
{
    int ret = 1;
    size_t stride = ((bitmap->width * 32 + 31) / 32) * 3;
    size_t bitmap_size = sizeof(BITMAPV5HEADER) + bitmap->height * stride;

    HGLOBAL memory = GlobalAlloc(GHND, bitmap_size);
    if (!memory)
    {
        fprintf(stderr, "Error allocating memory for bitmap data.\n");
        goto end;
    }

    char *dest = (char*) GlobalLock(memory);
    if (!dest)
    {
        fprintf(stderr, "Error accessing memory for bitmap data.\n");
        goto end;
    }

    BITMAPV5HEADER header = {0};
    header.bV5Size          = sizeof(BITMAPV5HEADER);
    header.bV5Width         = bitmap->width;
    header.bV5Height        = -bitmap->height;
    header.bV5Planes        = 1;
    header.bV5BitCount      = 24;
    header.bV5Compression   = BI_RGB;
    header.bV5SizeImage     = bitmap->height * stride;
    header.bV5XPelsPerMeter = 0;
    header.bV5YPelsPerMeter = 0;
    header.bV5ClrUsed       = 0;
    header.bV5ClrImportant  = 0;
    header.bV5BlueMask      = 0x000000FF;
    header.bV5GreenMask     = 0x0000FF00;
    header.bV5RedMask       = 0x00FF0000;
    header.bV5AlphaMask     = 0x00000000;

    *((BITMAPV5HEADER*)dest) = header;
    dest += sizeof(BITMAPV5HEADER);
    ShotPixel *src = bitmap->pixels;
    for (int y = 0; y < bitmap->height; y++)
    for (int x = 0; x < bitmap->width; x++)
    {
        dest[y * stride + x * 3 + 0] = src->blue;
        dest[y * stride + x * 3 + 1] = src->green;
        dest[y * stride + x * 3 + 2] = src->red;
        src++;
    }

    GlobalUnlock(memory);

    if (!OpenClipboard(NULL))
    {
        fprintf(stderr, "Error opening clipboard.\n");
        goto end;
    }
    if (!EmptyClipboard())
    {
        fprintf(stderr, "Error emptying clipboard.\n");
        goto end;
    }
    if (!SetClipboardData(CF_DIBV5, memory))
    {
        fprintf(stderr, "Error setting bitmap on clipboard.\n");
        goto end;
    }

    memory = NULL;
    CloseClipboard();
    ret = 0;

end:
    if (memory)
        GlobalFree(memory);
    return ret;
}
