#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "soc/gpio_num.h"

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/_intsup.h>

static const char *TAG = "LCD";

static const uint16_t LCD_ADDR = 0x7C >> 1;
static const uint16_t RGB_ADDR = 0x2D;
static const uint32_t SCL_SPEED_HZ = 100000;

static const uint8_t display_init_commands[4] = {0x28, 0x0C, 0x01, 0x06};
static const uint8_t backlight_init_color[3] = {126, 12, 0};

static const i2c_device_config_t display_dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = LCD_ADDR,
    .scl_speed_hz = SCL_SPEED_HZ,
};

static const i2c_device_config_t backlight_dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = RGB_ADDR,
    .scl_speed_hz = SCL_SPEED_HZ,
};

static const i2c_master_bus_config_t bus_config = {
    .i2c_port = -1,
    .sda_io_num = GPIO_NUM_2,
    .scl_io_num = GPIO_NUM_1,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};

void display_init(i2c_master_bus_handle_t * bus_handle, i2c_master_dev_handle_t * display_dev_handle) {
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &display_dev_cfg, display_dev_handle));

    for (uint8_t i = 0; i < sizeof(display_init_commands); i++) {
        uint8_t command_word[2] = {0x00, display_init_commands[i]};
        vTaskDelay(pdMS_TO_TICKS(20));
        ESP_ERROR_CHECK(i2c_master_transmit(*display_dev_handle, command_word, 2, 100));
    }
}

void backlight_init(i2c_master_bus_handle_t * bus_handle, i2c_master_dev_handle_t * backlight_dev_handle) {
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &backlight_dev_cfg, backlight_dev_handle));

    for (uint8_t i = 1; i < 4; i++) {
        const uint8_t command_word[2] = {i, backlight_init_color[i - 1]};
        vTaskDelay(pdMS_TO_TICKS(20));
        ESP_ERROR_CHECK(i2c_master_transmit(*backlight_dev_handle, command_word, 2, 100));
    }
}

void display_print_character(i2c_master_dev_handle_t * display_dev_handle, char character, uint8_t posY, uint8_t posX) {
    const uint8_t position = 0x80 + (posY << 6) + posX;
    const uint8_t display_char_sequence[4] = {0x80, position, 0x40, (uint8_t)character};
    ESP_ERROR_CHECK(i2c_master_transmit(*display_dev_handle, display_char_sequence, 4, 100));
}

void display_print_string_at_start(i2c_master_dev_handle_t * display_dev_handle, const char * str) {
    for (uint8_t i = 0; i < strlen(str); i++) {
        display_print_character(display_dev_handle, str[i], 0, i);
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_master_dev_handle_t display_dev_handle;
    display_init(&bus_handle, &display_dev_handle);

    i2c_master_dev_handle_t backlight_dev_handle;
    backlight_init(&bus_handle, &backlight_dev_handle);

    vTaskDelay(pdMS_TO_TICKS(20));
    display_print_string_at_start(&display_dev_handle, "Hello world!");
}
