#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include "grab.h"

struct ShotOptions
{
    int error;
    const char *output_path;
};

static void show_usage()
{
    printf("usage...\n");
}

static struct ShotOptions parse_options(int argc, char **argv)
{
    struct ShotOptions options;
    options.error = 0;
    options.output_path = NULL;

    const char *short_opt = "ho:";
    struct option long_opt[] =
    {
        {"help",   no_argument,       NULL, 'h'},
        {"output", required_argument, NULL, 'o'},
        {NULL,     0,                 NULL, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch (c)
        {
            case -1:
            case 0:
                break;

            case 'o':
                options.output_path = optarg;
                break;

            case 'h':
                show_usage();
                options.error = -1;
                break;

            case ':':
            case '?':
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                options.error = 1;
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                options.error = 1;
                break;
        }
    }

    return options;
}

int main(int argc, char **argv)
{
    struct ShotOptions options = parse_options(argc, argv);

    if (options.error != 0)
        return options.error;

    if (!options.output_path)
        return 1;

    ShotRegion region;
    region.x = 5;
    region.y = 10;
    region.width = 200;
    region.height = 100;

    ShotBitmap *bitmap = grab_screenshot(&region);
    assert(bitmap);
    bitmap_save_to_png(bitmap, options.output_path);
    bitmap_destroy(bitmap);

    return 0;
}
