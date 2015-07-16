#ifndef REGION_H
#define REGION_H
#include <stddef.h>

typedef struct
{
    size_t x, y;
    size_t width, height;
} ShotRegion;

int fill_region_from_string(const char *str, ShotRegion *r);

#endif
