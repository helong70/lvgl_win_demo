#ifndef MAINCONTAINER_H
#define MAINCONTAINER_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Main container API */
lv_obj_t * maincontainer_create(lv_obj_t * parent, int width, int height, int title_height);
lv_group_t * maincontainer_get_keyboard_group(void);

#ifdef __cplusplus
}
#endif

#endif /* MAINCONTAINER_H */