#include <stdio.h>
#include "region.h"

static int int_from_string(const char **str)
{
    int ret = 0;
    while (**str >= '0' && **str <= '9')
    {
        ret *= 10;
        ret += **str - '0';
        (*str)++;
    }
    return ret;
}

int fill_region_from_string(const char *str, ShotRegion *r)
{
    ShotRegion tmp;
    tmp.width = 100;
    tmp.height = 100;
    tmp.x = 0;
    tmp.y = 0;

    const char *ptr = str;
    tmp.width = int_from_string(&ptr);
    if (*ptr != 'x' && *ptr != 'X')
        return 1;
    ptr++;

    tmp.height = int_from_string(&ptr);
    if (*ptr != '\0')
    {
        if (*ptr != '+')
            return 1;
        ptr++;
        tmp.x = int_from_string(&ptr);
        if (*ptr != '+')
            return 1;
        ptr++;
        tmp.y = int_from_string(&ptr);
        if (*ptr != '\0')
            return 1;
    }

    r->x = tmp.x;
    r->y = tmp.y;
    r->width = tmp.width;
    r->height = tmp.height;

    return 0;
}
