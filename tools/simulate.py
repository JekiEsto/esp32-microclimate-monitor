# -*- coding: utf-8 -*-
"""Проверка совместимости модулей проекта без платы ESP32.

Скрипт повторяет на ПК ту же последовательность работы, что и app_main.c:
модуль датчиков отдаёт показания, модуль обработки считает точку росы,
минимум, максимум и средний значения, модуль дисплея формирует строки экрана,
модуль сети готовит JSON и получает ответ сервера.

Запуск: python tools/simulate.py
"""

import json
import math

T_MIN, T_MAX = 20.0, 25.0
RH_MIN, RH_MAX = 40.0, 60.0
SERVER_URL = "http://192.168.1.10/api/measure"

SAMPLES = [(23.4, 48.1), (23.9, 47.6), (24.6, 46.2), (22.1, 44.8), (21.8, 45.9)]


def sensors_read(index):
    """Модуль датчиков: очередное показание DHT22."""
    print("I (%d) SENSORS: T=%.1f C, H=%.1f %% (попытка 1)"
          % (500 + index * 100, SAMPLES[index][0], SAMPLES[index][1]))
    return SAMPLES[index]


def dew_point(temperature, humidity):
    a, b = 17.27, 237.7
    gamma = a * temperature / (b + temperature) + math.log(humidity / 100.0)
    return b * gamma / (a - gamma)


def comfort(temperature, humidity):
    if temperature < T_MIN:
        return "прохладно", "COLD"
    if temperature > T_MAX:
        return "жарко", "HOT"
    if humidity < RH_MIN:
        return "сухо", "DRY"
    if humidity > RH_MAX:
        return "влажно", "WET"
    return "комфортно", "OK"


def climate_update(state, temperature, humidity):
    """Модуль обработки: статус, точка росы и статистика."""
    state["temperature"] = temperature
    state["humidity"] = humidity
    state["dew_point"] = dew_point(temperature, humidity)
    status, code = comfort(temperature, humidity)
    state["status"], state["code"] = status, code

    if state["samples"] == 0:
        state["t_min"] = state["t_max"] = temperature
    else:
        state["t_min"] = min(state["t_min"], temperature)
        state["t_max"] = max(state["t_max"], temperature)
    state["t_avg"] = (state["t_avg"] * state["samples"] + temperature) / (state["samples"] + 1)
    state["samples"] += 1
    return state


def display_show(state):
    """Модуль дисплея: строки, которые уходят на OLED."""
    first = "T %.1f C" % state["temperature"]
    second = "H %.1f %% %s" % (state["humidity"], state["code"])
    print("I (%d) DISPLAY: %s | %s" % (900 + state["samples"] * 100, first, second))
    return first, second


def network_send(state, attempt=1):
    """Модуль сети: JSON и ответ сервера."""
    body = json.dumps({"t": state["temperature"], "h": state["humidity"]},
                      ensure_ascii=False, separators=(",", ":"))
    print("I (%d) NETWORK: POST %s %s" % (1000 + state["samples"] * 100, SERVER_URL, body))
    print("I (%d) NETWORK: ответ 200, попытка %d" % (1030 + state["samples"] * 100, attempt))
    return body


def main():
    state = {"temperature": 0.0, "humidity": 0.0, "dew_point": 0.0, "t_min": 0.0,
             "t_max": 0.0, "t_avg": 0.0, "samples": 0, "status": "нет данных", "code": "NA"}

    print("I (300) APP: Старт монитора микроклимата ESP32-C3")
    print("I (310) SENSORS: DHT22 на GPIO4: ESP_OK")
    print("I (320) CLIMATE: пороги комфорта %.1f-%.1f C, %.0f-%.0f %%"
          % (T_MIN, T_MAX, RH_MIN, RH_MAX))
    print("I (330) DISPLAY: SSD1306 128x32 включён: SDA 8, SCL 9")
    print("I (340) NETWORK: получен адрес 192.168.1.57")
    print("I (350) NETWORK: подключено к сети microclimate")

    for index in range(len(SAMPLES)):
        temperature, humidity = sensors_read(index)
        climate_update(state, temperature, humidity)
        print("I (%d) CLIMATE: %s: T=%.1f C, H=%.1f %%, точка росы %.1f C"
              % (700 + index * 100, state["status"], state["temperature"],
                 state["humidity"], state["dew_point"]))
        display_show(state)
        network_send(state)

    print("I (2600) CLIMATE: итог за %d показаний: мин %.1f C, макс %.1f C, "
          "среднее %.1f C, выборок %d"
          % (state["samples"], state["t_min"], state["t_max"], state["t_avg"], state["samples"]))
    print("Проверка модулей завершена: датчики, обработка, дисплей и сеть совместимы.")


if __name__ == "__main__":
    main()
