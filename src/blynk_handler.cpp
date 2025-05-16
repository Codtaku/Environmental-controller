#define BLYNK_PRINT Serial    // <-- ADD THIS DEFINITION HERE
#include <BlynkSimpleEsp32.h> // <-- ENSURE BLYNK HEADER IS INCLUDED HERE

#include "blynk_handler.h"
#include "blynk_handler.h"
#include "eeprom_manager.h" // Needed for saving settings changed via Blynk
#include "time_manager.h"   // Needed to get current time for V7
#include "sensors.h"        // Needed for reading sensors to send
#include "display.h"

// Include configuration for credentials and server info
#include "config.h"

// Authentication Token
char auth[] = BLYNK_AUTH_TOKEN;

// Timer for periodic data sending
unsigned long lastBlynkUpdate = 0;

// Caching variables for sending data only on change
int lastSentMi = -1;
int lastSentMa = -1;
int lastSentHumLimit = -1;
int lastSentGasLimit = -1;
float lastSentT = -1000; // Use unlikely initial values
float lastSentH = -1;
float lastSentSoil = -1;
int lastSentGas = -1;
float lastSentRtcTemp = -1000;
int lastSentDaysElapsed = -1;
int lastSentTimerCount = -1;
int lastRelay1State = -1; // Assuming initial state is OFF (HIGH logic for relay board)
int lastRelay2State = -1;
int lastRelay3State = -1; // Assuming active LOW
int lastRelay4State = -1; // Assuming active LOW

void setupBlynk()
{
    Serial.println("Configuring Blynk...");
    // Configure Blynk connection details using macros from config.h
    Blynk.config(BLYNK_AUTH_TOKEN, blynkServer, blynkPort);

    // Attempt initial connection (moved from wifi_manager)
    // Do this only if WiFi is already connected
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Attempting initial Blynk connection...");
        if (Blynk.connect(5000))
        { // 5 second timeout
            Serial.println("Blynk Connected Initialized!");
            connectionStatus = CONNECTED; // Update status
            lastBlynkConnected = true;
            offlineMode = false;
            // Sync state to Blynk now that we are connected
            syncSettingsToBlynk();
            syncRelayStatusExtended();
        }
        else
        {
            Serial.println("Initial Blynk Connection Failed!");
            // connectionStatus should already be OFFLINE from wifi_manager if WiFi failed,
            // or CONNECTED if WiFi is ok but Blynk failed. We set offlineMode=true.
            lastBlynkConnected = false;
            offlineMode = true;
        }
    }
    else
    {
        Serial.println("Skipping initial Blynk connect attempt (WiFi not connected).");
        offlineMode = true; // Ensure offlineMode is true if WiFi isn't connected
    }
}

bool isBlynkConnected()
{
    return Blynk.connected();
}

void runBlynk()
{
    if (!offlineMode && WiFi.status() == WL_CONNECTED)
    {
        Blynk.run();
        // Add heartbeat to maintain connection if needed (helps with some routers/firewalls)
        // Blynk.virtualWrite(V99, millis() / 1000); // Send uptime seconds to an unused pin
    }
}

