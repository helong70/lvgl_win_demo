#include "lv_hal_indev.h"
#include <stdio.h>
#include <string.h>

/* Conditional printf - only output in console mode */
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

/***********************
 *   STATIC VARIABLES
 ***********************/
static lv_indev_t * indev_mouse = NULL;
static lv_indev_t * indev_keyboard = NULL;
static lv_indev_data_t mouse_data;

/* Keyboard queue */
static keyboard_event_t keyboard_queue[KEYBOARD_QUEUE_SIZE];
static int keyboard_queue_head = 0;
static int keyboard_queue_tail = 0;
static keyboard_event_t current_keyboard_data;
static DWORD last_key_time = 0;

/***********************
 *   STATIC PROTOTYPES
 ***********************/
static void mouse_read_cb(lv_indev_t * indev, lv_indev_data_t * data);
static void keyboard_read_cb(lv_indev_t * indev, lv_indev_data_t * data);
static bool keyboard_queue_pop(keyboard_event_t* event);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

void lv_hal_indev_init(lv_display_t * display)
{
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
    lv_indev_set_read_cb(indev_mouse, mouse_read_cb);
    lv_indev_set_display(indev_mouse, display);
    
    /* Create keyboard input device */
    indev_keyboard = lv_indev_create();
    lv_indev_set_type(indev_keyboard, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keyboard, keyboard_read_cb);
    lv_indev_set_display(indev_keyboard, display);
    
    DEBUG_PRINTF("✓ Input devices initialized (mouse + keyboard)\n");
}

lv_indev_t * lv_hal_indev_get_mouse(void)
{
    return indev_mouse;
}

lv_indev_t * lv_hal_indev_get_keyboard(void)
{
    return indev_keyboard;
}

void lv_hal_keyboard_queue_push(uint32_t key, lv_indev_state_t state)
{
    DWORD current_time = GetTickCount();
    
    /* Calculate next position */
    int next_head = (keyboard_queue_head + 1) % KEYBOARD_QUEUE_SIZE;
    
    /* Check if queue is full */
    if (next_head == keyboard_queue_tail) {
        /* Queue is full, skip oldest event */
        keyboard_queue_tail = (keyboard_queue_tail + 1) % KEYBOARD_QUEUE_SIZE;
        DEBUG_PRINTF("Keyboard queue overflow, dropping oldest event\n");
    }
    
    /* Add new event */
    keyboard_queue[keyboard_queue_head].key = key;
    keyboard_queue[keyboard_queue_head].state = state;
    keyboard_queue[keyboard_queue_head].timestamp = current_time;
    
    keyboard_queue_head = next_head;
    
    DEBUG_PRINTF("Keyboard event queued: key=0x%X, state=%d, queue_size=%d\n", 
           key, state, (keyboard_queue_head - keyboard_queue_tail + KEYBOARD_QUEUE_SIZE) % KEYBOARD_QUEUE_SIZE);
}

void lv_hal_mouse_set_state(int x, int y, bool pressed)
{
    mouse_data.point.x = x;
    mouse_data.point.y = y;
    mouse_data.state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

/***********************
 *   STATIC FUNCTIONS
 ***********************/

static void mouse_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    /* Copy the current mouse data */
    data->point.x = mouse_data.point.x;
    data->point.y = mouse_data.point.y;
    data->state = mouse_data.state;
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

static void keyboard_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
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
            DEBUG_PRINTF("Processing keyboard event: key=0x%X ('%c'), state=%d\n", 
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
