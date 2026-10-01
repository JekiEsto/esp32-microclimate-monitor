#include "sensors.h"

#include "esp_log.h"

static const char *TAG = "SENSORS";
static int s_gpio = -1;

esp_err_t sensors_init(int gpio)
{
    s_gpio = gpio;
    ESP_LOGI(TAG, "Каркас модуля датчиков, вывод DATA на GPIO%d", gpio);
    return ESP_OK;
}

esp_err_t sensors_read(float *temperature, float *humidity)
{
    /* Каркас: обмен с DHT22 реализует ответственный за модуль датчиков. */
    *temperature = 0.0f;
    *humidity = 0.0f;
    return s_gpio < 0 ? ESP_ERR_INVALID_STATE : ESP_OK;
}
