#define USE_OPENGL 1

#include "lvgl/lvgl.h"
#include "ui/setting.h"
#include "ui/titlebar.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if USE_OPENGL
#include <GL/gl.h>
#endif
#include <gdiplus.h>
using namespace Gdiplus;

/* Custom key codes for special combinations */
#define LV_KEY_CTRL_A 0x1001  /* Custom key code for Ctrl+A */
#define LV_KEY_CTRL_C 0x1002  /* Custom key code for Ctrl+C */
#define LV_KEY_CTRL_V 0x1003  /* Custom key code for Ctrl+V */

/* Define macros for extracting coordinates from LPARAM */
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif
#ifndef GL_SAMPLES
#define GL_SAMPLES 0x80A9
#endif

/***********************
 *   STATIC VARIABLES
 ***********************/
static lv_display_t * display;
static lv_indev_t * indev_mouse;
static lv_indev_t * indev_keyboard;
static lv_group_t * g_keyboard_group = NULL;  /* Global keyboard group for clipboard operations */
static uint16_t buf1[800 * 600 / 10];
static uint16_t buf2[800 * 600 / 10];
static lv_indev_data_t mouse_data;

/* Enhanced keyboard input system */
#define KEYBOARD_QUEUE_SIZE 16
typedef struct {
    uint32_t key;
    lv_indev_state_t state;
    DWORD timestamp;
} keyboard_event_t;

static keyboard_event_t keyboard_queue[KEYBOARD_QUEUE_SIZE];
static int keyboard_queue_head = 0;
static int keyboard_queue_tail = 0;
static keyboard_event_t current_keyboard_data;
static DWORD last_key_time = 0;

/* Long press acceleration variables */
static uint32_t long_press_key = 0;        /* Currently pressed key */
static DWORD long_press_start_time = 0;    /* When the key was first pressed */
static DWORD long_press_last_repeat = 0;   /* Last time the key was repeated */
static uint32_t long_press_repeat_count = 0; /* How many times the key has repeated */

#if USE_OPENGL
/* OpenGL / windowing globals */
static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_hglrc = NULL;
static GLuint g_tex = 0;
static uint32_t g_framebuf[800 * 600]; /* RGBA8888 framebuffer for GL texture */
static int g_width = 800;
static int g_height = 600;
/* current window client size (may differ when window is resized/maximized) */
static int g_win_w = 800;
static int g_win_h = 600;
/* UI scale factor (based on system DPI). Default 1.0. */
static float g_ui_scale = 1.0f;
/* Titlebar dragging state */
static bool g_title_dragging = false;
static int g_drag_start_x = 0;
static int g_drag_start_y = 0;
static int g_win_start_x = 0;
static int g_win_start_y = 0;
#endif

/***********************
 *   STATIC FUNCTIONS
 ***********************/
static void hal_init(void);
static void ui_init(void);
static void btn_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void dropdown_event_cb(lv_event_t * e);
static void textarea_event_cb(lv_event_t * e);
static void textarea_custom_handler(lv_obj_t * textarea, lv_event_code_t code);
static void textarea2_custom_handler(lv_obj_t * textarea, lv_event_code_t code);

static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);
static void keyboard_read(lv_indev_t * indev, lv_indev_data_t * data);
static void keyboard_queue_push(uint32_t key, lv_indev_state_t state);
static bool keyboard_queue_pop(keyboard_event_t* event);
static void simulate_mouse_click(int x, int y);
/* Clipboard functions */
static bool copy_text_to_clipboard(const char* text);
static char* paste_text_from_clipboard(void);
/* Long press acceleration functions */
static uint32_t get_repeat_interval(uint32_t key, uint32_t repeat_count, DWORD press_duration);
static void reset_long_press_state(void);
#if USE_OPENGL
static bool init_opengl_window(void);
static void cleanup_opengl(void);
/* Expose g_hwnd to other modules */
HWND get_main_window_handle(void)
{
    return g_hwnd;
}
#endif

static void display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
#if USE_OPENGL
    /* Convert LVGL's px_map (RGB565) into RGBA8888 in g_framebuf and update GL texture */
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint16_t c = ((uint16_t*)px_map)[y * w + x];
            uint8_t r = (uint8_t)(((c >> 11) & 0x1F) << 3);
            uint8_t g = (uint8_t)(((c >> 5) & 0x3F) << 2);
            uint8_t b = (uint8_t)((c & 0x1F) << 3);
            int dst_x = area->x1 + x;
            int dst_y = area->y1 + y;
            g_framebuf[dst_y * g_width + dst_x] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
        }
    }

    /* Update GL texture subimage. Some drivers / old OpenGL implementations
     * may not support GL_UNPACK_ROW_LENGTH reliably. To be robust we copy
     * the subregion into a contiguous temporary buffer and upload that.
     */
    if (g_hglrc && g_tex) {
        wglMakeCurrent(g_hdc, g_hglrc);
        glBindTexture(GL_TEXTURE_2D, g_tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        /* If the update region already matches the framebuffer row stride
         * we can upload directly, otherwise copy into a temporary buffer. */
        if (w == g_width) {
            /* contiguous in memory, upload directly */
            glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &g_framebuf[area->y1 * g_width + area->x1]);
        } else {
            /* allocate a contiguous temporary buffer for the subregion */
            size_t row_bytes = (size_t)w * 4;
            size_t total = row_bytes * (size_t)h;
            uint8_t * tmp = (uint8_t*)malloc(total);
            if (tmp) {
                for (int yy = 0; yy < h; ++yy) {
                    uint32_t * src = &g_framebuf[(area->y1 + yy) * g_width + area->x1];
                    uint8_t * dst = tmp + (size_t)yy * row_bytes;
                    /* copy row as 4 bytes per pixel (RGBA8888) */
                    memcpy(dst, src, row_bytes);
                }
                glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
                free(tmp);
            } else {
                /* Fallback: upload row-by-row (slower) */
                for (int yy = 0; yy < h; ++yy) {
                    uint32_t * src = &g_framebuf[(area->y1 + yy) * g_width + area->x1];
                    glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1 + yy, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, src);
                }
            }
        }
    /* draw to window - use current client size so the texture is scaled to window */
        glViewport(0, 0, g_win_w, g_win_h);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glTexCoord2f(1, 0); glVertex2f(1, 1);
        glTexCoord2f(1, 1); glVertex2f(1, -1);
        glTexCoord2f(0, 1); glVertex2f(-1, -1);
        glEnd();
        SwapBuffers(g_hdc);
    }
