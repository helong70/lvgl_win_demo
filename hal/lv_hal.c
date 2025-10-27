#include "lv_hal.h"
#include "platform/windows/win32_platform.h"
#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>

/* Conditional printf - only output in console mode */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

/* Access platform OpenGL state */
#define g_hdc (*win32_get_hdc_ptr())
#define g_hglrc (*win32_get_hglrc_ptr())
#define g_tex (*win32_get_texture_ptr())
#define g_framebuf (win32_get_framebuffer_ptr())
#define g_width (*win32_get_width_ptr())
#define g_height (*win32_get_height_ptr())
#define g_ui_scale (*win32_get_ui_scale_ptr())

void lv_hal_init(void)
{
    lv_display_t * display;
    
    /* Allocate console for output */
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    DEBUG_PRINTF("=== LVGL HAL Initialization ===\n");

    /* Determine UI scale from system DPI */
    {
        HDC scr = GetDC(NULL);
        int dpi = GetDeviceCaps(scr, LOGPIXELSX);
        ReleaseDC(NULL, scr);
        g_ui_scale = 1.0f; /* Force 1.0 scale for testing */
        DEBUG_PRINTF("System DPI=%d, UI scale=%.2f (forced to 1.0 for testing)\n", dpi, g_ui_scale);
    }

    /* Initialize GDI+ */
    win32_init_gdiplus();

    /* Initialize window and OpenGL through platform module */
    if (!win32_init_window(g_width, g_height, g_ui_scale)) {
        DEBUG_PRINTF("ERROR: Failed to initialize OpenGL window\n");
        return;
    }
    
    /* Initialize display HAL first */
    display = lv_hal_disp_init();
    if (!display) {
        DEBUG_PRINTF("ERROR: Failed to initialize display HAL\n");
        return;
    }
    
    /* Create GL texture */
    wglMakeCurrent(g_hdc, g_hglrc);
    glGenTextures(1, &g_tex);
    glBindTexture(GL_TEXTURE_2D, g_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_width, g_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_framebuf);
    
    /* Initialize input devices HAL */
    lv_hal_indev_init(display);
    
    DEBUG_PRINTF("✓ HAL initialization complete\n");
}

lv_display_t * lv_hal_get_display(void)
{
    return lv_hal_disp_get();
}

lv_indev_t * lv_hal_get_mouse(void)
{
    return lv_hal_indev_get_mouse();
}

lv_indev_t * lv_hal_get_keyboard(void)
{
    return lv_hal_indev_get_keyboard();
}
