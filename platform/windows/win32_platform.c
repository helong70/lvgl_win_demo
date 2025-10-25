#include "win32_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <gdiplus.h>

using namespace Gdiplus;

/* Platform state - centralized window/OpenGL context */
static HWND s_main_hwnd = NULL;
static HDC s_hdc = NULL;
static HGLRC s_hglrc = NULL;
static GLuint s_texture = 0;
static uint32_t s_framebuf[800 * 600]; /* RGBA8888 framebuffer */
static int s_width = 800;
static int s_height = 600;
static int s_win_w = 800;
static int s_win_h = 600;
static float s_ui_scale = 1.0f;

/* GDI+ token */
static ULONG_PTR s_gdiplus_token = 0;

/* === Window Handle Accessors === */

void set_main_window_handle(HWND hwnd)
{
    s_main_hwnd = hwnd;
}

HWND get_main_window_handle(void)
{
    return s_main_hwnd;
}

/* === State Pointer Accessors === */

HWND* win32_get_hwnd_ptr(void) { return &s_main_hwnd; }
HDC* win32_get_hdc_ptr(void) { return &s_hdc; }
HGLRC* win32_get_hglrc_ptr(void) { return &s_hglrc; }
GLuint* win32_get_texture_ptr(void) { return &s_texture; }
uint32_t* win32_get_framebuffer_ptr(void) { return s_framebuf; }
int* win32_get_width_ptr(void) { return &s_width; }
int* win32_get_height_ptr(void) { return &s_height; }
int* win32_get_win_w_ptr(void) { return &s_win_w; }
int* win32_get_win_h_ptr(void) { return &s_win_h; }
float* win32_get_ui_scale_ptr(void) { return &s_ui_scale; }

/* === GDI+ Management === */

void win32_init_gdiplus(void)
{
    GdiplusStartupInput startup_input;
    GdiplusStartup(&s_gdiplus_token, &startup_input, NULL);
    printf("GDI+ initialized\n");
}

void win32_cleanup_gdiplus(void)
{
    if (s_gdiplus_token) {
        GdiplusShutdown(s_gdiplus_token);
        s_gdiplus_token = 0;
        printf("GDI+ cleaned up\n");
    }
}

/* === Clipboard Functions === */

bool win32_clipboard_copy(const char * text)
{
    if (!text || !*text) {
        return false;
    }

    if (!s_main_hwnd) {
        printf("Clipboard copy failed: window handle not set\n");
        return false;
    }

    if (!OpenClipboard(s_main_hwnd)) {
        printf("Failed to open clipboard for copy\n");
        return false;
    }

    if (!EmptyClipboard()) {
        printf("Failed to empty clipboard\n");
        CloseClipboard();
        return false;
    }

    size_t text_len = strlen(text) + 1;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text_len);
    if (!hGlobal) {
        printf("Failed to allocate memory for clipboard\n");
        CloseClipboard();
        return false;
    }

    char * buffer = (char *)GlobalLock(hGlobal);
    if (!buffer) {
        printf("Failed to lock clipboard memory\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    memcpy(buffer, text, text_len);
    GlobalUnlock(hGlobal);

    if (!SetClipboardData(CF_TEXT, hGlobal)) {
        printf("Failed to set clipboard data\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    printf("Text copied to clipboard: '%s'\n", text);
    return true;
}

char * win32_clipboard_paste(void)
{
    if (!s_main_hwnd) {
        printf("Clipboard paste failed: window handle not set\n");
        return NULL;
    }

    if (!OpenClipboard(s_main_hwnd)) {
        printf("Failed to open clipboard for paste\n");
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        printf("No text data in clipboard\n");
        CloseClipboard();
        return NULL;
    }

    char * source = (char *)GlobalLock(hData);
    if (!source) {
        printf("Failed to lock clipboard data\n");
        CloseClipboard();
        return NULL;
    }

    size_t length = strlen(source) + 1;
    char * result = (char *)malloc(length);
    if (!result) {
        printf("Failed to allocate memory for pasted text\n");
        GlobalUnlock(hData);
        CloseClipboard();
        return NULL;
    }

    memcpy(result, source, length);
    GlobalUnlock(hData);
    CloseClipboard();

    printf("Text pasted from clipboard: '%s'\n", result);
    return result;
}
