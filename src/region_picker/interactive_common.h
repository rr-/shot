#ifndef INTERACTIVE_COMMON_H
#define INTERACTIVE_COMMON_H
#include "region.h"

extern int IP_MOUSE_LEFT;
extern int IP_MOUSE_RIGHT;
extern int IP_KEY_LSHIFT;
extern int IP_KEY_RSHIFT;
extern int IP_KEY_LCONTROL;
extern int IP_KEY_RCONTROL;
extern int IP_KEY_ESCAPE;
extern int IP_KEY_Q;
extern int IP_KEY_H;
extern int IP_KEY_J;
extern int IP_KEY_K;
extern int IP_KEY_L;
extern int IP_KEY_LEFT;
extern int IP_KEY_DOWN;
extern int IP_KEY_UP;
extern int IP_KEY_RIGHT;
extern int IP_KEY_RETURN;

struct private;

struct Rectangle
{
    int pos[2];
    int size[2];
};

typedef struct
{
    struct private *priv;

    struct
    {
        int ctrl;
        int shift;
    } keyboard_state;

    struct
    {
        int moving;
        int resizing[2];
    } window_state;

    int last_mouse_pos[2];

    struct Rectangle rect;
    struct Rectangle workarea;

    int canceled;
} ShotInteractivePicker;

void ip_init(
    ShotInteractivePicker *ip, ShotRegion *region, const ShotRegion *workarea);
void ip_handle_key_down(ShotInteractivePicker *ip, int key);
void ip_handle_key_up(ShotInteractivePicker *ip, int key);
void ip_handle_mouse_down(
    ShotInteractivePicker *p, int button, int mouse_x, int mouse_y);
void ip_handle_mouse_up(ShotInteractivePicker *ip);
void ip_handle_mouse_move(
    ShotInteractivePicker *ip, int mouse_x, int mouse_y);

void ip_pull_window_rect(ShotInteractivePicker *ip);
void ip_sync_window_rect(ShotInteractivePicker *ip);
void ip_update_text(ShotInteractivePicker *ip);

#endif
