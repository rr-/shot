#ifndef BITMAP_H
#define BITMAP_H
#include <stdint.h>

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

ShotBitmap *bitmap_create(size_t width, size_t height);
void bitmap_destroy(ShotBitmap *bitmap);
ShotPixel *bitmap_get_pixel(ShotBitmap *bitmap, size_t x, size_t y);
int bitmap_save_to_png(ShotBitmap *bitmap, const char *path);

#endif
