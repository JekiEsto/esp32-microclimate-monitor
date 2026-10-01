#pragma once

#include "esp_err.h"

/* Параметры сети и адрес сервера сбора показаний. */
#define NETWORK_WIFI_SSID "microclimate"
#define NETWORK_WIFI_PASSWORD "microclimate"
#define NETWORK_SERVER_URL "http://192.168.1.10/api/measure"

/**
 * @brief Подключить плату к Wi-Fi в режиме станции.
 *
 * @param ssid     имя сети
 * @param password пароль сети
 * @return ESP_OK, если адрес получен
 */
esp_err_t network_init(const char *ssid, const char *password);

/**
 * @brief Отправить показания на сервер.
 *
 * @param temperature температура, C
 * @param humidity    влажность, %
 */
esp_err_t network_send(float temperature, float humidity);
