#include <stdio.h>
#include <windows.h>
#include "bitmap.h"

int bitmap_save_to_clipboard(ShotBitmap *bitmap)
{
    int ret = 1;
    size_t stride = ((bitmap->width * 32 + 31) / 32) * 3;
    size_t bitmap_size = sizeof(BITMAPINFOHEADER) + bitmap->height * stride;

    HGLOBAL memory = GlobalAlloc(GHND | GMEM_SHARE, bitmap_size);
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

    BITMAPINFOHEADER header = {0};
    header.biSize           = sizeof(BITMAPINFOHEADER);
    header.biWidth         = bitmap->width;
    header.biHeight        = -bitmap->height;
    header.biPlanes        = 1;
    header.biBitCount      = 24;
    header.biCompression   = BI_RGB;
    header.biSizeImage     = bitmap->height * stride;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed       = 0;
    header.biClrImportant  = 0;

    *((BITMAPINFOHEADER*)dest) = header;
    dest += sizeof(BITMAPINFOHEADER);
    ShotPixel *src = bitmap->pixels;
    for (unsigned int y = 0; y < bitmap->height; y++)
    for (unsigned int x = 0; x < bitmap->width; x++)
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
    if (!SetClipboardData(CF_DIB, memory))
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
