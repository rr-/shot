#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "grab.h"
#include "help.h"
#include "monitor.h"
#include "monitor_mgr.h"
#include "window.h"
#include "region_picker/active_monitor.h"
#include "region_picker/active_window.h"
#include "region_picker/errors.h"
#include "region_picker/interactive.h"
#include "region_picker/monitor.h"
#include "region_picker/string.h"
#include "region_picker/window.h"

#define STATUS_CONTINUE 0
#define STATUS_EXIT_ERROR 1
#define STATUS_EXIT_NO_ERROR 2

struct ShotOptions
{
    int status;
    int clipboard;
    const char *output_path;
    ShotRegion region;
};

static void show_usage(void)
{
    printf("%s", help_str);
}

static void show_version(void)
{
    printf("shot v%s\n", SHOT_VERSION);
}

static void show_usage_hint(const char *program_name)
{
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
}

static void show_monitors(const MonitorManager *monitor_mgr)
{
    printf("Available monitors: %d\n", monitor_mgr->monitor_count);
    for (unsigned int i = 0; i < monitor_mgr->monitor_count; i++)
    {
        Monitor *m = monitor_mgr->monitors[i];
        printf("%d - %dx%d+%d+%d", i, m->width, m->height, m->x, m->y);
        if (m->primary)
            printf(" (primary)");
        puts("");
    }
}

static void init_region(ShotRegion *region, const MonitorManager *monitor_mgr)
{
    //if user calls -i, it should be initialized with a 640x480 window
    region->width = 640;
    region->height = 480;
    region->x = 40;
    region->y = 40;

    //...and that window should be centered on the default screen
    for (unsigned int i = 0; i < monitor_mgr->monitor_count; i++)
    {
        const Monitor *m = monitor_mgr->monitors[i];
        if (m->primary)
        {
            region->x = m->x + (m->width - region->width) / 2;
            region->y = m->y + (m->height - region->height) / 2;
        }
    }
}

static struct ShotOptions parse_options(
    int argc, char **argv, const MonitorManager *monitor_mgr)
{
    assert(argv);
    assert(monitor_mgr);
    assert(monitor_mgr->monitor_count);

    struct ShotOptions options = {
        .clipboard = 0,
        .status = STATUS_CONTINUE,
        .output_path = NULL,
    };

    init_region(&options.region, monitor_mgr);

    int region_result = -1;

    const char *short_opt = "ho:r:diw:Wvm:Mlc";
    struct option long_opt[] = {
        {"list",           no_argument,       NULL, 'l'},
        {"help",           no_argument,       NULL, 'h'},
        {"output",         required_argument, NULL, 'o'},
        {"clipboard",      no_argument,       NULL, 'c'},
        {"region",         required_argument, NULL, 'r'},
        {"monitor",        required_argument, NULL, 'm'},
        {"window",         required_argument, NULL, 'w'},
        {"desktop",        no_argument,       NULL, 'd'},
        {"interactive",    no_argument,       NULL, 'i'},
        {"active-monitor", no_argument,       NULL, 'M'},
        {"active-window",  no_argument,       NULL, 'W'},
        {"version",        no_argument,       NULL, 'v'},
        {NULL,             0,                 NULL, 0}
    };

    while (options.status == STATUS_CONTINUE)
    {
        int c = getopt_long(argc, argv, short_opt, long_opt, NULL);
        if (c == -1)
            break;

        switch (c)
        {
            case -1:
            case 0:
                break;

            case 'v':
                show_version();
                options.status = STATUS_EXIT_NO_ERROR;
                break;

            case 'h':
                show_usage();
                options.status = STATUS_EXIT_NO_ERROR;
                break;

            case ':':
            case '?':
                show_usage_hint(argv[0]);
                options.status = STATUS_EXIT_ERROR;
                break;

            case 'l':
                show_monitors(monitor_mgr);
                options.status = STATUS_EXIT_NO_ERROR;
                break;

            case 'c':
                options.clipboard = 1;
                break;

            case 'o':
                options.output_path = optarg;
                break;

            case 'd':
                region_result = update_region_from_all_monitors(
                    &options.region, monitor_mgr);
                break;

            case 'm':
            {
                size_t n = atoi(optarg);
                if (n >= monitor_mgr->monitor_count)
                {
                    fprintf(
                        stderr,
                        "Invalid monitor number. "
                        "Valid monitor numbers = 0..%d\n",
                        monitor_mgr->monitor_count - 1);
                    region_result = ERR_INVALID_ARGUMENT;
                }
                else
                {
                    region_result = update_region_from_monitor(
                        &options.region, monitor_mgr->monitors[n]);
                }
                break;
            }

            case 'M':
            {
                region_result = update_region_from_active_monitor(
                    &options.region, monitor_mgr);
                break;
            }

            case 'r':
                region_result = update_region_from_string(
                    &options.region, optarg);
                break;

            case 'i':
            {
                ShotRegion working_area;
                region_result = update_region_from_all_monitors(
                    &working_area, monitor_mgr);
                assert(!region_result);
                activate_window();
                region_result = update_region_interactively(
                    &options.region, &working_area);
                break;
            }

            case 'w':
            {
                long int n = atoi(optarg);
                region_result = update_region_from_window(&options.region, n);
                break;
            }

            case 'W':
                region_result = update_region_from_active_window(
                    &options.region);
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
                show_usage_hint(argv[0]);
                options.status = STATUS_EXIT_ERROR;
                break;
        }
    }

    //if no region was chosen in the arguments
    if (region_result == -1)
    {
        //take the whole desktop
        region_result = update_region_from_all_monitors(
            &options.region, monitor_mgr);
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
        options.status = STATUS_EXIT_ERROR;
    }

    return options;
}

