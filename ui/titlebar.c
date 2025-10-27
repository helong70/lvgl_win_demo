#include "titlebar.h"
#include <stdio.h>

/* Conditional printf - only output in console mode */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

#define USE_OPENGL 1
#include <windows.h>

/* Static variables for titlebar */
static lv_obj_t * title_label = NULL;
static bool close_btn_pressed = false;
static bool min_btn_pressed = false;
static lv_color_t close_btn_default_color;

/* Animation callback for close button */
static void anim_ready_cb(lv_anim_t * a)
{
    (void)a; /* Ignore parameter */
#if USE_OPENGL
    PostQuitMessage(0); /* Close application */
#endif
}

lv_obj_t * titlebar_create(lv_obj_t * parent, int width)
{
    /* Create titlebar container */
    lv_obj_t * title_bar = lv_obj_create(parent);
    lv_obj_set_size(title_bar, width + 6, TITLEBAR_HEIGHT + 2);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, -2);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2E2E2E), 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(title_bar, 6, 0);
    
    /* Disable internal scrolling for the title bar */
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(title_bar, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(title_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(title_bar, titlebar_event_cb, LV_EVENT_ALL, NULL);

    /* Title text */
    title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "LVGL OpenGL Demo");
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);

    /* Close button on the right of titlebar */
    lv_obj_t * close_btn = lv_btn_create(title_bar);
    lv_obj_set_size(close_btn, 48, 36);
    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID, 22+5, 0);
    lv_obj_add_event_cb(close_btn, close_btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t * cb_label = lv_label_create(close_btn);
    lv_label_set_text(cb_label, LV_SYMBOL_CLOSE);
    lv_obj_align(cb_label, LV_ALIGN_CENTER, -5, 0);
    /* Style close button */
    lv_obj_set_style_radius(close_btn, 0, 0);
    lv_obj_set_style_shadow_width(close_btn, 0, 0);
    /* Save default background color for later restoration */
    close_btn_default_color = lv_obj_get_style_bg_color(close_btn, LV_PART_MAIN);

    /* Minimize button */
    lv_obj_t * min_btn = lv_btn_create(title_bar);
    lv_obj_set_size(min_btn, 38, 36);
    lv_obj_set_style_bg_color(min_btn, lv_color_hex(0xAF7F00), 0);
    lv_obj_align(min_btn, LV_ALIGN_RIGHT_MID, -26+5, 0);
    lv_obj_add_event_cb(min_btn, min_btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t * min_label = lv_label_create(min_btn);
    lv_label_set_text(min_label, LV_SYMBOL_MINUS);
    lv_obj_center(min_label);
    lv_obj_set_style_radius(min_btn, 0, 0);
    lv_obj_set_style_shadow_width(min_btn, 0, 0);

    DEBUG_PRINTF("Titlebar created with dimensions: %dx%d\n", width + 6, TITLEBAR_HEIGHT + 2);
    return title_bar;
}

void titlebar_set_title(lv_obj_t * titlebar, const char * title)
{
    if (title_label && title) {
        lv_label_set_text(title_label, title);
        DEBUG_PRINTF("Titlebar title set to: %s\n", title);
    }
}

void titlebar_event_cb(lv_event_t * e)
{
    /* Window dragging is now handled natively via WM_NCHITTEST in WndProc.
     * This function can be used for other title bar interactions if needed.
     */
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        /* Title bar was clicked - native dragging will handle movement */
        DEBUG_PRINTF("Title bar clicked\n");
    }
}

void close_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (code == LV_EVENT_PRESSED) {
        /* Mark button as pressed */
        close_btn_pressed = true;
        /* Change button color to dark red */
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0000af), 0);
        DEBUG_PRINTF("Close button pressed\n");
    }
    else if (code == LV_EVENT_RELEASED) {
        /* Check if mouse is still over the button */
        lv_indev_t * indev = lv_indev_get_act();
        if (close_btn_pressed && indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            /* Check if point is within button bounds */
            lv_area_t btn_area;
            lv_obj_get_coords(btn, &btn_area);
            
            if (point.x >= btn_area.x1 && point.x <= btn_area.x2 &&
                point.y >= btn_area.y1 && point.y <= btn_area.y2) {
                /* Mouse released inside button - trigger close */
                DEBUG_PRINTF("Close button clicked - starting exit animation\n");
                
                /* Create an animation to fade out the button */
                lv_anim_t anim;
                lv_anim_init(&anim);
                lv_anim_set_var(&anim, btn);
                lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
                lv_anim_set_time(&anim, 50); /* 50ms animation */
                lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);

                /* Set animation completion callback */
                lv_anim_set_ready_cb(&anim, anim_ready_cb);

                lv_anim_start(&anim);
            } else {
                /* Mouse released outside button - restore default color */
                lv_obj_set_style_bg_color(btn, close_btn_default_color, 0);
                DEBUG_PRINTF("Close button released outside bounds - cancelled\n");
            }
        }
        close_btn_pressed = false;
    }
    else if (code == LV_EVENT_PRESS_LOST) {
        /* Button press was lost (e.g., mouse moved out) - restore default color */
        lv_obj_set_style_bg_color(btn, close_btn_default_color, 0);
        close_btn_pressed = false;
        DEBUG_PRINTF("Close button press lost - color restored\n");
    }
}

void min_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (code == LV_EVENT_PRESSED) {
        /* Mark button as pressed */
        min_btn_pressed = true;
        DEBUG_PRINTF("Minimize button pressed\n");
    }
    else if (code == LV_EVENT_RELEASED) {
        /* Check if mouse is still over the button */
        lv_indev_t * indev = lv_indev_get_act();
        if (min_btn_pressed && indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            /* Check if point is within button bounds */
            lv_area_t btn_area;
            lv_obj_get_coords(btn, &btn_area);
            
            if (point.x >= btn_area.x1 && point.x <= btn_area.x2 &&
                point.y >= btn_area.y1 && point.y <= btn_area.y2) {
                /* Mouse released inside button - trigger minimize */
                DEBUG_PRINTF("Minimize button clicked\n");
                
#if USE_OPENGL
                HWND hwnd = get_main_window_handle();
                if (hwnd) {
                    /* Change button color for visual feedback */
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x8B5A00), 0);
                    
                    /* Minimize window directly */
                    ShowWindow(hwnd, SW_MINIMIZE);
                    DEBUG_PRINTF("Window minimized\n");
                    
                    /* Restore button color */
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAF7F00), 0);
                }
#endif
            } else {
                /* Mouse released outside button - restore original color */
                lv_obj_set_style_bg_color(btn, lv_color_hex(0xAF7F00), 0);
                DEBUG_PRINTF("Minimize button released outside bounds - cancelled\n");
            }
        }
        min_btn_pressed = false;
    }
    else if (code == LV_EVENT_PRESS_LOST) {
        /* Button press was lost - restore original color */
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xAF7F00), 0);
        min_btn_pressed = false;
        DEBUG_PRINTF("Minimize button press lost - color restored\n");
    }
}