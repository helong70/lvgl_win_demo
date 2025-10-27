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

/* Callback types for input events */
typedef void (*win32_mouse_callback_t)(int x, int y, bool pressed);
typedef void (*win32_keyboard_push_callback_t)(uint32_t key, uint32_t state);
typedef void (*win32_resize_callback_t)(int width, int height);

/* Window initialization and cleanup */
bool win32_init_window(int width, int height, float ui_scale);
void win32_cleanup_window(void);

/* Message loop processing - returns false when WM_QUIT received */
bool win32_process_messages(void);

/* Register callbacks for input events */
void win32_set_mouse_callback(win32_mouse_callback_t callback);
void win32_set_keyboard_callback(win32_keyboard_push_callback_t callback);
void win32_set_resize_callback(win32_resize_callback_t callback);
void win32_set_display_for_resize(void * display);

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