static char *get_random_name()
{
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    assert(tmp);

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tmp);

    char *name = malloc(30);
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

static char *get_output_path(const char *path)
{
    if (!path || !*path)
        return get_random_name();

    const char *end = &path[strlen(path) - 1];
    if (*end == '/' || *end == '\\')
    {
        char *random_name = get_random_name();
        assert(random_name);
        char *path_copy = malloc(strlen(path) + strlen(random_name) + 1);
        assert(path_copy);
        strcpy(path_copy, path);
        strcat(path_copy, random_name);
        free(random_name);
        return path_copy;
    }
    else
    {
        char *path_copy = malloc(strlen(path) + 1);
        assert(path_copy);
        strcpy(path_copy, path);
        return path_copy;
    }
}

int main(int argc, char **argv)
{
    char *output_path = NULL;
    int exit_code = 1;

    ShotBitmap *bitmap = NULL;

    hide_window();

    MonitorManager *monitor_mgr = monitor_mgr_create();
    if (!monitor_mgr)
    {
        fprintf(stderr, "Failed to initialize monitor manager, aborting.\n");
        goto end;
    }

    struct ShotOptions options = parse_options(argc, argv, monitor_mgr);

    if (options.status == STATUS_EXIT_ERROR)
    {
        goto end;
    }
    else if (options.status == STATUS_EXIT_NO_ERROR)
    {
        exit_code = 0;
        goto end;
    }

    if (!options.clipboard)
    {
        output_path = get_output_path(options.output_path);
        assert(output_path);
    }

    if (options.region.width <= 0 || options.region.height <= 0)
    {
        fprintf(stderr,
            "Cannot take screenshot with non-positive width or height.\n");
        goto end;
    }

    bitmap = grab_screenshot(&options.region);
    assert(bitmap);
    if (options.clipboard)
    {
        if (bitmap_save_to_clipboard(bitmap))
            goto end;
    }
    else
    {
        if (bitmap_save_to_png(bitmap, output_path))
            goto end;
    }
    exit_code = 0;

end:
    if (output_path)
        free(output_path);
    if (bitmap)
        bitmap_destroy(bitmap);
    monitor_mgr_destroy(monitor_mgr);
    return exit_code;
}
