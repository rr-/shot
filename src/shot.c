#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "grab.h"
#include "monitor.h"

const int MODE_DESKTOP = 0;

struct ShotOptions
{
    int error;
    int mode;
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
    options.mode = MODE_DESKTOP;

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

static const char *get_random_name()
{
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    assert(tmp);

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tmp);

    static char name[30];
    strcpy(name, time_str);
    strcat(name, "_");

    srand(t);
    for (int i = 0; i < 3; i++)
    {
        char random_char = 'a' + rand() % ('z' + 1 - 'a');
        strncat(name, &random_char, 1);
    }

    strcat(name, ".png");
    return name;
}

int main(int argc, char **argv)
{
    struct ShotOptions options = parse_options(argc, argv);

    if (options.error != 0)
        return options.error;

    if (!options.output_path)
        options.output_path = get_random_name();

    ShotRegion region =
    {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0,
    };

    if (options.mode == MODE_DESKTOP)
    {
        for (size_t i = 0; i < monitor_count(); i++)
        {
            Monitor *monitor = monitor_get(i);
            if (monitor->x < region.x)
                region.x = monitor->x;
            if (monitor->y < region.y)
                region.y = monitor->y;
            if (monitor->x + monitor->width > region.width)
                region.width = monitor->x + monitor->width;
            if (monitor->y + monitor->height > region.height)
                region.height = monitor->y + monitor->height;
            monitor_destroy(monitor);
        }
    }

    ShotBitmap *bitmap = grab_screenshot(&region);
    assert(bitmap);
    bitmap_save_to_png(bitmap, options.output_path);
    bitmap_destroy(bitmap);

    return 0;
}
