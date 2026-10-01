#include "climate.h"

#include <string.h>

static climate_stats_t s_stats;

esp_err_t climate_init(void)
{
    memset(&s_stats, 0, sizeof(s_stats));
    s_stats.status = "нет данных";
    s_stats.status_code = "NA";
    return ESP_OK;
}

esp_err_t climate_update(float temperature, float humidity)
{
    /* Каркас: расчёты выполняет ответственный за модуль обработки. */
    s_stats.temperature = temperature;
    s_stats.humidity = humidity;
    return ESP_OK;
}

const climate_stats_t *climate_get(void)
{
    return &s_stats;
}

const char *climate_status(void)
{
    return s_stats.status;
}

const char *climate_status_code(void)
{
    return s_stats.status_code;
}
