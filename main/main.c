#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "soc/gpio_num.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const char *TAG = "lcd";

static const uint16_t LCD_ADDR = 0x7C >> 1;
// static const uint16_t RGB_ADDR = 0x2D;

static const uint8_t display_init_commands[4] = {0x28, 0x0C, 0x01, 0x06};

void display_init(i2c_master_bus_handle_t * bus_handle, i2c_master_dev_handle_t * display_dev_handle) {
    const i2c_device_config_t display_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &display_dev_cfg, display_dev_handle));

    for (uint8_t i = 0; i < sizeof(display_init_commands); i++) {
        uint8_t command_word[2] = {0x00, 0x28};
        vTaskDelay(pdMS_TO_TICKS(50));
        ESP_ERROR_CHECK(i2c_master_transmit(*display_dev_handle, command_word, 2, 100));
    }
}

void backlight_init(i2c_master_bus_handle_t * bus_handle, i2c_master_dev_handle_t * backlight_dev_handle) {
    
}

void app_main(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = -1,
        .sda_io_num = GPIO_NUM_2,
        .scl_io_num = GPIO_NUM_1,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_master_dev_handle_t display_dev_handle;
    display_init(&bus_handle, &display_dev_handle);
}
