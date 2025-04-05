# GT911 driver for the Cheap Yellow Display 

A simple driver for GT911 without external dependencies based on TAMC_GT911 project.

## Example usage

```c
#include <gt911.h>

#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST 38
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 480
#define TOUCH_MAP_Y1 272

#define SCREEN_W 800
#define SCREEN_H 480

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

void read(void)
{
    init_touch();

    if (gt911_read(&gt911_dev) == ESP_OK)
    {
        bool touched = gt911_dev.is_touched;

        int32_t touch_last_x = 0;
        int32_t touch_last_y = 0;

        gt911_map_to_screen(&gt911_dev, SCREEN_W, SCREEN_H, gt911_dev.points[0].x, gt911_dev.points[0].y, &touch_last_x, &touch_last_y);

        // .... use the touch_last_x and y
    }
}

```