void sendSensorDataBlynk()
{
    if (!isBlynkConnected())
        return; // Don't try to send if not connected

    unsigned long now = millis();
    if (now - lastBlynkUpdate > BLYNK_UPDATE_INTERVAL)
    {

        // --- Send Sensor Readings (only if changed significantly) ---
        if (abs(t - lastSentT) > 0.1)
        { // Send temperature if changed by > 0.1 C
            Blynk.virtualWrite(V0, t);
            lastSentT = t;
        }
        if (abs(h - lastSentH) > 0.1)
        { // Send humidity if changed by > 0.5 %
            Blynk.virtualWrite(V1, h);
            lastSentH = h;
        }

        float soilMoist = readSoilMoisture();
        if (abs(soilMoist - lastSentSoil) > 1.0)
        { // Send soil moisture if changed by > 1.0 %
            Blynk.virtualWrite(V5, soilMoist);
            lastSentSoil = soilMoist;
            // Optional: Add notification logic based on soil moisture thresholds
            // if(soilMoist < 20) notifyBlynk("Warning: Soil is very dry!");
            // if(soilMoist > 80) notifyBlynk("Warning: Soil is very wet!");
        }

        int gasVal = readGasSensor();
        if (abs(gasVal - lastSentGas) > 50)
        { // Send gas reading if changed by > 50 units
            Blynk.virtualWrite(V6, gasVal);
            lastSentGas = gasVal;
            // Optional: Notify if gas level is critically high
            // if (gasVal > gasLimit * 1.5) notifyBlynk("CRITICAL: High gas reading!");
        }

        float rtcTemp = readRTCTemperature();
        if (abs(rtcTemp - lastSentRtcTemp) > 0.2)
        { // Send RTC temp if changed > 0.2 C
            Blynk.virtualWrite(V10, rtcTemp);
            lastSentRtcTemp = rtcTemp;
        }

        // --- Sync Settings & Status (only if changed) ---
        syncSettingsToBlynk();     // Check and send settings if they differ
        syncRelayStatusExtended(); // Check and send relay statuses if they differ

        // --- Sync Timer Mode Info (usually updated by time_manager, send if changed) ---
        DateTime nowRTC = rtc.now();
        uint32_t currentDay = nowRTC.unixtime() / 86400;
        int daysElapsed = (timerStartDay > 0) ? (currentDay - timerStartDay) : 0;
        if (daysElapsed < 0)
            daysElapsed = 0; // Sanity check

        if (daysElapsed != lastSentDaysElapsed)
        {
            Blynk.virtualWrite(V21, daysElapsed); // Days since timer start
            lastSentDaysElapsed = daysElapsed;
        }
        if (timerCount != lastSentTimerCount)
        {
            Blynk.virtualWrite(V22, timerCount); // Number of timer entries
            lastSentTimerCount = timerCount;
        }

        lastBlynkUpdate = now; // Update timestamp
    }
}

// Sends settings values to Blynk widgets IF they have changed from the last sent values
void syncSettingsToBlynk()
{
    if (!isBlynkConnected())
        return;

    if (mi != lastSentMi)
    {
        Blynk.virtualWrite(V2, mi);
        lastSentMi = mi;
    }
    if (ma != lastSentMa)
    {
        Blynk.virtualWrite(V3, ma);
        lastSentMa = ma;
    }
    if (humLimit != lastSentHumLimit)
    {
        Blynk.virtualWrite(V18, humLimit);
        lastSentHumLimit = humLimit;
    }
    if (gasLimit != lastSentGasLimit)
    {
        Blynk.virtualWrite(V17, gasLimit);
        lastSentGasLimit = gasLimit;
    }
    // Send current mode to V8 if it was changed locally?
    // Blynk.virtualWrite(V8, mod); // This might fight with user input on V8, be careful

    // Sync selected timer entry values if the index is valid
    if (currentTimerIndex < timerCount)
    {
        Blynk.virtualWrite(V15, timerEntries[currentTimerIndex].minTemp);
        Blynk.virtualWrite(V16, timerEntries[currentTimerIndex].maxTemp);
        // Maybe also send the index itself back to V9 to confirm?
        // Blynk.virtualWrite(V9, currentTimerIndex);
    }
}

