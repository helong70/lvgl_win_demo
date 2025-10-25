#include "setting.h"
#include <stdio.h>
#include <string.h>

/* Static variables for settings UI */
static lv_obj_t * settings_dialog = NULL;
static lv_obj_t * settings_bg = NULL;
static settings_config_t current_settings;
static bool settings_initialized = false;

/* UI control references */
static lv_obj_t * animation_slider = NULL;
static lv_obj_t * theme_dropdown = NULL;
static lv_obj_t * sound_switch = NULL;
static lv_obj_t * transparency_slider = NULL;
static lv_obj_t * autosave_switch = NULL;

/* Forward declarations */
static void settings_dialog_close_event_cb(lv_event_t * e);
static void settings_apply_event_cb(lv_event_t * e);
static void settings_reset_event_cb(lv_event_t * e);
static void settings_button_event_cb(lv_event_t * e);
static void settings_create_dialog_content(lv_obj_t * dialog);
static void settings_load_defaults(void);
static void settings_animation_deleted_cb(lv_anim_t * a);

/* Gear icon symbol (using Unicode gear symbol or creating a simple substitute) */
#define SETTINGS_ICON LV_SYMBOL_SETTINGS

void settings_init(void)
{
    if (settings_initialized) return;
    
    settings_load_defaults();
    settings_initialized = true;
    
    printf("Settings system initialized\n");
}

void settings_load_defaults(void)
{
    current_settings.animation_speed = 50;   /* Medium speed */
    current_settings.theme_index = 0;        /* Light theme */
    current_settings.sound_enabled = true;   /* Sound on */
    current_settings.transparency = 95;      /* Mostly opaque */
    current_settings.auto_save = true;       /* Auto-save enabled */
    
    printf("Settings loaded with defaults\n");
}

lv_obj_t * settings_create_button(lv_obj_t * parent)
{
    /* Create settings button with gear icon */
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_size(btn, 50, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -20, 20);
    
    /* Style the button */
    lv_obj_set_style_radius(btn, 25, 0);  /* Make it circular */
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x607D8B), 0);  /* Blue-grey */
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x455A64), LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_30, 0);
    
    /* Add gear icon label */
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, SETTINGS_ICON);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);
    
    /* Add event handler */
    lv_obj_add_event_cb(btn, settings_button_event_cb, LV_EVENT_CLICKED, NULL);
    
    printf("Settings button created\n");
    return btn;
}

static void settings_button_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t * parent = lv_obj_get_parent(btn);  /* Use the button's parent (content container) */
    
    printf("Settings button clicked\n");
    settings_show_dialog(parent);
}

