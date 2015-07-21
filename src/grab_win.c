#include <assert.h>
#include <Windows.h>
#include "grab.h"

ShotBitmap *grab_screenshot(ShotRegion *region)
{
    ShotBitmap *bitmap_out = NULL;
    char *data = NULL;
    assert(region);

    HDC hdc_screen = GetDC(0);
    HDC hdc = CreateCompatibleDC(hdc_screen);
    HBITMAP hbitmap = CreateCompatibleBitmap(
        hdc_screen, region->width, region->height);

    SelectObject(hdc, hbitmap);

    BitBlt(hdc, 0, 0, region->width, region->height,
        hdc_screen, region->x, region->y, SRCCOPY);

    BITMAP bitmap;
    GetObject(hbitmap, sizeof(BITMAP), &bitmap);
    assert(bitmap.bmBitsPixel == 32);

    BITMAPINFO bitmap_info;
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = bitmap.bmWidth;
    bitmap_info.bmiHeader.biHeight = bitmap.bmHeight;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 24;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int scanline_size = region->width * 3;
    while (scanline_size % 4 != 0)
        scanline_size++;
    data = malloc(scanline_size * bitmap.bmHeight);
    if (!data)
        goto err;

    int result = GetDIBits(hdc, hbitmap, 0, bitmap.bmHeight,
        data, &bitmap_info, DIB_RGB_COLORS);
    if (!result)
        goto err;

    assert(bitmap_info.bmiHeader.biBitCount == 24);

    bitmap_out = bitmap_create(region->width, region->height);
    assert(bitmap_out);

    for (unsigned int y = 0; y < region->height; y++)
    {
        char *data_ptr = data + y * scanline_size;
        for (unsigned int x = 0; x < region->width; x++)
        {
            ShotPixel *pixel = bitmap_get_pixel(
                bitmap_out, x, region->height - 1 - y);
            pixel->blue = *data_ptr++;
            pixel->green = *data_ptr++;
            pixel->red = *data_ptr++;
        }
    }

err:
    free(data);
    DeleteObject(hbitmap);

    return bitmap_out;
}
