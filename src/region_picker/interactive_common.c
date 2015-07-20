#include <stdio.h>
#include <assert.h>
#include "region_picker/interactive_common.h"

#define MIN_SIZE 30

static void _min(int *a, int b) { if (*a > b) *a = b; }
static void _max(int *a, int b) { if (*a < b) *a = b; }

void ip_handle_key_down(ShotInteractivePicker *ip, int key)
{
    assert(ip);
    int delta[2] = { 0, 0 };

    if (key == IP_KEY_LSHIFT || key == IP_KEY_RSHIFT)
    {
        ip->keyboard_state.shift = 1;
    }
    else if (key == IP_KEY_LCONTROL || key == IP_KEY_RCONTROL)
    {
        ip->keyboard_state.ctrl = 1;
    }
    else if (key == IP_KEY_ESCAPE || key == IP_KEY_Q)
    {
        ip->canceled = 1;
    }
    else if (key == IP_KEY_RETURN)
    {
        ip->canceled = -1;
    }
    else if (key == IP_KEY_LEFT || key == IP_KEY_H)
    {
        delta[0] = -1;
    }
    else if (key == IP_KEY_RIGHT || key == IP_KEY_L)
    {
        delta[0] = 1;
    }
    else if (key == IP_KEY_DOWN || key == IP_KEY_J)
    {
        delta[1] = 1;
    }
    else if (key == IP_KEY_UP || key == IP_KEY_K)
    {
        delta[1] = -1;
    }

    if (delta[0] || delta[1])
    {
        ip_pull_window_rect(ip);

        const struct Rectangle *wa = &ip->workarea;
        struct Rectangle *r = &ip->rect;

        if (!ip->keyboard_state.shift)
        {
            for (int i = 0; i < 2; i++)
                delta[i] *= 25;
        }

        if (ip->keyboard_state.ctrl)
        {
            for (int i = 0; i < 2; i++)
            {
                r->size[i] += delta[i];
                _max(&r->size[i], MIN_SIZE);
                _min(&r->size[i], wa->pos[i] + wa->size[i] - r->pos[i]);
            }
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                r->pos[i] += delta[i];
                _max(&r->pos[i], wa->pos[i]);
                _min(&r->pos[i], wa->pos[i] + wa->size[i] - r->size[i]);
            }
        }

        ip_sync_window_rect(ip);
        ip_update_text(ip);
    }
}

void ip_handle_key_up(ShotInteractivePicker *ip, int key)
{
    assert(ip);
    if (key == IP_KEY_LSHIFT || key == IP_KEY_RSHIFT)
    {
        ip->keyboard_state.shift = 0;
    }
    else if (key == IP_KEY_LCONTROL || key == IP_KEY_RCONTROL)
    {
        ip->keyboard_state.ctrl = 0;
    }
}

void ip_handle_mouse_down(
    ShotInteractivePicker *ip, int button, int mouse_x, int mouse_y)
{
    assert(ip);
    ip_pull_window_rect(ip);
    ip->last_mouse_pos[0] = mouse_x;
    ip->last_mouse_pos[1] = mouse_y;
    if (button == IP_MOUSE_LEFT)
    {
        ip->window_state.resizing[0] = 0;
        ip->window_state.resizing[1] = 0;
        ip->window_state.moving = 1;
    }
    else if (button == IP_MOUSE_RIGHT)
    {
        ip->window_state.moving = 0;
        int mouse_pos[2] = { mouse_x, mouse_y };
        for (int i = 0; i < 2; i++)
        {
            ip->window_state.resizing[i] = 0;
            if (mouse_pos[i] < ip->rect.size[i] / 3)
                ip->window_state.resizing[i] = -1;
            else if (mouse_pos[i] > ip->rect.size[i] * 2/3)
                ip->window_state.resizing[i] = 1;
        }
    }
}

void ip_handle_mouse_up(ShotInteractivePicker *ip)
{
    assert(ip);
    ip->window_state.resizing[0] = 0;
    ip->window_state.resizing[1] = 0;
    ip->window_state.moving = 0;
}

void ip_handle_mouse_move(
    ShotInteractivePicker *ip, int mouse_x, int mouse_y)
{
    assert(ip);

    const struct Rectangle *wa = &ip->workarea;
    struct Rectangle *r = &ip->rect;
    int mouse_pos[2] = { mouse_x, mouse_y };
    int old_pos[2] = { r->pos[0], r->pos[1] };

    if (ip->window_state.moving)
    {
        for (int i = 0; i < 2; i++)
        {
            r->pos[i] += mouse_pos[i] - ip->last_mouse_pos[i];
            if (r->pos[i] < wa->pos[i])
                r->pos[i] = wa->pos[i];
            if (r->pos[i] > wa->pos[i] + wa->size[i] - r->size[i])
                r->pos[i] = wa->pos[i] + wa->size[i] - r->size[i];
        }
        ip_sync_window_rect(ip);
        ip_update_text(ip);
    }

    else if (ip->window_state.resizing[0] || ip->window_state.resizing[1])
    {
        int mouse_delta[2] = {
            mouse_x - ip->last_mouse_pos[0],
            mouse_y - ip->last_mouse_pos[1]
        };

        for (int i = 0; i < 2; i++)
        {
            if (ip->window_state.resizing[i] == -1)
            {
                int npos = r->pos[i] + mouse_delta[i];
                _max(&npos, wa->pos[i]);
                _min(&npos, wa->pos[i] + wa->size[i] - r->size[i]);
                int nsize = ip->rect.size[i] + old_pos[i] - npos;
                _max(&nsize, MIN_SIZE);
                _min(&nsize, wa->pos[i] + wa->size[i] - r->pos[i]);
                npos = ip->rect.size[i] + old_pos[i] - nsize;
                ip->rect.size[i] = nsize;
                r->pos[i] = npos;
            }
            else if (ip->window_state.resizing[i] == 1)
            {
                ip->rect.size[i] += mouse_delta[i];
                _max(&r->size[i], MIN_SIZE);
                _min(&r->size[i], wa->pos[i] + wa->size[i] - r->pos[i]);
            }
        }

        ip_sync_window_rect(ip);
        ip_update_text(ip);
    }

    for (int i = 0; i < 2; i++)
        ip->last_mouse_pos[i] = mouse_pos[i] + old_pos[i] - r->pos[i];
}

void ip_init(
    ShotInteractivePicker *ip, ShotRegion *region, const ShotRegion *workarea)
{
    ip->workarea.pos[0] = workarea->x;
    ip->workarea.pos[1] = workarea->y;
    ip->workarea.size[0] = workarea->width;
    ip->workarea.size[1] = workarea->height;
    ip->rect.pos[0] = region->x;
    ip->rect.pos[1] = region->y;
    ip->rect.size[0] = region->width;
    ip->rect.size[1] = region->height;
    ip->keyboard_state.ctrl = 0;
    ip->keyboard_state.shift = 0;
    ip->window_state.moving = 0;
    ip->window_state.resizing[0] = 0;
    ip->window_state.resizing[1] = 0;
    ip->canceled = 0;

    const struct Rectangle *wa = &ip->workarea;
    struct Rectangle *r = &ip->rect;
    for (int i = 0; i < 2; i++)
    {
        _max(&r->size[i], MIN_SIZE);
        _min(&r->size[i], wa->pos[i] + wa->size[i] - r->pos[i]);
        _max(&r->pos[i], wa->pos[i]);
        _min(&r->pos[i], wa->pos[i] + wa->size[i] - r->size[i]);
    }
}
