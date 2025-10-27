#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "lvgl/lvgl.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Titlebar configuration */
#define TITLEBAR_HEIGHT 36          /* Height in logical pixels */
#define TITLEBAR_DRAG_EXCLUDE_RIGHT 79  /* Pixels to exclude from right edge (for buttons) */

/* Function declarations */
lv_obj_t * titlebar_create(lv_obj_t * parent, int width);
void titlebar_set_title(lv_obj_t * titlebar, const char * title);

/* Event callbacks */
void titlebar_event_cb(lv_event_t * e);
void close_btn_event_cb(lv_event_t * e);
void min_btn_event_cb(lv_event_t * e);

/* Get main window handle from main.c */
HWND get_main_window_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* TITLEBAR_H */