#ifndef GRAB_H
#define GRAB_H
#include <stddef.h>
#include "bitmap.h"

typedef struct
{
    size_t x, y;
    size_t width, height;
} ShotRegion;

extern ShotBitmap *grab_screenshot(ShotRegion *region);

#endif
