#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "globals.h"
#include <RTClib.h>     // For DateTime
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

// Setup function
void setupWiFiAndServer();

// --- CORRECTED Handler Declarations ---
void handleRoot();
void handleCSS();
void handleJS();
void handleGaugeJS();       // Handler for local gauge.js
void handleStatus();
void handleSetMode();
void handleSetSettings();
void handleSyncTime();
void handleSetTimerEntry();
void handleResetTimerEntry();
// --- END Corrections ---

// Main loop function
void handleWebRequests();

// --- REMOVED Unused Declarations ---
// void handleData();
// void handleWebToggle();
// void handleWebRelayStatus();
// --- END Removed ---

#endif // WIFI_MANAGER_H