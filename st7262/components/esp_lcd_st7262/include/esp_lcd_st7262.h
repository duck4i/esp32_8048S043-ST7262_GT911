#ifndef _ESP_LCD_ST7262_H_
#define _ESP_LCD_ST7262_H_
#include <stdint.h>
#include <esp_lcd_panel_rgb.h>

typedef struct {
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

typedef struct {
    uint32_t polarity;
    uint32_t back_porch;
    uint32_t front_porch;
    uint32_t pulse_width;
} esp_lcd_panel_st7262_sync_t;

typedef struct {
    esp_lcd_panel_st7262_sync_t hsync;
    esp_lcd_panel_st7262_sync_t vsync;
    uint32_t pclk_hz;
    uint32_t pclk_active_neg;
} esp_lcd_panel_st7262_timing_t;

typedef struct {
    int bl; // Backlight control
    int de;
    int hsync;
    int vsync;
    int pclk;
} esp_lcd_panel_st7262_gpio_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    esp_lcd_panel_st7262_gpio_t gpio;
    esp_lcd_panel_st7262_timing_t timing;
    esp_lcd_panel_st7262_rgb565_t colour;
} esp_lcd_panel_st7262_conf_t;

typedef esp_lcd_panel_st7262_conf_t *esp_lcd_panel_st7262_config_handle_t;

typedef struct {
    esp_lcd_panel_handle_t handle;
} esp_lcd_panel_st7262_panel_t;

typedef esp_lcd_panel_st7262_panel_t *esp_lcd_panel_st7262_handle_t; 

/** Concrete device implementations */

static const esp_lcd_panel_st7262_conf_t ESP_LCD_PANEL_ST7262_8048S043 = 
{
    .width = 800,
    .height = 600,
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
        .pclk_hz = 16 * 100 * 100,
        .pclk_active_neg = 1,
    },
    .colour = {
        .r_0 = 45,
        .r_1 = 48,
        .r_2 = 47,
        .r_3 = 21,
        .r_4 = 14,
        .g_0 = 5,
        .g_1 = 6,
        .g_2 = 7,
        .g_3 = 15,
        .g_4 = 16,
        .g_5 = 4,
        .b_0 = 8,
        .b_1 = 3,
        .b_2 = 46,
        .b_3 = 9,
        .b_4 = 1
    }
};

esp_err_t esp_lcd_panel_st7262_init(const esp_lcd_panel_st7262_config_handle_t conf, esp_lcd_panel_st7262_handle_t out_handle);
esp_err_t esp_lcd_panel_st7262_del(const esp_lcd_panel_st7262_handle_t panel);
esp_err_t esp_lcd_panel_st7262_mirror(const esp_lcd_panel_st7262_handle_t panel, bool mirror_x, bool mirror_y);
esp_err_t esp_lcd_panel_st7262_swap_xy(const esp_lcd_panel_st7262_handle_t panel, bool swap_axes);
esp_err_t esp_lcd_panel_st7262_refresh(const esp_lcd_panel_st7262_handle_t panel);
esp_err_t esp_lcd_panel_st7262_restart(const esp_lcd_panel_st7262_handle_t panel);
esp_err_t esp_lcd_panel_st7262_disp_on_ff(const esp_lcd_panel_st7262_handle_t conf, bool on);
esp_err_t esp_lcd_panel_st7262_draw_bitmap(const esp_lcd_panel_st7262_handle_t panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);

esp_err_t esp_lcd_panel_st7262_backlight_on_ff(const esp_lcd_panel_st7262_config_handle_t conf, bool on);

#endif