// Send relay statuses to V23-V26 (LED widgets)
void syncRelayStatusExtended()
{
    if (!isBlynkConnected())
        return;

    // Read current state (LOW means ON for typical relay modules)
    bool r1_on = (digitalRead(relay1) == LOW);
    bool r2_on = (digitalRead(relay2) == LOW);
    bool r3_on = (digitalRead(relay3) == LOW); // Active low assumed
    bool r4_on = (digitalRead(relay4) == LOW); // Active low assumed

    // Send only if state changed
    if (r1_on != lastRelay1State)
    {
        Blynk.virtualWrite(V23, r1_on ? 255 : 0); // 255 for ON, 0 for OFF
        lastRelay1State = r1_on;
    }
    if (r2_on != lastRelay2State)
    {
        Blynk.virtualWrite(V24, r2_on ? 255 : 0);
        lastRelay2State = r2_on;
    }
    if (r3_on != lastRelay3State)
    {
        Blynk.virtualWrite(V25, r3_on ? 255 : 0);
        lastRelay3State = r3_on;
    }
    if (r4_on != lastRelay4State)
    {
        Blynk.virtualWrite(V26, r4_on ? 255 : 0);
        lastRelay4State = r4_on;
    }
}

// Send notification, but only once per interval
void notifyBlynk(String msg)
{
    unsigned long now = millis();
    if (isBlynkConnected() && (now - lastNotifyTime > NOTIFY_INTERVAL))
    {
        Blynk.notify(msg);
        lastNotifyTime = now;
        Serial.print("Blynk Notification Sent: ");
        Serial.println(msg);
    }
    else if (isBlynkConnected())
    {
        Serial.print("Blynk Notification Skipped (Rate Limit): ");
        Serial.println(msg);
    }
}
void blynkVirtualWriteInt(int pin, int value)
{
    if (isBlynkConnected())
    {
        Blynk.virtualWrite(pin, value);
    }
}

void blynkVirtualWriteFloat(int pin, float value)
{
    if (isBlynkConnected())
    {
        Blynk.virtualWrite(pin, value);
    }
}

void blynkVirtualWriteString(int pin, const String &value)
{
    if (isBlynkConnected())
    {
        Blynk.virtualWrite(pin, value);
    }
}

void blynkVirtualWriteString(int pin, const char *value)
{
    if (isBlynkConnected())
    {
        Blynk.virtualWrite(pin, value);
    }
}

//==================================
// BLYNK_WRITE Handlers
//==================================

BLYNK_WRITE(V2)
{                                    // Update Min Temp Threshold
    int initialMi = mi;              // Store initial value before changes
    int requestedMi = param.asInt(); // Get value from Blynk

    // Apply validation: Clamp mi if it tries to exceed ma
    if (requestedMi > ma)
    {
        mi = ma; // Clamp mi: cannot exceed ma
        Serial.print("Blynk requested Mi ");
        Serial.print(requestedMi);
        Serial.print(", clamped to Ma: ");
        Serial.println(mi); // Added Log
    }
    else
    {
        mi = requestedMi; // Value is valid
                          // Add lower bound check if desired: if (mi < 0) mi = 0;
    }

    // Save IF value actually changed from its initial state
    if (mi != initialMi)
    {
        saveSettingsEEPROM(); // Save the change
        lastSentMi = mi;      // Update cache to prevent immediate resend
        Serial.print("Min Temp set (Blynk): ");
        Serial.println(mi); // Log change
    }

    // IMPORTANT: If the final value 'mi' differs from what Blynk sent ('requestedMi'),
    // update the Blynk widget immediately to show the clamped value.
    if (mi != requestedMi)
    {
        blynkVirtualWriteInt(VPIN_MIN_TEMP_SET, mi);                 // Use wrapper function
        Serial.println("Updating Blynk V2 widget due to clamping."); // Added Log
    }
}