void settings_show_dialog(lv_obj_t * parent)
{
    if (settings_dialog != NULL) {
        return; /* Dialog already shown */
    }
    
    /* Create semi-transparent background */
    settings_bg = lv_obj_create(parent);
    /* Get the parent's actual size */
    lv_coord_t parent_width = lv_obj_get_width(parent);
    lv_coord_t parent_height = lv_obj_get_height(parent);
    printf("Parent size: %d x %d\n", parent_width, parent_height);

    
    /* Make background larger to ensure full coverage */
    lv_obj_set_size(settings_bg, parent_width + 80, parent_height + 80);
    lv_obj_set_pos(settings_bg, -40, -50);  /* Offset to cover any gaps */
    
    lv_obj_set_style_bg_color(settings_bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(settings_bg, LV_OPA_TRANSP, 0);  /* Completely transparent background */
    lv_obj_set_style_border_width(settings_bg, 0, 0);
    lv_obj_set_style_pad_all(settings_bg, 0, 0);
    lv_obj_set_style_margin_all(settings_bg, 0, 0);
    lv_obj_set_style_outline_width(settings_bg, 0, 0);
    lv_obj_clear_flag(settings_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(settings_bg, 0, 0);  /* No rounded corners */
    printf("Background size set to: %d x %d at pos (%d, %d)\n", 
           lv_obj_get_width(settings_bg), lv_obj_get_height(settings_bg),
           lv_obj_get_x(settings_bg), lv_obj_get_y(settings_bg));
    
    /* Create main dialog */
    settings_dialog = lv_obj_create(settings_bg);
    lv_obj_set_size(settings_dialog, 320, 420);
    lv_obj_center(settings_dialog);
    
    /* Style the dialog */
    lv_obj_set_style_radius(settings_dialog, 15, 0);
    lv_obj_set_style_bg_color(settings_dialog, lv_color_white(), 0);
    lv_obj_set_style_border_color(settings_dialog, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_border_width(settings_dialog, 2, 0);
    lv_obj_set_style_shadow_width(settings_dialog, 20, 0);
    lv_obj_set_style_shadow_opa(settings_dialog, LV_OPA_40, 0);
    lv_obj_set_style_pad_all(settings_dialog, 15, 0);
    
    /* Create dialog content */
    settings_create_dialog_content(settings_dialog);
    
    /* Ensure normal scale (no animation for now) */
    lv_obj_set_style_transform_scale(settings_dialog, 256, 0);  /* Normal size (256 = 1.0x) */
    
    printf("Settings dialog shown\n");
}

static void settings_create_dialog_content(lv_obj_t * dialog)
{
    /* Title */
    lv_obj_t * title = lv_label_create(dialog);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    
    /* Close button */
    lv_obj_t * close_btn = lv_button_create(dialog);
    lv_obj_set_size(close_btn, 30, 30);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_radius(close_btn, 15, 0);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xF44336), 0);
    
    lv_obj_t * close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(close_label, lv_color_white(), 0);
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(close_btn, settings_dialog_close_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Content area */
    lv_obj_t * content = lv_obj_create(dialog);
    lv_obj_set_size(content, LV_PCT(100), 300);
    lv_obj_align_to(content, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    
    /* Animation Speed Setting */
    lv_obj_t * anim_label = lv_label_create(content);
    lv_label_set_text(anim_label, "Animation Speed:");
    lv_obj_set_style_text_color(anim_label, lv_color_hex(0x555555), 0);
    lv_obj_align(anim_label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    animation_slider = lv_slider_create(content);
    lv_obj_set_width(animation_slider, LV_PCT(100));
    lv_obj_align_to(animation_slider, anim_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_slider_set_range(animation_slider, 10, 100);
    lv_slider_set_value(animation_slider, current_settings.animation_speed, LV_ANIM_OFF);
    
    /* Theme Selection */
    lv_obj_t * theme_label = lv_label_create(content);
    lv_label_set_text(theme_label, "Theme:");
    lv_obj_set_style_text_color(theme_label, lv_color_hex(0x555555), 0);
    lv_obj_align_to(theme_label, animation_slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    
    theme_dropdown = lv_dropdown_create(content);
    lv_dropdown_set_options(theme_dropdown, "Light\nDark\nAuto");
    lv_obj_set_width(theme_dropdown, LV_PCT(100));
    lv_obj_align_to(theme_dropdown, theme_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_dropdown_set_selected(theme_dropdown, current_settings.theme_index);
    
    /* Sound Toggle */
    lv_obj_t * sound_label = lv_label_create(content);
    lv_label_set_text(sound_label, "Sound Effects:");
    lv_obj_set_style_text_color(sound_label, lv_color_hex(0x555555), 0);
    lv_obj_align_to(sound_label, theme_dropdown, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    
    sound_switch = lv_switch_create(content);
    lv_obj_align_to(sound_switch, sound_label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    if (current_settings.sound_enabled) {
        lv_obj_add_state(sound_switch, LV_STATE_CHECKED);
    }
    
    /* Transparency Setting */
    lv_obj_t * trans_label = lv_label_create(content);
    lv_label_set_text(trans_label, "Window Transparency:");
    lv_obj_set_style_text_color(trans_label, lv_color_hex(0x555555), 0);
    lv_obj_align_to(trans_label, sound_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 25);
    
    transparency_slider = lv_slider_create(content);
    lv_obj_set_width(transparency_slider, LV_PCT(100));
    lv_obj_align_to(transparency_slider, trans_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_slider_set_range(transparency_slider, 30, 100);
    lv_slider_set_value(transparency_slider, current_settings.transparency, LV_ANIM_OFF);
    
    /* Auto-save Toggle */
    lv_obj_t * autosave_label = lv_label_create(content);
    lv_label_set_text(autosave_label, "Auto-save Settings:");
    lv_obj_set_style_text_color(autosave_label, lv_color_hex(0x555555), 0);
    lv_obj_align_to(autosave_label, transparency_slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    
    autosave_switch = lv_switch_create(content);
    lv_obj_align_to(autosave_switch, autosave_label, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    if (current_settings.auto_save) {
        lv_obj_add_state(autosave_switch, LV_STATE_CHECKED);
    }
    
    /* Action buttons */
    lv_obj_t * btn_area = lv_obj_create(dialog);
    lv_obj_set_size(btn_area, LV_PCT(100), 60);
    lv_obj_align(btn_area, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_border_width(btn_area, 0, 0);
    lv_obj_set_style_bg_opa(btn_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btn_area, 0, 0);
    
    /* Apply button */
    lv_obj_t * apply_btn = lv_button_create(btn_area);
    lv_obj_set_size(apply_btn, 80, 35);
    lv_obj_align(apply_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(apply_btn, lv_color_hex(0x4CAF50), 0);
    
    lv_obj_t * apply_label = lv_label_create(apply_btn);
    lv_label_set_text(apply_label, "Apply");
    lv_obj_set_style_text_color(apply_label, lv_color_white(), 0);
    lv_obj_center(apply_label);
    
    lv_obj_add_event_cb(apply_btn, settings_apply_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* Reset button */
    lv_obj_t * reset_btn = lv_button_create(btn_area);
    lv_obj_set_size(reset_btn, 80, 35);
    lv_obj_align_to(reset_btn, apply_btn, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0xFF9800), 0);
    
    lv_obj_t * reset_label = lv_label_create(reset_btn);
    lv_label_set_text(reset_label, "Reset");
    lv_obj_set_style_text_color(reset_label, lv_color_white(), 0);
    lv_obj_center(reset_label);
    
    lv_obj_add_event_cb(reset_btn, settings_reset_event_cb, LV_EVENT_CLICKED, NULL);
}

static void settings_dialog_close_event_cb(lv_event_t * e)
{
    printf("Settings dialog close clicked\n");
    settings_hide_dialog();
}

static void settings_apply_event_cb(lv_event_t * e)
{
    printf("Settings apply clicked\n");
    
    /* Read values from UI controls */
    if (animation_slider) {
        current_settings.animation_speed = lv_slider_get_value(animation_slider);
    }
    if (theme_dropdown) {
        current_settings.theme_index = lv_dropdown_get_selected(theme_dropdown);
    }
    if (sound_switch) {
        current_settings.sound_enabled = lv_obj_has_state(sound_switch, LV_STATE_CHECKED);
    }
    if (transparency_slider) {
        current_settings.transparency = lv_slider_get_value(transparency_slider);
    }
    if (autosave_switch) {
        current_settings.auto_save = lv_obj_has_state(autosave_switch, LV_STATE_CHECKED);
    }
    
    settings_apply_changes();
    settings_hide_dialog();
}

static void settings_reset_event_cb(lv_event_t * e)
{
    printf("Settings reset clicked\n");
    settings_reset_defaults();
    settings_hide_dialog();
}

void settings_hide_dialog(void)
{
    if (settings_dialog == NULL) {
        return; /* Dialog not shown */
    }
    
    /* Simple cleanup without animation for now */
    settings_animation_deleted_cb(NULL);
    
    printf("Settings dialog hidden\n");
}

bool settings_is_visible(void)
{
    return settings_dialog != NULL;
}

settings_config_t * settings_get_config(void)
{
    return &current_settings;
}

void settings_apply_changes(void)
{
    printf("Applying settings changes:\n");
    printf("  Animation Speed: %d\n", current_settings.animation_speed);
    printf("  Theme: %d\n", current_settings.theme_index);
    printf("  Sound: %s\n", current_settings.sound_enabled ? "ON" : "OFF");
    printf("  Transparency: %d%%\n", current_settings.transparency);
    printf("  Auto-save: %s\n", current_settings.auto_save ? "ON" : "OFF");
    
    /* Here you would implement actual setting changes */
    /* For example, applying themes, adjusting animation speeds, etc. */
}

void settings_reset_defaults(void)
{
    settings_load_defaults();
    printf("Settings reset to defaults\n");
}

static void settings_animation_deleted_cb(lv_anim_t * a)
{
    /* Called when the exit animation completes */
    if (settings_bg) {
        lv_obj_delete(settings_bg);
        settings_bg = NULL;
    }
    settings_dialog = NULL;
    animation_slider = NULL;
    theme_dropdown = NULL;
    sound_switch = NULL;
    transparency_slider = NULL;
    autosave_switch = NULL;
    
    printf("Settings dialog cleanup completed\n");
}