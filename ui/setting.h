#ifndef SETTING_H
#define SETTING_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Settings UI public interface */

/**
 * Initialize the settings system
 */
void settings_init(void);

/**
 * Create and show the settings dialog
 * @param parent Parent object to center the dialog on
 */
void settings_show_dialog(lv_obj_t * parent);

/**
 * Hide and close the settings dialog
 */
void settings_hide_dialog(void);

/**
 * Check if settings dialog is currently visible
 * @return true if visible, false otherwise
 */
bool settings_is_visible(void);

/**
 * Create the settings button (gear icon)
 * @param parent Parent object to attach the button to
 * @return Pointer to the created button object
 */
lv_obj_t * settings_create_button(lv_obj_t * parent);

/* Settings configuration structure */
typedef struct {
    uint32_t animation_speed;    /* Animation speed (0-100) */
    uint32_t theme_index;        /* Theme selection (0=Light, 1=Dark) */
    bool sound_enabled;          /* Sound effects enabled */
    uint32_t transparency;       /* Window transparency (0-100) */
    bool auto_save;              /* Auto-save settings */
} settings_config_t;

/**
 * Get current settings configuration
 * @return Pointer to current settings
 */
settings_config_t * settings_get_config(void);

/**
 * Apply settings changes
 */
void settings_apply_changes(void);

/**
 * Reset settings to defaults
 */
void settings_reset_defaults(void);

#ifdef __cplusplus
}
#endif

#endif /* SETTING_H */