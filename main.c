#define USE_OPENGL 1

#include "lvgl/lvgl.h"
#include "ui/custom_keys.h"
#include "ui/maincontainer.h"
#include "ui/setting.h"
#include "ui/titlebar.h"
#include "platform/windows/win32_platform.h"
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
/* OpenGL / windowing globals - now managed by platform module */
#define g_hwnd (*win32_get_hwnd_ptr())
#define g_hdc (*win32_get_hdc_ptr())
#define g_hglrc (*win32_get_hglrc_ptr())
#define g_tex (*win32_get_texture_ptr())
#define g_framebuf (win32_get_framebuffer_ptr())
#define g_width (*win32_get_width_ptr())
#define g_height (*win32_get_height_ptr())
#define g_win_w (*win32_get_win_w_ptr())
#define g_win_h (*win32_get_win_h_ptr())
#define g_ui_scale (*win32_get_ui_scale_ptr())
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

static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);
static void keyboard_read(lv_indev_t * indev, lv_indev_data_t * data);
static void keyboard_queue_push(uint32_t key, lv_indev_state_t state);
static bool keyboard_queue_pop(keyboard_event_t* event);
static void simulate_mouse_click(int x, int y);

/* Platform callbacks */
static void on_mouse_event(int x, int y, bool pressed);
static void on_keyboard_event(uint32_t key, uint32_t state);
static void on_resize_event(int width, int height);

/* Clipboard functions */
/* Long press acceleration functions */
static uint32_t get_repeat_interval(uint32_t key, uint32_t repeat_count, DWORD press_duration);
static void reset_long_press_state(void);
/* WndProc and window functions now in platform module */

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
    bool running = true;
    while (running) {
        /* Process Windows messages through platform module */
        if (!win32_process_messages()) {
            running = false;
            break;
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
    win32_cleanup_window();
    win32_cleanup_gdiplus();
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

    /* Initialize GDI+ */
    win32_init_gdiplus();

    /* Initialize window and OpenGL through platform module */
    if (!win32_init_window(g_width, g_height, g_ui_scale)) {
        printf("ERROR: Failed to initialize OpenGL window. Falling back to console output.\n");
    } else {
        /* Register callbacks */
        win32_set_mouse_callback(on_mouse_event);
        win32_set_keyboard_callback(on_keyboard_event);
        win32_set_resize_callback(on_resize_event);
        win32_set_display_for_resize(display);
        
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
    LV_UNUSED(title_bar);

    /* Create primary content container via dedicated module */
    lv_obj_t * content = maincontainer_create(screen, g_width, g_height, title_h);
    LV_UNUSED(content);

    /* Attach keyboard group provided by the main container to the keyboard indev */
    lv_group_t * keyboard_group = maincontainer_get_keyboard_group();
    if (keyboard_group) {
        lv_indev_set_group(indev_keyboard, keyboard_group);
    }

    /* Force screen refresh */
    lv_obj_invalidate(screen);
    lv_refr_now(display);
}

static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    /* Copy the current mouse data */
    data->point.x = mouse_data.point.x;
    data->point.y = mouse_data.point.y;
    data->state = mouse_data.state;
}

/* Platform callback implementations */
static void on_mouse_event(int x, int y, bool pressed)
{
    mouse_data.point.x = x;
    mouse_data.point.y = y;
    mouse_data.state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void on_keyboard_event(uint32_t key, uint32_t state)
{
    lv_indev_state_t indev_state = (state == 1) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    keyboard_queue_push(key, indev_state);
    printf("Keyboard callback: key=0x%X, state=%d\n", key, state);
}

static void on_resize_event(int width, int height)
{
    if (display) {
        extern lv_obj_t * lv_screen_active(void);
        lv_obj_invalidate(lv_screen_active());
        lv_refr_now(display);
    }
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
/* WndProc, init_opengl_window, and cleanup_opengl moved to platform module */
#endif

/* GDI+ managed by platform module now */

/* GDI+ managed by platform module now */

