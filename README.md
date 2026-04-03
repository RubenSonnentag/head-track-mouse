# head-track-mouse

Head-Tracking-Maus fuer einen Teensy 4.1 mit zwei IMUs.

## Pinbelegung

| Funktion | Arduino Bezeichnung | Board-Beschriftung (Teensy 4.1) | Hinweis |
| --- | --- | --- | --- |
| **CS IMU 1** | `10` | 10 | SPI Chip Select Sensor 1 |
| **CS IMU 2** | `9` | 9 | SPI Chip Select Sensor 2 |
| **MOSI** | `11` | SDA | SPI Daten |
| **MISO** | `12` | 12 | SPI Daten |
| **SCK** | `13` | SCL | SPI Clock |
| **Touch Send** | `2` | 2 | Sendepin Touchsensor |
| **Touch Receive** | `3` | 3 | Empfang + Touchflaeche |
| **Sip/Puff** | `A0` | **14** | Analog Eingang |
| **Joystick X** | `A1` | **15** | Analog Eingang |
| **Joystick Y** | `A2` | **16** | Analog Eingang |

Die im Code fest verdrahteten Pins stehen in [src/head_track_config.h](/c:/Users/User/Documents/GitHub/head-track-mouse/src/head_track_config.h).

## Verhalten

Beim Start oeffnet das Programm die serielle Schnittstelle mit `115200` Baud, initialisiert die IMUs und wartet auf den Taster an `A2`.

Mit einem Tastendruck wird kalibriert und das Head-Tracking aktiviert. Ein weiterer Tastendruck pausiert die Mausausgabe.

## Build

```bash
platformio run
```

## Monitor

```bash
platformio device monitor
```
