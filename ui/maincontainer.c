#include "maincontainer.h"
#include "custom_keys.h"
#include "setting.h"
#include <stdio.h>
#include <string.h>

/* Static state */
static lv_group_t * s_keyboard_group = NULL;

/* Forward declarations for internal helpers */
static void create_demo_content(lv_obj_t * content);
static void btn_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void dropdown_event_cb(lv_event_t * e);
static void textarea_event_cb(lv_event_t * e);
static void textarea_custom_handler(lv_obj_t * textarea, lv_event_code_t code);
static void textarea2_custom_handler(lv_obj_t * textarea, lv_event_code_t code);

lv_obj_t * maincontainer_create(lv_obj_t * parent, int width, int height, int title_height)
{
    lv_obj_t * content = lv_obj_create(parent);
    lv_obj_set_size(content, width + 3, height - title_height + 4);
    lv_obj_set_pos(content, -2, title_height - 4);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_clip_corner(content, true, 0);
    lv_obj_set_style_shadow_width(content, 0, 0);
    lv_obj_set_style_shadow_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);

    create_demo_content(content);

    return content;
}

lv_group_t * maincontainer_get_keyboard_group(void)
{
    return s_keyboard_group;
}

static void create_demo_content(lv_obj_t * content)
{
    /* Primary button */
    lv_obj_t * btn = lv_btn_create(content);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_radius(btn, 12, 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    /* Slider */
    lv_obj_t * slider = lv_slider_create(content);
    lv_obj_set_size(slider, 200, 25);
    lv_obj_align(slider, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Dropdown */
    lv_obj_t * dropdown = lv_dropdown_create(content);
    lv_dropdown_set_options(dropdown, "Option 1\nOption 2\nOption 3\nOption 4\nOption 5");
    lv_obj_set_size(dropdown, 180, 40);
    lv_obj_align(dropdown, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_event_cb(dropdown, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_radius(dropdown, 8, 0);
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(dropdown, 1, 0);

    /* Text areas */
    lv_obj_t * textarea = lv_textarea_create(content);
    lv_obj_set_size(textarea, 280, 45);
    lv_obj_align(textarea, LV_ALIGN_BOTTOM_LEFT, 20, -80);
    lv_textarea_set_placeholder_text(textarea, "Username...");
    lv_textarea_set_text(textarea, "");
    lv_obj_set_user_data(textarea, (void*)1);
    lv_obj_add_event_cb(textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(textarea, 8, 0);
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(textarea, 2, 0);
    lv_obj_set_style_pad_all(textarea, 8, 0);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0x2196F3), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(textarea, 2, LV_STATE_FOCUSED);

    lv_obj_t * textarea2 = lv_textarea_create(content);
    lv_obj_set_size(textarea2, 280, 45);
    lv_obj_align_to(textarea2, textarea, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_textarea_set_placeholder_text(textarea2, "Password...");
    lv_textarea_set_text(textarea2, "");
    lv_textarea_set_password_mode(textarea2, true);
    lv_obj_set_user_data(textarea2, (void*)2);
    lv_obj_add_event_cb(textarea2, textarea_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_radius(textarea2, 8, 0);
    lv_obj_set_style_bg_color(textarea2, lv_color_hex(0xFFF8E1), 0);
    lv_obj_set_style_border_color(textarea2, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(textarea2, 2, 0);
    lv_obj_set_style_pad_all(textarea2, 8, 0);
    lv_obj_set_style_border_color(textarea2, lv_color_hex(0xFF9800), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(textarea2, 2, LV_STATE_FOCUSED);

    /* Keyboard navigation group */
    s_keyboard_group = lv_group_create();
    lv_group_add_obj(s_keyboard_group, textarea);
    lv_group_add_obj(s_keyboard_group, textarea2);
    lv_obj_add_state(textarea, LV_STATE_FOCUSED);

    /* Status label */
    lv_obj_t * status = lv_label_create(content);
    lv_label_set_text(status, "UI Layout: Modern arrangement\nKeyboard navigation enabled");
    lv_obj_align(status, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_text_color(status, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_RIGHT, 0);

    /* Settings button */
    settings_init();
    settings_create_button(content);

    printf("Main container demo content created\n");
}

static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);

    static bool clicked = false;
    clicked = !clicked;

    if (clicked) {
        lv_label_set_text(label, "Clicked!");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), 0);
        printf("*** BUTTON CLICKED - UI INTERACTION WORKS! ***\n");
    } else {
        lv_label_set_text(label, "Click Me!");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), 0);
        printf("*** BUTTON RESET ***\n");
    }

    lv_obj_invalidate(btn);
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    lv_obj_invalidate(slider);
    lv_obj_invalidate(lv_obj_get_parent(slider));
    printf("Slider changed: %d\n", (int)v);
}

static void dropdown_event_cb(lv_event_t * e)
{
    lv_obj_t * dropdown = (lv_obj_t *)lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);

    char option_text[32];
    lv_dropdown_get_selected_str(dropdown, option_text, sizeof(option_text));

    printf("Dropdown selection changed: Index=%d, Text='%s'\n", selected, option_text);

    lv_obj_invalidate(dropdown);
}

static void textarea_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * textarea = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_indev_get_key(lv_indev_get_act());
        if (key == LV_KEY_CTRL_A) {
            printf("Executing select all in textarea\n");
            const char * text = lv_textarea_get_text(textarea);
            if (text && strlen(text) > 0) {
                lv_textarea_set_cursor_pos(textarea, 0);
                uint32_t text_len = strlen(text);
                lv_textarea_set_cursor_pos(textarea, text_len);
                printf("Selected all text in textarea: '%s' (length: %d)\n", text, text_len);
            }
            return;
        } else if (key == LV_KEY_ESC) {
            printf("ESC key pressed\n");
            return;
        }
    }

    void * user_data = lv_obj_get_user_data(textarea);
    if (user_data == (void*)1) {
        textarea_custom_handler(textarea, code);
    } else if (user_data == (void*)2) {
        textarea2_custom_handler(textarea, code);
    } else {
        textarea_custom_handler(textarea, code);
    }
}

