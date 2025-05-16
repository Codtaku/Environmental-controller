# ESP32 Environmental Controller

**Version:** 1.1
**Date:** 2025-04-21
**Author:** Vuong Duy (Codtaku)

## Overview

This project uses an ESP32 microcontroller to create a versatile environmental control system suitable for applications like incubators, greenhouses, or terrariums. It monitors key environmental parameters and controls actuators based on user-defined settings and modes.

## Features

* **Sensor Monitoring:**
    * Temperature: DS18B20 (Primary), DHT22 (Backup)
    * Humidity: DHT22
    * Air Quality: MQ135 Gas Sensor
    * Soil Moisture: Capacitive Sensor
* **Control:**
    * 4 Relays for heating, cooling, dehumidifying, misting (configurable)
    * Multiple Modes: AUTO, ON, OFF, HALF, TIMER
* **User Interface:**
    * 20x4 I2C LCD Display
    * Rotary Encoder with push-button for navigation and settings
* **Connectivity:**
    * WiFi connection to local network
    * Blynk integration for remote monitoring and control via mobile app
    * (Optional) Local AP mode with a simple web interface
* **Data Persistence:** Settings saved to EEPROM
* **Timekeeping:** DS3231 RTC for accurate time and TIMER mode scheduling

## Hardware Requirements

* ESP32 Development Board
* Sensors: DS18B20, DHT22, MQ135, Soil Moisture Sensor
* Actuators: 4-Channel Relay Module, Buzzer
* Display: 20x4 I2C LCD (check address, usually 0x27 or 0x3F)
* Input: Rotary Encoder with Button
* Time: DS3231 RTC Module
* Power Supply (appropriate for ESP32 and relays)
* Wiring components (breadboard, jumper wires, etc.)

## Software Setup

* **IDE:** Arduino IDE or PlatformIO (recommended)
* **Libraries:** (List libraries mentioned in `main.cpp`)
    * Blynk by Volodymyr Shymanskyy
    * OneWire by Paul Stoffregen
    * DallasTemperature by Miles Burton
    * DHT sensor library by Adafruit
    * LiquidCrystal_I2C by Frank de Brabander
    * RTClib by Adafruit
    * WebServer (ESP32 built-in)
    * EEPROM (ESP32 built-in)
    * Wire (ESP32 built-in)
    * FS/SPIFFS (ESP32 built-in)
* **Configuration:**
    * Update WiFi credentials (`ssid1`/`pass1`, `ssid2`/`pass2`) in `src/main.cpp`.
    * Update Blynk Auth Token (`BLYNK_AUTH_TOKEN`) in `src/main.cpp`.
    * Configure Blynk App with necessary widgets (Virtual Pins V0-V3, V5-V26, V99).
    * (If using AP Mode) Upload `src/data/index.html` to ESP32 SPIFFS as `/index.html`.

## Usage

* On startup, the device attempts to connect to WiFi and Blynk.
* The LCD displays current sensor readings, relay status, and mode.
* Use the rotary encoder button to enter the settings menu.
* Turn the encoder to navigate the menu or change values.
* Press the button to select/edit or save/go back.
* Hold the button for 5 seconds to restart the device.
* Use the Blynk app for remote monitoring and control.

## Development Roadmap

### Phase 1: Firmware Foundation & Refinement (ESP32)

**Goal:** Create a stable, reliable, and well-organized base firmware on the ESP32.
**Tasks:**
* **Code Cleanup & Modularization:** Refactor your existing main.cpp. Break down large functions (like loop, setup) into smaller, focused functions (e.g., read_sensors(), update_lcd(), check_controls(), handle_encoder()). Add clear comments explaining each part.
* **Integrate Optimizations:** Ensure the optimized relay sync and less frequent NTP sync logic discussed earlier are implemented and working correctly.
* **Robust Error Handling:** Add checks for invalid sensor readings (e.g., DS18B20 returning -127, DHT returning NaN). Decide how to handle failures (e.g., log an error, use a backup value, trigger an alert state, display error on LCD). Check rtc.isrunning() before relying on RTC time.
* **PID Temperature Control:** Implement PID logic for controlling the primary heating/cooling relays (Relay 1 & 2). This requires tuning but provides much more stable temperature control than simple thresholds. This is a significant upgrade for poultry.
* **Watchdog Timer:** Implement the ESP32's hardware watchdog timer to automatically reset the device if the code hangs.
* **Configuration:** Consider how settings (WiFi, thresholds) will be managed. Initially, they can remain hardcoded or loaded from EEPROM, but think about easier ways for the future (WiFiManager library, config file on SPIFFS).

