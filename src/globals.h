#ifndef GLOBALS_H
#define GLOBALS_H

#include "config.h" // Include configuration constants
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <RTClib.h>
#include <WebServer.h> // For WebServer object if AP mode is used globally
#include <WiFi.h>      // Needed for IPAddress type if used, String is simpler here
#include <WString.h>   // Include for String type if not implicitly included

//==================================
// External Peripheral Instances
//==================================
extern LiquidCrystal_I2C screen;
extern OneWire oneWire;
extern DallasTemperature ds18b20;
extern DHT dht;
extern RTC_DS3231 rtc;
extern WebServer server; // Declare WebServer if AP mode is globally managed

//==================================
// Global State Variables
//==================================
extern volatile int mod;                      // Control Mode: 0:AUTO, 1:OFF, 2:ON, 3:HALF, 4:TIMER
extern volatile int ma;                       // Max temperature threshold
extern volatile int mi;                       // Min temperature threshold
extern volatile float t;                      // Current temperature (DS18B20 primary, DHT backup)
extern volatile float newTemp;                // Temperature reading from DHT (used as fallback)
extern volatile float h;                      // Current humidity (DHT)
extern volatile unsigned long switchingDelay; // Relay switching delay (seconds)
extern volatile int humLimit;                 // Humidity limit (%)
extern volatile int gasLimit;                 // Gas limit (raw ADC value, 0-4095)
extern volatile long encoderPos;              // Written by Core 1 task, potentially read by Core 0 if needed elsewhere
extern String currentIPAddress;

//==================================
// System State & Timing
//==================================
extern volatile bool offlineMode; // True if connection to Blynk/WiFi fails
extern unsigned long lastDHTRead;

// LCD/UI State
extern volatile int screenMode;          // 1 = Main, 2 = Settings
extern volatile int settingsState;       // 0 = Navigation, 1 = Editing (in settings screen)
extern volatile int selectedField;       // Currently selected item in settings menu
extern unsigned long lastInteraction;    // Timestamp of last user interaction (encoder/button)
extern int settingsMenuOffset;           // Top visible item index in scrolling settings menu
extern const int SETTINGS_TOTAL_ITEMS;   // Total number of items in the main settings menu
extern const int VISIBLE_SETTINGS_LINES; // How many lines are visible in the settings menu

// Blynk State & Caching
extern unsigned long lastBlynkUpdate; // Timestamp of last data push to Blynk
extern int lastSentMi;
extern int lastSentMa;
extern int lastSentHumLimit;
extern int lastSentGasLimit;
extern float lastSentT;
extern float lastSentH;
extern float lastSentSoil;
extern int lastSentGas;
extern bool lastBlynkConnected;           // Previous Blynk connection state
extern unsigned long lastConnectionCheck; // Timestamp of last Blynk connection check
extern unsigned long lastNotifyTime;      // Timestamp of last Blynk notification

// Rotary Encoder State
extern volatile int lastA;           // Previous state of encoder pin A
extern volatile long encoderPos;     // Current position count of the encoder
extern volatile bool encoderUpdated; // Flag indicating encoder position changed

// Button State
extern unsigned long buttonHoldStart; // Timestamp when button press started
extern bool longPressTriggered;       // Flag indicating a long press action was triggered
extern bool lastButtonState;          // Previous state of the encoder button

// RTC & Time State
extern unsigned long lastNtpSync; // Timestamp of last NTP synchronization
extern bool initialNtpSyncDone;   // Flag indicating if the first NTP sync was successful

// globals.h (~ Line 78)
struct TimerEntry
{
  uint16_t dayOffset; // Day offset from start day (0 = start day)
  int minTemp;        // Min temp for this period
  int maxTemp;        // Max temp for this period
};
// extern TimerEntry timerEntries[MAX_TIMERS]; // <-- OLD
extern TimerEntry timerEntries[APP_MAX_TIMERS]; // <-- NEW (Use renamed constant)
extern uint8_t timerCount;                      // Number of active timer entries
extern uint32_t timerStartDay;                  // Day (days since epoch) when timer mode was initiated/last reset
extern uint8_t currentTimerIndex;               // Index of the timer entry being edited via Blynk

// Connection Status Enum
enum ConnectionStatus
{
  CONNECTING,
  CONNECTED,
  OFFLINE
};
extern volatile ConnectionStatus connectionStatus;

// LCD Caching
extern char prevLines[LCD_HEIGHT][LCD_LENGTH + 1]; // Cache for LCD lines to reduce flicker

#endif // GLOBALS_H