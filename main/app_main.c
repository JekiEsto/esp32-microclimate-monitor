#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "climate.h"
#include "display.h"
#include "network.h"
#include "sensors.h"

static const char *TAG = "APP";

/* Период опроса датчика и обновления показаний, мс. */
#define APP_PERIOD_MS 5000

void app_main(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;

    ESP_LOGI(TAG, "Старт монитора микроклимата ESP32-C3");

    ESP_ERROR_CHECK(sensors_init(SENSOR_GPIO));
    ESP_ERROR_CHECK(climate_init());
    ESP_ERROR_CHECK(display_init());
    ESP_ERROR_CHECK(network_init(NETWORK_WIFI_SSID, NETWORK_WIFI_PASSWORD));

    while (true) {
        sensors_read(&temperature, &humidity);

        /* Отчёт по микроклимату */
        climate_update(temperature, humidity);

        display_show(temperature, humidity, climate_status_code());
        vTaskDelay(pdMS_TO_TICKS(APP_PERIOD_MS));
    }
}
