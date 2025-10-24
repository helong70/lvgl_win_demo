#define USE_OPENGL 1

#include "lvgl/lvgl.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#if USE_OPENGL
#include <GL/gl.h>
#endif

/***********************
 *   STATIC VARIABLES
 ***********************/
static lv_display_t * display;
static lv_indev_t * indev;
static uint16_t buf1[800 * 600 / 10];
static uint16_t buf2[800 * 600 / 10];
static lv_indev_data_t mouse_data;

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
static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);
static void simulate_mouse_click(int x, int y);
#if USE_OPENGL
/* Titlebar callbacks/prototypes */
static void titlebar_event_cb(lv_event_t * e);
static void close_btn_event_cb(lv_event_t * e);
#endif
#if USE_OPENGL
static bool init_opengl_window(void);
static void cleanup_opengl(void);
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
        Sleep(5);
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
        g_ui_scale = (float)dpi / 96.0f;
        if (g_ui_scale < 1.0f) g_ui_scale = 1.0f;
        if (g_ui_scale > 3.0f) g_ui_scale = 3.0f; /* clamp reasonable max */
        printf("System DPI=%d, UI scale=%.2f\n", dpi, g_ui_scale);
    }

    /* Initialize OpenGL window and texture */
    if (!init_opengl_window()) {
        printf("ERROR: Failed to initialize OpenGL window. Falling back to console output.\n");
    } else {
        /* create GL texture */
        wglMakeCurrent(g_hdc, g_hglrc);
        glGenTextures(1, &g_tex);
        glBindTexture(GL_TEXTURE_2D, g_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    
    /* Create input device */
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, mouse_read);
    lv_indev_set_display(indev, display);
    
    printf("✓ Display and input device initialized\n");
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
    const int title_h = 36; /* logical pixels in LVGL coords */
    lv_obj_t * title_bar = lv_obj_create(screen);
    lv_obj_set_size(title_bar, g_width+4, title_h);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, -2);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2E2E2E), 0);
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_COVER, 0);
    /* Disable internal scrolling for the title bar so its children cannot be
     * vertically scrolled by touch/drag gestures. We still receive events for
     * press/pressing/release to implement window dragging.
     */
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(title_bar, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(title_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(title_bar, titlebar_event_cb, LV_EVENT_ALL, NULL);

    /* Slight rounding for the title bar corners to match UI aesthetic */
    lv_obj_set_style_radius(title_bar, 6, 0);

    /* Title text */
    lv_obj_t * title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "LVGL OpenGL Demo");
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);

    /* Close button on the right of titlebar */
    lv_obj_t * close_btn = lv_btn_create(title_bar);
    lv_obj_set_size(close_btn, 28, 24);
    lv_obj_align(close_btn, LV_ALIGN_RIGHT_MID, 8, 0);
    lv_obj_add_event_cb(close_btn, close_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * cb_label = lv_label_create(close_btn);
    lv_label_set_text(cb_label, "X");
    lv_obj_center(cb_label);
    /* Round the close button for consistency */
    lv_obj_set_style_radius(close_btn, 8, 0);

    /* Content container sits below the title bar and contains the app UI */
    lv_obj_t * content = lv_obj_create(screen);
    lv_obj_set_size(content, g_width, g_height - title_h);
    lv_obj_set_pos(content, 0, title_h);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);

    /* Rounded corners for content area for a modern look */
    lv_obj_set_style_radius(content, 12, 0);
    lv_obj_set_style_clip_corner(content, true, 0); /* clip children to rounded corners */

    /* Disable scrolling on the main content so users cannot swipe left/right */
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);

    /* Create a simple button inside content (centered within content) */
    lv_obj_t * btn = lv_btn_create(content);
    lv_obj_set_size(btn, 150, 60);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), 0);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(btn, 12, 0);

    /* Slider below the button (in content) */
    lv_obj_t * slider = lv_slider_create(content);
    lv_obj_set_size(slider, 250, 30);
    lv_obj_align_to(slider, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Status label under the slider */
    lv_obj_t * status = lv_label_create(content);
    lv_label_set_text(status, "Press '1' to click button, '2' for rectangle, 'q' to quit");
    lv_obj_align_to(status, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_set_style_text_color(status, lv_color_hex(0x666666), 0);

    /* Force screen refresh */
    lv_obj_invalidate(screen);
    lv_refr_now(display);
}

static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
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
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    /* Keep slider visuals consistent: invalidate the slider and parent screen */
    lv_obj_invalidate(slider);
    lv_obj_invalidate(lv_obj_get_parent(slider));
    printf("Slider changed: %d\n", (int)v);
}

static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    /* Copy the current mouse data */
    data->point.x = mouse_data.point.x;
    data->point.y = mouse_data.point.y;
    data->state = mouse_data.state;
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

/* Titlebar callbacks -----------------------------------------------------*/
/* 添加一个普通的 C 函数用于动画完成后的回调 */
static void anim_ready_cb(lv_anim_t * a)
{
    (void)a; /* 忽略参数 */
    PostQuitMessage(0); /* 关闭应用程序 */
}

/* 修改 close_btn_event_cb */
static void close_btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);

    /* Change button color to red */
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x00000af), 0);

    /* Create an animation to fade out the button */
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, btn);
    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&anim, 50); /* 500ms animation */
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);

    /* 使用普通函数作为动画完成后的回调 */
    lv_anim_set_ready_cb(&anim, anim_ready_cb);

    lv_anim_start(&anim);
}

static void titlebar_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    /* Implement capture-based dragging: when the titlebar is pressed we
     * capture the mouse (SetCapture) and track the global cursor position
     * with GetCursorPos while LV_EVENT_PRESSING events arrive. This
     * guarantees we keep receiving mouse moves even if the cursor leaves
     * the client area. On release we call ReleaseCapture.
     */
    if (code == LV_EVENT_PRESSED) {
        if (g_hwnd) {
            POINT pt;
            RECT rc;
            /* Record global cursor position and window origin */
            if (GetCursorPos(&pt) && GetWindowRect(g_hwnd, &rc)) {
                g_title_dragging = true;
                g_drag_start_x = pt.x;
                g_drag_start_y = pt.y;
                g_win_start_x = rc.left;
                g_win_start_y = rc.top;
                /* Ensure the window receives mouse messages until release */
                SetCapture(g_hwnd);
            }
        }
    } else if (code == LV_EVENT_PRESSING) {
        if (g_title_dragging && g_hwnd) {
            POINT pt;
            if (GetCursorPos(&pt)) {
                int dx = pt.x - g_drag_start_x;
                int dy = pt.y - g_drag_start_y;
                SetWindowPos(g_hwnd, NULL, g_win_start_x + dx, g_win_start_y + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (g_title_dragging) {
            g_title_dragging = false;
            /* Release capture in case we set it earlier */
            ReleaseCapture();
        }
    }
}

#if USE_OPENGL
/* Simple Win32 window + OpenGL initialization */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
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
    g_hwnd = CreateWindowExA(0, wc.lpszClassName, NULL, style,
                             x, y, win_w, win_h,
                             NULL, NULL, hInstance, NULL);

    if (!g_hwnd) {
        printf("CreateWindow failed\n");
        return false;
    }

    /* 设置圆角 */
    HRGN rgn = CreateRoundRectRgn(0, 0, win_w+1, win_h+1, 30, 30); // 圆角半径为 20
    SetWindowRgn(g_hwnd, rgn, TRUE);

    g_hdc = GetDC(g_hwnd);

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;

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