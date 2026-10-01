#include "climate.h"

#include <math.h>
#include <string.h>

#include "esp_log.h"

static const char *TAG = "CLIMATE";
static climate_stats_t s_stats;

esp_err_t climate_init(void)
{
    memset(&s_stats, 0, sizeof(s_stats));
    s_stats.status = "нет данных";
    s_stats.status_code = "NA";
    return ESP_OK;
}

/* Точка росы по приближению Магнуса. */
static float climate_dew_point(float temperature, float humidity)
{
    const float a = 17.27f;
    const float b = 237.7f;
    float gamma = (a * temperature) / (b + temperature) + logf(humidity / 100.0f);
    return (b * gamma) / (a - gamma);
}

/* Статус комфорта по порогам из climate.h. */
static void climate_comfort(float temperature, float humidity)
{
    if (temperature < CLIMATE_T_MIN) {
        s_stats.status = "прохладно";
        s_stats.status_code = "COLD";
    } else if (temperature > CLIMATE_T_MAX) {
        s_stats.status = "жарко";
        s_stats.status_code = "HOT";
    } else if (humidity < CLIMATE_RH_MIN) {
        s_stats.status = "сухо";
        s_stats.status_code = "DRY";
    } else if (humidity > CLIMATE_RH_MAX) {
        s_stats.status = "влажно";
        s_stats.status_code = "WET";
    } else {
        s_stats.status = "комфортно";
        s_stats.status_code = "OK";
    }
}

/* Накопление минимума, максимума и среднего за сеанс наблюдения. */
static void climate_accumulate(float temperature)
{
    if (s_stats.samples == 0) {
        s_stats.t_min = temperature;
        s_stats.t_max = temperature;
    } else {
        if (temperature < s_stats.t_min) {
            s_stats.t_min = temperature;
        }
        if (temperature > s_stats.t_max) {
            s_stats.t_max = temperature;
        }
    }

    s_stats.t_avg = (s_stats.t_avg * s_stats.samples + temperature) / (s_stats.samples + 1);
    s_stats.samples++;
}

esp_err_t climate_update(float temperature, float humidity)
{
    s_stats.temperature = temperature;
    s_stats.humidity = humidity;
    s_stats.dew_point = climate_dew_point(temperature, humidity);
    climate_comfort(temperature, humidity);
    climate_accumulate(temperature);

    ESP_LOGI(TAG, "%s: T=%.1f C, H=%.1f %%, точка росы %.1f C, мин %.1f, макс %.1f, среднее %.1f",
             s_stats.status, temperature, humidity, s_stats.dew_point,
             s_stats.t_min, s_stats.t_max, s_stats.t_avg);
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
