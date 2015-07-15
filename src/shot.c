#include <assert.h>
#include "grab.h"

int main(void)
{
    ShotRegion region;
    region.x = 5;
    region.y = 10;
    region.width = 200;
    region.height = 100;

    ShotBitmap *bitmap = grab_screenshot(&region);
    assert(bitmap);
    bitmap_save_to_png(bitmap, "bitmap.png");
    bitmap_destroy(bitmap);

    return 0;
}
