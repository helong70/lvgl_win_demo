#ifndef LV_HAL_DISP_H
#define LV_HAL_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include <stdint.h>
#include <stdbool.h>

/* Display configuration */
#define LV_HAL_DISP_WIDTH  800
#define LV_HAL_DISP_HEIGHT 600
#define LV_HAL_DISP_BUF_SIZE (LV_HAL_DISP_WIDTH * LV_HAL_DISP_HEIGHT / 10)

/* Initialize display */
lv_display_t * lv_hal_disp_init(void);

/* Get display pointer */
lv_display_t * lv_hal_disp_get(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_HAL_DISP_H */