### Phase 2: Basic Network Communication & Web Interface

**Goal:** Enable the ESP32 to communicate reliably over the local network and create a functional web interface for basic monitoring and control (laying groundwork for the app).
**Tasks:**
* **Choose Communication Protocol:** Select how the ESP32 will talk to your future backend/app. MQTT is highly recommended for IoT. Alternatively, use HTTP requests or WebSockets. Implement the chosen client library on the ESP32.
* **ESP32 Network Interface:** Modify the ESP32 code to publish sensor data and status regularly via MQTT (or respond to HTTP requests). Add functionality to receive commands (like changing mode, adjusting setpoints) via the chosen protocol.
* **Develop Basic Web UI:** Create a new web interface (HTML, CSS, JavaScript) – this will eventually replace the simple index.html. Host it initially on your computer or a Raspberry Pi on the same network. This UI should:
    * Connect to the MQTT broker (using MQTT over WebSockets) or make HTTP requests to the ESP32.
    * Display current sensor readings and device status.
    * Allow basic control (e.g., change mode, turn relays on/off manually for testing).

### Phase 3: Backend Server & Custom Mobile App

**Goal:** Build the core server application and the mobile app to replace Blynk and provide tailored functionality.
**Tasks:**
* **Backend Development:**
    * Set up a server (e.g., using Python/Flask, Node.js/Express, on a Raspberry Pi, VPS, or cloud platform).
    * Implement the server-side MQTT broker (or handle communication if using HTTP/WebSockets).
    * Set up a database (e.g., InfluxDB for time-series data, PostgreSQL, MySQL) to store historical sensor readings and user settings.
    * Develop API endpoints for the mobile app and web UI to fetch data and send commands.
    * Implement user authentication if needed.
* **Custom Mobile App:**
    * Develop the app (iOS/Android using native tools or cross-platform like Flutter/React Native).
    * Connect the app to your backend server API.
    * Display real-time sensor data and device status.
    * Show historical data using charts (requires backend database).
    * Implement controls for all settings (mode, temperature, humidity, schedules, etc.).
    * Implement push notifications for alerts triggered by the backend.
* **Data Logging:** Ensure the ESP32 (or backend via MQTT) reliably logs data to the backend database.

### Phase 4: Advanced Features & OTA Updates

**Goal:** Integrate Over-The-Air updates and chicken-specific control logic.
**Tasks:**
* **OTA Firmware Updates:** Implement OTA updates for the ESP32 firmware (e.g., using ArduinoOTA library for local updates, or a more robust HTTP update mechanism managed by your backend/web UI). This is essential for deploying updates without physical access.
* **Chicken-Specific Logic (Firmware & App/UI):**
    * Implement Ventilation Control logic on the ESP32 based on humidity and gas readings, configurable via the App/UI.
    * Implement Lighting Schedule Control on the ESP32 using the RTC, with schedules configurable via the App/UI.
    * Implement Age-Based Profiles within the App/UI, allowing users to save and load sets of parameters.
* **Enhanced Web UI:** Improve the web UI to match the features of the mobile app (graphing, advanced configuration).
* **Alerting System:** Refine the alert logic on the backend server to trigger app push notifications based on rules configured by the user in the app.

### Phase 5: AI Integration (Cloud-Based First)

**Goal:** Leverage the collected data and backend infrastructure to add intelligence.
**Tasks:**
* **Data Analysis:** Ensure you have sufficient historical data logged in your database.
* **Backend AI Models:** Develop and train models on your server:
    * Anomaly Detection: Identify unusual sensor patterns that deviate from normal operation.
    * Prediction: Forecast near-term temperature/humidity.
* **Integration:**
    * Use anomaly detection results to trigger alerts in your app.
    * Display predictions in the App/UI.
    * (Advanced) Use predictions to proactively send commands to the ESP32 to adjust heating/ventilation ahead of time.

### Phase 6: Future Enhancements (Optional)

**Goal:** Explore more complex AI or hardware additions.
**Tasks:**
* **Edge AI:** If specific on-device processing is needed (e.g., audio analysis for cough detection), investigate TinyML deployment on the ESP32.
* **Hardware Additions:** Add and integrate cameras (for computer vision AI), microphones (for audio AI), specific gas sensors (Ammonia), feed/water level sensors, SD card module.
* **MQTT for Home Automation:** If desired, ensure the ESP32's MQTT implementation is compatible with platforms like Home Assistant.
