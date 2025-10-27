#include "win32_platform.h"
#include "ui/custom_keys.h"
#include "ui/maincontainer.h"
#include "ui/titlebar.h"  /* Include for TITLEBAR_HEIGHT and TITLEBAR_DRAG_EXCLUDE_RIGHT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <gdiplus.h>
#include "lvgl/lvgl.h"

/* Conditional printf - only output in console mode */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

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

/* Callbacks for input events */
static win32_mouse_callback_t s_mouse_callback = NULL;
static win32_keyboard_push_callback_t s_keyboard_callback = NULL;
static win32_resize_callback_t s_resize_callback = NULL;
static lv_display_t * s_lvgl_display = NULL;

/* Long press and key state tracking */
static uint32_t s_long_press_key = 0;
static DWORD s_long_press_start_time = 0;
static DWORD s_long_press_last_repeat = 0;
static uint32_t s_long_press_repeat_count = 0;
static DWORD s_last_key_time = 0;

/* Forward declaration of WndProc */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
    DEBUG_PRINTF("GDI+ initialized\n");
}

void win32_cleanup_gdiplus(void)
{
    if (s_gdiplus_token) {
        GdiplusShutdown(s_gdiplus_token);
        s_gdiplus_token = 0;
        DEBUG_PRINTF("GDI+ cleaned up\n");
    }
}

/* === Rounded Region Creation (GDI+ for smooth edges) === */

static HRGN create_rounded_region_gdiplus(int width, int height, int radius)
{
    /* Use GDI+ GraphicsPath for smooth anti-aliased rounded rectangle */
    GraphicsPath* path = new GraphicsPath();
    
    /* Create rounded rectangle path */
    path->AddArc(0, 0, radius * 2, radius * 2, 180, 90);                    /* Top-left */
    path->AddLine(radius, 0, width - radius, 0);                            /* Top edge */
    path->AddArc(width - radius * 2, 0, radius * 2, radius * 2, 270, 90);  /* Top-right */
    path->AddLine(width, radius, width, height - radius);                   /* Right edge */
    path->AddArc(width - radius * 2, height - radius * 2, radius * 2, radius * 2, 0, 90);  /* Bottom-right */
    path->AddLine(width - radius, height, radius, height);                  /* Bottom edge */
    path->AddArc(0, height - radius * 2, radius * 2, radius * 2, 90, 90);  /* Bottom-left */
    path->CloseFigure();                                                     /* Left edge */
    
    /* Convert GraphicsPath to GDI Region */
    Region* region = new Region(path);
    
    /* Create temporary Graphics object from window DC for GetHRGN */
    HDC hdc = GetDC(s_main_hwnd);
    Graphics* graphics = new Graphics(hdc);
    HRGN hrgn = region->GetHRGN(graphics);
    
    delete graphics;
    ReleaseDC(s_main_hwnd, hdc);
    delete region;
    delete path;
    
    return hrgn;
}

/* === Clipboard Functions === */

