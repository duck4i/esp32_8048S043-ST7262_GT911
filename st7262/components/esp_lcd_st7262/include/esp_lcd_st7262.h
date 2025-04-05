/**
 * @file esp_lcd_st7262.h
 * @brief Header file for the ESP32 ST7262 LCD driver.
 *
 * This file contains the definitions, configurations, and function prototypes
 * for interfacing with the ST7262 LCD panel using the ESP32.
 */

#ifndef _ESP_LCD_ST7262_H_
#define _ESP_LCD_ST7262_H_
#include <stdint.h>
#include <esp_lcd_panel_rgb.h>

/**
 * @brief Structure definition for the ST7262 LCD driver configuration.
 *
 * This structure is used to configure and initialize the ST7262 LCD driver.
 * It contains various parameters required for proper operation of the driver.
 */
typedef struct
{
    /* R */
    int r_0;
    int r_1;
    int r_2;
    int r_3;
    int r_4;
    /* G */
    int g_0;
    int g_1;
    int g_2;
    int g_3;
    int g_4;
    int g_5;
    /* B */
    int b_0;
    int b_1;
    int b_2;
    int b_3;
    int b_4;
} esp_lcd_panel_st7262_rgb565_t;

/**
 * @brief Synchronization structure for the ST7262 LCD panel driver.
 *
 * This structure is used to define synchronization parameters for the
 * ST7262 LCD panel, which may include timing configurations or other
 * synchronization-related settings.
 */
typedef struct
{
    uint32_t polarity;
    uint32_t back_porch;
    uint32_t front_porch;
    uint32_t pulse_width;

} esp_lcd_panel_st7262_sync_t;

/**
 * @brief Structure defining the timing parameters for the ST7262 LCD panel.
 *
 * This structure is used to configure the timing settings required for
 * proper operation of the ST7262 LCD panel. The specific fields and
 * their descriptions should be defined elsewhere in the code.
 */
typedef struct
{
    esp_lcd_panel_st7262_sync_t hsync;
    esp_lcd_panel_st7262_sync_t vsync;
    uint32_t pclk_hz;
    uint32_t pclk_active_neg;
} esp_lcd_panel_st7262_timing_t;

/**
 * @brief Structure representing the GPIO configuration for the ST7262 LCD panel.
 *
 * This structure defines the GPIO pins and their configurations used to interface
 * with the ST7262 LCD panel. It is typically used to specify the hardware connections
 * between the ESP32 and the LCD panel.
 */
typedef struct
{
    int bl; // Backlight control
    int de;
    int hsync;
    int vsync;
    int pclk;
} esp_lcd_panel_st7262_gpio_t;

/**
 * @brief Structure representing the configuration for the ST7262 LCD panel.
 *
 * This structure contains the configuration parameters for the ST7262 LCD panel,
 * including its dimensions, GPIO settings, timing parameters, and color settings.
 */
typedef struct
{
    uint32_t width;
    uint32_t height;
    bool swap_BGR565;
    esp_lcd_panel_st7262_gpio_t gpio;
    esp_lcd_panel_st7262_timing_t timing;
    esp_lcd_panel_st7262_rgb565_t colour;
} esp_lcd_panel_st7262_conf_t;

typedef esp_lcd_panel_st7262_conf_t *esp_lcd_panel_st7262_config_handle_t;

/**
 * @brief ST7262 LCD panel specific structure
 *
 * This structure defines the specific configuration and state
 * for the ST7262 LCD panel. It is used internally by the driver
 * to manage the panel's operations and settings.
 */
typedef struct
{
    esp_lcd_panel_handle_t handle;
} esp_lcd_panel_st7262_panel_t;

typedef esp_lcd_panel_st7262_panel_t *esp_lcd_panel_st7262_panel_handle_t;

/** Concrete device implementations */

