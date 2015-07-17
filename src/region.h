#ifndef REGION_H
#define REGION_H
#include <stddef.h>

typedef struct
{
    int x, y;
    unsigned int width, height;
} ShotRegion;

int fill_region_from_string(const char *str, ShotRegion *r);

#endif
