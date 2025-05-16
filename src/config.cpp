// config.cpp
#include "config.h" // Include the header to ensure consistency

//==================================
// Blynk & Wi-Fi configuration Definitions
//==================================

// Preferred networks - Define here
const char *ssid1 = "Hoshimi Miyabi";     // Replace with your SSID
const char *pass1 = "vduy2008";           // Replace with your Password
const char *ssid2 = "Tran Huu Gia Trang"; // Replace with your Secondary SSID
const char *pass2 = "0993985297";         // Replace with your Secondary Password

// Custom Blynk server info - Define here
const char *blynkServer = "blynk.dke.vn";
const uint16_t blynkPort = 8888;

// AP Mode Config - Define here
const char *apSSID = "ESP32_Control";
const char *apPass = "esp32access";

// Note: BLYNK_AUTH_TOKEN is a #define, not a variable, so it stays in config.h
// Note: Pin numbers are #defines, they stay in config.h