#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_cache.h>
#include <esp_psram.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_lcd_st7262.h>
#include <lvgl.h>
#include <lv_demos.h>

// #define LVGL_PORT 1
#define TAG "ESP32-MAIN"

#ifndef LVGL_PORT

static void render_flush_display(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_st7262_panel_handle_t panel = (esp_lcd_panel_st7262_panel_handle_t)lv_display_get_user_data(display);
    esp_lcd_panel_st7262_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (uint16_t *)px_map);
    lv_display_flush_ready(display);
}

static uint32_t esp_tick(void)
{
    return esp_timer_get_time() / 1000;
}

#endif

void setup_lvgl(uint32_t width, uint32_t height, esp_lcd_panel_st7262_panel_handle_t panel)
{
    ESP_LOGI(TAG, "Setting up LVGL...");

    lv_init();
    lv_tick_set_cb(esp_tick);

    lv_display_t *disp_handle = lv_display_create(width, height);
    lv_display_set_flush_cb(disp_handle, render_flush_display);
    lv_display_set_user_data(disp_handle, panel);

    lv_display_set_color_format(disp_handle, LV_COLOR_FORMAT_RGB565);

    size_t size = width * height * sizeof(lv_color16_t) / 4;
    lv_color16_t *draw_buf = (lv_color16_t *)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (draw_buf == NULL)
    {
        ESP_LOGE(TAG, "Could not allocate draw buffer memory");
        return;
    }

    lv_display_set_buffers(disp_handle, draw_buf, NULL, size, LV_DISP_RENDER_MODE_PARTIAL);

    lv_demo_widgets();
    lv_demo_widgets_start_slideshow();

    ESP_LOGI(TAG, "LVGL Demo started.");
}

void main_task(void *parg)
{
    ESP_LOGI(TAG, "Main task started.");

    static esp_lcd_panel_st7262_panel_t panel;
    esp_lcd_panel_st7262_conf_t panel_config = ESP_LCD_PANEL_ST7262_8048S043;

    esp_err_t error = esp_lcd_panel_st7262_new(&panel_config, &panel);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create ST7262 LCD panel: %s", esp_err_to_name(error));
        return;
    }

    error = esp_lcd_panel_st7262_reset(&panel);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to reset ST7262 LCD panel: %s", esp_err_to_name(error));
        return;
    }

    error = esp_lcd_panel_st7262_init(&panel);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ST7262 LCD panel: %s", esp_err_to_name(error));
        return;
    }

    error = esp_lcd_panel_st7262_backlight_on_ff(&panel_config, true);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to turn on backlight: %s", esp_err_to_name(error));
        return;
    }

#if 0
    // Define a 5x5 pixel array (red gradient)
    uint16_t test_pixels[5][5] = {
        {0xF800, 0xF000, 0xE800, 0xE000, 0xD800}, // Row 0: Bright red to darker red
        {0xD000, 0xC800, 0xC000, 0xB800, 0xB000}, // Row 1: Continuing gradient
        {0xA800, 0xA000, 0x9800, 0x9000, 0x8800}, // Row 2: Continuing gradient
        {0x8000, 0x7800, 0x7000, 0x6800, 0x6000}, // Row 3: Continuing gradient
        {0x5800, 0x5000, 0x4800, 0x4000, 0x3800}  // Row 4: Continuing gradient
    };

    esp_lcd_panel_st7262_draw_bitmap(panel_handle, 0, 0, 4, 4, (void *)test_pixels);
    esp_lcd_panel_st7262_refresh(panel_handle);
    ESP_LOGI(TAG, "Drew test pattern manual pixels");
#endif

    /*
    void* fb = NULL;
    esp_lcd_rgb_panel_get_frame_buffer(panel_handle->handle, 1, (void **)&fb);
    //
    lv_memcpy(fb, test_pixels, 5 * 5 * sizeof(uint16_t));
    */

#if 0
    uint16_t *test_pixels = heap_caps_malloc(panel_config->width * panel_config->height * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (test_pixels != NULL)
    {
        for (int i = 0; i < panel_config->width * panel_config->height; i++)
        {
            test_pixels[i] = 0xF800; // Bright red in RGB565 format
        }
        esp_lcd_panel_st7262_draw_bitmap(panel_handle, 0, 0, panel_config->width - 1, panel_config->height - 1, test_pixels);
        //esp_lcd_panel_st7262_refresh(panel_handle);
        ESP_LOGI(TAG, "Drew test pattern");
        free(test_pixels);
    }
#endif

#if 0
    void *fb = NULL;
    esp_lcd_rgb_panel_get_frame_buffer(panel_handle->handle, 1, (void **)&fb);

    if (fb != NULL)
    {
        for (int i = 0; i < panel_config->width * panel_config->height; i++)
        {
            *((uint16_t *)fb + i) = 0xF800; // Bright red in RGB565 format
        }
         esp_lcd_panel_st7262_draw_bitmap(panel_handle, 0, 0, panel_config->width - 1, panel_config->height - 1, fb);
        // esp_lcd_panel_st7262_refresh(panel_handle);
        ESP_LOGI(TAG, "Drew test pattern");
        // free(test_pixels);
    }

    Cache_WriteBack_Addr(fb, 20);
#endif

    setup_lvgl(panel_config.width, panel_config.height, &panel);

    while (true)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 system boot.");

    if (esp_psram_is_initialized() == false)
    {
        ESP_LOGI(TAG, "PSRAM not initialized, initializing...");
        esp_err_t error = esp_psram_init();
        if (error != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to initialize PSRAM: %s", esp_err_to_name(error));
            return;
        }
    }

    ESP_LOGI(TAG, "Free internal heap: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    TaskHandle_t main_task_handle = NULL;
    xTaskCreate(main_task, "main_task", 9064, NULL, 5, &main_task_handle);
}