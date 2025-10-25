#include "win32_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HWND s_main_hwnd = NULL;

void set_main_window_handle(HWND hwnd)
{
    s_main_hwnd = hwnd;
}

HWND get_main_window_handle(void)
{
    return s_main_hwnd;
}

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