static void textarea_custom_handler(lv_obj_t * textarea, lv_event_code_t code)
{
    switch (code) {
        case LV_EVENT_CLICKED:
            printf("Textarea clicked - ready for input\n");
            break;
        case LV_EVENT_FOCUSED:
            printf("Textarea focused\n");
            lv_obj_set_style_border_color(textarea, lv_color_hex(0x2196F3), 0);
            break;
        case LV_EVENT_DEFOCUSED:
            printf("Textarea defocused\n");
            lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
            break;
        case LV_EVENT_VALUE_CHANGED: {
            const char * text = lv_textarea_get_text(textarea);
            printf("Textarea content changed: '%s'\n", text);
            if (strlen(text) > 100) {
                printf("Text too long, truncating...\n");
                char truncated[101];
                strncpy(truncated, text, 100);
                truncated[100] = '\0';
                lv_textarea_set_text(textarea, truncated);
            }
            break;
        }
        case LV_EVENT_READY: {
            const char * text = lv_textarea_get_text(textarea);
            printf("Textarea ready (Enter pressed): '%s'\n", text);
            break;
        }
        default:
            break;
    }
}

static void textarea2_custom_handler(lv_obj_t * textarea, lv_event_code_t code)
{
    switch (code) {
        case LV_EVENT_CLICKED:
            printf("Password field clicked - ready for input\n");
            break;
        case LV_EVENT_FOCUSED:
            printf("Password field focused\n");
            lv_obj_set_style_border_color(textarea, lv_color_hex(0xFF9800), 0);
            lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFF3E0), 0);
            break;
        case LV_EVENT_DEFOCUSED:
            printf("Password field defocused\n");
            lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), 0);
            lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFF8E1), 0);
            break;
        case LV_EVENT_VALUE_CHANGED: {
            const char * text = lv_textarea_get_text(textarea);
            printf("Password field content changed (length: %d)\n", (int)strlen(text));
            if (strlen(text) > 20) {
                printf("Password too long, truncating to 20 characters...\n");
                char truncated[21];
                strncpy(truncated, text, 20);
                truncated[20] = '\0';
                lv_textarea_set_text(textarea, truncated);
            }
            break;
        }
        case LV_EVENT_READY: {
            const char * text = lv_textarea_get_text(textarea);
            printf("Password field ready (Enter pressed, length: %d)\n", (int)strlen(text));
            printf("=== Form Submission Simulation ===\n");
            printf("Password: [HIDDEN] (length: %d)\n", (int)strlen(text));
            printf("=== End Submission ===\n");
            break;
        }
        default:
            break;
    }
}
