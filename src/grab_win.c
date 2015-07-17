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

    size_t scanline_len = bitmap.bmWidth * bitmap.bmBitsPixel;
    size_t data_len = scanline_len * bitmap.bmHeight;
    BITMAPINFO bitmap_info;
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = bitmap.bmWidth;
    bitmap_info.bmiHeader.biHeight = bitmap.bmHeight;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 24;
    bitmap_info.bmiHeader.biCompression = 0;

    data = malloc(data_len);
    if (!data)
        goto err;

    int result = GetDIBits(hdc, hbitmap, 0, bitmap.bmHeight,
        data, &bitmap_info, DIB_RGB_COLORS);
    if (!result)
        goto err;

    bitmap_out = bitmap_create(region->width, region->height);
    assert(bitmap_out);

    char *data_ptr = data;
    assert(data_len >= 3 * region->height * region->width);
    for (unsigned int y = 0; y < region->height; y++)
    {
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
