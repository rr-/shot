#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    size_t x, y;
    size_t width, height;
} ShotRegion;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ShotPixel;

typedef struct
{
    ShotPixel *pixels;
    size_t width;
    size_t height;
} ShotBitmap;

ShotBitmap *bitmap_create(size_t width, size_t height)
{
    ShotBitmap *bitmap = malloc(sizeof(ShotBitmap));
    bitmap->width = width;
    bitmap->height = height;
    bitmap->pixels = calloc(sizeof(ShotPixel), width * height);
    return bitmap;
}

void bitmap_destroy(ShotBitmap *bitmap)
{
    assert(bitmap);
    free(bitmap->pixels);
    free(bitmap);
}

ShotPixel *bitmap_get_pixel(ShotBitmap *bitmap, size_t x, size_t y)
{
    return &bitmap->pixels[bitmap->width * y + x];
}

static int bitmap_save_to_png(ShotBitmap *bitmap, const char *path)
{
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    int status = -1;

    FILE *fp = fopen(path, "wb");
    if (!fp)
        goto end;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto end;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        goto end;

    if (setjmp(png_jmpbuf(png_ptr)))
        goto end;

    png_set_IHDR(png_ptr,
        info_ptr,
        bitmap->width,
        bitmap->height,
        8, //depth
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_byte **row_pointers =
        png_malloc(png_ptr, bitmap->height * sizeof(png_byte*));
    for (size_t y = 0; y < bitmap->height; y++)
    {
        png_byte *row = png_malloc(png_ptr, bitmap->width * 3); //channels
        row_pointers[y] = row;
        for (size_t x = 0; x < bitmap->width; x++)
        {
            ShotPixel *pixel = bitmap_get_pixel(bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    status = 0;
    for (size_t y = 0; y < bitmap->height; y++)
        png_free(png_ptr, row_pointers[y]);
    png_free(png_ptr, row_pointers);

end:
    if (png_ptr && info_ptr)
        png_destroy_write_struct (&png_ptr, &info_ptr);
    if (fp)
        fclose(fp);
    return status;
}

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
