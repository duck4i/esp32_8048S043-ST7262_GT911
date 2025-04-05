# ESP LCD ST7262 display driver component 

This component uses the Espressif LCD RGB module to setup any ST7262 display and comes with some preconfigured built in display options.

## Displays 

Supported displays:
- 8048S043

## Usage 

This component works the same as the other esp_lcd modules, once created you will get a handle to `esp_lcd_panel_st7262_panel_t` struct that has the panel handle of `esp_lcd_panel_handle_t`. 

Example configuration:

```c
#include <esp_lcd_st7262.h>

esp_lcd_panel_st7262_panel_t panel;
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

// Fill the screen with red colour
uint16_t *test_pixels = heap_caps_malloc(panel_config.width * panel_config.height * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
if (test_pixels != NULL)
{
    for (int i = 0; i < panel_config.width * panel_config.height; i++)
    {
        test_pixels[i] = 0xF800; // Bright red in RGB565 format
    }

    esp_lcd_panel_st7262_draw_bitmap(&panel, 0, 0, panel_config.width - 1, panel_config.height - 1, test_pixels);
    free(test_pixels);
}

```