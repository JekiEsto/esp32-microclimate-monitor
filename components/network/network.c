#include "network.h"

#include <stdio.h>
#include <string.h>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

static const char *TAG = "NETWORK";

#define WIFI_CONNECTED_BIT BIT0
#define NETWORK_CONNECT_TIMEOUT_MS 15000
#define NETWORK_HTTP_TIMEOUT_MS 5000
#define NETWORK_SEND_ATTEMPTS 3

static EventGroupHandle_t s_events;

static void network_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Соединение потеряно, повторное подключение");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Получен адрес " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_events, WIFI_CONNECTED_BIT);
    }
}

esp_err_t network_init(const char *ssid, const char *password)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(err, TAG, "хранилище настроек");

    s_events = xEventGroupCreate();
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "сетевой стек");
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "цикл событий");
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&init_config), TAG, "инициализация Wi-Fi");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                   network_event_handler, NULL),
                        TAG, "обработчик событий Wi-Fi");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                   network_event_handler, NULL),
                        TAG, "обработчик получения адреса");

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "режим станции");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "параметры сети");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "запуск Wi-Fi");

    EventBits_t bits = xEventGroupWaitBits(s_events, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(NETWORK_CONNECT_TIMEOUT_MS));
    if ((bits & WIFI_CONNECTED_BIT) == 0) {
        ESP_LOGW(TAG, "Сеть %s не ответила за %d мс", ssid, NETWORK_CONNECT_TIMEOUT_MS);
        return ESP_ERR_TIMEOUT;
    }

    ESP_LOGI(TAG, "Подключено к сети %s", ssid);
    return ESP_OK;
}

esp_err_t network_send(float temperature, float humidity)
{
    char body[96];
    snprintf(body, sizeof(body), "{\"t\":%.1f,\"h\":%.1f}", temperature, humidity);

    esp_http_client_config_t http_config = {
        .url = NETWORK_SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = NETWORK_HTTP_TIMEOUT_MS,
    };
    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Не удалось создать HTTP-клиент");
        return ESP_ERR_NO_MEM;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, strlen(body));

    /* При обрыве связи показание отправляется повторно. */
    for (int attempt = 1; attempt <= NETWORK_SEND_ATTEMPTS; attempt++) {
        esp_err_t err = esp_http_client_perform(client);
        int status = esp_http_client_get_status_code(client);
        if (err == ESP_OK && status == 200) {
            ESP_LOGI(TAG, "Отправлено %s, ответ %d (попытка %d)", body, status, attempt);
            esp_http_client_cleanup(client);
            return ESP_OK;
        }
        ESP_LOGW(TAG, "Попытка %d из %d: %s, ответ %d",
                 attempt, NETWORK_SEND_ATTEMPTS, esp_err_to_name(err), status);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    esp_http_client_cleanup(client);
    return ESP_FAIL;
}