/**
 * @brief Configuration structure for the ESP LCD panel ST7262 with model 8048S043.
 *
 * This static constant defines the specific configuration parameters for the
 * ST7262 LCD panel model 8048S043. It is used to initialize and configure the
 * panel for proper operation with the ESP32.
 *
 * Usage:
 * - Pass this configuration structure to the appropriate initialization
 *   function when setting up the LCD panel.
 *
 * Note:
 * - Ensure that the hardware connections and other dependencies are properly
 *   configured before using this configuration.
 */
static const esp_lcd_panel_st7262_conf_t ESP_LCD_PANEL_ST7262_8048S043 =
{
    .width = 800,
    .height = 480,
    .swap_BGR565 = true,
    .gpio = {
        .bl = 2,
        .de = 40,
        .vsync = 41,
        .hsync = 39,
        .pclk = 42,
    },
    .timing = {
        .hsync = {
            .polarity = 0,
            .front_porch = 8,
            .back_porch = 8,
            .pulse_width = 4,
        },
        .vsync = {
            .polarity = 0,
            .front_porch = 8,
            .back_porch = 8,
            .pulse_width = 4,
        },
        .pclk_hz = 16 * 1000 * 1000,
        .pclk_active_neg = 1,
    },
    .colour = {
        .r_0 = 45, .r_1 = 48, .r_2 = 47, .r_3 = 21, .r_4 = 14, 
        .g_0 = 5, .g_1 = 6, .g_2 = 7, .g_3 = 15, .g_4 = 16, .g_5 = 4, 
        .b_0 = 8, .b_1 = 3, .b_2 = 46, .b_3 = 9, .b_4 = 1
    }
};

/**
 * @brief Create a new ST7262 LCD panel instance
 *
 * @param conf Configuration handle for the ST7262 panel
 * @param out_handle Output handle for the created panel instance
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_new(const esp_lcd_panel_st7262_config_handle_t conf, esp_lcd_panel_st7262_panel_handle_t out_handle);

/**
 * @brief Delete an ST7262 LCD panel instance
 *
 * @param panel Handle to the ST7262 panel instance to delete
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_del(const esp_lcd_panel_st7262_panel_handle_t panel);

/**
 * @brief Initialize the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_init(const esp_lcd_panel_st7262_panel_handle_t panel);

/**
 * @brief Set the mirroring mode of the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @param mirror_x Enable or disable horizontal mirroring
 * @param mirror_y Enable or disable vertical mirroring
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_mirror(const esp_lcd_panel_st7262_panel_handle_t panel, bool mirror_x, bool mirror_y);

/**
 * @brief Swap the X and Y axes of the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @param swap_axes Enable or disable axis swapping
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_swap_xy(const esp_lcd_panel_st7262_panel_handle_t panel, bool swap_axes);

/**
 * @brief Reset the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_reset(const esp_lcd_panel_st7262_panel_handle_t panel);

/**
 * @brief Refresh the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_refresh(const esp_lcd_panel_st7262_panel_handle_t panel);

/**
 * @brief Restart the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */

esp_err_t esp_lcd_panel_st7262_restart(const esp_lcd_panel_st7262_panel_handle_t panel);

/**
 * @brief Turn the display on or off for the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @param on Enable or disable the display
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_disp_on_ff(const esp_lcd_panel_st7262_panel_handle_t panel, bool on);

/**
 * @brief Draw a bitmap on the ST7262 LCD panel
 *
 * @param panel Handle to the ST7262 panel instance
 * @param x_start Starting X coordinate
 * @param y_start Starting Y coordinate
 * @param x_end Ending X coordinate
 * @param y_end Ending Y coordinate
 * @param color_data Pointer to the color data for the bitmap
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_draw_bitmap(const esp_lcd_panel_st7262_panel_handle_t panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);


/**
 * @brief Turn the backlight on or off for the ST7262 LCD panel
 *
 * @param conf Configuration handle for the ST7262 panel
 * @param on Enable or disable the backlight
 * @return
 *      - ESP_OK: Success
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Other errors
 */
esp_err_t esp_lcd_panel_st7262_backlight_on_ff(const esp_lcd_panel_st7262_config_handle_t conf, bool on);

#endif