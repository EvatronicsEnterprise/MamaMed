# MamaMed - Open-Source Maternal Vital Signs Telemetry Unit

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Hardware Platform](https://img.shields.io/badge/Hardware-ESP32-blue.svg)](https://www.espressif.com/)

MamaMed is an open-source, low-cost biomedical IoT telemetry hardware platform designed by **Evatronics Enterprise** to lower maternal mortality in low-resource and rural health centers.

## Key Features
- **Local 16x2 LCD Display**: Real-time on-screen display (I2C 16x2 LCD) allowing local community health workers to inspect patient heart rate, SpO2, blood pressure, and body temperature locally without requiring internet connectivity.
- **Hybrid Telemetry**: REST API cloud telemetry over Wi-Fi/Cellular with automatic GSM (SIM800L) fallback.
- **Automated Threshold Alert Engine**: Evaluates physiological trends locally at the edge and dispatches instant SMS alerts to remote physicians whenever high-risk thresholds (e.g., pre-eclampsia indicators or hypoxia) are detected.

## Hardware Components
- **Microcontroller**: ESP32-WROOM-32 / ESP32-S3
- **Local Display**: 16x2 Character LCD with I2C PCF8574 Adapter (SDA: GPIO 21, SCL: GPIO 22)
- **GSM Modem**: SIM800L Module (UART2 RX: GPIO 16, TX: GPIO 17)
- **Sensors**: MAX30102 Pulse Oximeter & Heart Rate Module, DS18B20 Temperature Sensor, NIBP Pressure Sensor Interface.

## Getting Started
1. Clone this repository.
2. Open in **VS Code with PlatformIO** or **Arduino IDE**.
3. Install required libraries: `LiquidCrystal_I2C`, `WiFi`, `HTTPClient`.
4. Configure Wi-Fi credentials and emergency contact numbers in `src/main.cpp`.
5. Flash to your target ESP32 board.

## License
Released under the [MIT License](LICENSE) by Evatronics Enterprise