BLYNK_WRITE(V3)
{                                    // Update Max Temp Threshold
    int initialMa = ma;              // Store initial value before changes
    int requestedMa = param.asInt(); // Get value from Blynk

    // Apply validation: Clamp ma if it tries to go below mi
    if (requestedMa < mi)
    {
        ma = mi; // Clamp ma: cannot be less than mi
        Serial.print("Blynk requested Ma ");
        Serial.print(requestedMa);
        Serial.print(", clamped to Mi: ");
        Serial.println(ma); // Added Log
    }
    else
    {
        ma = requestedMa; // Value is valid
                          // Add upper bound check if desired: if (ma > 50) ma = 50;
    }

    // Save IF value actually changed from its initial state
    if (ma != initialMa)
    {
        saveSettingsEEPROM(); // Save the change
        lastSentMa = ma;      // Update cache
        Serial.print("Max Temp set (Blynk): ");
        Serial.println(ma); // Log change
    }

    // IMPORTANT: If the final value 'ma' differs from what Blynk sent ('requestedMa'),
    // update the Blynk widget immediately to show the clamped value.
    if (ma != requestedMa)
    {
        blynkVirtualWriteInt(VPIN_MAX_TEMP_SET, ma);                 // Use wrapper function
        Serial.println("Updating Blynk V3 widget due to clamping."); // Added Log
    }
}

BLYNK_WRITE(V8)
{ // Update Mode
    int newMode = param.asInt();
    if (newMode >= 1 && newMode <= 5)
    { // Validate mode range
        if (newMode != mod)
        {
            mod = newMode;
            Serial.print("Mode set (Blynk): ");
            Serial.println(mod);
            // Optional: Save mode to EEPROM if persistence is desired
            // saveSettingsEEPROM();
            // Update LCD immediately if possible? Depends on architecture.
            clearLineCache(); // Force LCD update on next cycle
        }
    }
    else
    {
        Serial.print("Invalid mode received from Blynk: ");
        Serial.println(newMode);
    }
    // Always write back the current mode to keep Blynk widget in sync
    Blynk.virtualWrite(V8, mod);
}

BLYNK_WRITE(V18)
{ // Update Humidity Limit
    int newHumLimit = param.asInt();
    if (newHumLimit >= 0 && newHumLimit <= 100)
    { // Validate range
        if (newHumLimit != humLimit)
        {
            humLimit = newHumLimit;
            saveSettingsEEPROM();
            lastSentHumLimit = humLimit; // Update cache
            Serial.print("Humidity Limit set (Blynk): ");
            Serial.println(humLimit);
        }
    }
    else
    {
        Serial.print("Invalid Humidity Limit received from Blynk: ");
        Serial.println(newHumLimit);
    }
    // Write back the current value to keep slider in sync
    Blynk.virtualWrite(V18, humLimit);
}

BLYNK_WRITE(V17)
{ // Update Gas Limit
    int newGasLimit = param.asInt();
    if (newGasLimit >= 0 && newGasLimit <= 4095)
    { // Validate range
        if (newGasLimit != gasLimit)
        {
            gasLimit = newGasLimit;
            saveSettingsEEPROM();
            lastSentGasLimit = gasLimit; // Update cache
            Serial.print("Gas Limit set (Blynk): ");
            Serial.println(gasLimit);
        }
    }
    else
    {
        Serial.print("Invalid Gas Limit received from Blynk: ");
        Serial.println(newGasLimit);
    }
    // Write back the current value to keep slider in sync
    Blynk.virtualWrite(V17, gasLimit);
}

//--- Timer Mode Callbacks ---

BLYNK_WRITE(V7)
{ // Button to Start/Reset Timer Auto Adjusting Start Day
    if (param.asInt() == 1)
    {                                           // Trigger only on button press (value 1)
        DateTime now = rtc.now();               // Assumes RTC is working
        timerStartDay = now.unixtime() / 86400; // Days since epoch
        saveSettingsEEPROM();
        char buf[12];
        sprintf(buf, "%02d/%02d/%02d", now.day(), now.month(), now.year() % 100);
        Blynk.virtualWrite(V20, buf); // Update display widget showing start date
        Serial.print("Timer Mode Start Day Set (Blynk): ");
        Serial.println(timerStartDay);
        notifyBlynk("Timer mode auto-adjust started/reset.");
    }
}

