#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bitmap.h"

typedef struct
{
    size_t x, y;
    size_t width, height;
} ShotRegion;

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

            pixel_out->red = color & 0xFF;
            pixel_out->green = (color >> 8) & 0xFF;
            pixel_out->blue = (color >> 16) & 0xFF;
        }
    }

    XDestroyImage(image);
    return bitmap;
}

int main(void)
{
    ShotRegion region;
    region.x = 5;
    region.y = 10;
    region.width = 200;
    region.height = 100;

    ShotBitmap *bitmap = grab_screenshot(&region);
    bitmap_save_to_png(bitmap, "bitmap.png");
    bitmap_destroy(bitmap);

    return 0;
}
