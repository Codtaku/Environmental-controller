// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==================================
// Blynk & Wi-Fi configuration
//==================================
#define BLYNK_AUTH_TOKEN "96ohEladMpK4GxtZLHcEi-JMs66vO_68" // Keep your actual token secure
// #define BLYNK_PRINT Serial // <-- REMOVE THIS LINE

// Preferred networks - Declare as extern
extern const char *ssid1;
extern const char *pass1;
extern const char *ssid2;
extern const char *pass2;

// Custom Blynk server info - Declare as extern
extern const char *blynkServer;
extern const uint16_t blynkPort; // Constants like uint16_t are usually okay in headers

// AP Mode Config - Declare as extern
extern const char *apSSID;
extern const char *apPass;

//==================================
// Blynk Virtual Pins
//==================================
#define VPIN_TEMP 0         // Temperature (DS18B20/DHT)
#define VPIN_HUMIDITY 1     // Humidity (DHT)
#define VPIN_MIN_TEMP_SET 2 // Min Temp Setting (Slider/Input)
#define VPIN_MAX_TEMP_SET 3 // Max Temp Setting (Slider/Input)
#define VPIN_SOIL_MOIST 5   // Soil Moisture (%)
#define VPIN_GAS_READING 6  // Gas Sensor Raw Reading
#define VPIN_TIMER_START 7  // Timer Mode Start/Reset Button
#define VPIN_MODE_SELECT 8  // Mode Selector (Segmented Switch)
#define VPIN_TIMER_IDX 9    // Timer Index Select (Slider/Step)
#define VPIN_RTC_TEMP 10    // DS3231 Internal Temperature
#define VPIN_TIMER_ADD 11   // Add Timer Entry Button
#define VPIN_TIMER_SAVE 12  // Save Timer Edits Button
#define VPIN_TIMER_RESET 13 // Reset Selected Timer Button
#define VPIN_TIMER_DEL 14   // Delete Last Timer Button
#define VPIN_TIMER_MIN 15   // Selected Timer Min Temp (Slider)
#define VPIN_TIMER_MAX 16   // Selected Timer Max Temp (Slider)
#define VPIN_GAS_LIMIT 17   // Gas Limit Setting (Slider)
#define VPIN_HUM_LIMIT 18   // Humidity Limit Setting (Slider)
#define VPIN_TIMER_DATE 20  // Timer Start Date Display
#define VPIN_TIMER_DAYS 21  // Timer Days Elapsed Display
#define VPIN_TIMER_COUNT 22 // Timer Entry Count Display
#define VPIN_RELAY1_LED 23  // Relay 1 Status LED
#define VPIN_RELAY2_LED 24  // Relay 2 Status LED
#define VPIN_RELAY3_LED 25  // Relay 3 Status LED
#define VPIN_RELAY4_LED 26  // Relay 4 Status LED
#define VPIN_HEARTBEAT 99   // Blynk Heartbeat/Uptime

//==================================
// Pin Definitions
//==================================
// Sensors
#define OneWire_BUS 17 // DS18B20 Temperature Sensor
#define DHT_BUS 4      // DHT22 Temperature/Humidity Sensor Data Pin
#define MQ135_BUS 34   // MQ135 Gas Sensor Analog Pin
#define SOIL_SENSOR 35 // Soil Moisture Sensor Analog Pin

// Actuators & Indicators
#define BUZZER_PIN 16
#define relay1 32 // Temperature relay 1 (e.g., Heating)
#define relay2 33 // Temperature relay 2 (e.g., Cooling/Fan)
#define relay3 25 // Humidity/Gas relay (e.g., Dehumidifier/Vent - active low)
#define relay4 26 // Humidity relay (e.g., Misting system - active low)

// Display
#define LCD_ADDRESS 0x27 // I2C Address of the LCD (Check yours, might be 0x3F)
#define LCD_HEIGHT 4
#define LCD_LENGTH 20
#define I2C_SDA 21 // Default ESP32 I2C SDA
#define I2C_SCL 22 // Default ESP32 I2C SCL

// Input
#define ENCODER_PIN_A 19 // Rotary Encoder CLK Pin
#define ENCODER_PIN_B 18 // Rotary Encoder DT Pin
#define ENCODER_BUTTON 5 // Rotary Encoder SW Pin

//==================================
// Sensor & Control Parameters
//==================================
#define DHT_TYPE DHT22 // Type of DHT sensor used

//==================================
// Timing Constants (milliseconds unless specified)
//==================================
const unsigned long DHT_READ_INTERVAL = 1000;         // Read DHT every 2 seconds
const unsigned long SCREEN2_TIMEOUT = 300000;         // 5 minutes for settings screen timeout
const unsigned long BLYNK_UPDATE_INTERVAL = 2000;     // Send data to Blynk every 10 seconds
const unsigned long LCD_CLEAR_INTERVAL = 1000;        // Interval for refreshing LCD lines (reduces flicker)
const unsigned long CONNECTION_CHECK_INTERVAL = 5000; // Check Blynk connection status every 5 seconds
const unsigned long NTP_SYNC_INTERVAL = 3600000;      // Sync RTC with NTP every 1 hour (3600 * 1000)
const unsigned long NOTIFY_INTERVAL = 900000;         // Send Blynk notifications at most every 30 minutes (30 * 60 * 1000)

//==================================
// EEPROM Configuration
//==================================
#define EEPROM_SIZE 512                 // Total EEPROM size to allocate
#define EEPROM_ADDR_MIN 0              // 4 bytes for min temp
#define EEPROM_ADDR_MAX 4              // 4 bytes for max temp
#define EEPROM_ADDR_DELAY 8            // 4 bytes for switching delay
#define EEPROM_ADDR_HUM_LIMIT 12       // 4 bytes for humidity limit
#define EEPROM_ADDR_TIMER_COUNT 16     // 1 byte for number of timer entries
#define EEPROM_ADDR_TIMER_START_DAY 17 // 4 bytes for timer start day (unix days)
#define EEPROM_ADDR_GAS_LIMIT 30       // 4 bytes for gas limit
#define EEPROM_ADDR_TIMERS 21          // Starting address for timer entries data
#define APP_MAX_TIMERS 30   // Maximum number of timer entries <-- NEW (Renamed)
#define TIMER_ENTRY_SIZE 6 // Size of each timer entry (2 bytes offset, 2 bytes min, 2 bytes max)
// #define TIMER_ENTRY_SIZE 6 // Size of each timer entry (2 bytes offset, 2 bytes min, 2 bytes max)

//==================================
// System Configuration
//==================================
#define ENCODER_TASK_STACK_SIZE 2048 // Stack size for the encoder reading task
#define ENCODER_TASK_PRIORITY 2      // Priority for the encoder task
#define ENCODER_TASK_CORE 1          // Core to run the encoder task on

#endif // CONFIG_H