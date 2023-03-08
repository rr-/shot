#include <assert.h>
#include <png.h>
#include <stdlib.h>
#include "bitmap.h"

ShotBitmap *bitmap_create(unsigned int width, unsigned int height)
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

ShotPixel *bitmap_get_pixel(ShotBitmap *bitmap, unsigned int x, unsigned int y)
{
    return &bitmap->pixels[bitmap->width * y + x];
}

int bitmap_save_to_png(ShotBitmap *bitmap, const char *path)
{
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    volatile int ret = 1;

    FILE *fp = fopen(path, "wb");
    if (!fp)
    {
        fprintf(stderr, "Can't open %s for writing.\n", path);
        goto end;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "Can't intialize PNG writer.\n");
        goto end;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "Can't intialize PNG writer.\n");
        goto end;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Can't intialize PNG writer.\n");
        goto end;
    }

    png_set_IHDR(png_ptr,
        info_ptr,
        bitmap->width,
        bitmap->height,
        8, //depth
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_byte **row_pointers = png_malloc(
        png_ptr, bitmap->height * sizeof(png_byte*));
    for (unsigned int y = 0; y < bitmap->height; y++)
    {
        png_byte *row = png_malloc(png_ptr, bitmap->width * 3); //channels
        row_pointers[y] = row;
        for (unsigned int x = 0; x < bitmap->width; x++)
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

    ret = 0;
    for (unsigned int y = 0; y < bitmap->height; y++)
        png_free(png_ptr, row_pointers[y]);
    png_free(png_ptr, row_pointers);

end:
    if (png_ptr && info_ptr)
        png_destroy_write_struct (&png_ptr, &info_ptr);
    if (fp)
        fclose(fp);
    return ret;
}
