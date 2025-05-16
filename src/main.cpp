//==================================
// Main Program File
// ESP32 Environmental Controller
//==================================

// Include Core Arduino/ESP32 Headers
#include <Arduino.h>
#include <Wire.h> // For I2C communication

// Include Project Modules
#include "config.h"         // Pin definitions, credentials, constants
#include "globals.h"        // Global variables and peripheral instances
#include "sensors.h"        // Sensor reading functions
#include "display.h"        // LCD display functions
#include "relay_control.h"  // Relay logic and modeset
#include "eeprom_manager.h" // EEPROM saving/loading
#include "input_handler.h"  // Rotary encoder and button handling
#include "time_manager.h"   // RTC and NTP time functions
#include "wifi_manager.h"   // WiFi connection and AP mode management
#include "blynk_handler.h"  // Blynk communication and callbacks
#include <WiFi.h>

//==================================
// Global Variable Definitions
// (Define variables declared as 'extern' in globals.h)
//==================================
volatile int mod = 0;                      // Default mode: AUTO
volatile int ma = 32;                      // Default Max Temp
volatile int mi = 28;                      // Default Min Temp
volatile float t = -100;                   // Initialize temperature to invalid value
volatile float newTemp = -100;             // Initialize backup temperature
volatile float h = -1;                     // Initialize humidity to invalid value
volatile unsigned long switchingDelay = 5; // Default switching delay
volatile int humLimit = 60;                // Default Humidity Limit
volatile int gasLimit = 2000;              // Default Gas Limit
String currentIPAddress = "Initializing...";

volatile bool offlineMode = true; // Start assuming offline
unsigned long lastDHTRead = 0;

// LCD/UI State
volatile int screenMode = 1;    // Start on Main screen
volatile int settingsState = 0; // Start in Navigation mode if Settings entered
volatile int selectedField = 0;
unsigned long lastInteraction = 0;
int settingsMenuOffset = 0;
// SETTINGS_TOTAL_ITEMS and VISIBLE_SETTINGS_LINES are defined in display.cpp

// Blynk Caching/State
unsigned long lastNotifyTime = 0;
bool lastBlynkConnected = false;
unsigned long lastConnectionCheck = 0;

// Timer Mode State
TimerEntry timerEntries[APP_MAX_TIMERS];
uint8_t timerCount = 0;
uint32_t timerStartDay = 0; // Will be loaded from EEPROM or set by NTP/Blynk
uint8_t currentTimerIndex = 0;

// Connection Status
volatile ConnectionStatus connectionStatus = CONNECTING; // Initial state

//==================================
// Setup Function
//==================================
void setup()
{
    // Start Serial Monitor
    Serial.begin(115200); // Use a faster baud rate
    while (!Serial)
        ; // Wait for serial connection (optional)
    Serial.println("\n\n===================================");
    Serial.println("ESP32 Environmental Controller v1.1");
    Serial.println("===================================");

    // Initialize I2C Bus (for LCD and RTC)
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize Core Modules
    setupDisplay();       // Setup LCD first to show progress
    setupEEPROM();        // Initialize EEPROM access
    loadSettingsEEPROM(); // Load saved settings
    setupSensors();       // Initialize temp, humidity, gas, soil sensors
    setupRelays();        // Initialize relay pins to OFF state
    setupInput();         // Setup encoder pins and button, attach ISRs
    xTaskCreatePinnedToCore(
        encoderTask,             // Function to implement the task (in input_handler.cpp)
        "EncoderTask",           // Name of the task
        ENCODER_TASK_STACK_SIZE, // Stack size (defined in config.h, e.g., 2048 or 4096)
        NULL,                    // Task input parameter
        ENCODER_TASK_PRIORITY,   // Priority of the task (defined in config.h, e.g., 1 or 2)
        &encoderTaskHandle,      // Task handle (optional)
        ENCODER_TASK_CORE        // Core where the task should run (Core 1)
    );
    setupTime(); // Initialize RTC

    // Display loading message or initial status
    screen.clear();
    printLine(0, "System Starting...");
    printLine(1, "Loading settings...");
    delay(500); // Brief pause

    // Attempt Network Connection (WiFi and Blynk)
    printLine(2, "Connecting WiFi...");
    // connectWiFiBlynk(); // This function handles WiFi & initial Blynk connection attempt
    setupWiFiAndServer();
    setupBlynk();
    // Final setup messages
    printLine(3, "Setup Complete.");
    Serial.println("System Setup Complete.");
    delay(1500);
    clearLineCache();                                          // Clear cache for main screen display
    lastInteraction = millis();                                // Reset interaction timer
    lastConnectionCheck = millis();                            // Reset connection check timer
    lastBlynkUpdate = millis() - BLYNK_UPDATE_INTERVAL + 2000; // Force early Blynk update
} // End setup()

