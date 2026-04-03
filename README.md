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

Die IMU-/Touch-Defaults liegen in [head_track_config.h](/c:/Users/User/Documents/GitHub/head-track-mouse/src/mousemovement/head_track_config.h). Die effektive Runtime-Konfiguration der Module wird in [main.cpp](/c:/Users/User/Documents/GitHub/head-track-mouse/src/main.cpp) gesetzt.

## Verhalten

Beim Start oeffnet das Programm die serielle Schnittstelle mit `115200` Baud und initialisiert drei getrennte Input-Module:

- `MouseMovement`
  - liest die beiden IMUs
  - kalibriert ueber den Taster an Pin `1`
  - steuert die 16-Bit-Mausbewegung
  - der Touchsensor an Pin `2/3` wirkt wie "Maus anheben/absetzen"
- `MouseClick`
  - liest Sip/Puff an `A0`
  - `sip` haelt rechte Maustaste
  - `puff` haelt linke Maustaste
- `WasdControls`
  - liest den Analog-Joystick an `A1/A2`
  - erzeugt gehaltene `W/A/S/D`-Tasten
  - Diagonalen wie `WA` oder `SD` sind moeglich

Der Taster an Pin `1` startet nur die Kalibrierung. Er schaltet die Mausfunktion nicht ein oder aus.

## Architektur

Die Firmware ist modular aufgebaut. Jedes Hauptmodul folgt demselben Muster:

- `setup(...)`
  - uebergibt Pinbelegung und Laufzeitparameter
  - initialisiert interne Hardware- oder HID-Zustaende
- `process()`
  - wird in jedem `loop()`-Durchlauf aufgerufen
  - liest Eingaben
  - aktualisiert internen Zustand
  - sendet bei Bedarf HID-Ereignisse

Der Einstiegspunkt ist [main.cpp](/c:/Users/User/Documents/GitHub/head-track-mouse/src/main.cpp). Dort werden die Module instanziiert, konfiguriert und in `loop()` nacheinander verarbeitet.

### Module

- [MouseMovement](/c:/Users/User/Documents/GitHub/head-track-mouse/src/mousemovement/MouseMovement.h)
  - Wrapper um die Alpakka-inspirierte Head-Tracking-Implementierung
  - kapselt IMU-Initialisierung, Kalibrierung, Touch-Gating und Mausbewegung
- [MouseClick](/c:/Users/User/Documents/GitHub/head-track-mouse/src/mouseclick/MouseClick.h)
  - separates Modul fuer Sip/Puff-Mausklicks
- [WasdControls](/c:/Users/User/Documents/GitHub/head-track-mouse/src/wasdcontrols/WasdControls.h)
  - separates Modul fuer Joystick-zu-Tastatur-Mapping
- [logging.h](/c:/Users/User/Documents/GitHub/head-track-mouse/src/logging.h)
  - kleines compile-time Logging pro Komponente

### MouseMovement intern

Das `MouseMovement`-Modul ist selbst wieder in interne Bausteine zerlegt:

- `imu.*`
  - SPI-Zugriff auf beide IMUs
  - Kalibrierung und Sensorreadout
- `internal_gyro.*`
  - Alpakka-inspirierte Bewegungsberechnung
- `mouse_pipeline.*`
  - entkoppelte 16-Bit-Mauspipeline
- `touch_input.*`
  - Touchsensor fuer "Maus anheben/absetzen"
- `runtime_config.*`
  - uebergibt die in `main` gesetzte Konfiguration an interne Teile

### USB/HID

Die Firmware verwendet `USB_SERIAL_HID` als USB-Profil. Dadurch meldet sich das Geraet als:

- serielle Schnittstelle fuer Logs
- HID-Maus
- HID-Tastatur

Die 16-Bit-Mausbewegung wird ueber die minimalen Teensy-Core-Overrides im Projekt ermoeglicht:

- [usb_desc.c](/c:/Users/User/Documents/GitHub/head-track-mouse/teensy_core_override/teensy4/usb_desc.c)
- [usb_mouse.c](/c:/Users/User/Documents/GitHub/head-track-mouse/teensy_core_override/teensy4/usb_mouse.c)

## Build

```bash
platformio run
```

## Monitor

```bash
platformio device monitor
```
