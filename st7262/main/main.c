#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_lcd_st7262.h>

#define TAG "ESP32-MAIN"

void main_task(void *parg)
{
    ESP_LOGI(TAG, "Hello from FreeRTOS task!");

    // Initialize the ST7262 LCD panel
    esp_lcd_panel_st7262_init();
    
    while (true)
    {
        ESP_LOGI(TAG, "Running...");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 system boot.");

    TaskHandle_t main_task_handle = NULL;
    xTaskCreate(main_task, "main_task", 4096, NULL, 5, &main_task_handle);
}