#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "grab.h"
#include "monitor.h"
#include "monitor_mgr.h"
#include "region_picker/errors.h"
#include "region_picker/interactive.h"
#include "region_picker/monitor.h"
#include "region_picker/string.h"

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

static struct ShotOptions parse_options(
    int argc, char **argv, MonitorManager *monitor_mgr)
{
    struct ShotOptions options =
    {
        .error = 0,
        .output_path = NULL,
    };

    int region_result = update_region_from_all_monitors(
        monitor_mgr, &options.region);

    const char *short_opt = "ho:r:di";
    struct option long_opt[] =
    {
        {"help",        no_argument,       NULL, 'h'},
        {"output",      required_argument, NULL, 'o'},
        {"region",      required_argument, NULL, 'r'},
        {"monitor",     required_argument, NULL, 'm'},
        {"desktop",     no_argument,       NULL, 'd'},
        {"interactive", no_argument,       NULL, 'i'},
        {NULL,          0,                 NULL, 0}
    };

    while (!options.error)
    {
        int c = getopt_long(argc, argv, short_opt, long_opt, NULL);
        if (c == -1)
            break;

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
                show_usage_hint(argv[0]);
                options.error = 1;
                break;

            case 'd':
                region_result = update_region_from_all_monitors(
                    monitor_mgr, &options.region);
                break;

            case 'm':
                region_result = update_region_from_monitor(
                    monitor_mgr, &options.region, atoi(optarg));
                break;

            case 'r':
                region_result = update_region_from_string(
                    &options.region, optarg);
                break;

            case 'i':
                region_result = update_region_interactively(&options.region);
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
                show_usage_hint(argv[0]);
                options.error = 1;
                break;
        }
    }

    if (region_result)
    {
        if (region_result == ERR_CANCELED)
        {
            fprintf(stderr, "Canceled due to user input.\n");
        }
        else if (region_result == ERR_NOT_IMPLEMENTED)
        {
            fprintf(stderr, "Not implemented. Sorry...\n");
        }
        else if (region_result == ERR_INVALID_ARGUMENT)
        {
            fprintf(stderr, "Invalid argument, aborting.\n");
            show_usage_hint(argv[0]);
        }
        else if (region_result == ERR_OTHER)
        {
            fprintf(stderr, "An error occurred, aborting.\n");
        }
        options.error = 1;
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
    int exit_code = 0;
    MonitorManager *monitor_mgr = monitor_mgr_create();
    assert(monitor_mgr);

    struct ShotOptions options = parse_options(argc, argv, monitor_mgr);

    if (options.error != 0)
    {
        exit_code = options.error;
        goto end;
    }

    if (!options.output_path)
        options.output_path = get_random_name();

    if (!options.region.width || !options.region.height)
    {
        fprintf(stderr, "Cannot take screenshot with 0 width or height.\n");
        exit_code = 1;
        goto end;
    }

    ShotBitmap *bitmap = grab_screenshot(&options.region);
    assert(bitmap);
    bitmap_save_to_png(bitmap, options.output_path);
    bitmap_destroy(bitmap);

end:
    monitor_mgr_destroy(monitor_mgr);
    return exit_code;
}
