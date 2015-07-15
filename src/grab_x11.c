#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "grab.h"

ShotBitmap *grab_screenshot(ShotRegion *region)
{
    assert(region);

    Display *display = XOpenDisplay(NULL);
    assert(display);

    XImage *image = XGetImage(
        display, RootWindow(display, DefaultScreen(display)),
        region->x, region->y, region->width, region->height,
        AllPlanes, ZPixmap);
    assert(image);

    ShotBitmap *bitmap = bitmap_create(region->width, region->height);
    assert(bitmap);

    for (size_t y = 0; y < bitmap->height; y++)
    {
        for (size_t x = 0; x < bitmap->width; x++)
        {
            uint32_t color = XGetPixel(image, x, y);
            ShotPixel *pixel_out = bitmap_get_pixel(bitmap, x, y);
            assert(pixel_out);

            pixel_out->red = (color >> 16) & 0xFF;
            pixel_out->green = (color >> 8) & 0xFF;
            pixel_out->blue = color & 0xFF;
        }
    }

    XDestroyImage(image);
    XCloseDisplay(display);
    return bitmap;
}
