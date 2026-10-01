#pragma once

#include "esp_err.h"

/* GPIO, к которому подключён вывод DATA датчика DHT22. */
#define SENSOR_GPIO 4

/**
 * @brief Настроить вывод датчика температуры и влажности.
 *
 * @param gpio номер GPIO вывода DATA
 * @return ESP_OK при успешной настройке
 */
esp_err_t sensors_init(int gpio);

/**
 * @brief Считать текущие показания датчика.
 *
 * @param[out] temperature температура, градусы Цельсия
 * @param[out] humidity    относительная влажность, проценты
 * @return ESP_OK, если данные получены и проверены
 */
esp_err_t sensors_read(float *temperature, float *humidity);