BLYNK_WRITE(V9)
{ // Select Timer Entry Index to Edit
    int idx = param.asInt();
    if (idx >= 0 && idx < timerCount)
    {
        if (idx != currentTimerIndex)
        {
            currentTimerIndex = idx;
            Serial.print("Selected Timer Index (Blynk): ");
            Serial.println(currentTimerIndex);
            // Update V15 and V16 widgets to show the values for the selected index
            syncSettingsToBlynk(); // This will now send V15/V16 for the currentTimerIndex
        }
    }
    else if (timerCount == 0)
    {
        Serial.println("No timer entries to select (Blynk).");
        // Maybe update V15/V16 to default or disabled state?
        Blynk.virtualWrite(V15, 0); // Example: set to 0 if no timer
        Blynk.virtualWrite(V16, 0);
    }
    else
    {
        Serial.print("Invalid Timer Index received (Blynk): ");
        Serial.print(idx);
        Serial.print(" (Count: ");
        Serial.print(timerCount);
        Serial.println(")");
        // Write back the current valid index to keep widget in sync
        Blynk.virtualWrite(V9, currentTimerIndex);
    }
}

BLYNK_WRITE(V15)
{ // Adjust Min Temp for Selected Timer Entry
    int newMin = param.asInt();
    if (currentTimerIndex < timerCount)
    { // Ensure index is valid
        if (newMin != timerEntries[currentTimerIndex].minTemp)
        {
            timerEntries[currentTimerIndex].minTemp = newMin;
            // Optional: Add validation (e.g., min <= max)
            if (timerEntries[currentTimerIndex].minTemp > timerEntries[currentTimerIndex].maxTemp)
            {
                timerEntries[currentTimerIndex].minTemp = timerEntries[currentTimerIndex].maxTemp;
                // Force slider update if value was corrected
                Blynk.virtualWrite(V15, timerEntries[currentTimerIndex].minTemp);
            }
            // Defer saving to V12 button press
            // saveSettingsEEPROM();
            Serial.print("Timer[");
            Serial.print(currentTimerIndex);
            Serial.print("] Min Temp staged (Blynk): ");
            Serial.println(newMin);
        }
    }
    else
    {
        Serial.println("Cannot set Timer Min Temp: Invalid index (Blynk).");
        // Write back a default/disabled value?
        Blynk.virtualWrite(V15, 0);
    }
}

BLYNK_WRITE(V16)
{ // Adjust Max Temp for Selected Timer Entry
    int newMax = param.asInt();
    if (currentTimerIndex < timerCount)
    { // Ensure index is valid
        if (newMax != timerEntries[currentTimerIndex].maxTemp)
        {
            timerEntries[currentTimerIndex].maxTemp = newMax;
            // Optional: Add validation (e.g., max >= min)
            if (timerEntries[currentTimerIndex].maxTemp < timerEntries[currentTimerIndex].minTemp)
            {
                timerEntries[currentTimerIndex].maxTemp = timerEntries[currentTimerIndex].minTemp;
                // Force slider update if value was corrected
                Blynk.virtualWrite(V16, timerEntries[currentTimerIndex].maxTemp);
            }
            // Defer saving to V12 button press
            // saveSettingsEEPROM();
            Serial.print("Timer[");
            Serial.print(currentTimerIndex);
            Serial.print("] Max Temp staged (Blynk): ");
            Serial.println(newMax);
        }
    }
    else
    {
        Serial.println("Cannot set Timer Max Temp: Invalid index (Blynk).");
        // Write back a default/disabled value?
        Blynk.virtualWrite(V16, 0);
    }
}

BLYNK_WRITE(V12)
{ // Button to Save Current Timer Edits
    if (param.asInt() == 1)
    {
        if (timerCount > 0)
        {
            saveSettingsEEPROM(); // Save all settings, including potentially changed timer entries
            Serial.println("Timer settings saved (Blynk).");
            Blynk.virtualWrite(V12, "Saved!"); // Provide feedback on button
            // delay(1000); // Allow user to see feedback
            Blynk.virtualWrite(V12, "Save Timers"); // Reset button text
            notifyBlynk("Timer settings saved.");
        }
        else
        {
            Serial.println("No timer settings to save (Blynk).");
            Blynk.virtualWrite(V12, "Nothing to save");
            // delay(1000);
            Blynk.virtualWrite(V12, "Save Timers");
        }
    }
}

