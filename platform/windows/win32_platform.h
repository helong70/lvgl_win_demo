#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

/* Include OpenGL for GLuint type */
#ifndef APIENTRY
#define APIENTRY
#endif
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Get/Set main window handle */
void set_main_window_handle(HWND hwnd);
HWND get_main_window_handle(void);

/* Clipboard helpers */
bool win32_clipboard_copy(const char * text);
char * win32_clipboard_paste(void);

/* OpenGL/Window state accessors for main.c */
/* These allow main.c to access platform internals without duplicating state */
HWND* win32_get_hwnd_ptr(void);
HDC* win32_get_hdc_ptr(void);
HGLRC* win32_get_hglrc_ptr(void);
GLuint* win32_get_texture_ptr(void);
uint32_t* win32_get_framebuffer_ptr(void);
int* win32_get_width_ptr(void);
int* win32_get_height_ptr(void);
int* win32_get_win_w_ptr(void);
int* win32_get_win_h_ptr(void);
float* win32_get_ui_scale_ptr(void);

/* GDI+ initialization/cleanup */
void win32_init_gdiplus(void);
void win32_cleanup_gdiplus(void);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_PLATFORM_H */