bool win32_clipboard_copy(const char * text)
{
    if (!text || !*text) {
        return false;
    }

    if (!s_main_hwnd) {
        DEBUG_PRINTF("Clipboard copy failed: window handle not set\n");
        return false;
    }

    if (!OpenClipboard(s_main_hwnd)) {
        DEBUG_PRINTF("Failed to open clipboard for copy\n");
        return false;
    }

    if (!EmptyClipboard()) {
        DEBUG_PRINTF("Failed to empty clipboard\n");
        CloseClipboard();
        return false;
    }

    size_t text_len = strlen(text) + 1;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text_len);
    if (!hGlobal) {
        DEBUG_PRINTF("Failed to allocate memory for clipboard\n");
        CloseClipboard();
        return false;
    }

    char * buffer = (char *)GlobalLock(hGlobal);
    if (!buffer) {
        DEBUG_PRINTF("Failed to lock clipboard memory\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    memcpy(buffer, text, text_len);
    GlobalUnlock(hGlobal);

    if (!SetClipboardData(CF_TEXT, hGlobal)) {
        DEBUG_PRINTF("Failed to set clipboard data\n");
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    DEBUG_PRINTF("Text copied to clipboard: '%s'\n", text);
    return true;
}

char * win32_clipboard_paste(void)
{
    if (!s_main_hwnd) {
        DEBUG_PRINTF("Clipboard paste failed: window handle not set\n");
        return NULL;
    }

    if (!OpenClipboard(s_main_hwnd)) {
        DEBUG_PRINTF("Failed to open clipboard for paste\n");
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        DEBUG_PRINTF("No text data in clipboard\n");
        CloseClipboard();
        return NULL;
    }

    char * source = (char *)GlobalLock(hData);
    if (!source) {
        DEBUG_PRINTF("Failed to lock clipboard data\n");
        CloseClipboard();
        return NULL;
    }

    size_t length = strlen(source) + 1;
    char * result = (char *)malloc(length);
    if (!result) {
        DEBUG_PRINTF("Failed to allocate memory for pasted text\n");
        GlobalUnlock(hData);
        CloseClipboard();
        return NULL;
    }

    memcpy(result, source, length);
    GlobalUnlock(hData);
    CloseClipboard();

    DEBUG_PRINTF("Text pasted from clipboard: '%s'\n", result);
    return result;
}

/* === Input Helper Functions === */

#include <math.h>
#include "lvgl/lvgl.h"

/* OpenGL constants that might not be defined in older GL headers */
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif
#ifndef GL_SAMPLES
#define GL_SAMPLES 0x80A9
#endif

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
    double acceleration_factor = pow(0.85, repeat_count);
    uint32_t interval = (uint32_t)(base_interval * acceleration_factor);
    
    /* Ensure we don't go below minimum interval */
    if (interval < min_interval) {
        interval = min_interval;
    }
    
    DEBUG_PRINTF("Long press acceleration: key=0x%X, repeat=%d, duration=%d, interval=%d\n", 
           key, repeat_count, press_duration, interval);
    
    return interval;
}

/* Reset long press state when key is released or different key is pressed */
static void reset_long_press_state(void)
{
    if (s_long_press_key != 0) {
        DEBUG_PRINTF("Resetting long press state for key 0x%X (had %d repeats)\n", 
               s_long_press_key, s_long_press_repeat_count);
    }
    
    s_long_press_key = 0;
    s_long_press_start_time = 0;
    s_long_press_last_repeat = 0;
    s_long_press_repeat_count = 0;
}

/* === Callback Registration === */

void win32_set_mouse_callback(win32_mouse_callback_t callback)
{
    s_mouse_callback = callback;
}

void win32_set_keyboard_callback(win32_keyboard_push_callback_t callback)
{
    s_keyboard_callback = callback;
}

void win32_set_resize_callback(win32_resize_callback_t callback)
{
    s_resize_callback = callback;
}

void win32_set_display_for_resize(void * display)
{
    s_lvgl_display = (lv_display_t *)display;
}

/* === Window Procedure === */

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_NCHITTEST: {
            /* Allow dragging the window by clicking anywhere in the title bar area */
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(hwnd, &pt);
            
            /* Check if cursor is in title bar area (configurable via TITLEBAR_HEIGHT) */
            if (pt.y >= 0 && pt.y <= TITLEBAR_HEIGHT) {
                /* Check if it's not over button area (configurable via TITLEBAR_DRAG_EXCLUDE_RIGHT) */
                RECT rc;
                GetClientRect(hwnd, &rc);
                if (pt.x < rc.right - TITLEBAR_DRAG_EXCLUDE_RIGHT) {
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
            s_win_w = new_w > 0 ? new_w : 0;
            s_win_h = new_h > 0 ? new_h : 0;
            if (s_hglrc && s_hdc) {
                wglMakeCurrent(s_hdc, s_hglrc);
                glViewport(0, 0, s_win_w, s_win_h);
                /* Notify via callback if registered */
                if (s_resize_callback) {
                    s_resize_callback(s_win_w, s_win_h);
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            /* Notify via callback if registered */
            if (s_mouse_callback) {
                s_mouse_callback(x, y, true);
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            /* Notify via callback if registered */
            if (s_mouse_callback) {
                s_mouse_callback(x, y, false);
            }
            return 0;
        }
        case WM_MOUSEMOVE: {
            int x = (int)(short)LOWORD(lParam);
            int y = (int)(short)HIWORD(lParam);
            /* Check if left button is pressed during move (for dragging) */
            bool is_pressed = (wParam & MK_LBUTTON) != 0;
            /* Notify via callback if registered */
            if (s_mouse_callback) {
                s_mouse_callback(x, y, is_pressed);
            }
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            uint32_t key = (uint32_t)wParam;
            DWORD current_time = GetTickCount();
            
            /* Long press acceleration logic */
            bool should_process = false;
            
            if (s_long_press_key == key) {
                /* Same key being held down - check if we should repeat */
                DWORD press_duration = current_time - s_long_press_start_time;
                DWORD since_last_repeat = current_time - s_long_press_last_repeat;
                
                uint32_t required_interval = get_repeat_interval(key, s_long_press_repeat_count, press_duration);
                
                if (since_last_repeat >= required_interval) {
                    should_process = true;
                    s_long_press_last_repeat = current_time;
                    s_long_press_repeat_count++;
                }
            } else {
                /* New key or first press of this key */
                reset_long_press_state();
                s_long_press_key = key;
                s_long_press_start_time = current_time;
                s_long_press_last_repeat = current_time;
                s_long_press_repeat_count = 0;
                should_process = true;
            }
            
            if (!should_process) {
                return 0; /* Skip this key event */
            }
            
            DEBUG_PRINTF("Key pressed: 0x%X (%d) [repeat=%d, duration=%dms]\n", 
                   key, key, s_long_press_repeat_count, current_time - s_long_press_start_time);
            
            /* Check NumLock state for numpad handling */
            bool numlock_on = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
            
            /* Check for Ctrl key combinations */
            bool ctrl_pressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            
            /* Convert Windows virtual key codes to LVGL key codes */
            uint32_t lv_key = 0;
            
            /* Handle Ctrl+A for select all */
            if (ctrl_pressed && key == 'A') {
                DEBUG_PRINTF("Ctrl+A detected - Select All\n");
                lv_key = LV_KEY_CTRL_A; /* Use our custom key code for Ctrl+A */
                /* We'll handle the actual select all in the textarea event handler */
            }
            /* Handle Ctrl+C for copy */
            else if (ctrl_pressed && key == 'C') {
                DEBUG_PRINTF("Ctrl+C detected - Copy\n");
                /* Get the currently focused textarea */
                lv_group_t * keyboard_group = maincontainer_get_keyboard_group();
                lv_obj_t * active_ta = keyboard_group ? lv_group_get_focused(keyboard_group) : NULL;
                if (active_ta && lv_obj_check_type(active_ta, &lv_textarea_class)) {
                    const char * text = lv_textarea_get_text(active_ta);
                    if (text && strlen(text) > 0) {
                        win32_clipboard_copy(text);
                    } else {
                        DEBUG_PRINTF("No text to copy\n");
                    }
                } else {
                    DEBUG_PRINTF("No active textarea for copy\n");
                }
                return 0; /* Don't forward to LVGL */
            }
            /* Handle Ctrl+V for paste */
            else if (ctrl_pressed && key == 'V') {
                DEBUG_PRINTF("Ctrl+V detected - Paste\n");
                /* Get the currently focused textarea */
                lv_group_t * keyboard_group = maincontainer_get_keyboard_group();
                lv_obj_t * active_ta = keyboard_group ? lv_group_get_focused(keyboard_group) : NULL;
                if (active_ta && lv_obj_check_type(active_ta, &lv_textarea_class)) {
                    char * pasted_text = win32_clipboard_paste();
                    if (pasted_text) {
                        /* Replace current text with pasted text */
                        lv_textarea_set_text(active_ta, pasted_text);
                        free(pasted_text);
                    }
                } else {
                    DEBUG_PRINTF("No active textarea for paste\n");
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
                    DEBUG_PRINTF("Backspace key detected, mapping to LV_KEY_BACKSPACE (0x%X)\n", LV_KEY_BACKSPACE);
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
                
                DEBUG_PRINTF("WM_KEYDOWN processing: VK=0x%X (%s), lv_key=0x%X ('%c'), NumLock=%s\n", 
                       key, key_name[0] ? key_name : "Unknown", lv_key, 
                       (lv_key >= 32 && lv_key <= 126) ? (char)lv_key : '?', 
                       numlock_on ? "ON" : "OFF");
                /* Notify via callback if registered */
                if (s_keyboard_callback) {
                    s_keyboard_callback(lv_key, true);
                }
            } else {
                DEBUG_PRINTF("WM_KEYDOWN ignored: VK=0x%X, NumLock=%s\n", key, numlock_on ? "ON" : "OFF");
            }
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            uint32_t key = (uint32_t)wParam;
            
            /* Reset long press state when key is released */
            if (s_long_press_key == key) {
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
                if (current_time - s_last_key_time >= 2) { /* 2ms debounce for chars - very responsive */
                    DEBUG_PRINTF("WM_CHAR processing character: '%c' (0x%X)\n", 
                           (character >= 32 && character <= 126) ? (char)character : '?', character);
                    /* Notify via callback if registered */
                    if (s_keyboard_callback) {
                        s_keyboard_callback(character, true);
                    }
                    s_last_key_time = current_time;
                }
            } else {
                DEBUG_PRINTF("WM_CHAR ignored%s: '%c' (0x%X)\n", 
                       already_handled ? " (already handled)" : " (non-printable)", 
                       (character >= 32 && character <= 126) ? (char)character : '?', character);
            }
            return 0;
        }
        case WM_ERASEBKGND: {
            /* Prevent background erase to avoid flicker - OpenGL handles all drawing */
            return 1;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            /* Don't draw anything here - OpenGL handles all rendering.
             * The blue rectangle was causing flicker on restore.
             * Just validate the window to prevent continuous WM_PAINT messages.
             */
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* === Window Initialization === */

bool win32_init_window(int width, int height, float ui_scale)
{
    s_width = width;
    s_height = height;
    s_ui_scale = ui_scale;
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.style = CS_OWNDC;  /* Only CS_OWNDC, no CS_HREDRAW/CS_VREDRAW to reduce redraws */
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;  /* No background brush to prevent flicker */
    wc.lpszClassName = "lvgl_opengl_window";

    if (!RegisterClass(&wc)) {
        DEBUG_PRINTF("RegisterClass failed\n");
        return false;
    }

    int win_w = (int)(width * ui_scale);
    int win_h = (int)(height * ui_scale);
    int sx = GetSystemMetrics(SM_CXSCREEN);
    int sy = GetSystemMetrics(SM_CYSCREEN);
    int x = (sx - win_w) / 2;
    int y = (sy - win_h) / 2;

    DWORD style = WS_POPUP | WS_VISIBLE;
    DWORD exStyle = WS_EX_LAYERED;

    s_main_hwnd = CreateWindowExA(exStyle, wc.lpszClassName, NULL, style,
                         x, y, win_w, win_h,
                         NULL, NULL, hInstance, NULL);

    if (!s_main_hwnd) {
        DEBUG_PRINTF("CreateWindow failed\n");
        return false;
    }

    /* 【实现位置5】设置圆角窗口 - 使用 DWM API (Windows 11) 或 GDI+ 降级方案 */
    
    /* Force custom radius: Skip DWM and use GDI+ for full control over corner radius */
    #if 0  /* Set to 0 to re-enable Windows 11 DWM native rounded corners */
    DEBUG_PRINTF("Using GDI+ custom rounded corners (radius: %d pixels)\n", WIN32_CORNER_RADIUS);
    HRGN rgn = create_rounded_region_gdiplus(win_w, win_h, WIN32_CORNER_RADIUS);
    if (rgn) {
        SetWindowRgn(s_main_hwnd, rgn, TRUE);
        /* Note: DeleteObject not called - SetWindowRgn takes ownership */
    } else {
        /* Final fallback to simple CreateRoundRectRgn if GDI+ fails */
        DEBUG_PRINTF("GDI+ region creation failed, using CreateRoundRectRgn\n");
        rgn = CreateRoundRectRgn(0, 0, win_w, win_h, WIN32_CORNER_RADIUS_FALLBACK, WIN32_CORNER_RADIUS_FALLBACK);
        SetWindowRgn(s_main_hwnd, rgn, TRUE);
        DeleteObject(rgn);
    }
    #else
    /* Try Windows 11 DWM rounded corners first (system default, no custom radius) */
    DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
    HRESULT hr = DwmSetWindowAttribute(s_main_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, 
                                       &corner, sizeof(corner));
    
    if (FAILED(hr)) {
        /* Fallback to GDI+ smooth rounded region for Windows 7/8/10 */
        DEBUG_PRINTF("DWM rounded corners not supported (likely Windows 10 or earlier), using GDI+ fallback\n");
        HRGN rgn = create_rounded_region_gdiplus(win_w, win_h, WIN32_CORNER_RADIUS);
        if (rgn) {
            SetWindowRgn(s_main_hwnd, rgn, TRUE);
            /* Note: DeleteObject not called - SetWindowRgn takes ownership */
        } else {
            /* Final fallback to simple CreateRoundRectRgn if GDI+ fails */
            DEBUG_PRINTF("GDI+ region creation failed, using CreateRoundRectRgn\n");
            rgn = CreateRoundRectRgn(0, 0, win_w, win_h, WIN32_CORNER_RADIUS_FALLBACK, WIN32_CORNER_RADIUS_FALLBACK);
            SetWindowRgn(s_main_hwnd, rgn, TRUE);
            DeleteObject(rgn);
        }
    } else {
        DEBUG_PRINTF("DWM rounded corners applied successfully (Windows 11)\n");
    }
    #endif

    /* 【实现位置6】设置分层窗口属性(透明度等) */
    /* Set layered window attributes */
    SetLayeredWindowAttributes(s_main_hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);

    s_hdc = GetDC(s_main_hwnd);

    /* Initialize OpenGL */
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;

    int pf = ChoosePixelFormat(s_hdc, &pfd);
    if (!pf || !SetPixelFormat(s_hdc, pf, &pfd)) {
        DEBUG_PRINTF("ChoosePixelFormat/SetPixelFormat failed\n");
        return false;
    }

    s_hglrc = wglCreateContext(s_hdc);
    if (!s_hglrc || !wglMakeCurrent(s_hdc, s_hglrc)) {
        DEBUG_PRINTF("wglCreateContext/wglMakeCurrent failed\n");
        return false;
    }

    glEnable(GL_MULTISAMPLE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    GLint samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    DEBUG_PRINTF("OpenGL initialized with %d samples\n", samples);

    ShowWindow(s_main_hwnd, SW_SHOW);
    UpdateWindow(s_main_hwnd);

    RECT rc;
    if (GetClientRect(s_main_hwnd, &rc)) {
        s_win_w = rc.right - rc.left;
        s_win_h = rc.bottom - rc.top;
    } else {
        s_win_w = width;
        s_win_h = height;
    }

    return true;
}

void win32_cleanup_window(void)
{
    if (s_hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(s_hglrc);
        s_hglrc = NULL;
    }
    if (s_hdc && s_main_hwnd) {
        ReleaseDC(s_main_hwnd, s_hdc);
        s_hdc = NULL;
    }
    if (s_main_hwnd) {
        DestroyWindow(s_main_hwnd);
        s_main_hwnd = NULL;
    }
    DEBUG_PRINTF("Window cleaned up\n");
}

bool win32_process_messages(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
