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

#define USE_TOUCH 1
#define USE_LVGL 1
// #define USE_LVGL_PORT 1
//  #define TEST_FULL_SCREEN 1

#define STACK_SIZE 8192
#define TASK_PRIORITY 9

#define TAG "ESP32-MAIN"

#ifdef USE_LVGL
#include <lvgl.h>
#include <lv_demos.h>

#if USE_TOUCH
#include "gt911.h"

#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST 38
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 480
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 272
#define TOUCH_MAP_Y2 0

static gt911_handle_t gt911_dev;

void init_touch(void)
{
    ESP_LOGI(TAG, "Initializing GT911 touchscreen");

    // Initialize the GT911 touchscreen controller
    esp_err_t ret = gt911_init(&gt911_dev, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST,
                               TOUCH_MAP_X1, TOUCH_MAP_Y1, I2C_NUM_0, GT911_ADDR1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize GT911: %s", esp_err_to_name(ret));
        return;
    }

    // Set rotation if needed
    ret = gt911_set_rotation(&gt911_dev, ROTATION_INVERTED);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set rotation: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "GT911 initialized successfully");
}

void input_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    static bool input_initalized = false;
    if (!input_initalized)
    {
        init_touch();
        input_initalized = true;
    }

    if (gt911_read(&gt911_dev) == ESP_OK)
    {
        data->state = gt911_dev.is_touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

        //int touch_last_x = (gt911_dev.points[0].x - TOUCH_MAP_X1) * (800 - 1) / (TOUCH_MAP_X2 - TOUCH_MAP_X1);
        //int touch_last_y = (gt911_dev.points[0].y - TOUCH_MAP_Y1) * (480 - 1) / (TOUCH_MAP_Y2 - TOUCH_MAP_Y1);
 
        int32_t touch_last_x = 0;
        int32_t touch_last_y = 0;

        gt911_map_to_screen(&gt911_dev, 800, 480, gt911_dev.points[0].x, gt911_dev.points[0].y, &touch_last_x, &touch_last_y);

        data->point.x = touch_last_x;
        data->point.y = touch_last_y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

#endif

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

static void setup_lvgl(uint32_t width, uint32_t height, esp_lcd_panel_st7262_panel_handle_t panel)
{
    ESP_LOGI(TAG, "Setting up LVGL...");

    lv_init();
    lv_tick_set_cb(esp_tick);

    lv_display_t *disp_handle = lv_display_create(width, height);
    lv_display_set_flush_cb(disp_handle, render_flush_display);
    lv_display_set_user_data(disp_handle, panel);

    lv_display_set_color_format(disp_handle, LV_COLOR_FORMAT_RGB565);

    size_t size = width * height * sizeof(lv_color16_t) / 4;
    lv_color16_t *draw_buf = (lv_color16_t *)heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (draw_buf == NULL)
    {
        ESP_LOGE(TAG, "Could not allocate draw buffer memory");
        return;
    }

    lv_display_set_buffers(disp_handle, draw_buf, NULL, size, LV_DISP_RENDER_MODE_PARTIAL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, input_read);

    lv_demo_widgets();

#ifndef USE_TOUCH
    lv_demo_widgets_start_slideshow();
#endif

    ESP_LOGI(TAG, "LVGL Demo started.");
}

#endif

#ifdef USE_LVGL_PORT
#include <esp_lvgl_port.h>
#include <lv_demos.h>

static void setup_lvgl_port(uint32_t width, uint32_t height, esp_lcd_panel_st7262_panel_handle_t panel)
{
    ESP_LOGI(TAG, "Setting up LVGL port...");

    const lvgl_port_cfg_t lvgl_port_config = {
        .task_affinity = 1,
        .task_stack = STACK_SIZE,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5,
        .task_priority = TASK_PRIORITY,
    };

    esp_err_t error = lvgl_port_init(&lvgl_port_config);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize LVGL port: %s", esp_err_to_name(error));
        return;
    }

    const lvgl_port_display_cfg_t display_config = {
        .panel_handle = panel->handle,
        .hres = width,
        .vres = height,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .buffer_size = width * height * sizeof(lv_color16_t) / 4,
        .monochrome = false,
        .flags = {
            .direct_mode = false,
            .buff_spiram = true,
            .buff_dma = false,
        }};

    const lvgl_port_display_rgb_cfg_t rgb_display_config = {
        .flags = {
            .bb_mode = false,
            .avoid_tearing = false,
        }};

    lv_display_t *display_handle = lvgl_port_add_disp_rgb(&display_config, &rgb_display_config);
    if (display_handle == NULL)
    {
        ESP_LOGE(TAG, "Failed to add LVGL display");
        return;
    }

    lvgl_port_lock(0);
    lv_demo_widgets();
    lv_demo_widgets_start_slideshow();
    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL Demo started.");
}

#endif

void main_task(void *parg)
{
    ESP_LOGI(TAG, "Main task started.");

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

#if TEST_FULL_SCREEN
    uint16_t *test_pixels = heap_caps_malloc(panel_config.width * panel_config.height * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (test_pixels != NULL)
    {
        for (int i = 0; i < panel_config.width * panel_config.height; i++)
        {
            test_pixels[i] = 0xF800; // Bright red in RGB565 format
        }
        esp_lcd_panel_st7262_draw_bitmap(&panel, 0, 0, panel_config.width - 1, panel_config.height - 1, test_pixels);
        ESP_LOGI(TAG, "Drew test pattern");
        free(test_pixels);
    }
#endif

#ifdef USE_LVGL
    setup_lvgl(panel_config.width, panel_config.height, &panel);

    while (true)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
#elif USE_LVGL_PORT
    setup_lvgl_port(panel_config.width, panel_config.height, &panel);
#else
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "Main task running...");
    }
#endif
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

#ifndef USE_LVGL_PORT
    TaskHandle_t main_task_handle = NULL;
    xTaskCreate(main_task, "main_task", STACK_SIZE, NULL, TASK_PRIORITY, &main_task_handle);
#else
    main_task(NULL);
#endif
}