#else
    printf("UI Rendered: (%d,%d) to (%d,%d)\n", area->x1, area->y1, area->x2, area->y2);
#endif

    /* Tell LVGL flush is complete */
    lv_display_flush_ready(disp);
}

int main(void)
{
    /*Initialize LVGL*/
    lv_init();
    printf("✓ LVGL initialized\n");

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    /*Create a simple UI*/
    ui_init();
    
    /* Force initial refresh */
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(display);
    printf("✓ Initial UI refresh completed\n");

    printf("\n*** UI CREATED WITH MOUSE INPUT SIMULATION ***\n");
    printf("Watch the console for UI rendering activity!\n");
    printf("Commands:\n");
    printf("  '1' - Simulate click on button (center: 400,300)\n");
    printf("  '2' - Simulate click on red rectangle (200,150)\n");
    printf("  'q' - Quit\n\n");

    /* Run loop */
#if USE_OPENGL
    printf("Entering Windows message loop (OpenGL). Close window to exit.\n");
    MSG msg;
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        lv_timer_handler();
        
        /* Skip sleep for better responsiveness during delete operations */
        static DWORD last_activity = 0;
        bool active_deleting = (GetTickCount() - last_activity < 100); /* Consider active if within 100ms */
        if (!active_deleting) {
            Sleep(1); /* Only sleep when not actively typing/deleting */
        }
        
        /* Update activity time if there are keyboard events */
        if (keyboard_queue_head != keyboard_queue_tail) {
            last_activity = GetTickCount();
        }
    }
    /* cleanup */
    cleanup_opengl();
#else
    /*Handle LVGL tasks with simulated input*/
    while(1) {
        /* Handle LVGL tasks */
        lv_timer_handler();
        
        /* Check for user input commands */
        if (_kbhit()) {
            char c = getchar();
            switch(c) {
                case '1':
                    printf(">>> Simulating click on button...\n");
                    simulate_mouse_click(400, 300);
                    break;
                case '2':
                    printf(">>> Simulating click on red rectangle...\n");
                    simulate_mouse_click(200, 150);
                    break;
                case 'q':
                case 'Q':
                    printf("Exiting...\n");
                    return 0;
                default:
                    break;
            }
        }
        
        Sleep(5);
    }
#endif

    return 0;
}

static void hal_init(void)
{
    /* Allocate console for output */
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    printf("=== LVGL OpenGL/Console Demo ===\n");

#if USE_OPENGL
    /* Determine UI scale from system DPI so we can enlarge the window for high-DPI displays.
     * We keep LVGL's internal framebuffer at 800x600 but scale the window so the content
     * is rendered larger on high-DPI screens.
     */
    {
        HDC scr = GetDC(NULL);
        int dpi = GetDeviceCaps(scr, LOGPIXELSX);
        ReleaseDC(NULL, scr);
        g_ui_scale = 1.0f; /* Force 1.0 scale for testing */
        printf("System DPI=%d, UI scale=%.2f (forced to 1.0 for testing)\n", dpi, g_ui_scale);
    }

    /* Initialize OpenGL window and texture */
    if (!init_opengl_window()) {
        printf("ERROR: Failed to initialize OpenGL window. Falling back to console output.\n");
    } else {
        /* create GL texture */
        wglMakeCurrent(g_hdc, g_hglrc);
        glGenTextures(1, &g_tex);
        glBindTexture(GL_TEXTURE_2D, g_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_width, g_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_framebuf);
    }
#endif

    /* Create display */
    display = lv_display_create(800, 600);
    if (!display) {
        printf("ERROR: Failed to create display\n");
        return;
    }
    
    /* Set display buffers */
    lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, display_flush_cb);
    
    /* Initialize mouse data */
    mouse_data.point.x = 0;
    mouse_data.point.y = 0;
    mouse_data.state = LV_INDEV_STATE_RELEASED;
    
    /* Initialize keyboard data and queue */
    memset(keyboard_queue, 0, sizeof(keyboard_queue));
    keyboard_queue_head = 0;
    keyboard_queue_tail = 0;
    current_keyboard_data.key = 0;
    current_keyboard_data.state = LV_INDEV_STATE_RELEASED;
    current_keyboard_data.timestamp = 0;
    last_key_time = 0;
    
    /* Create mouse input device */
    indev_mouse = lv_indev_create();
    lv_indev_set_type(indev_mouse, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_mouse, mouse_read);
    lv_indev_set_display(indev_mouse, display);
    
    /* Create keyboard input device */
    indev_keyboard = lv_indev_create();
    lv_indev_set_type(indev_keyboard, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keyboard, keyboard_read);
    lv_indev_set_display(indev_keyboard, display);
    
    printf("✓ Display and input devices initialized\n");
}

