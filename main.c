#include "lvgl/lvgl.h"
#include "hal/lv_hal.h"
#include "hal/lv_hal_indev.h"
#include "ui/custom_keys.h"
#include "ui/maincontainer.h"
#include "ui/setting.h"
#include "ui/titlebar.h"
#include "platform/windows/win32_platform.h"
#include <windows.h>
#include <stdio.h>

/* Conditional printf for console mode only */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

/* Access platform state for UI dimensions */
#define g_width (*win32_get_width_ptr())
#define g_height (*win32_get_height_ptr())

/***********************
 *   FORWARD DECLARATIONS
 ***********************/
static void ui_init(void);
static void on_mouse_event(int x, int y, bool pressed);
static void on_keyboard_event(uint32_t key, uint32_t state);
static void on_resize_event(int width, int height);

/***********************
 *   MAIN
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENABLE_CONSOLE_OUTPUT
/* GUI mode: Use WinMain entry point (no console window) */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    /* Explicitly free any console that may have been allocated */
    FreeConsole();
#else
/* Console mode: Use standard main entry point (with console window) */
int main(void)
{
#endif
    lv_display_t * display;
    lv_indev_t * indev_keyboard;
    
    /* Initialize LVGL */
    lv_init();
    DEBUG_PRINTF("✓ LVGL initialized\n");

    /* Initialize the HAL (display, input devices, platform) */
    lv_hal_init();
    display = lv_hal_get_display();
    indev_keyboard = lv_hal_get_keyboard();

    /* Register platform callbacks */
    win32_set_mouse_callback(on_mouse_event);
    win32_set_keyboard_callback(on_keyboard_event);
    win32_set_resize_callback(on_resize_event);
    win32_set_display_for_resize(display);

    /* Create UI */
    ui_init();
    
    /* Force initial refresh */
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(display);
    DEBUG_PRINTF("✓ Initial UI refresh completed\n");

    DEBUG_PRINTF("\n*** LVGL Windows Demo Started ***\n");
    DEBUG_PRINTF("Close window to exit.\n\n");

    /* Main event loop */
    DEBUG_PRINTF("Entering Windows message loop (OpenGL).\n");
    bool running = true;
    while (running) {
        /* Process Windows messages */
        if (!win32_process_messages()) {
            running = false;
            break;
        }
        
        /* Process LVGL tasks */
        lv_timer_handler();
        
        /* Minimal sleep for responsiveness */
        Sleep(1);
    }
    
    /* Cleanup */
    win32_cleanup_window();
    win32_cleanup_gdiplus();

    return 0;
}

#ifdef __cplusplus
}
#endif


static void ui_init(void)
{
    DEBUG_PRINTF("Creating UI elements...\n");

    /* Get the active screen */
    lv_obj_t * screen = lv_screen_active();
    DEBUG_PRINTF("✓ Active screen obtained: %p\n", screen);

    /* Set background */
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(screen, 6, 0);
    
    /* Disable scrolling on root screen */
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    /* Create titlebar */
    const int title_h = TITLEBAR_HEIGHT;
    lv_obj_t * title_bar = titlebar_create(screen, g_width);
    LV_UNUSED(title_bar);

    /* Create main container */
    lv_obj_t * content = maincontainer_create(screen, g_width, g_height, title_h);
    LV_UNUSED(content);

    /* Attach keyboard group to keyboard indev */
    lv_group_t * keyboard_group = maincontainer_get_keyboard_group();
    lv_indev_t * indev_keyboard = lv_hal_get_keyboard();
    if (keyboard_group && indev_keyboard) {
        lv_indev_set_group(indev_keyboard, keyboard_group);
    }

    /* Force screen refresh */
    lv_obj_invalidate(screen);
}

/* Platform callback: mouse event handler */
static void on_mouse_event(int x, int y, bool pressed)
{
    lv_hal_mouse_set_state(x, y, pressed);
}

/* Platform callback: keyboard event handler */
static void on_keyboard_event(uint32_t key, uint32_t state)
{
    lv_indev_state_t indev_state = (state == 1) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    lv_hal_keyboard_queue_push(key, indev_state);
}

/* Platform callback: resize event handler */
static void on_resize_event(int width, int height)
{
    lv_display_t * display = lv_hal_get_display();
    if (display) {
        lv_obj_invalidate(lv_screen_active());
        lv_refr_now(display);
    }
}
