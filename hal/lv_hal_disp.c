#include "lv_hal_disp.h"
#include "platform/windows/win32_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/gl.h>

/* Conditional printf - only output in console mode */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

/***********************
 *   STATIC VARIABLES
 ***********************/
static lv_display_t * display = NULL;
static uint16_t buf1[LV_HAL_DISP_BUF_SIZE];
static uint16_t buf2[LV_HAL_DISP_BUF_SIZE];

/* Access platform OpenGL state */
#define g_hwnd (*win32_get_hwnd_ptr())
#define g_hdc (*win32_get_hdc_ptr())
#define g_hglrc (*win32_get_hglrc_ptr())
#define g_tex (*win32_get_texture_ptr())
#define g_framebuf (win32_get_framebuffer_ptr())
#define g_width (*win32_get_width_ptr())
#define g_height (*win32_get_height_ptr())
#define g_win_w (*win32_get_win_w_ptr())
#define g_win_h (*win32_get_win_h_ptr())

/***********************
 *   STATIC PROTOTYPES
 ***********************/
static void display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

lv_display_t * lv_hal_disp_init(void)
{
    /* Create display */
    display = lv_display_create(LV_HAL_DISP_WIDTH, LV_HAL_DISP_HEIGHT);
    if (!display) {
        DEBUG_PRINTF("ERROR: Failed to create display\n");
        return NULL;
    }
    
    /* Set display buffers */
    lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, display_flush_cb);
    
    DEBUG_PRINTF("✓ Display initialized (%dx%d)\n", LV_HAL_DISP_WIDTH, LV_HAL_DISP_HEIGHT);
    
    return display;
}

lv_display_t * lv_hal_disp_get(void)
{
    return display;
}

/***********************
 *   STATIC FUNCTIONS
 ***********************/

static void display_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
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

    /* Update GL texture subimage */
    if (g_hglrc && g_tex) {
        wglMakeCurrent(g_hdc, g_hglrc);
        glBindTexture(GL_TEXTURE_2D, g_tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        /* Upload texture data */
        if (w == g_width) {
            /* Contiguous in memory, upload directly */
            glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1, w, h, GL_RGBA, GL_UNSIGNED_BYTE, 
                           &g_framebuf[area->y1 * g_width + area->x1]);
        } else {
            /* Allocate temporary buffer for the subregion */
            size_t row_bytes = (size_t)w * 4;
            size_t total = row_bytes * (size_t)h;
            uint8_t * tmp = (uint8_t*)malloc(total);
            if (tmp) {
                for (int yy = 0; yy < h; ++yy) {
                    uint32_t * src = &g_framebuf[(area->y1 + yy) * g_width + area->x1];
                    uint8_t * dst = tmp + (size_t)yy * row_bytes;
                    memcpy(dst, src, row_bytes);
                }
                glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
                free(tmp);
            } else {
                /* Fallback: upload row-by-row */
                for (int yy = 0; yy < h; ++yy) {
                    uint32_t * src = &g_framebuf[(area->y1 + yy) * g_width + area->x1];
                    glTexSubImage2D(GL_TEXTURE_2D, 0, area->x1, area->y1 + yy, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, src);
                }
            }
        }
        
        /* Draw to window */
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

    /* Tell LVGL flush is complete */
    lv_display_flush_ready(disp);
}
