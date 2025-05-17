#include "wifi_manager.h"
#include "RTClib.h"
#include "time_manager.h"
#include "blynk_handler.h" // For Blynk.connect()
#include "display.h"       // For updating status on LCD
#include "ArduinoJson.h"
#include "sensors.h"
#include "eeprom_manager.h"

// WebServer instance (used only in AP mode)
WebServer server(80);

// Flag to track if AP mode setup was done
bool serverInitialized = false;
bool spiffsInitialized = false;

// Handler to provide all status info as JSON
bool serveFile(String path, String contentType) {
    if (!spiffsInitialized) {
         server.send(500, "text/plain", "SPIFFS Error");
         return false;
    }
    if (path.endsWith("/") || path.isEmpty()) {
        path = "/index.html"; // Default file
    }
    Serial.print("Serving file: "); Serial.println(path);

    if (!SPIFFS.exists(path)) {
        Serial.println(" -> File Not Found");
        server.send(404, "text/plain", "File Not Found: " + path);
        return false;
    }

    File file = SPIFFS.open(path, "r");
    if (!file) {
         Serial.println(" -> Failed to open file!");
         server.send(500, "text/plain", "Failed to open file");
         return false;
    }

    // Stream the file content with the correct MIME type
    // server.streamFile automatically closes the file
    size_t sent = server.streamFile(file, contentType);
    file.close(); // Ensure file is closed even if streamFile doesn't
    Serial.print(" -> Sent "); Serial.print(sent); Serial.println(" bytes");
    return true;
}   
void handleCSS() { serveFile("/style.css", "text/css"); }
void handleJS() { serveFile("/script.js", "application/javascript"); }
void handleRoot() { serveFile("/index.html", "text/html"); }
// void handleRoot() { 
//     serveFile("/gauge.js", "application/javascript"); // Serve gauge.js first
//     serveFile("/style.css", "text/css"); // Serve CSS
//     serveFile("/index_non_js.html", "text/html"); 
// }
void handleGaugeJS() { serveFile("/gauge.js", "application/javascript"); }
void handleStatus()
{
    JsonDocument jsonDoc; // Adjust size as needed
    String ipStr = "N/A";
    if(WiFi.status() == WL_CONNECTED) {
         ipStr = WiFi.localIP().toString();
    } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
         ipStr = WiFi.softAPIP().toString();
    }
    char timeBuf[9]; // HH:MM:SS
    char dateBuf[12]; // DD/MM/YYYY
    DateTime nowRTC = rtc.now(); // Get current time from RTC instance
    sprintf(timeBuf, "%02d:%02d:%02d", nowRTC.hour(), nowRTC.minute(), nowRTC.second());

    uint32_t currentDay = nowRTC.unixtime() / 86400;
    int daysPassed = (timerStartDay > 0) ? (currentDay - timerStartDay) : 0;
    if (daysPassed < 0) daysPassed = 0;

    // Format start date (assuming timerStartDay is days since epoch 2000-01-01)
    // This calculation is approximate, might need refinement depending on exact epoch used by unixtime()
    DateTime startDate = DateTime(SECONDS_FROM_1970_TO_2000 + timerStartDay * 86400UL);
    if (timerStartDay > 0) {
         sprintf(dateBuf, "%02d/%02d/%04d", startDate.day(), startDate.month(), startDate.year());
    } else {
         strcpy(dateBuf, "--/--/----");
    }
    // Read volatile globals safely if needed (or assume loop updates them frequently enough)
    noInterrupts();
    jsonDoc["temp"] = t;
    jsonDoc["humidity"] = h;
    jsonDoc["mode"] = mod;
    jsonDoc["min_temp"] = mi;
    jsonDoc["max_temp"] = ma;
    // Read relay states (assuming HIGH = ON based on recent user feedback)
    jsonDoc["relay1"] = (digitalRead(relay1) == LOW);
    jsonDoc["relay2"] = (digitalRead(relay2) == LOW);
    jsonDoc["relay3"] = (digitalRead(relay3) == LOW); // Assuming R3 is Active LOW
    jsonDoc["relay4"] = (digitalRead(relay4) == LOW); // Assuming R4 is Active LOW
    interrupts();

    // Read non-volatile sensor values directly
    jsonDoc["gas"] = readGasSensor();
    jsonDoc["soil"] = readSoilMoisture();
    jsonDoc["delay"] = switchingDelay; // Ensure this line exists
    jsonDoc["hum_limit"] = humLimit;    // Ensure this line exists
    jsonDoc["gas_limit"] = gasLimit;    // Ensure this line exists
    uint8_t currentTimerCount = timerCount;
    jsonDoc["ip_address"] = ipStr;
    // Add other values like humLimit, gasLimit if desired
    jsonDoc["currentTimeStr"] = timeBuf;
    jsonDoc["startDateStr"] = dateBuf;
    jsonDoc["daysPassed"] = daysPassed;
    // Add timer entries as a JSON array
    JsonArray timerArray = jsonDoc.createNestedArray("timerEntries");
    for (uint8_t i = 0; i < currentTimerCount; i++) {
        JsonObject entry = timerArray.createNestedObject();
        entry["offset"] = timerEntries[i].dayOffset;
        entry["min"] = timerEntries[i].minTemp;
        entry["max"] = timerEntries[i].maxTemp;
    }
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    server.send(200, "application/json", jsonString);
}
void handleSyncTime() {
    Serial.println("Web UI requested NTP Sync...");
    updateRTCFromNTP(); // Call the function from time_manager
    server.send(200, "text/plain", "NTP Sync Triggered");
}
void handleSetMode()
{
    if (!server.hasArg("mode"))
    {
        server.send(400, "text/plain", "Missing 'mode' parameter");
        return;
    }
    int requestedMode = server.arg("mode").toInt();

    if (requestedMode >= 0 && requestedMode <= 4) {
        noInterrupts(); mod = requestedMode; interrupts();
        Serial.print("Mode set via Web UI: "); Serial.println(mod);
        clearLineCache();
        server.send(200, "text/plain", "Mode Set OK");
    }
    else
    {
        server.send(400, "text/plain", "Invalid mode value");
    }
}

