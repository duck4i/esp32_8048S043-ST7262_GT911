#ifndef GT911_H
#define GT911_H

#include <driver/i2c.h>
#include <esp_err.h>

#define GT911_ADDR1 (uint8_t)0x5D
#define GT911_ADDR2 (uint8_t)0x14

#define ROTATION_LEFT (uint8_t)0
#define ROTATION_INVERTED (uint8_t)1
#define ROTATION_RIGHT (uint8_t)2
#define ROTATION_NORMAL (uint8_t)3

// Real-time command (Write only)
#define GT911_COMMAND (uint16_t)0x8040
#define GT911_ESD_CHECK (uint16_t)0x8041
#define GT911_COMMAND_CHECK (uint16_t)0x8046

// Configuration information (R/W)
#define GT911_CONFIG_START (uint16_t)0x8047
#define GT911_CONFIG_VERSION (uint16_t)0x8047
#define GT911_X_OUTPUT_MAX_LOW (uint16_t)0x8048
#define GT911_X_OUTPUT_MAX_HIGH (uint16_t)0x8049
#define GT911_Y_OUTPUT_MAX_LOW (uint16_t)0x804A
#define GT911_Y_OUTPUT_MAX_HIGH (uint16_t)0x804B
#define GT911_TOUCH_NUMBER (uint16_t)0x804C
#define GT911_CONFIG_CHKSUM (uint16_t)0X80FF
#define GT911_CONFIG_FRESH (uint16_t)0X8100
#define GT911_CONFIG_SIZE (uint16_t)0xFF - 0x46

// Coordinate information
#define GT911_PRODUCT_ID (uint16_t)0X8140
#define GT911_FIRMWARE_VERSION (uint16_t)0X8140
#define GT911_RESOLUTION (uint16_t)0X8140
#define GT911_VENDOR_ID (uint16_t)0X8140
#define GT911_IMFORMATION (uint16_t)0X8140
#define GT911_POINT_INFO (uint16_t)0X814E
#define GT911_POINT_1 (uint16_t)0X814F
#define GT911_POINT_2 (uint16_t)0X8157
#define GT911_POINT_3 (uint16_t)0X815F
#define GT911_POINT_4 (uint16_t)0X8167
#define GT911_POINT_5 (uint16_t)0X816F

// I2C configuration
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number
#define I2C_MASTER_FREQ_HZ 400000   // I2C master clock frequency
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_TIMEOUT_MS 1000

// Touch point structure
typedef struct
{
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t size;
} gt911_point_t;

// GT911 handle structure
typedef struct
{
    uint8_t addr;
    uint8_t pin_sda;
    uint8_t pin_scl;
    uint8_t pin_int;
    uint8_t pin_rst;
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
    uint8_t config_buf[GT911_CONFIG_SIZE];
    uint8_t is_large_detect;
    uint8_t touches;
    bool is_touched;
    gt911_point_t points[5];
    i2c_port_t i2c_port;
} gt911_handle_t;

// Function declarations
/**
 * @brief Initialize the GT911 touch controller.
 *
 * This function initializes the GT911 touch controller with the specified parameters,
 * including I2C pins, interrupt pin, reset pin, screen dimensions, I2C port, and device address.
 *
 * @param[out] dev       Pointer to the GT911 device handle to be initialized.
 * @param[in]  sda       GPIO number for the I2C SDA pin.
 * @param[in]  scl       GPIO number for the I2C SCL pin.
 * @param[in]  int_pin   GPIO number for the interrupt pin.
 * @param[in]  rst_pin   GPIO number for the reset pin.
 * @param[in]  width     Width of the touch screen in pixels.
 * @param[in]  height    Height of the touch screen in pixels.
 * @param[in]  i2c_port  I2C port number to be used (e.g., I2C_NUM_0 or I2C_NUM_1).
 * @param[in]  addr      I2C address of the GT911 device.
 *
 * @return
 *     - ESP_OK: Initialization successful.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments provided.
 *     - ESP_FAIL: Initialization failed due to other reasons.
 */
esp_err_t gt911_init(gt911_handle_t *dev, uint8_t sda, uint8_t scl, uint8_t int_pin, uint8_t rst_pin, uint16_t width, uint16_t height, i2c_port_t i2c_port, uint8_t addr);

/**
 * @brief Reset the GT911 touch controller.
 *
 * This function performs a hardware reset on the GT911 touch controller
 * by toggling the reset pin. It ensures the device is properly initialized
 * and ready for operation.
 *
 * @param[in] dev Pointer to the GT911 device handle.
 *
 * @return
 *     - ESP_OK: Reset was successful.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments provided.
 *     - ESP_FAIL: Reset failed due to an internal error.
 */
esp_err_t gt911_reset(gt911_handle_t *dev);

/**
 * @brief Set the screen rotation for the GT911 touch controller.
 *
 * @param dev Pointer to the GT911 device handle.
 * @param rot Rotation value (0-3) representing the desired screen orientation:
 *            - 0: No rotation (default orientation)
 *            - 1: 90 degrees clockwise
 *            - 2: 180 degrees clockwise
 *            - 3: 270 degrees clockwise
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_ERR_INVALID_ARG: Invalid arguments
 *     - ESP_FAIL: Other errors
 */
esp_err_t gt911_set_rotation(gt911_handle_t *dev, uint8_t rot);

/**
 * @brief Set the resolution of the GT911 touch panel.
 *
 * This function configures the resolution of the GT911 touch panel by setting
 * the width and height parameters. It updates the device's internal settings
 * to match the specified resolution.
 *
 * @param[in] dev Pointer to the GT911 device handle.
 * @param[in] width The desired width of the touch panel in pixels.
 * @param[in] height The desired height of the touch panel in pixels.
 *
 * @return
 *     - ESP_OK: Resolution was successfully set.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments were provided.
 *     - ESP_FAIL: Failed to set the resolution due to a communication error or other issue.
 */
esp_err_t gt911_set_resolution(gt911_handle_t *dev, uint16_t width, uint16_t height);

/**
 * @brief Reads data from the GT911 touch controller.
 *
 * @param[in] dev Pointer to the GT911 device handle.
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_ERR_INVALID_ARG: Invalid arguments
 *     - ESP_FAIL: Communication or other failure
 */
esp_err_t gt911_read(gt911_handle_t *dev);

/**
 * @brief Maps the touch coordinates from the GT911 touch controller to the screen coordinates.
 *
 * This function takes the raw touch coordinates reported by the GT911 touch controller
 * and maps them to the corresponding screen coordinates based on the screen's width
 * and height.
 *
 * @param[in] dev Pointer to the GT911 device handle.
 * @param[in] scr_width Width of the screen in pixels.
 * @param[in] scr_height Height of the screen in pixels.
 * @param[in] x Raw X coordinate from the touch controller.
 * @param[in] y Raw Y coordinate from the touch controller.
 * @param[out] mapped_x Pointer to store the mapped X coordinate on the screen.
 * @param[out] mapped_y Pointer to store the mapped Y coordinate on the screen.
 *
 * @return
 *     - ESP_OK: Mapping was successful.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments were provided.
 *     - Other error codes depending on the implementation.
 */
esp_err_t gt911_map_to_screen(gt911_handle_t *dev, int32_t scr_width, int32_t scr_height, int32_t x, int32_t y, int32_t *mapped_x, int32_t *mapped_y);

#endif // GT911_H