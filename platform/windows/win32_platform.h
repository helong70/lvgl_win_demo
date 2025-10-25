#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include <windows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Store or clear the primary HWND used by the application */
void set_main_window_handle(HWND hwnd);

/* Retrieve the primary HWND (may return NULL before initialization) */
HWND get_main_window_handle(void);

/* Clipboard helpers backed by the current HWND */
bool win32_clipboard_copy(const char * text);
char * win32_clipboard_paste(void);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_PLATFORM_H */
