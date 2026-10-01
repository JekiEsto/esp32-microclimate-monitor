#pragma once

#include "esp_err.h"

/* Пороги комфорта для жилого помещения. */
#define CLIMATE_T_MIN 20.0f
#define CLIMATE_T_MAX 25.0f
#define CLIMATE_RH_MIN 40.0f
#define CLIMATE_RH_MAX 60.0f

/* Накопленная статистика наблюдений. */
typedef struct {
    float temperature;      /* текущая температура, C */
    float humidity;         /* текущая влажность, % */
    float dew_point;        /* точка росы, C */
    float t_min;            /* минимум температуры, C */
    float t_max;            /* максимум температуры, C */
    float t_avg;            /* средняя температура, C */
    unsigned samples;       /* число обработанных показаний */
    const char *status;     /* статус для журнала (по-русски) */
    const char *status_code;/* короткий код статуса для дисплея */
} climate_stats_t;

/** @brief Подготовить модуль обработки показаний. */
esp_err_t climate_init(void);

/** @brief Обработать очередное показание датчика. */
esp_err_t climate_update(float temperature, float humidity);

/** @brief Текущая накопленная статистика. */
const climate_stats_t *climate_get(void);

/** @brief Статус комфорта для журнала. */
const char *climate_status(void);

/** @brief Короткий код статуса для дисплея: OK, COLD, HOT, DRY, WET, NA. */
const char *climate_status_code(void);
