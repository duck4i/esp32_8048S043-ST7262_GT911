#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_dev.h>
#include "esp_lcd_st7262.h"

#define TAG "ESP_LCD_ST7262"

const esp_lcd_panel_st7262_conf_t *_panel = NULL;

esp_err_t esp_lcd_panel_st7262_new(const esp_lcd_panel_st7262_config_handle_t conf, esp_lcd_panel_st7262_panel_handle_t out_handle)
{
    ESP_LOGI(TAG, "Initializing ST7262 LCD panel...");
    if (conf == NULL)
    {
        ESP_LOGE(TAG, "Invalid configuration for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }
    if (out_handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid output handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    _panel = conf;

    esp_lcd_panel_handle_t display_handle = NULL;
    esp_lcd_rgb_panel_config_t config =
        {
            .data_width = 16,
            .bits_per_pixel = 16,
            .clk_src = LCD_CLK_SRC_DEFAULT,
            //.bounce_buffer_size_px = 0, //conf->height,
            //.sram_trans_align = 8,
            //.psram_trans_align = 64,
            //.dma_burst_size = 64,
            .disp_gpio_num = GPIO_NUM_NC,
            .num_fbs = 0,
            .pclk_gpio_num = conf->gpio.pclk,
            .de_gpio_num = conf->gpio.de,
            .hsync_gpio_num = conf->gpio.hsync,
            .vsync_gpio_num = conf->gpio.vsync,
            .data_gpio_nums = {
                conf->colour.r_0,
                conf->colour.r_1,
                conf->colour.r_2,
                conf->colour.r_3,
                conf->colour.r_4,
                conf->colour.g_0,
                conf->colour.g_1,
                conf->colour.g_2,
                conf->colour.g_3,
                conf->colour.g_4,
                conf->colour.g_5,
                conf->colour.b_0,
                conf->colour.b_1,
                conf->colour.b_2,
                conf->colour.b_3,
                conf->colour.b_4,
            },
            .timings = {
                .v_res = conf->height,
                .h_res = conf->width,
                .hsync_back_porch = conf->timing.hsync.back_porch,
                .hsync_front_porch = conf->timing.hsync.front_porch,
                .hsync_pulse_width = conf->timing.hsync.pulse_width,
                .vsync_back_porch = conf->timing.vsync.back_porch,
                .vsync_front_porch = conf->timing.vsync.front_porch,
                .vsync_pulse_width = conf->timing.vsync.pulse_width,
                .pclk_hz = conf->timing.pclk_hz,
                .flags = {
                    .pclk_active_neg = conf->timing.pclk_active_neg,
                    .hsync_idle_low = (uint32_t)((conf->timing.hsync.polarity == 0) ? 1 : 0),
                    .vsync_idle_low = (uint32_t)((conf->timing.vsync.polarity == 0) ? 1 : 0),
                    .de_idle_high = 0,
                },
            },
            .flags = {
                .disp_active_low = true,
                .fb_in_psram = true,
                .refresh_on_demand = false,
                .no_fb = false,
                .double_fb = false,
            },
        };

    if (conf->swap_BGR565)
    {
        config.data_gpio_nums[0] = conf->colour.b_0;
        config.data_gpio_nums[1] = conf->colour.b_1;
        config.data_gpio_nums[2] = conf->colour.b_2;
        config.data_gpio_nums[3] = conf->colour.b_3;
        config.data_gpio_nums[4] = conf->colour.b_4;
        config.data_gpio_nums[5] = conf->colour.g_0;
        config.data_gpio_nums[6] = conf->colour.g_1;
        config.data_gpio_nums[7] = conf->colour.g_2;
        config.data_gpio_nums[8] = conf->colour.g_3;
        config.data_gpio_nums[9] = conf->colour.g_4;
        config.data_gpio_nums[10] = conf->colour.g_5;
        config.data_gpio_nums[11] = conf->colour.r_0;
        config.data_gpio_nums[12] = conf->colour.r_1;
        config.data_gpio_nums[13] = conf->colour.r_2;
        config.data_gpio_nums[14] = conf->colour.r_3;
        config.data_gpio_nums[15] = conf->colour.r_4;
    }

    esp_err_t error = esp_lcd_new_rgb_panel(&config, &display_handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    out_handle->handle = display_handle; // return handle

    ESP_LOGI(TAG, "ST7262 LCD panel initialized successfully.");
    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_del(esp_lcd_panel_st7262_panel_handle_t handle)
{
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_del(handle->handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_init(const esp_lcd_panel_st7262_panel_handle_t panel)
{
    if (panel == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_init(panel->handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_reset(const esp_lcd_panel_st7262_panel_handle_t panel)
{
    if (panel == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_reset(panel->handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to reset ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_refresh(esp_lcd_panel_st7262_panel_handle_t handle)
{
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_rgb_panel_refresh(handle->handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to refresh ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_restart(esp_lcd_panel_st7262_panel_handle_t handle)
{
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_rgb_panel_restart(handle->handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to refresh ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_mirror(esp_lcd_panel_st7262_panel_handle_t handle, bool mirror_x, bool mirror_y)
{
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_mirror(handle->handle, mirror_x, mirror_y);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ST7262 LCD panel mirror: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_swap_xy(esp_lcd_panel_st7262_panel_handle_t handle, bool swap_axes)
{
    if (handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_swap_xy(handle->handle, swap_axes);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ST7262 LCD panel swap axes: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_disp_on_ff(const esp_lcd_panel_st7262_panel_handle_t panel, bool on)
{
    if (panel == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    //  Activate blacklight
    esp_err_t error = esp_lcd_panel_disp_on_off(panel->handle, on);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ST7262 LCD panel display on/off: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_backlight_on_ff(const esp_lcd_panel_st7262_config_handle_t conf, bool on)
{
    if (conf == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    //  Activate blacklight
    esp_err_t error = gpio_set_direction(conf->gpio.bl, GPIO_MODE_OUTPUT);
    error = gpio_set_level(conf->gpio.bl, (int)on);

    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ST7262 LCD panel display on/off: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

esp_err_t esp_lcd_panel_st7262_draw_bitmap(const esp_lcd_panel_st7262_panel_handle_t panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    if (panel == NULL || panel->handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid handle for ST7262 LCD panel. Pointer is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = esp_lcd_panel_draw_bitmap(panel->handle, x_start, y_start, x_end, y_end, color_data);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to draw bitmap on ST7262 LCD panel: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}