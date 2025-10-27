#ifndef LV_HAL_H
#define LV_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_hal_disp.h"
#include "lv_hal_indev.h"
#include "lvgl/lvgl.h"
#include <stdbool.h>

/* Initialize all HAL components (display + input devices) */
void lv_hal_init(void);

/* Get HAL component pointers */
lv_display_t * lv_hal_get_display(void);
lv_indev_t * lv_hal_get_mouse(void);
lv_indev_t * lv_hal_get_keyboard(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_HAL_H */
