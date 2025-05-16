#ifndef BLYNK_HANDLER_H
#define BLYNK_HANDLER_H

#include "globals.h"
// #include <BlynkSimpleEsp32.h>

// Function Declarations
void setupBlynk();              // Configures Blynk connection parameters
void runBlynk();                // Calls Blynk.run() if connected
void sendSensorDataBlynk();     // Sends sensor readings to Blynk virtual pins
void syncSettingsToBlynk();     // Sends current settings (min, max, limits) to Blynk
void syncRelayStatusExtended(); // Sends status of all relays to Blynk LEDs
void notifyBlynk(String msg);   // Sends a notification via Blynk (with rate limiting)
bool isBlynkConnected();        // Returns true if Blynk is currently connected

void blynkVirtualWriteInt(int pin, int value);
void blynkVirtualWriteFloat(int pin, float value);
void blynkVirtualWriteString(int pin, const String &value); // Use String object
void blynkVirtualWriteString(int pin, const char *value);   // Use C-style string
// Blynk Virtual Pin Write Handlers (must be declared here)
// BLYNK_WRITE(V2);  // Min Temp Threshold
// BLYNK_WRITE(V3);  // Max Temp Threshold
// BLYNK_WRITE(V7);  // Start Timer Auto Adjusting (Button)
// BLYNK_WRITE(V8);  // Mode Selector (Segmented Switch)
// BLYNK_WRITE(V9);  // Timer Day Index Selector (Slider/Step)
// BLYNK_WRITE(V11); // Add New Timer Entry (Button)
// BLYNK_WRITE(V12); // Save Timer Data (Button)
// BLYNK_WRITE(V13); // Reset Selected Timer Entry (Button)
// BLYNK_WRITE(V14); // Delete Last Timer Entry (Button)
// BLYNK_WRITE(V15); // Selected Timer Min Temp (Slider)
// BLYNK_WRITE(V16); // Selected Timer Max Temp (Slider)
// BLYNK_WRITE(V17); // Gas Limit (Slider)
// BLYNK_WRITE(V18); // Humidity Limit (Slider)
// V0, V1, V5, V6, V10 are for sensor readings (handled by sendSensorDataBlynk)
// V20, V21, V22 are for displaying timer info (handled by updateRTCandDS3231Data -> send via Blynk)
// V23-V26 are for relay status LEDs (handled by syncRelayStatusExtended)
// V99 is for heartbeat

#endif // BLYNK_HANDLER_H