BLYNK_WRITE(V11)
{ // Button to Add a New Timer Entry
    if (param.asInt() == 1)
    {
        if (timerCount < APP_MAX_TIMERS)
        {
            // Determine offset for the new entry (day after the last one, or day 0 if first)
            uint16_t newOffset = (timerCount > 0) ? timerEntries[timerCount - 1].dayOffset + 1 : 0;
            timerEntries[timerCount].dayOffset = newOffset;
            // Set default temperatures for the new entry
            timerEntries[timerCount].minTemp = 28; // Default min
            timerEntries[timerCount].maxTemp = 32; // Default max
            timerCount++;                          // Increment the count
            saveSettingsEEPROM();                  // Save immediately
            Serial.print("New Timer Entry Added (Blynk). Count: ");
            Serial.println(timerCount);
            Blynk.virtualWrite(V11, "Added!");
            // Update the index selector range/value potentially?
            currentTimerIndex = timerCount - 1; // Select the newly added timer
            syncSettingsToBlynk();              // Update V9, V15, V16
            notifyBlynk("New timer schedule entry added.");
            delay(1000);
            Blynk.virtualWrite(V11, "Add Timer");
        }
        else
        {
            Serial.println("Cannot Add Timer Entry: Max limit reached (Blynk).");
            Blynk.virtualWrite(V11, "Limit Reached");
            notifyBlynk("Max timer entries reached.");
            delay(1000);
            Blynk.virtualWrite(V11, "Add Timer");
        }
    }
}

BLYNK_WRITE(V14)
{ // Button to Delete the Last Timer Entry
    if (param.asInt() == 1)
    {
        if (timerCount > 0)
        {
            timerCount--;         // Simply decrement the count (data remains but isn't used)
            saveSettingsEEPROM(); // Save the new count
            Serial.print("Last Timer Entry Deleted (Blynk). Count: ");
            Serial.println(timerCount);
            Blynk.virtualWrite(V14, "Deleted!");
            // Adjust selected index if it's now out of bounds
            if (currentTimerIndex >= timerCount && timerCount > 0)
            {
                currentTimerIndex = timerCount - 1;
            }
            else if (timerCount == 0)
            {
                currentTimerIndex = 0; // Or handle appropriately
            }
            syncSettingsToBlynk(); // Update V9, V15, V16
            notifyBlynk("Last timer schedule entry deleted.");
            delay(1000);
            Blynk.virtualWrite(V14, "Delete Last");
        }
        else
        {
            Serial.println("No Timer Entry to Delete (Blynk).");
            Blynk.virtualWrite(V14, "None Exist");
            delay(1000);
            Blynk.virtualWrite(V14, "Delete Last");
        }
    }
}

BLYNK_WRITE(V13)
{ // Button to Reset Selected Timer Entry to Defaults (28/32)
    if (param.asInt() == 1)
    {
        if (currentTimerIndex < timerCount)
        { // Ensure index is valid
            timerEntries[currentTimerIndex].minTemp = 28;
            timerEntries[currentTimerIndex].maxTemp = 32;
            // Defer saving to V12 button press
            // saveSettingsEEPROM();
            Serial.print("Timer[");
            Serial.print(currentTimerIndex);
            Serial.println("] reset to defaults (Blynk). Staged for save.");
            Blynk.virtualWrite(V13, "Reset!");
            // Update V15 and V16 immediately to show the change
            syncSettingsToBlynk();
            delay(1000);
            Blynk.virtualWrite(V13, "Reset Timer");
        }
        else
        {
            Serial.println("Cannot Reset Timer: Invalid index (Blynk).");
            Blynk.virtualWrite(V13, "Select Timer");
            delay(1000);
            Blynk.virtualWrite(V13, "Reset Timer");
        }
    }
}