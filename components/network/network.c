#include "network.h"

#include "esp_log.h"

static const char *TAG = "NETWORK";

esp_err_t network_init(const char *ssid, const char *password)
{
    /* Каркас: подключение к сети реализует ответственный за модуль сети. */
    ESP_LOGI(TAG, "Каркас модуля сети, сеть %s, пароль из %u символов",
             ssid, (unsigned)strlen(password));
    return ESP_OK;
}

esp_err_t network_send(float temperature, float humidity)
{
    ESP_LOGI(TAG, "Каркас отправки: %.1f C, %.1f %%", temperature, humidity);
    return ESP_OK;
}
