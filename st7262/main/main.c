#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_lcd_st7262.h>

#define TAG "ESP32-MAIN"

void main_task(void *parg)
{
    ESP_LOGI(TAG, "Main task started.");

    const esp_lcd_panel_st7262_config_handle_t panel_config = &ESP_LCD_PANEL_ST7262_8048S043;
    esp_lcd_panel_st7262_panel_t panel;

    esp_err_t error = esp_lcd_panel_st7262_init(panel_config, &panel);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ST7262 LCD panel: %s", esp_err_to_name(error));
        return;
    }

    while (true)
    {
        ESP_LOGI(TAG, "Running...");
        static bool on = true;

        // Toggle the backlight
        esp_lcd_panel_st7262_backlight_on_ff(panel_config, on);

        on = !on;

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 system boot.");

    TaskHandle_t main_task_handle = NULL;
    xTaskCreate(main_task, "main_task", 4096, NULL, 5, &main_task_handle);
}