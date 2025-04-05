#include "gt911.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "GT911"

static bool gt911_gpio_is_valid(uint8_t pin)
{
    return (pin >= GPIO_NUM_0 && pin <= GPIO_NUM_MAX);
}

static void gt911_safe_set_pin_direction(uint8_t pin, gpio_mode_t mode)
{
    if (gt911_gpio_is_valid(pin))
    {
        gpio_set_direction(pin, mode);
    }
}

static void gt911_safe_set_pin_level(uint8_t pin, uint8_t level)
{
    if (gt911_gpio_is_valid(pin))
    {
        gpio_set_level(pin, level);
    }
}

// Helper functions for I2C communication
static esp_err_t gt911_i2c_init(i2c_port_t i2c_port, uint8_t sda, uint8_t scl)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ};

    esp_err_t ret = i2c_param_config(i2c_port, &conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C parameter configuration failed");
        return ret;
    }

    ret = i2c_driver_install(i2c_port, I2C_MODE_MASTER,
                             I2C_MASTER_RX_BUF_DISABLE,
                             I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C driver installation failed");
        return ret;
    }

    return ESP_OK;
}

static esp_err_t gt911_write_byte(gt911_handle_t *dev, uint16_t reg, uint8_t val)
{
    esp_err_t ret;
    uint8_t buffer[3];

    buffer[0] = (reg >> 8) & 0xFF; // Register address high byte
    buffer[1] = reg & 0xFF;        // Register address low byte
    buffer[2] = val;               // Value to write

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, 3, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t gt911_read_byte(gt911_handle_t *dev, uint16_t reg, uint8_t *val)
{
    esp_err_t ret;
    uint8_t buffer[2];

    buffer[0] = (reg >> 8) & 0xFF; // Register address high byte
    buffer[1] = reg & 0xFF;        // Register address low byte

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, 2, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set read register address");
        return ret;
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, val, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t gt911_write_block(gt911_handle_t *dev, uint16_t reg, uint8_t *val, uint8_t size)
{
    esp_err_t ret;
    uint8_t buffer[2];

    buffer[0] = (reg >> 8) & 0xFF; // Register address high byte
    buffer[1] = reg & 0xFF;        // Register address low byte

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, 2, true);
    i2c_master_write(cmd, val, size, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t gt911_read_block(gt911_handle_t *dev, uint16_t reg, uint8_t *buf, uint8_t size)
{
    esp_err_t ret;
    uint8_t buffer[2];

    buffer[0] = (reg >> 8) & 0xFF; // Register address high byte
    buffer[1] = reg & 0xFF;        // Register address low byte

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer, 2, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set read block register address");
        return ret;
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_READ, true);
    if (size > 1)
    {
        i2c_master_read(cmd, buf, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buf + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->i2c_port, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

// Function to calculate checksum for configuration
static void gt911_calculate_checksum(gt911_handle_t *dev)
{
    uint8_t checksum = 0;

    for (uint8_t i = 0; i < GT911_CONFIG_SIZE; i++)
    {
        checksum += dev->config_buf[i];
    }

    checksum = (~checksum) + 1;
    dev->config_buf[GT911_CONFIG_CHKSUM - GT911_CONFIG_START] = checksum;
}

// Function to refresh the configuration
static esp_err_t gt911_reflash_config(gt911_handle_t *dev)
{
    esp_err_t ret;

    gt911_calculate_checksum(dev);

    ret = gt911_write_byte(dev, GT911_CONFIG_CHKSUM,
                           dev->config_buf[GT911_CONFIG_CHKSUM - GT911_CONFIG_START]);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write checksum");
        return ret;
    }

    ret = gt911_write_byte(dev, GT911_CONFIG_FRESH, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to refresh configuration");
        return ret;
    }

    return ESP_OK;
}

// Function to read a touch point from data buffer
static gt911_point_t gt911_read_point(gt911_handle_t *dev, uint8_t *data)
{
    gt911_point_t point;
    uint16_t temp;

    point.id = data[0];
    point.x = data[1] + (data[2] << 8);
    point.y = data[3] + (data[4] << 8);
    point.size = data[5] + (data[6] << 8);

    switch (dev->rotation)
    {
    case ROTATION_NORMAL:
        point.x = dev->width - point.x;
        point.y = dev->height - point.y;
        break;
    case ROTATION_LEFT:
        temp = point.x;
        point.x = dev->width - point.y;
        point.y = temp;
        break;
    case ROTATION_INVERTED:
        // No change needed
        break;
    case ROTATION_RIGHT:
        temp = point.x;
        point.x = point.y;
        point.y = dev->height - temp;
        break;
    }

    return point;
}

// Public functions
esp_err_t gt911_init(gt911_handle_t *dev, uint8_t sda, uint8_t scl, uint8_t int_pin, uint8_t rst_pin,
                     uint16_t width, uint16_t height, i2c_port_t i2c_port, uint8_t addr)
{
    esp_err_t ret;

    // Initialize structure members
    dev->addr = addr;
    dev->pin_sda = sda;
    dev->pin_scl = scl;
    dev->pin_int = int_pin;
    dev->pin_rst = rst_pin;
    dev->width = width;
    dev->height = height;
    dev->rotation = ROTATION_NORMAL;
    dev->i2c_port = i2c_port;
    dev->is_touched = false;
    dev->touches = 0;

    // Initialize I2C
    ret = gt911_i2c_init(i2c_port, sda, scl);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize I2C");
        return ret;
    }

    // Reset the device
    ret = gt911_reset(dev);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }

    return ESP_OK;
}

esp_err_t gt911_reset(gt911_handle_t *dev)
{
    esp_err_t ret;

    // Configure GPIO pins
    gpio_pad_select_gpio(dev->pin_int);
    gpio_pad_select_gpio(dev->pin_rst);

    gt911_safe_set_pin_direction(dev->pin_int, GPIO_MODE_OUTPUT);
    gt911_safe_set_pin_direction(dev->pin_rst, GPIO_MODE_OUTPUT);

    // Reset sequence
    gt911_safe_set_pin_level(dev->pin_int, 0);
    gt911_safe_set_pin_level(dev->pin_rst, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    gt911_safe_set_pin_level(dev->pin_int, dev->addr == GT911_ADDR2);
    vTaskDelay(1 / portTICK_PERIOD_MS);

    gt911_safe_set_pin_level(dev->pin_rst, 1);
    vTaskDelay(5 / portTICK_PERIOD_MS);

    gt911_safe_set_pin_level(dev->pin_int, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Set INT pin to input mode for touch detection
    gt911_safe_set_pin_direction(dev->pin_int, GPIO_MODE_INPUT);
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Read configuration from GT911
    ret = gt911_read_block(dev, GT911_CONFIG_START, dev->config_buf, GT911_CONFIG_SIZE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read configuration data");
        return ret;
    }

    // Set resolution
    ret = gt911_set_resolution(dev, dev->width, dev->height);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set resolution");
        return ret;
    }

    return ESP_OK;
}

esp_err_t gt911_set_rotation(gt911_handle_t *dev, uint8_t rot)
{
    if (rot > ROTATION_NORMAL)
    {
        ESP_LOGE(TAG, "Invalid rotation value");
        return ESP_ERR_INVALID_ARG;
    }

    dev->rotation = rot;
    return ESP_OK;
}

esp_err_t gt911_set_resolution(gt911_handle_t *dev, uint16_t width, uint16_t height)
{
    dev->width = width;
    dev->height = height;

    // Update configuration buffer with new resolution
    dev->config_buf[GT911_X_OUTPUT_MAX_LOW - GT911_CONFIG_START] = width & 0xFF;
    dev->config_buf[GT911_X_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = (width >> 8) & 0xFF;
    dev->config_buf[GT911_Y_OUTPUT_MAX_LOW - GT911_CONFIG_START] = height & 0xFF;
    dev->config_buf[GT911_Y_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = (height >> 8) & 0xFF;

    // Reflash configuration
    return gt911_reflash_config(dev);
}

esp_err_t gt911_read(gt911_handle_t *dev)
{
    esp_err_t ret;
    uint8_t data[7];
    uint8_t point_info;

    // Read the point info register
    ret = gt911_read_byte(dev, GT911_POINT_INFO, &point_info);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read point info");
        return ret;
    }

    uint8_t buffer_status = (point_info >> 7) & 1;
    uint8_t proximity_valid = (point_info >> 5) & 1;
    uint8_t have_key = (point_info >> 4) & 1;
    dev->is_large_detect = (point_info >> 6) & 1;
    dev->touches = point_info & 0xF;
    dev->is_touched = dev->touches > 0;

    ESP_LOGD(TAG, "Buffer status: %d, Large detect: %d, Proximity: %d, Key: %d, Touches: %d",
             buffer_status, dev->is_large_detect, proximity_valid, have_key, dev->touches);

    // Read touch points if buffer has valid data and there are touches detected
    if (buffer_status == 1 && dev->is_touched)
    {
        for (uint8_t i = 0; i < dev->touches && i < 5; i++)
        {
            ret = gt911_read_block(dev, GT911_POINT_1 + i * 8, data, 7);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to read point %d data", i);
                return ret;
            }

            dev->points[i] = gt911_read_point(dev, data);
            ESP_LOGD(TAG, "Touch %d: ID=%d, X=%d, Y=%d, Size=%d",
                     i, dev->points[i].id, dev->points[i].x, dev->points[i].y, dev->points[i].size);
        }
    }

    // Clear the point info register
    ret = gt911_write_byte(dev, GT911_POINT_INFO, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear point info register");
        return ret;
    }

    return ESP_OK;
}

esp_err_t gt911_map_to_screen(gt911_handle_t *dev, int32_t scr_width, int32_t scr_height, int32_t x, int32_t y, int32_t *mapped_x, int32_t *mapped_y)
{
    if (dev == NULL || mapped_x == NULL || mapped_y == NULL)
    {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->width == 0 || dev->height == 0)
    {
        ESP_LOGE(TAG, "Invalid device dimensions: width or height is zero");
        return ESP_ERR_INVALID_ARG;
    }

    *mapped_x = x * scr_width / dev->width;
    *mapped_y = y * scr_height / dev->height;

    return ESP_OK;
}