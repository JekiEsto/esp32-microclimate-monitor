#pragma once

#include "esp_err.h"

/* Адрес SSD1306 на шине I2C и выводы платы. */
#define DISPLAY_I2C_ADDR 0x3C
#define DISPLAY_SDA_GPIO 8
#define DISPLAY_SCL_GPIO 9

/** @brief Поднять шину I2C и включить дисплей. */
esp_err_t display_init(void);

/**
 * @brief Показать показания на OLED.
 *
 * @param temperature температура, C
 * @param humidity    влажность, %
 * @param status_code короткий код статуса комфорта
 */
esp_err_t display_show(float temperature, float humidity, const char *status_code);