void handleSetSettings() {
    if (!server.hasArg("min") || !server.hasArg("max") || !server.hasArg("delay") || !server.hasArg("hum") || !server.hasArg("gas") ) {
       server.send(400, "text/plain", "Missing required setting parameters (min, max, delay, hum, gas)");
       return;
   }
    int requestedMin = server.arg("min").toInt();
    int requestedMax = server.arg("max").toInt();
    int requestedDelay = server.arg("delay").toInt();
    int requestedHumLimit = server.arg("hum").toInt();
    int requestedGasLimit = server.arg("gas").toInt();

   // --- Validation ---
   if (requestedMin > requestedMax) { server.send(400, "text/plain", "Validation failed: Min > Max"); return; }
   if (requestedDelay < 1 || requestedDelay > 300) { server.send(400, "text/plain", "Validation failed: Delay range"); return; }
   if (requestedHumLimit < 0 || requestedHumLimit > 100) { server.send(400, "text/plain", "Validation failed: HumLimit range"); return; }
   if (requestedGasLimit < 0 || requestedGasLimit > 4095) { server.send(400, "text/plain", "Validation failed: GasLimit range"); return; }
   // --- End Validation ---

   noInterrupts();
   mi = requestedMin;
   ma = requestedMax;
   switchingDelay = (unsigned long)requestedDelay;
   humLimit = requestedHumLimit;
   gasLimit = requestedGasLimit;
   interrupts();

   Serial.println("Settings updated via Web UI:"); /* ... print all settings ... */
   Serial.print("  Min/Max T: "); Serial.print(mi); Serial.print("/"); Serial.println(ma);
   Serial.print("  Delay: "); Serial.println(switchingDelay);
   Serial.print("  Hum Limit: "); Serial.println(humLimit);
   Serial.print("  Gas Limit: "); Serial.println(gasLimit);

   saveSettingsEEPROM(); // Save the new settings

   // Update Blynk widgets immediately
   blynkVirtualWriteInt(VPIN_MIN_TEMP_SET, mi);
   blynkVirtualWriteInt(VPIN_MAX_TEMP_SET, ma);
   blynkVirtualWriteInt(VPIN_HUM_LIMIT, humLimit);
   blynkVirtualWriteInt(VPIN_GAS_LIMIT, gasLimit);
   // blynkVirtualWriteInt(VPIN_SWITCH_DELAY, switchingDelay); // Add VPIN define if needed

   server.send(200, "text/plain", "All Settings Updated OK");
}
void handleSetTimerEntry() {
    if (!server.hasArg("index") || !server.hasArg("min") || !server.hasArg("max")) {
       server.send(400, "text/plain", "Missing required parameters (index, min, max)");
       return;
   }
    int index = server.arg("index").toInt();
    int requestedMin = server.arg("min").toInt();
    int requestedMax = server.arg("max").toInt();

    // --- Validation ---
    // Check index bounds
    if (index < 0 || index >= timerCount || index >= APP_MAX_TIMERS) {
         server.send(400, "text/plain", "Invalid timer index");
         return;
    }
    // Check Min <= Max
    if (requestedMin > requestedMax) {
        server.send(400, "text/plain", "Validation failed: Min > Max");
        return;
   }
    // Add temp range validation if desired
    // --- End Validation ---

    Serial.printf("Updating Timer Entry %d: Min=%d, Max=%d\n", index, requestedMin, requestedMax);

    // Update the specific entry in the global array (volatile access protection)
    noInterrupts();
    timerEntries[index].minTemp = requestedMin;
    timerEntries[index].maxTemp = requestedMax;
    interrupts();

    saveSettingsEEPROM(); // Save all settings after modifying the entry

    // Update corresponding Blynk widgets if they exist (using V15/V16 depends on currentTimerIndex)
    // This requires knowing which Blynk index matches this Web UI index - might need more complex logic
    // For now, just save to EEPROM. Blynk UI might need manual refresh or update via V12 button.

   server.send(200, "text/plain", "Timer Entry Updated OK");
}
void handleResetTimerEntry() {
    if (!server.hasArg("index")) {
       server.send(400, "text/plain", "Missing required parameter (index)");
       return;
   }
    int index = server.arg("index").toInt();

    // Validation
    noInterrupts(); uint8_t currentCount = timerCount; interrupts();
    if (index < 0 || index >= currentCount || index >= APP_MAX_TIMERS) {
         server.send(400, "text/plain", "Invalid timer index");
         return;
    }

    Serial.printf("Resetting Timer Entry %d to defaults (28/32)\n", index);

    // Update the specific entry in the global array
    noInterrupts();
    timerEntries[index].minTemp = 28; // Default value
    timerEntries[index].maxTemp = 32; // Default value
    interrupts();

    saveSettingsEEPROM(); // Save all settings after modifying the entry

   server.send(200, "text/plain", "Timer Entry Reset OK");
}
void setupWiFiAndServer()
{
    Serial.println("Setting up WiFi (STA attempt + AP Mode)...");
    connectionStatus = CONNECTING; 
    Serial.println("Scanning for preferred networks...");

    int n = WiFi.scanNetworks();
    String foundSSID = "";
    String foundPass = "";
    if (n == 0)
    {
        Serial.println("No WiFi networks found.");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found:");
        for (int i = 0; i < n; ++i)
        {
            String currentSSID = WiFi.SSID(i);
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(currentSSID); // Print found networks
            // Check against preferred list (using defined constants from config.h)
            if (currentSSID.equals(ssid1))
            {
                foundSSID = ssid1;
                foundPass = pass1;
                Serial.print("Found preferred network 1: ");
                Serial.println(foundSSID);
                break; // Stop scanning once found
            }
            else if (currentSSID.equals(ssid2))
            {
                foundSSID = ssid2;
                foundPass = pass2;
                Serial.print("Found preferred network 2: ");
                Serial.println(foundSSID);
                break; // Stop scanning once found
            }
            delay(10); // Small delay between checks
        }
    }

    if (foundSSID.length() > 0)
    {
        Serial.print("Attempting Station connection to ");
        Serial.println(foundSSID);
        WiFi.begin(foundSSID.c_str(), foundPass.c_str());
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000)
        { // 15s timeout
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Station Mode Connected!");
            Serial.print("STA IP Address: ");
            Serial.println(WiFi.localIP());
            offlineMode = false;          // Initially assume online if STA connects
            connectionStatus = CONNECTED; // Status might change later based on Blynk
        }
        else
        {
            Serial.println("Station Mode Connection Failed.");
            WiFi.disconnect(); // Ensure it's not stuck trying
            offlineMode = true;
            connectionStatus = OFFLINE;
        }
    }
    else
    {
        Serial.println("No preferred networks found for Station mode.");
        offlineMode = true;
        connectionStatus = OFFLINE;
    }

    // 2. Configure and Start Access Point (Always)
    Serial.println("Configuring Access Point...");
    IPAddress apIP(192, 168, 4, 1); // Default AP IP
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, gateway, subnet);

    if (WiFi.softAP(apSSID, apPass))
    {
        Serial.print("AP Mode Started. SSID: ");
        Serial.println(apSSID);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
        // AP mode itself doesn't guarantee internet/Blynk connectivity
    }
    else
    {
        Serial.println("ERROR: Failed to start AP Mode!");
    }

    // 3. Initialize SPIFFS (if not already done)
    if (!spiffsInitialized)
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("ERROR: Failed to mount SPIFFS! Web UI may not work.");
        }
        else
        {
            Serial.println("SPIFFS Mounted.");
            spiffsInitialized = true;
        }
    }

    // 4. Setup and Start Web Server (if not already done)
    if (!serverInitialized && spiffsInitialized) {
        Serial.println("Registering Web Server routes...");
        server.on("/", HTTP_GET, handleRoot);         // "/" uses handleRoot
        server.on("/status", HTTP_GET, handleStatus);
        server.on("/setmode", HTTP_GET, handleSetMode);
        server.on("/setsettings", HTTP_GET, handleSetSettings);
        server.on("/synctime", HTTP_GET, handleSyncTime);
        server.on("/settimerentry", HTTP_GET, handleSetTimerEntry);
        server.on("/resettimerentry", HTTP_GET, handleResetTimerEntry);
        server.onNotFound([](){ server.send(404, "text/plain", "Not Found"); });
        server.begin();
        serverInitialized = true;
        Serial.println("HTTP Server Started.");
    }
    else if (!spiffsInitialized)
    {
        Serial.println("HTTP Server NOT started (SPIFFS failed).");
    }

    updateDisplay(); // Update LCD with initial status after attempts
    clearLineCache();
}

// Handles incoming client requests when in AP mode
// Call this from the main loop()
void handleWebRequests()
{
    // Only handle clients if AP mode is active and initialized
    if (serverInitialized) {
        server.handleClient();
    }
}