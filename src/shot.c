#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "grab.h"
#include "monitor.h"

struct ShotOptions
{
    int error;
    const char *output_path;
    ShotRegion region;
};

static void show_usage(void)
{
    printf("usage...\n");
}

static void show_usage_hint(const char *program_name)
{
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
}

static void get_desktop_region(ShotRegion *region)
{
    unsigned int count = monitor_count();
    Monitor **monitors = (Monitor**)malloc(monitor_count());
    assert(monitors);
    for (unsigned int i = 0; i < count; i++)
    {
        monitors[i] = monitor_get(i);
        assert(monitors[i]);
    }

    int min_x = monitors[0]->x;
    int min_y = monitors[0]->y;
    for (unsigned int i = 1; i < monitor_count(); i++)
    {
        if (!i || monitors[i]->x < min_x) min_x = monitors[i]->x;
        if (!i || monitors[i]->y < min_y) min_y = monitors[i]->y;
    }

    for (unsigned int i = 0; i < count; i++)
    {
        ShotRegion pr =
        {
            .x = monitors[i]->x,
            .y = monitors[i]->y,
            .width  = monitors[i]->width  + monitors[i]->x - min_x,
            .height = monitors[i]->height + monitors[i]->y - min_y,
        };
        if (!i || pr.x      < region->x)      region->x      = pr.x;
        if (!i || pr.y      < region->y)      region->y      = pr.y;
        if (!i || pr.width  > region->width)  region->width  = pr.width;
        if (!i || pr.height > region->height) region->height = pr.height;
    }

    for (unsigned int i = 0; i < count; i++)
        monitor_destroy(monitors[i]);
    free(monitors);
}

static int get_monitor_region(ShotRegion *region, unsigned int monitor_number)
{
    Monitor *monitor = monitor_get(monitor_number);
    if (!monitor)
    {
        fprintf(
            stderr,
            "Invalid monitor number. Valid monitor numbers = 0..%d\n",
            monitor_count() - 1);
        return 1;
    }
    region->x = monitor->x;
    region->y = monitor->y;
    region->width = monitor->width;
    region->height = monitor->height;
    monitor_destroy(monitor);
    return 0;
}

static int get_string_region(ShotRegion *region, const char *string)
{
    if (fill_region_from_string(string, region))
    {
        fprintf(stderr, "Invalid region string.\n");
        return 1;
    }
    return 0;
}

static struct ShotOptions parse_options(int argc, char **argv)
{
    struct ShotOptions options =
    {
        .error = 0,
        .output_path = NULL,
    };
    get_desktop_region(&options.region);

    const char *short_opt = "ho:r:d";
    struct option long_opt[] =
    {
        {"help",    no_argument,       NULL, 'h'},
        {"output",  required_argument, NULL, 'o'},
        {"region",  required_argument, NULL, 'r'},
        {"monitor", required_argument, NULL, 'm'},
        {"desktop", no_argument,       NULL, 'd'},
        {NULL,      0,                 NULL, 0}
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

            case 'd':
                get_desktop_region(&options.region);
                break;

            case 'm':
                if (get_monitor_region(&options.region, atoi(optarg)))
                {
                    show_usage_hint(argv[0]);
                    options.error = 1;
                }
                break;

            case 'r':
                if (get_string_region(&options.region, optarg))
                {
                    show_usage_hint(argv[0]);
                    options.error = 1;
                }
                break;

            case ':':
            case '?':
                show_usage_hint(argv[0]);
                options.error = 1;
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
                show_usage_hint(argv[0]);
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

    if (!options.region.width || !options.region.height)
    {
        fprintf(stderr, "Cannot take screenshot with 0 width or height.\n");
        return 1;
    }

    ShotBitmap *bitmap = grab_screenshot(&options.region);
    assert(bitmap);
    bitmap_save_to_png(bitmap, options.output_path);
    bitmap_destroy(bitmap);

    return 0;
}