static void ui_init(void)
{
    printf("Creating UI elements...\n");

    /* Get the active screen and ensure it's visible */
    lv_obj_t * screen = lv_screen_active();
    printf("✓ Active screen obtained: %p\n", screen);

    /* Set a light gray background to make elements visible */
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(screen, 6, 0);
    /* Disable any scrolling on the root screen to prevent the whole UI
     * (including title bar) from being panned/dragged out of view.
     */
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    /* Titlebar: create a thin bar at the top implemented with LVGL */
    const int title_h = TITLEBAR_HEIGHT; /* Use constant from titlebar.h */
    lv_obj_t * title_bar = titlebar_create(screen, g_width);

    /* Content container sits below the title bar and contains the app UI */
    lv_obj_t * content = lv_obj_create(screen);
    lv_obj_set_size(content, g_width+3, g_height - title_h+4);
    lv_obj_set_pos(content, 0-2, title_h-4);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);

    /* Rounded corners for content area for a modern look */
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_clip_corner(content, true, 0); /* clip children to rounded corners */
    
    /* Disable shadow for content container */
    lv_obj_set_style_shadow_width(content, 0, 0);
    lv_obj_set_style_shadow_opa(content, LV_OPA_TRANSP, 0);

    /* Disable scrolling on the main content so users cannot swipe left/right */
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);

    /* Create a simple button inside content (top-left area) */
    lv_obj_t * btn = lv_btn_create(content);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), 0);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(btn, 12, 0);

    /* Slider in top-right area (in content) */
    lv_obj_t * slider = lv_slider_create(content);
    lv_obj_set_size(slider, 200, 25);
    lv_obj_align(slider, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Dropdown in center area */
    lv_obj_t * dropdown = lv_dropdown_create(content);
    lv_dropdown_set_options(dropdown, "Option 1\nOption 2\nOption 3\nOption 4\nOption 5");
    lv_obj_set_size(dropdown, 180, 40);
    lv_obj_align(dropdown, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_event_cb(dropdown, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    /* Style the dropdown */
    lv_obj_set_style_radius(dropdown, 8, 0);
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(dropdown, 1, 0);

    /* First text area (username input) - bottom left area */
    lv_obj_t * textarea = lv_textarea_create(content);
    lv_obj_set_size(textarea, 280, 45); /* Wider for better UX */
    lv_obj_align(textarea, LV_ALIGN_BOTTOM_LEFT, 20, -80); /* Bottom area with margin */
    lv_textarea_set_placeholder_text(textarea, "Username...");
    lv_textarea_set_text(textarea, "");
    /* 设置用户数据标识第一个输入框 */
    lv_obj_set_user_data(textarea, (void*)1);
    lv_obj_add_event_cb(textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
    
    /* Style the first textarea */
    lv_obj_set_style_radius(textarea, 8, 0);
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(textarea, 2, 0);
    lv_obj_set_style_pad_all(textarea, 8, 0);
    
    /* Focus border color */
    lv_obj_set_style_border_color(textarea, lv_color_hex(0x2196F3), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(textarea, 2, LV_STATE_FOCUSED);
    
    /* Create a group for keyboard navigation and assign textarea to it */
    g_keyboard_group = lv_group_create();
    lv_group_add_obj(g_keyboard_group, textarea);
    lv_indev_set_group(indev_keyboard, g_keyboard_group);
    
    /* Set textarea as focused by default so it can receive keyboard input */
    lv_obj_add_state(textarea, LV_STATE_FOCUSED);

    /* Second text area (password input) - below first textarea */
    lv_obj_t * textarea2 = lv_textarea_create(content);
    lv_obj_set_size(textarea2, 280, 45); /* Same width as first textarea */
    lv_obj_align_to(textarea2, textarea, LV_ALIGN_OUT_BOTTOM_MID, 0, 10); /* Below first textarea */
    lv_textarea_set_placeholder_text(textarea2, "Password...");
    lv_textarea_set_text(textarea2, "");
    lv_textarea_set_password_mode(textarea2, true); /* Enable password mode */
    /* 设置用户数据标识第二个输入框 */
    lv_obj_set_user_data(textarea2, (void*)2);
    lv_obj_add_event_cb(textarea2, textarea_event_cb, LV_EVENT_ALL, NULL);
    
    /* Style the second textarea */
    lv_obj_set_style_radius(textarea2, 8, 0);
    lv_obj_set_style_bg_color(textarea2, lv_color_hex(0xFFF8E1), 0); /* Light yellow background */
    lv_obj_set_style_border_color(textarea2, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(textarea2, 2, 0);
    lv_obj_set_style_pad_all(textarea2, 8, 0);
    
    /* Focus border color for second textarea */
    lv_obj_set_style_border_color(textarea2, lv_color_hex(0xFF9800), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(textarea2, 2, LV_STATE_FOCUSED);
    
    /* Add second textarea to keyboard group */
    lv_group_add_obj(g_keyboard_group, textarea2);

    /* Status label in bottom-right corner */
    lv_obj_t * status = lv_label_create(content);
    lv_label_set_text(status, "UI Layout: Modern arrangement\nKeyboard navigation enabled");
    lv_obj_align(status, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_text_color(status, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_RIGHT, 0);

    /* Initialize settings system */
    settings_init();
    
    /* Create settings button in top-right corner */
    lv_obj_t * settings_btn = settings_create_button(content);
    
    printf("Settings button added to UI\n");

    /* Force screen refresh */
    lv_obj_invalidate(screen);
    lv_refr_now(display);
}

static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    
    static bool clicked = false;
    clicked = !clicked;
    
    if(clicked) {
        lv_label_set_text(label, "Clicked!");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), 0);
        printf("*** BUTTON CLICKED - UI INTERACTION WORKS! ***\n");
    } else {
        lv_label_set_text(label, "Click Me!");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), 0);
        printf("*** BUTTON RESET ***\n");
    }
    /* Force a redraw of the button to avoid visual artifacts */
    lv_obj_invalidate(btn);
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = (lv_obj_t*)lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    /* Keep slider visuals consistent: invalidate the slider and parent screen */
    lv_obj_invalidate(slider);
    lv_obj_invalidate(lv_obj_get_parent(slider));
    printf("Slider changed: %d\n", (int)v);
}

static void dropdown_event_cb(lv_event_t * e)
{
    lv_obj_t * dropdown = (lv_obj_t*)lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    /* Get the selected option text */
    char option_text[32];
    lv_dropdown_get_selected_str(dropdown, option_text, sizeof(option_text));
    
    printf("Dropdown selection changed: Index=%d, Text='%s'\n", selected, option_text);
    
    /* Force a redraw of the dropdown */
    lv_obj_invalidate(dropdown);
}

/* 通用输入框事件回调函数 */
static void textarea_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * textarea = (lv_obj_t*)lv_event_get_target(e);
    
    /* 通用处理逻辑 */
    if (code == LV_EVENT_KEY) {
        /* 处理键盘事件 */
        uint32_t key = lv_indev_get_key(lv_indev_get_act());
        if (key == LV_KEY_CTRL_A) {
            /* Ctrl+A 全选功能 - 通用逻辑 */
            printf("Executing select all in textarea\n");
            const char * text = lv_textarea_get_text(textarea);
            if (text && strlen(text) > 0) {
                /* LVGL textarea select all implementation */
                lv_textarea_set_cursor_pos(textarea, 0);
                uint32_t text_len = strlen(text);
                lv_textarea_set_cursor_pos(textarea, text_len);
                printf("Selected all text in textarea: '%s' (length: %d)\n", text, text_len);
            }
            return; /* 处理完毕，不需要调用自定义处理函数 */
        } else if (key == LV_KEY_ESC) {
            /* ESC键处理 */
            printf("ESC key pressed\n");
            return;
        }
    }
    
    /* 根据textarea对象确定调用哪个自定义处理函数 */
    /* 通过用户数据区分不同的textarea */
    void * user_data = lv_obj_get_user_data(textarea);
    if (user_data == (void*)1) {
        /* 第一个textarea (username) */
        textarea_custom_handler(textarea, code);
    } else if (user_data == (void*)2) {
        /* 第二个textarea (password) */
        textarea2_custom_handler(textarea, code);
    } else {
        /* 默认处理第一个textarea */
        textarea_custom_handler(textarea, code);
    }
}

/* 第一个输入框（用户名）的自定义处理函数 */
static void textarea_custom_handler(lv_obj_t * textarea, lv_event_code_t code)
{
    if (code == LV_EVENT_CLICKED) {
        printf("Textarea clicked - ready for input\n");
    }
    else if (code == LV_EVENT_FOCUSED) {
        printf("Textarea focused\n");
        /* Change border color when focused */
        lv_obj_set_style_border_color(textarea, lv_color_hex(0x2196F3), 0);
    }
    else if (code == LV_EVENT_DEFOCUSED) {
        printf("Textarea defocused\n");
        /* Restore normal border color when unfocused */
        lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
    }
    else if (code == LV_EVENT_VALUE_CHANGED) {
        /* Get current text content */
        const char * text = lv_textarea_get_text(textarea);
        printf("Textarea content changed: '%s'\n", text);
        
        /* Optional: Limit text length */
        if (strlen(text) > 100) {
            printf("Text too long, truncating...\n");
            char truncated[101];
            strncpy(truncated, text, 100);
            truncated[100] = '\0';
            lv_textarea_set_text(textarea, truncated);
        }
    }
    else if (code == LV_EVENT_READY) {
        /* This event is triggered when Enter is pressed (in one-line mode) */
        const char * text = lv_textarea_get_text(textarea);
        printf("Textarea ready (Enter pressed): '%s'\n", text);
    }
}

/* 第二个输入框（密码）的自定义处理函数 */
static void textarea2_custom_handler(lv_obj_t * textarea, lv_event_code_t code)
{
    if (code == LV_EVENT_CLICKED) {
        printf("Password field clicked - ready for input\n");
    }
    else if (code == LV_EVENT_FOCUSED) {
        printf("Password field focused\n");
        /* Change border color when focused */
        lv_obj_set_style_border_color(textarea, lv_color_hex(0xFF9800), 0);
        lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFF3E0), 0); /* Slightly darker when focused */
    }
    else if (code == LV_EVENT_DEFOCUSED) {
        printf("Password field defocused\n");
        /* Restore normal border color when unfocused */
        lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFF8E1), 0); /* Back to light yellow */
    }
    else if (code == LV_EVENT_VALUE_CHANGED) {
        /* Get current text content (even though it's masked) */
        const char * text = lv_textarea_get_text(textarea);
        printf("Password field content changed (length: %d)\n", (int)strlen(text));
        
        /* Optional: Limit password length */
        if (strlen(text) > 20) {
            printf("Password too long, truncating to 20 characters...\n");
            char truncated[21];
            strncpy(truncated, text, 20);
            truncated[20] = '\0';
            lv_textarea_set_text(textarea, truncated);
        }
    }
    else if (code == LV_EVENT_READY) {
        /* This event is triggered when Enter is pressed */
        const char * text = lv_textarea_get_text(textarea);
        printf("Password field ready (Enter pressed, length: %d)\n", (int)strlen(text));
        
        /* Simulate form submission */
        printf("=== Form Submission Simulation ===\n");
        printf("Password: [HIDDEN] (length: %d)\n", (int)strlen(text));
        printf("=== End Submission ===\n");
    }
}



