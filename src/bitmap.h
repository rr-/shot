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
    unsigned int width;
    unsigned int height;
} ShotBitmap;

ShotBitmap *bitmap_create(unsigned int width, unsigned int height);
void bitmap_destroy(ShotBitmap *bitmap);
ShotPixel *bitmap_get_pixel(ShotBitmap *bitmap, unsigned int x, unsigned int y);
int bitmap_save_to_png(ShotBitmap *bitmap, const char *path);
int bitmap_save_to_clipboard(ShotBitmap *bitmap);

#endif