//==================================
// Main Loop Function
//==================================
void loop()
{

    // --- Handle Network Communication ---
    runBlynk();          // Process Blynk messages and maintain connection if online
    handleWebRequests(); // Process incoming HTTP requests if in AP mode

    // --- Check and Update Connection Status ---
    unsigned long now = millis();
    if (now - lastConnectionCheck > CONNECTION_CHECK_INTERVAL)
    {
        ConnectionStatus previousStatus = connectionStatus; // Store previous status for change detection

        // *** MODIFIED LOGIC START ***
        // 1. Check WiFi status FIRST
        if (WiFi.status() != WL_CONNECTED)
        {
            // If WiFi is down, we are definitely OFFLINE
            if (connectionStatus != OFFLINE)
            { // Only log/update if status changed
                Serial.println("WiFi Connection Lost!");
                connectionStatus = OFFLINE;
                offlineMode = true;
                lastBlynkConnected = false; // Assume Blynk is also down
            }
        }
        else
        {
            // WiFi is connected, NOW check Blynk status
            bool currentBlynkConnected = isBlynkConnected();
            if (currentBlynkConnected)
            {
                // WiFi and Blynk are OK
                if (connectionStatus != CONNECTED)
                { // Only log/update if status changed
                    Serial.println("Network Status: Connected (WiFi & Blynk)");
                    connectionStatus = CONNECTED;
                    offlineMode = false;
                }
            }
            else
            {
                // WiFi is OK, but Blynk is NOT connected
                if (connectionStatus != OFFLINE)
                { // Only log/update if status changed
                    // Consider if you want a different state like "WIFI_NO_BLYNK"
                    // For now, treat as OFFLINE for display purposes
                    Serial.println("Network Status: Offline (WiFi OK, Blynk disconnected)");
                    connectionStatus = OFFLINE;
                    offlineMode = true; // Treat as offline if Blynk is needed
                }
                // Attempt to reconnect Blynk periodically if WiFi is okay but Blynk isn't
                // Blynk.connect(); // Be careful: Blynk.connect() can block
                // Consider a non-blocking connect attempt or relying on Blynk.run()
            }
            lastBlynkConnected = currentBlynkConnected; // Update Blynk status tracking
        }
        // *** MODIFIED LOGIC END ***

        // Force LCD update if the status actually changed
        if (connectionStatus != previousStatus)
        {
            clearLineCache();
        }
        lastConnectionCheck = now;

        // Periodically try to reconnect if offline
        if (offlineMode && WiFi.getMode() != WIFI_AP)
        { // Don't try reconnect if intentionally in AP mode
            static unsigned long lastReconnectAttempt = 0;
            if (now - lastReconnectAttempt > 60000)
            { // Try reconnect every 60 seconds
                Serial.println("Attempting periodic reconnect...");
                setupWiFiAndServer();
                lastReconnectAttempt = now;
            }
        }
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        // If connected to local network, show Station IP
        IPAddress staIP = WiFi.localIP();
        if (staIP != INADDR_NONE)
        {
            currentIPAddress = staIP.toString();
        }
        else
        {
            currentIPAddress = "STA Error"; // Should not happen if status is connected
        }
    }
    else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
    {
        // If not connected via STA, but AP is running, show AP IP
        IPAddress apIP = WiFi.softAPIP();
        if (apIP != INADDR_NONE)
        {
            currentIPAddress = apIP.toString();
        }
        else
        {
            currentIPAddress = "AP Error";
        }
    }
    else
    {
        // No connection and AP not running (or failed)
        currentIPAddress = "No Network";
    }

    // --- Read Sensors ---
    //   readTemperatures(); // Reads DS18B20 and DHT temp (updates 't' and 'newTemp')
    //   readHumidity();     // Reads DHT humidity (updates 'h')
    readAllSensors();
    // Gas and Soil are read on demand (e.g., within updateHumidityRelays, sendSensorDataBlynk)

    // --- Handle User Input ---
    handleEncoderRotation(); // Process any encoder changes flagged by ISR
    checkEncoderButton();    // Check for button presses (short/long)

    // --- Run Control Logic (only if not in settings menu) ---
    if (screenMode == 1)
    {
        modeset(); // Apply relay control based on current mode (AUTO, TIMER, ON, OFF, HALF)
                   // Includes call to updateHumidityRelays() unless in Manual OFF mode
    }

    // --- Update Time & Timers ---
    updateRTCFromNTP();  // Attempt NTP sync periodically if connected
    checkAutoTimerAdd(); // Check if a new day requires adding default timer entry

    // --- Send Data to Blynk (Rate Limited) ---
    sendSensorDataBlynk(); // Sends sensor readings, settings, relay status if connected

    // --- Update Display ---
    updateDisplay(); // Renders the correct screen (Main or Settings) and handles timeout

    // Small delay to prevent loop from running too fast (optional)
    // Can be useful if tasks are very quick, reduces unnecessary processing
    // delay(10); // Be careful not to make this too long

} // End loop()