static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    /* Copy the current mouse data */
    data->point.x = mouse_data.point.x;
    data->point.y = mouse_data.point.y;
    data->state = mouse_data.state;
}

static void keyboard_queue_push(uint32_t key, lv_indev_state_t state)
{
    DWORD current_time = GetTickCount();
    
    /* Calculate next position */
    int next_head = (keyboard_queue_head + 1) % KEYBOARD_QUEUE_SIZE;
    
    /* Check if queue is full */
    if (next_head == keyboard_queue_tail) {
        /* Queue is full, skip oldest event */
        keyboard_queue_tail = (keyboard_queue_tail + 1) % KEYBOARD_QUEUE_SIZE;
        printf("Keyboard queue overflow, dropping oldest event\n");
    }
    
    /* Add new event */
    keyboard_queue[keyboard_queue_head].key = key;
    keyboard_queue[keyboard_queue_head].state = state;
    keyboard_queue[keyboard_queue_head].timestamp = current_time;
    
    keyboard_queue_head = next_head;
    
    printf("Keyboard event queued: key=0x%X, state=%d, queue_size=%d\n", 
           key, state, (keyboard_queue_head - keyboard_queue_tail + KEYBOARD_QUEUE_SIZE) % KEYBOARD_QUEUE_SIZE);
}

static bool keyboard_queue_pop(keyboard_event_t* event)
{
    /* Check if queue is empty */
    if (keyboard_queue_head == keyboard_queue_tail) {
        return false;
    }
    
    /* Get event from tail */
    *event = keyboard_queue[keyboard_queue_tail];
    keyboard_queue_tail = (keyboard_queue_tail + 1) % KEYBOARD_QUEUE_SIZE;
    
    return true;
}

