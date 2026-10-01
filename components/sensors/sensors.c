#include "sensors.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSORS";

/* Сколько раз пробуем прочитать датчик за один вызов. */
#define SENSORS_ATTEMPTS 3

static int s_gpio = -1;

/* Ожидание заданного уровня на линии с ограничением по времени. */
static bool dht_wait_level(int level, int timeout_us)
{
    for (int waited = 0; waited < timeout_us; waited++) {
        if (gpio_get_level(s_gpio) == level) {
            return true;
        }
        esp_rom_delay_us(1);
    }
    return false;
}

/* Одно чтение: возвращает ESP_OK, если контрольная сумма сошлась. */
static esp_err_t dht_read_once(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    gpio_set_level(s_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    gpio_set_level(s_gpio, 1);

    if (!dht_wait_level(0, 100) || !dht_wait_level(1, 100) || !dht_wait_level(0, 100)) {
        return ESP_ERR_TIMEOUT;
    }

    for (int bit = 0; bit < 40; bit++) {
        if (!dht_wait_level(1, 100)) {
            return ESP_ERR_TIMEOUT;
        }
        esp_rom_delay_us(35);
        data[bit / 8] = (data[bit / 8] << 1) | gpio_get_level(s_gpio);
        if (!dht_wait_level(0, 100)) {
            return ESP_ERR_TIMEOUT;
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    *humidity = (data[0] * 256 + data[1]) / 10.0f;
    *temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
    if (data[2] & 0x80) {
        *temperature = -*temperature;
    }
    return ESP_OK;
}

esp_err_t sensors_init(int gpio)
{
    s_gpio = gpio;

    gpio_config_t config = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    esp_err_t err = gpio_config(&config);
    ESP_LOGI(TAG, "DHT22 на GPIO%d: %s", gpio, esp_err_to_name(err));
    return err;
}

esp_err_t sensors_read(float *temperature, float *humidity)
{
    if (s_gpio < 0) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = ESP_FAIL;
    for (int attempt = 1; attempt <= SENSORS_ATTEMPTS; attempt++) {
        err = dht_read_once(temperature, humidity);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "T=%.1f C, H=%.1f %% (попытка %d)",
                     *temperature, *humidity, attempt);
            return ESP_OK;
        }
        ESP_LOGW(TAG, "Попытка %d из %d: %s", attempt, SENSORS_ATTEMPTS, esp_err_to_name(err));
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return err;
}
