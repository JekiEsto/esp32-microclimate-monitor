#include "display.h"

#include "esp_log.h"

static const char *TAG = "DISPLAY";

esp_err_t display_init(void)
{
    ESP_LOGI(TAG, "Каркас модуля дисплея, SSD1306 по адресу 0x%02X", DISPLAY_I2C_ADDR);
    return ESP_OK;
}

esp_err_t display_show(float temperature, float humidity, const char *status_code)
{
    /* Каркас: вывод на OLED реализует ответственный за модуль дисплея. */
    ESP_LOGI(TAG, "Показания: %.1f C, %.1f %%, статус %s",
             temperature, humidity, status_code);
    return ESP_OK;
}