/* Clipboard functions */
static bool copy_text_to_clipboard(const char* text)
{
    if (!text || strlen(text) == 0) {
        return false;
    }
    
    /* Open clipboard */
    if (!OpenClipboard(g_hwnd)) {
        printf("Failed to open clipboard for copy\n");
        return false;
    }
    
    /* Empty clipboard */
    if (!EmptyClipboard()) {
        printf("Failed to empty clipboard\n");
        CloseClipboard();
        return false;
    }
    
    /* Allocate global memory for text */
    size_t text_len = strlen(text) + 1;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text_len);
    if (!hGlobal) {
        printf("Failed to allocate memory for clipboard\n");
        CloseClipboard();
        return false;
    }
    
    /* Lock memory and copy text */
    char* pGlobal = (char*)GlobalLock(hGlobal);
    if (!pGlobal) {
        printf("Failed to lock clipboard memory\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    
    strcpy(pGlobal, text);
    GlobalUnlock(hGlobal);
    
    /* Set clipboard data */
    if (!SetClipboardData(CF_TEXT, hGlobal)) {
        printf("Failed to set clipboard data\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    
    /* Close clipboard */
    CloseClipboard();
    
    printf("Text copied to clipboard: '%s'\n", text);
    return true;
}

static char* paste_text_from_clipboard(void)
{
    /* Open clipboard */
    if (!OpenClipboard(g_hwnd)) {
        printf("Failed to open clipboard for paste\n");
        return NULL;
    }
    
    /* Get clipboard data */
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        printf("No text data in clipboard\n");
        CloseClipboard();
        return NULL;
    }
    
    /* Lock memory and get text */
    char* pData = (char*)GlobalLock(hData);
    if (!pData) {
        printf("Failed to lock clipboard data\n");
        CloseClipboard();
        return NULL;
    }
    
    /* Allocate memory for return value */
    size_t data_len = strlen(pData) + 1;
    char* result = (char*)malloc(data_len);
    if (!result) {
        printf("Failed to allocate memory for pasted text\n");
        GlobalUnlock(hData);
        CloseClipboard();
        return NULL;
    }
    
    /* Copy text */
    strcpy(result, pData);
    
    /* Unlock and close */
    GlobalUnlock(hData);
    CloseClipboard();
    
    printf("Text pasted from clipboard: '%s'\n", result);
    return result;
}

/* Calculate dynamic repeat interval for long press acceleration */
static uint32_t get_repeat_interval(uint32_t key, uint32_t repeat_count, DWORD press_duration)
{
    /* Base intervals in milliseconds */
    uint32_t initial_delay = 500;      /* Initial delay before repeating starts */
    uint32_t base_interval = 80;       /* Base repeat interval */
    uint32_t min_interval = 8;         /* Minimum interval (maximum speed) */
    
    /* Special handling for delete/backspace keys - they accelerate faster */
    bool is_delete_key = (key == LV_KEY_BACKSPACE || key == LV_KEY_DEL);
    if (is_delete_key) {
        base_interval = 40;
        min_interval = 2;
    }
    
    /* Don't start repeating until initial delay has passed */
    if (press_duration < initial_delay) {
        return initial_delay - press_duration;
    }
    
    /* Calculate acceleration based on repeat count */
    /* Formula: interval = base_interval * (0.9 ^ repeat_count) */
    /* But with a minimum limit */
    double acceleration_factor = pow(0.85, repeat_count);
    uint32_t interval = (uint32_t)(base_interval * acceleration_factor);
    
    /* Ensure we don't go below minimum interval */
    if (interval < min_interval) {
        interval = min_interval;
    }
    
    printf("Long press acceleration: key=0x%X, repeat=%d, duration=%d, interval=%d\n", 
           key, repeat_count, press_duration, interval);
    
    return interval;
}

/* Reset long press state when key is released or different key is pressed */
static void reset_long_press_state(void)
{
    if (long_press_key != 0) {
        printf("Resetting long press state for key 0x%X (had %d repeats)\n", 
               long_press_key, long_press_repeat_count);
    }
    
    long_press_key = 0;
    long_press_start_time = 0;
    long_press_last_repeat = 0;
    long_press_repeat_count = 0;
}

static void keyboard_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    static DWORD last_read_time = 0;
    DWORD current_time = GetTickCount();
    
    /* Faster processing for delete/backspace keys */
    bool is_delete_key = (current_keyboard_data.key == LV_KEY_BACKSPACE || 
                         current_keyboard_data.key == LV_KEY_DEL);
    uint32_t read_interval = is_delete_key ? 2 : 15; /* 2ms for delete keys, 15ms for others */
    
    /* Process next key from queue if current key is released or enough time passed */
    if (current_keyboard_data.state == LV_INDEV_STATE_RELEASED || 
        (current_time - last_read_time) > read_interval) {
        
        keyboard_event_t next_event;
        if (keyboard_queue_pop(&next_event)) {
            current_keyboard_data = next_event;
            last_read_time = current_time;
            printf("Processing keyboard event: key=0x%X ('%c'), state=%d\n", 
                   next_event.key, 
                   (next_event.key >= 32 && next_event.key <= 126) ? (char)next_event.key : '?',
                   next_event.state);
        }
    }
    
    /* Copy current keyboard data to LVGL */
    data->key = current_keyboard_data.key;
    data->state = current_keyboard_data.state;
    
    /* Auto-release pressed keys after a short delay */
    /* Faster release for delete keys to enable rapid repeat */
    uint32_t release_time = is_delete_key ? 2 : 10;
    if (current_keyboard_data.state == LV_INDEV_STATE_PRESSED && 
        (current_time - current_keyboard_data.timestamp) > release_time) {
        current_keyboard_data.state = LV_INDEV_STATE_RELEASED;
    }
}

static void simulate_mouse_click(int x, int y)
{
    /* Set mouse position and press */
    mouse_data.point.x = x;
    mouse_data.point.y = y;
    mouse_data.state = LV_INDEV_STATE_PRESSED;
    
    /* Process LVGL tasks to handle press */
    lv_timer_handler();
    Sleep(50);  /* Short delay */
    
    /* Release mouse */
    mouse_data.state = LV_INDEV_STATE_RELEASED;
    
    /* Process LVGL tasks to handle release */
    lv_timer_handler();
    
    printf("Mouse click simulated at (%d, %d)\n", x, y);
}

#if USE_OPENGL
/* Simple Win32 window + OpenGL initialization */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_NCHITTEST: {
            /* Allow dragging the window by clicking anywhere in the title bar area */
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(hwnd, &pt);
            
            /* Check if cursor is in title bar area (top 36 pixels) */
            if (pt.y >= 0 && pt.y <= 36) {
                /* Check if it's not over the close button area (right 70 pixels to accommodate 48px button + 22px offset) */
                RECT rc;
                GetClientRect(hwnd, &rc);
                if (pt.x < rc.right - 180) {
                    return HTCAPTION; /* This enables native window dragging */
                }
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            int new_w = (int)(short)LOWORD(lParam);
            int new_h = (int)(short)HIWORD(lParam);
            g_win_w = new_w > 0 ? new_w : 0;
            g_win_h = new_h > 0 ? new_h : 0;
            if (g_hglrc && g_hdc) {
                wglMakeCurrent(g_hdc, g_hglrc);
                glViewport(0, 0, g_win_w, g_win_h);
                /* Force a redraw of LVGL screen to repaint at new size */
                if (display) {
                    lv_obj_invalidate(lv_screen_active());
                    lv_refr_now(display);
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            mouse_data.point.x = x;
            mouse_data.point.y = y;
            mouse_data.state = LV_INDEV_STATE_PRESSED;
            return 0;
        }
        case WM_LBUTTONUP: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            mouse_data.point.x = x;
            mouse_data.point.y = y;
            mouse_data.state = LV_INDEV_STATE_RELEASED;
            return 0;
        }
        case WM_MOUSEMOVE: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            mouse_data.point.x = x;
            mouse_data.point.y = y;
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            uint32_t key = (uint32_t)wParam;
            DWORD current_time = GetTickCount();
            
            /* Long press acceleration logic */
            bool should_process = false;
            
            if (long_press_key == key) {
                /* Same key being held down - check if we should repeat */
                DWORD press_duration = current_time - long_press_start_time;
                DWORD since_last_repeat = current_time - long_press_last_repeat;
                
                uint32_t required_interval = get_repeat_interval(key, long_press_repeat_count, press_duration);
                
                if (since_last_repeat >= required_interval) {
                    should_process = true;
                    long_press_last_repeat = current_time;
                    long_press_repeat_count++;
                }
            } else {
                /* New key or first press of this key */
                reset_long_press_state();
                long_press_key = key;
                long_press_start_time = current_time;
                long_press_last_repeat = current_time;
                long_press_repeat_count = 0;
                should_process = true;
            }
            
            if (!should_process) {
                return 0; /* Skip this key event */
            }
            
            printf("Key pressed: 0x%X (%d) [repeat=%d, duration=%dms]\n", 
                   key, key, long_press_repeat_count, current_time - long_press_start_time);
            
            /* Check NumLock state for numpad handling */
            bool numlock_on = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
            
            /* Check for Ctrl key combinations */
            bool ctrl_pressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            
            /* Convert Windows virtual key codes to LVGL key codes */
            uint32_t lv_key = 0;
            
            /* Handle Ctrl+A for select all */
            if (ctrl_pressed && key == 'A') {
                printf("Ctrl+A detected - Select All\n");
                lv_key = LV_KEY_CTRL_A; /* Use our custom key code for Ctrl+A */
                /* We'll handle the actual select all in the textarea event handler */
            }
            /* Handle Ctrl+C for copy */
            else if (ctrl_pressed && key == 'C') {
                printf("Ctrl+C detected - Copy\n");
                /* Get the currently focused textarea */
                lv_obj_t * active_ta = lv_group_get_focused(g_keyboard_group);
                if (active_ta && lv_obj_check_type(active_ta, &lv_textarea_class)) {
                    const char * text = lv_textarea_get_text(active_ta);
                    if (text && strlen(text) > 0) {
                        copy_text_to_clipboard(text);
                    } else {
                        printf("No text to copy\n");
                    }
                } else {
                    printf("No active textarea for copy\n");
                }
                return 0; /* Don't forward to LVGL */
            }
            /* Handle Ctrl+V for paste */
            else if (ctrl_pressed && key == 'V') {
                printf("Ctrl+V detected - Paste\n");
                /* Get the currently focused textarea */
                lv_obj_t * active_ta = lv_group_get_focused(g_keyboard_group);
                if (active_ta && lv_obj_check_type(active_ta, &lv_textarea_class)) {
                    char * pasted_text = paste_text_from_clipboard();
                    if (pasted_text) {
                        /* Replace current text with pasted text */
                        lv_textarea_set_text(active_ta, pasted_text);
                        free(pasted_text);
                    }
                } else {
                    printf("No active textarea for paste\n");
                }
                return 0; /* Don't forward to LVGL */
            } else {
            switch (key) {
                case VK_UP:       lv_key = LV_KEY_UP; break;
                case VK_DOWN:     lv_key = LV_KEY_DOWN; break;
                case VK_LEFT:     lv_key = LV_KEY_LEFT; break;
                case VK_RIGHT:    lv_key = LV_KEY_RIGHT; break;
                case VK_ESCAPE:   lv_key = LV_KEY_ESC; break;
                case VK_DELETE:   lv_key = LV_KEY_DEL; break;
                case VK_BACK:     
                    lv_key = LV_KEY_BACKSPACE; 
                    printf("Backspace key detected, mapping to LV_KEY_BACKSPACE (0x%X)\n", LV_KEY_BACKSPACE);
                    break;
                case VK_RETURN:   lv_key = LV_KEY_ENTER; break;
                case VK_TAB:      lv_key = LV_KEY_NEXT; break;
                case VK_HOME:     lv_key = LV_KEY_HOME; break;
                case VK_END:      lv_key = LV_KEY_END; break;
                
                /* Handle numpad keys when NumLock is OFF (they act as navigation keys) */
                case VK_NUMPAD0:  lv_key = numlock_on ? '0' : VK_INSERT; break;
                case VK_NUMPAD1:  lv_key = numlock_on ? '1' : VK_END; break;
                case VK_NUMPAD2:  lv_key = numlock_on ? '2' : VK_DOWN; break;
                case VK_NUMPAD3:  lv_key = numlock_on ? '3' : VK_NEXT; break;
                case VK_NUMPAD4:  lv_key = numlock_on ? '4' : VK_LEFT; break;
                case VK_NUMPAD5:  lv_key = numlock_on ? '5' : 0; break; /* No function when NumLock off */
                case VK_NUMPAD6:  lv_key = numlock_on ? '6' : VK_RIGHT; break;
                case VK_NUMPAD7:  lv_key = numlock_on ? '7' : VK_HOME; break;
                case VK_NUMPAD8:  lv_key = numlock_on ? '8' : VK_UP; break;
                case VK_NUMPAD9:  lv_key = numlock_on ? '9' : VK_PRIOR; break;
                
                /* Handle numpad operator keys - these work regardless of NumLock */
                case VK_MULTIPLY: lv_key = '*'; break; /* Numpad * */
                case VK_ADD:      lv_key = '+'; break; /* Numpad + */
                case VK_SUBTRACT: lv_key = '-'; break; /* Numpad - */
                case VK_DIVIDE:   lv_key = '/'; break; /* Numpad / */
                case VK_DECIMAL:  lv_key = '.'; break; /* Numpad . */
                
                /* Handle common punctuation keys directly */
                case VK_OEM_PERIOD: /* . > */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '>' : '.'; 
                    break;
                case VK_OEM_COMMA:  /* , < */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '<' : ','; 
                    break;
                case VK_OEM_2:      /* /? key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '?' : '/'; 
                    break;
                case VK_OEM_1:      /* ;: key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? ':' : ';'; 
                    break;
                case VK_OEM_7:      /* '" key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '"' : '\''; 
                    break;
                case VK_OEM_4:      /* [{ key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '{' : '['; 
                    break;
                case VK_OEM_6:      /* ]} key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '}' : ']'; 
                    break;
                case VK_OEM_5:      /* \| key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '|' : '\\'; 
                    break;
                case VK_OEM_3:      /* `~ key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '~' : '`'; 
                    break;
                case VK_OEM_MINUS:  /* -_ key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '_' : '-'; 
                    break;
                case VK_OEM_PLUS:   /* =+ key */
                    lv_key = (GetKeyState(VK_SHIFT) & 0x8000) ? '+' : '='; 
                    break;
                default:
                    /* Process printable characters (letters, numbers, symbols) */
                    /* Support both uppercase VK codes (0x41-0x5A) and lowercase ASCII (0x61-0x7A) */
                    bool is_letter = (key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z');
                    bool is_number = (key >= 0x30 && key <= 0x39); /* VK_0 to VK_9 */
                    
                    if (is_letter || is_number || key == VK_SPACE || 
                        (key >= VK_OEM_1 && key <= VK_OEM_3) || (key >= VK_OEM_4 && key <= VK_OEM_8) ||
                        key == VK_OEM_MINUS || key == VK_OEM_PLUS) {
                        
                        if (is_letter) {
                            /* Handle both uppercase VK codes and lowercase ASCII */
                            if (key >= 'A' && key <= 'Z') {
                                /* Standard VK_A to VK_Z codes */
                                bool shift_pressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                                lv_key = shift_pressed ? key : (key + 32); /* Convert to lowercase if no shift */
                            } else if (key >= 'a' && key <= 'z') {
                                /* Direct lowercase codes - use as is or convert to uppercase if shift */
                                bool shift_pressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                                lv_key = shift_pressed ? (key - 32) : key; /* Convert to uppercase if shift */
                            }
                        } else if (is_number) {
                            /* Main number row keys */
                            bool shift_pressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                            if (shift_pressed) {
                                /* Handle shifted number keys: )!@#$%^&*( */
                                char shifted_nums[] = ")!@#$%^&*(";
                                lv_key = shifted_nums[key - 0x30];
                            } else {
                                lv_key = key; /* Direct number key */
                            }
                        } else if (key == VK_SPACE) {
                            lv_key = ' ';
                        }
                        /* For other keys, let WM_CHAR handle them */
                    }
                    break;
            } /* end of switch */
            } /* end of else (not Ctrl+A) */
            
            if (lv_key != 0) {
                char key_name[32] = "";
                /* Add key name for common keys */
                switch (key) {
                    case VK_MULTIPLY: strcpy(key_name, "NumPad*"); break;
                    case VK_DIVIDE: strcpy(key_name, "NumPad/"); break;
                    case VK_ADD: strcpy(key_name, "NumPad+"); break;
                    case VK_SUBTRACT: strcpy(key_name, "NumPad-"); break;
                    case VK_DECIMAL: strcpy(key_name, "NumPad."); break;
                    case VK_OEM_1: strcpy(key_name, ";:"); break;
                    case VK_OEM_2: strcpy(key_name, "/?"); break;
                    case VK_OEM_3: strcpy(key_name, "`~"); break;
                    case VK_OEM_4: strcpy(key_name, "[{"); break;
                    case VK_OEM_5: strcpy(key_name, "\\|"); break;
                    case VK_OEM_6: strcpy(key_name, "]}"); break;
                    case VK_OEM_7: strcpy(key_name, "'\""); break;
                    case VK_OEM_MINUS: strcpy(key_name, "-_"); break;
                    case VK_OEM_PLUS: strcpy(key_name, "=+"); break;
                    case VK_OEM_PERIOD: strcpy(key_name, ".>"); break;
                    case VK_OEM_COMMA: strcpy(key_name, ",<"); break;
                    case VK_NUMPAD0: case VK_NUMPAD1: case VK_NUMPAD2: case VK_NUMPAD3: case VK_NUMPAD4:
                    case VK_NUMPAD5: case VK_NUMPAD6: case VK_NUMPAD7: case VK_NUMPAD8: case VK_NUMPAD9:
                        sprintf(key_name, "NumPad%d", key - VK_NUMPAD0); break;
                    default: 
                        if (key >= 'A' && key <= 'Z') sprintf(key_name, "Key_%c", (char)key);
                        else if (key >= '0' && key <= '9') sprintf(key_name, "Key_%c", (char)key);
                        break;
                }
                
                printf("WM_KEYDOWN processing: VK=0x%X (%s), lv_key=0x%X ('%c'), NumLock=%s\n", 
                       key, key_name[0] ? key_name : "Unknown", lv_key, 
                       (lv_key >= 32 && lv_key <= 126) ? (char)lv_key : '?', 
                       numlock_on ? "ON" : "OFF");
                keyboard_queue_push(lv_key, LV_INDEV_STATE_PRESSED);
            } else {
                printf("WM_KEYDOWN ignored: VK=0x%X, NumLock=%s\n", key, numlock_on ? "ON" : "OFF");
            }
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            uint32_t key = (uint32_t)wParam;
            
            /* Reset long press state when key is released */
            if (long_press_key == key) {
                reset_long_press_state();
            }
            
            /* For most keys, we don't need to track key-up events for LVGL text input */
            return 0;
        }
        case WM_CHAR: {
            /* Handle character input that wasn't handled in WM_KEYDOWN */
            uint32_t character = (uint32_t)wParam;
            DWORD current_time = GetTickCount();
            
            /* Only process characters not already handled in WM_KEYDOWN */
            /* Skip letters, numbers, space, and common punctuation handled in WM_KEYDOWN */
            bool already_handled = ((character >= 'a' && character <= 'z') || 
                                   (character >= 'A' && character <= 'Z') || 
                                   (character >= '0' && character <= '9') || 
                                   character == ' ' || character == '.' || character == ',' ||
                                   character == '/' || character == '?' || character == ';' ||
                                   character == ':' || character == '\'' || character == '"' ||
                                   character == '<' || character == '>');
            
            /* Process printable characters including extended ASCII */
            if (!already_handled && ((character >= 32 && character <= 126) || (character >= 160 && character <= 255))) {
                /* Anti-bounce for character input */
                if (current_time - last_key_time >= 2) { /* 2ms debounce for chars - very responsive */
                    printf("WM_CHAR processing character: '%c' (0x%X)\n", 
                           (character >= 32 && character <= 126) ? (char)character : '?', character);
                    keyboard_queue_push(character, LV_INDEV_STATE_PRESSED);
                    last_key_time = current_time;
                }
            } else {
                printf("WM_CHAR ignored%s: '%c' (0x%X)\n", 
                       already_handled ? " (already handled)" : " (non-printable)", 
                       (character >= 32 && character <= 126) ? (char)character : '?', character);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            Graphics graphics(hdc);
            graphics.SetSmoothingMode(SmoothingModeHighQuality);

            // 设置画刷和圆角矩形参数
            SolidBrush brush(Color(255, 0, 122, 204)); // 半透明蓝色
            Rect rect(10, 10, g_win_w - 20, g_win_h - 20);
            int cornerRadius = 60;

            // 绘制圆角矩形
            GraphicsPath path;
            path.AddArc(rect.X, rect.Y, cornerRadius, cornerRadius, 180, 90);
            path.AddArc(rect.X + rect.Width - cornerRadius, rect.Y, cornerRadius, cornerRadius, 270, 90);
            path.AddArc(rect.X + rect.Width - cornerRadius, rect.Y + rect.Height - cornerRadius, cornerRadius, cornerRadius, 0, 90);
            path.AddArc(rect.X, rect.Y + rect.Height - cornerRadius, cornerRadius, cornerRadius, 90, 90);
            path.CloseFigure();

            graphics.FillPath(&brush, &path);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static bool init_opengl_window(void)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "lvgl_opengl_window";

    if (!RegisterClass(&wc)) {
        printf("RegisterClass failed\n");
        return false;
    }

    /* Create window */
    int win_w = (int)(g_width * g_ui_scale);
    int win_h = (int)(g_height * g_ui_scale);
    int sx = GetSystemMetrics(SM_CXSCREEN);
    int sy = GetSystemMetrics(SM_CYSCREEN);
    int x = (sx - win_w) / 2;
    int y = (sy - win_h) / 2;

    DWORD style = WS_POPUP | WS_VISIBLE;
    DWORD exStyle = WS_EX_LAYERED;

    g_hwnd = CreateWindowExA(exStyle, wc.lpszClassName, NULL, style,
                         x, y, win_w, win_h,
                         NULL, NULL, hInstance, NULL);

    if (!g_hwnd) {
        printf("CreateWindow failed\n");
        return false;
    }

    /* 设置圆角和透明背景 */
    HRGN rgn = CreateRoundRectRgn(0, 0, win_w + 0, win_h + 0, 50, 50); // 圆角半径为 30
    SetWindowRgn(g_hwnd, rgn, TRUE);
    DeleteObject(rgn); // 删除区域对象以释放资源

    /* 设置透明背景 */
    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);

    g_hdc = GetDC(g_hwnd);

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;

    // 更新像素格式描述符以支持多重采样
    pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;

    int pf = ChoosePixelFormat(g_hdc, &pfd);
    if (!pf) {
        printf("ChoosePixelFormat failed\n");
        return false;
    }
    if (!SetPixelFormat(g_hdc, pf, &pfd)) {
        printf("SetPixelFormat failed\n");
        return false;
    }

    g_hglrc = wglCreateContext(g_hdc);
    if (!g_hglrc) {
        printf("wglCreateContext failed\n");
        return false;
    }

    if (!wglMakeCurrent(g_hdc, g_hglrc)) {
        printf("wglMakeCurrent failed\n");
        return false;
    }

    // 启用多重采样以减少锯齿
    glEnable(GL_MULTISAMPLE);

    // 改进纹理过滤模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 启用全局抗锯齿
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // 检查多重采样状态
    GLint samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    printf("Number of samples: %d\n", samples);

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    RECT rc;
    if (GetClientRect(g_hwnd, &rc)) {
        g_win_w = rc.right - rc.left;
        g_win_h = rc.bottom - rc.top;
    } else {
        g_win_w = g_width;
        g_win_h = g_height;
    }
    return true;
}

static void cleanup_opengl(void)
{
    if (g_hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hglrc);
        g_hglrc = NULL;
    }
    if (g_hdc && g_hwnd) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}
#endif

// 初始化 GDI+
ULONG_PTR gdiplusToken;

void init_gdiplus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void cleanup_gdiplus() {
    GdiplusShutdown(gdiplusToken);
}

