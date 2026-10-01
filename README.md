# ESP32 Microclimate Monitor

Монитор микроклимата на ESP32-C3: читает температуру и влажность (DHT22),
считает точку росы и статус комфорта, показывает показания на OLED SSD1306
и отправляет их на сервер по Wi-Fi.

Репозиторий проекта: https://github.com/JekiEsto/esp32-microclimate-monitor

## Структура проекта

| Каталог | Содержание |
|---|---|
| `main/` | точка входа `app_main` и сборка приложения |
| `components/sensors/` | чтение датчика DHT22 |
| `components/climate/` | обработка показаний микроклимата |
| `components/display/` | вывод на OLED SSD1306 |
| `components/network/` | Wi-Fi и отправка показаний на сервер |
| `tools/` | проверка логики модулей на ПК |
| `docs/` | правила работы с репозиторием |

## Сборка и запуск

```sh
idf.py set-target esp32c3
idf.py build
idf.py -p COM3 flash monitor
```

## Команда

| Участник | Модуль | Ветка |
|---|---|---|
| Антонов М.А. | `components/network` | `feature/network` |
| Быковцев И.П. | `components/sensors`, интеграция | `feature/sensors` |
| Пирогов А.Е. | `components/display` | `feature/display` |
| Сотников Н.А. | `components/climate` | `feature/climate` |

## Проверка модулей без платы

```sh
python tools/simulate.py
```
