#include "display.h"

#include <stdio.h>
#include <string.h>

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "font3x5.h"

static const char *TAG = "DISPLAY";

/* Команды включения SSD1306: 128x32, горизонтальная адресация. */
static const uint8_t SSD1306_INIT[] = {
    0xAE, 0x20, 0x00, 0xB0, 0xC8, 0x00, 0x10, 0x40, 0x81, 0x7F,
    0xA1, 0xA6, 0xA8, 0x1F, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0xF1,
    0xDA, 0x02, 0xDB, 0x40, 0x8D, 0x14, 0xAF,
};

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_oled;

static const glyph3x5_t *font3x5_find(char symbol)
{
    for (size_t i = 0; i < FONT_3X5_COUNT; i++) {
        if (FONT_3X5[i].symbol == symbol) {
            return &FONT_3X5[i];
        }
    }
    return NULL;
}

/* Столбец знака для страницы дисплея: пять бит по вертикали. */
static uint8_t glyph_column(const glyph3x5_t *glyph, int column)
{
    uint8_t value = 0;
    for (int row = 0; row < 5; row++) {
        if (glyph->rows[row] & (1 << (2 - column))) {
            value |= (1 << row);
        }
    }
    return value;
}

static esp_err_t ssd1306_command(const uint8_t *command, size_t length)
{
    uint8_t buffer[32];
    buffer[0] = 0x00; /* Co = 0, D/C = 0: команда */
    memcpy(&buffer[1], command, length);
    return i2c_master_transmit(s_oled, buffer, length + 1, 100);
}

/* Очистка страницы перед выводом новых показаний. */
static esp_err_t display_clear_page(int page)
{
    uint8_t zeros[1 + 128] = {0x40};

    ESP_RETURN_ON_ERROR(ssd1306_command((const uint8_t[]){0xB0 | (page & 0x07)}, 1),
                        TAG, "выбор страницы");
    return i2c_master_transmit(s_oled, zeros, sizeof(zeros), 100);
}

/* Вывод строки знаками 3x5 в указанную страницу дисплея. */
static esp_err_t display_line(int page, const char *text)
{
    uint8_t buffer[1 + 128] = {0x40}; /* D/C = 1: данные */
    size_t length = 0;

    for (size_t i = 0; text[i] != '\0' && length + FONT_3X5_WIDTH <= 128; i++) {
        const glyph3x5_t *glyph = font3x5_find(text[i]);
        for (int column = 0; column < 3; column++) {
            buffer[1 + length++] = glyph ? glyph_column(glyph, column) : 0;
        }
        buffer[1 + length++] = 0; /* пробел между знаками */
    }

    ESP_RETURN_ON_ERROR(display_clear_page(page), TAG, "очистка страницы");
    ESP_RETURN_ON_ERROR(ssd1306_command((const uint8_t[]){0xB0 | (page & 0x07)}, 1),
                        TAG, "выбор страницы");
    return i2c_master_transmit(s_oled, buffer, length + 1, 100);
}

esp_err_t display_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = DISPLAY_SDA_GPIO,
        .scl_io_num = DISPLAY_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &s_bus), TAG, "шина I2C");

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DISPLAY_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus, &device_config, &s_oled),
                        TAG, "дисплей SSD1306");
    ESP_RETURN_ON_ERROR(ssd1306_command(SSD1306_INIT, sizeof(SSD1306_INIT)),
                        TAG, "включение дисплея");

    ESP_LOGI(TAG, "SSD1306 128x32 включён: SDA %d, SCL %d",
             DISPLAY_SDA_GPIO, DISPLAY_SCL_GPIO);
    return ESP_OK;
}

esp_err_t display_show(float temperature, float humidity, const char *status_code)
{
    char line_temperature[24];
    char line_humidity[24];

    snprintf(line_temperature, sizeof(line_temperature), "T %.1f C", temperature);
    snprintf(line_humidity, sizeof(line_humidity), "H %.1f %% %s",
             humidity, status_code ? status_code : "NA");

    ESP_RETURN_ON_ERROR(display_line(1, line_temperature), TAG, "строка температуры");
    ESP_RETURN_ON_ERROR(display_line(4, line_humidity), TAG, "строка влажности");

    ESP_LOGI(TAG, "%s | %s", line_temperature, line_humidity);
    return ESP_OK;
}
