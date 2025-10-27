#ifndef LV_HAL_INDEV_H
#define LV_HAL_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

/* Keyboard queue configuration */
#define KEYBOARD_QUEUE_SIZE 16

/* Keyboard event structure */
typedef struct {
    uint32_t key;
    lv_indev_state_t state;
    DWORD timestamp;
} keyboard_event_t;

/* Initialize input devices */
void lv_hal_indev_init(lv_display_t * display);

/* Get input device pointers */
lv_indev_t * lv_hal_indev_get_mouse(void);
lv_indev_t * lv_hal_indev_get_keyboard(void);

/* Keyboard queue operations (called by platform callbacks) */
void lv_hal_keyboard_queue_push(uint32_t key, lv_indev_state_t state);

/* Mouse event handler (called by platform callbacks) */
void lv_hal_mouse_set_state(int x, int y, bool pressed);

#ifdef __cplusplus
}
#endif

#endif /* LV_HAL_INDEV_H */
