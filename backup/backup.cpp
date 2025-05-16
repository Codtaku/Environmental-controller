// #define BLYNK_AUTH_TOKEN "bp_m7_HBWnyqUnCiY2uBpK7HWD1AJqeG"
// #define BLYNK_PRINT Serial

// #include <Arduino.h>
// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>
// #include <EEPROM.h>
// #include <OneWire.h>
// #include <DallasTemperature.h>
// #include <DHT.h>
// #include <LiquidCrystal_I2C.h>
// #include <FS.h>
// #include <SPIFFS.h>
// #include <WebServer.h>
// #include <Wire.h>
// #include <RTClib.h>     // For DS3231 RTC

// //==================================
// // Blynk & Wi-Fi configuration 
// //==================================
// char auth[] = "bp_m7_HBWnyqUnCiY2uBpK7HWD1AJqeG";

// // Preferred networks
// const char* ssid1 = "Hoshimi Miyabi";
// const char* pass1 = "vduy2008";
// const char* ssid2 = "Tran Huu Gia Trang";
// const char* pass2 = "0993985297";

// // Custom Blynk server info
// const char* blynkServer = "blynk.iot-cm.com";
// const uint16_t blynkPort = 8080;

// bool offlineMode = false; // true if connection to Blynk fails

// //==================================
// // Peripherals and system configuration
// //==================================

// // DS18B20 for temperature measurement
// #define OneWire_BUS 17
// OneWire oneWire(OneWire_BUS);
// DallasTemperature ds18b20(&oneWire);

// // DHT for humidity measurement
// #define DHT_BUS      4       // Data pin for DHT sensor
// #define MQ135_BUS    34      // Analog pin for MQ135 gas sensor
// #define SOIL_SENSOR  35      // Soil moisture sensor pin

// #define DHT_TYPE     DHT22   // Use DHT22
// DHT dht(DHT_BUS, DHT_TYPE);

// // Buzzer and relay pins
// #define BUZZER_PIN   16
// #define relay1       32   // Temperature relay 1
// #define relay2       33   // Temperature relay 2
// #define relay3       25   // Humidity/Gas relay (active low)
// #define relay4       26   // Humidity relay (misting system; active low)
  
// // LCD configuration
// #define LCD_ADDRESS 0x27
// #define LCD_HEIGHT 4
// #define LCD_LENGTH 20
// LiquidCrystal_I2C screen(LCD_ADDRESS, LCD_LENGTH, LCD_HEIGHT);

// // Rotary Encoder pins
// #define ENCODER_PIN_A 19
// #define ENCODER_PIN_B 18
// #define ENCODER_BUTTON 5

// //==================================
// // Global variables & settings
// //==================================
// volatile int mod = 0;    // Mode variable; 0:AUTO, 1:OFF, 2:ON, 3:HALF, 4:TIMER
// volatile int ma = 32;
// volatile int mi = 28;
// volatile float t, newTemp;    // Temperature from DS18B20 primary, fallback from DHT
// volatile float h;             // Humidity from DHT
// unsigned long lastDHTRead = 0;
// const unsigned long DHT_READ_INTERVAL = 2000;  // 2 seconds

// volatile unsigned long switchingDelay = 5; // Switching delay in seconds for relays
// volatile int humLimit = 60;   // Humidity limit (%). Now can be updated via Blynk V18.
// int gasLimit = 2000;          // Gas limit (0–4095), adjustable via Blynk V17

// // LCD screen management
// volatile int screenMode = 1;         // 1 = Main, 2 = Settings
// volatile int settingsState = 0;      // 0 = navigation, 1 = editing
// volatile int selectedField = 0;
// unsigned long lastInteraction = 0;
// const unsigned long SCREEN2_TIMEOUT = 300000; // 5 minutes
// unsigned long lastBlynkUpdate = 0;
// const unsigned long BLYNK_UPDATE_INTERVAL = 10000; // 10 seconds
// int lastSentMi = -1;
// int lastSentMa = -1;
// int lastSentHumLimit = -1;
// int lastSentGasLimit = -1;
// float lastSentT = -100;
// float lastSentH = -1;
// float lastSentSoil = -1;
// int lastSentGas = -1;
// static bool lastBlynkConnected = false;
// unsigned long lastConnectionCheck = 0;
// const unsigned long CONNECTION_CHECK_INTERVAL = 5000; // 5 seconds

// int settingsMenuOffset = 0;
// const int SETTINGS_TOTAL_ITEMS = 7;
// const int VISIBLE_SETTINGS_LINES = 4;

// // LCD update interval timing
// unsigned long lastLcdClear = 0;
// const unsigned long LCD_CLEAR_INTERVAL = 1000;

// // Rotary encoder state
// volatile int lastA = HIGH;
// volatile long encoderPos = 0;
// volatile bool encoderUpdated = false;

// // Button press detection
// unsigned long buttonHoldStart = 0;
// bool longPressTriggered = false;
// bool lastButtonState = HIGH;

// //==================================
// // EEPROM configuration
// //==================================
// // Existing settings (mi, ma, switchingDelay, humLimit) occupy 16 bytes.
// #define EEPROM_SIZE 64
// #define EEPROM_ADDR_MIN 0          // 4 bytes
// #define EEPROM_ADDR_MAX 4          // 4 bytes
// #define EEPROM_ADDR_DELAY 8        // 4 bytes
// #define EEPROM_ADDR_HUM_LIMIT 12   // 4 bytes
// // Timer mode data:
// #define EEPROM_ADDR_TIMER_COUNT 16
// #define EEPROM_ADDR_TIMER_START_DAY 17
// #define EEPROM_ADDR_TIMERS 21
// #define MAX_TIMERS 5
// #define TIMER_ENTRY_SIZE 6
// // Gas limit stored in EEPROM:
// #define EEPROM_ADDR_GAS_LIMIT 30   // 4 bytes

// //==================================
// // RTC DS3231 instance
// //==================================
// RTC_DS3231 rtc;
// unsigned long lastNtpSync = 0;
// const unsigned long NTP_SYNC_INTERVAL = 5000; // Sync every 1 hour (in milliseconds)
// bool initialNtpSyncDone = false;

// //==================================
// // Timer Mode Structures and Globals
// //==================================
// struct TimerEntry {
//   uint16_t dayOffset;  // Day offset (e.g. 0 for start day, 1 for next day, etc.)
//   int minTemp;
//   int maxTemp;
// };

// TimerEntry timerEntries[MAX_TIMERS];
// uint8_t timerCount = 0;      // Number of timer entries stored
// uint32_t timerStartDay = 0;  // Day (in days since epoch) when timer mode was started
// uint8_t currentTimerIndex = 0;  // Selected timer entry for adjustments via Blynk

// // Connection states
// enum ConnectionStatus {
//   CONNECTING,
//   CONNECTED,
//   OFFLINE
// };

// volatile ConnectionStatus connectionStatus = CONNECTING;
// //==================================
// // Notification timing
// //==================================
// unsigned long lastNotifyTime = 0;
// const unsigned long NOTIFY_INTERVAL = 30 * 60 * 1000; // 30 minutes in milliseconds

// // Helper function to send a notification only if enough time has passed
// void notifyBlynk(String msg) {
//   if (millis() - lastNotifyTime > NOTIFY_INTERVAL) {
//     Blynk.notify(msg);
//     lastNotifyTime = millis();
//   }
// }

// //==================================
// // Web Server (for AP mode)
// //==================================
// WebServer server(80);

// //==================================
// // LCD Update Helper Functions
// //==================================
// #define LCD_NUM_LINES LCD_HEIGHT
// #define LCD_LINE_LEN LCD_LENGTH
// static char prevLines[LCD_NUM_LINES][LCD_LINE_LEN + 1] = { "", "", "", "" };

// void printLine(int line, const String &text) {
//   String padded = text;
//   while (padded.length() < LCD_LINE_LEN) {
//     padded += " ";
//   }
//   if (padded != String(prevLines[line])) {
//     screen.setCursor(0, line);
//     screen.print(padded);
//     padded.toCharArray(prevLines[line], LCD_LINE_LEN + 1);
//   }
// }

// void clearLineCache() {
//   for (int i = 0; i < LCD_NUM_LINES; i++) {
//     prevLines[i][0] = '\0';
//   }
// }

// //==================================
// // Forward declarations
// //==================================
// void readEncoder();
// void checkEncoderButton();
// void connectWiFiBlynk();
// void startAPMode();
// void handleWebRoot();
// void updateTemperatureRelay();
// void updateHumidityRelays();
// void timerRelayControl();
// void updateRTCandDS3231Data();
// void sendSensorDataBlynk();
// void syncRelayStatusExtended();
// float readSoilMoisture();

// //==================================
// // Encoder task (runs on Core 1)
// //==================================
// void encoderTask(void * parameter) {
//   for(;;) {
//     readEncoder();
//     vTaskDelay(5 / portTICK_PERIOD_MS);
//   }
// }

// void readEncoder() {
//   int a = digitalRead(ENCODER_PIN_A);
//   int b = digitalRead(ENCODER_PIN_B);
//   if (lastA == HIGH && a == LOW) {
//     if (b == HIGH)
//       encoderPos++;
//     else
//       encoderPos--;
//     encoderUpdated = true;
//     lastInteraction = millis();
//   }
//   lastA = a;
// }

// void beepEncoder() {
//   digitalWrite(BUZZER_PIN, HIGH);
//   digitalWrite(BUZZER_PIN, LOW);
// }

// //==================================
// // Temperature Relay Control (AUTO mode) with Extreme Condition override
// //==================================
// void updateTemperatureRelay() {
//   static int currentAutoZone = -1;
//   static int lastTargetZone = -1;
//   static unsigned long zoneChangeStart = 0;
  
//   int targetZone;
//   // Check for extreme temperature deviation (more than 7° out of range)
//   if (t < mi - 7 || t > ma + 7) {
//     targetZone = 1;  // intermediate zone
//     notifyBlynk("Extreme temperature condition detected!");
//   } else {
//     if (t <= mi)
//       targetZone = 0;    // cold zone: both relays LOW
//     else if (t > mi && t < ma)
//       targetZone = 1;    // moderate: relay1 LOW, relay2 HIGH
//     else
//       targetZone = 2;    // hot: both relays HIGH
//   }
  
//   if (targetZone != lastTargetZone) {
//     lastTargetZone = targetZone;
//     zoneChangeStart = millis();
//   }
  
//   if (millis() - zoneChangeStart >= switchingDelay * 1000UL && currentAutoZone != targetZone) {
//     currentAutoZone = targetZone;
//     // Set relay1 and relay2 based on target zone:
//     if (targetZone == 0) {
//       digitalWrite(relay1, LOW);
//       digitalWrite(relay2, LOW);
//     } else if (targetZone == 1) {
//       digitalWrite(relay1, LOW);
//       digitalWrite(relay2, HIGH);
//     } else { // targetZone == 2
//       digitalWrite(relay1, HIGH);
//       digitalWrite(relay2, HIGH);
//     }
//   }
// }

// //==================================
// // EEPROM Save/Load for settings, Gas limit, and Timer Mode data
// //==================================
// void saveSettingsEEPROM() {
//   EEPROM.put(EEPROM_ADDR_MIN, mi);
//   EEPROM.put(EEPROM_ADDR_MAX, ma);
//   EEPROM.put(EEPROM_ADDR_DELAY, switchingDelay);
//   EEPROM.put(EEPROM_ADDR_HUM_LIMIT, humLimit);
//   EEPROM.put(EEPROM_ADDR_TIMER_COUNT, timerCount);
//   EEPROM.put(EEPROM_ADDR_TIMER_START_DAY, timerStartDay);
//   EEPROM.put(EEPROM_ADDR_GAS_LIMIT, gasLimit);
//   for (uint8_t i = 0; i < timerCount; i++) {
//     int addr = EEPROM_ADDR_TIMERS + i * TIMER_ENTRY_SIZE;
//     EEPROM.put(addr, timerEntries[i].dayOffset);
//     EEPROM.put(addr + 2, timerEntries[i].minTemp);
//     EEPROM.put(addr + 4, timerEntries[i].maxTemp);
//   }
//   EEPROM.commit();
// }

// //==================================
// // Humidity and Gas Control for relay3 & relay4
// //==================================
// void updateHumidityRelays() {
//   int gasVal = analogRead(MQ135_BUS);
//   bool humCommand = false;
//   bool mistCommand = false;
  
//   // Humidity conditions
//   if (h > humLimit + 5) {
//     humCommand = true;
//   } else if (h < humLimit - 5) {
//     mistCommand = true;
//   } else {
//     if (h >= humLimit)
//       humCommand = true;
//   }
  
//   // Gas override: Activate dehumidify if gas exceeds limit
//   if (gasVal > gasLimit) {
//     humCommand = true;
//     if (gasVal > gasLimit * 1.5) {
//       notifyBlynk("High gas reading detected!");
//     }
//   }
  
//   digitalWrite(relay3, humCommand ? LOW : HIGH);
//   digitalWrite(relay4, mistCommand ? LOW : HIGH);
// }

// //==================================
// // Timer Mode Relay Control using RTC-adjusted thresholds
// //==================================
// void timerRelayControl() {
//   // Obtain current day count from RTC (days since epoch)
//   DateTime now = rtc.now();
//   uint32_t currentDay = now.unixtime() / 86400;
  
//   // Automatically add a new timer entry for the new day (default 28/32) if needed
//   static uint32_t lastDayAdjusted = 0;
//   if (currentDay > lastDayAdjusted) {
//     if (timerCount < MAX_TIMERS) {
//       uint16_t newOffset = (timerCount > 0) ? timerEntries[timerCount - 1].dayOffset + 1 : 0;
//       timerEntries[timerCount].dayOffset = newOffset;
//       timerEntries[timerCount].minTemp = 28;
//       timerEntries[timerCount].maxTemp = 32;
//       timerCount++;
//       saveSettingsEEPROM();
//       notifyBlynk("New auto-adjust timer entry added.");
//     }
//     lastDayAdjusted = currentDay;
//   }
  
//   // Compute days passed since timer mode started
//   int daysPassed = currentDay - timerStartDay;
//   if (daysPassed < 0) daysPassed = 0;
  
//   // Determine effective min/max from timer entries (default values if no entry applies)
//   int effectiveMin = 28;
//   int effectiveMax = 32;
//   for (uint8_t i = 0; i < timerCount; i++) {
//     if (timerEntries[i].dayOffset <= (uint16_t)daysPassed) {
//       effectiveMin = timerEntries[i].minTemp;
//       effectiveMax = timerEntries[i].maxTemp;
//     }
//   }
  
//   // Similar to the auto mode for temperature relays:
//   static int currentTimerZone = -1;
//   static int lastTimerZone = -1;
//   static unsigned long timerZoneChangeStart = 0;
  
//   int targetZone;
//   if (t <= effectiveMin)
//     targetZone = 0;
//   else if (t > effectiveMin && t < effectiveMax)
//     targetZone = 1;
//   else
//     targetZone = 2;
  
//   if (targetZone != lastTimerZone) {
//     lastTimerZone = targetZone;
//     timerZoneChangeStart = millis();
//   }
  
//   if (millis() - timerZoneChangeStart >= switchingDelay * 1000UL && currentTimerZone != targetZone) {
//     currentTimerZone = targetZone;
//     if (targetZone == 0) {
//       digitalWrite(relay1, LOW);
//       digitalWrite(relay2, LOW);
//     } else if (targetZone == 1) {
//       digitalWrite(relay1, LOW);
//       digitalWrite(relay2, HIGH);
//     } else {
//       digitalWrite(relay1, HIGH);
//       digitalWrite(relay2, HIGH);
//     }
//   }
// }

// //==================================
// // Modeset: Decide which relay control function to use
// //==================================
// void modeset() {
//   int m = abs(mod) % 5; // 0=AUTO, 1=OFF, 2=ON, 3=HALF, 4=TIMER
//   if (m == 0) {
//     updateTemperatureRelay();
//   }
//   else if (m == 1) {
//     digitalWrite(relay1, HIGH);
//     digitalWrite(relay2, HIGH);
//   }
//   else if (m == 2) {
//     digitalWrite(relay1, LOW);
//     digitalWrite(relay2, LOW);
//   }
//   else if (m == 3) {
//     digitalWrite(relay1, LOW);
//     digitalWrite(relay2, HIGH);
//   }
//   else if (m == 4) {
//     timerRelayControl();
//   }
// }

// //==================================
// // LCD display: Main Screen
// //==================================
// void lcdScreen1() {
//   String line0 = "T:" + String(t, 1) + "C H:" + String(h, 1) + "%";
//   String line1 = "Min:" + String(mi) + " Max:" + String(ma);
//   String line2 = "R1:" + String(digitalRead(relay1)==HIGH ? "OFF" : "ON") +
//                  " R2:" + String(digitalRead(relay2)==HIGH ? "OFF" : "ON");
//   String line3;
//   switch (connectionStatus) {
//     case CONNECTING:
//       line3 = "Connecting...";
//       break;
//     case CONNECTED:
//       line3 = "Online";
//       break;
//     case OFFLINE:
      
//         line3 = "Offline";
//       break;
//     }
  
//     printLine(0, line0);
//     printLine(1, line1);
//     printLine(2, line2);
//     printLine(3, line3);
// }

// //==================================
// // LCD display: Settings Screen (for main settings)
// //==================================
// void lcdSettingsScreen() {
//   String lineText;
//   if (settingsState == 0) {    // Navigation mode
//     if (selectedField < settingsMenuOffset)
//       settingsMenuOffset = selectedField;
//     else if (selectedField >= settingsMenuOffset + VISIBLE_SETTINGS_LINES)
//       settingsMenuOffset = selectedField - VISIBLE_SETTINGS_LINES + 1;
    
//     for (int i = 0; i < VISIBLE_SETTINGS_LINES; i++) {
//       int itemIndex = settingsMenuOffset + i;
//       if (itemIndex >= SETTINGS_TOTAL_ITEMS) {
//         printLine(i, "                    ");
//         continue;
//       }
//       lineText = (itemIndex == selectedField) ? ">" : " ";
//       switch (itemIndex) {
//         case 0:
//           lineText += "Back";
//           break;
//         case 1:
//           lineText += "Min:" + String(mi);
//           break;
//         case 2:
//           lineText += "Max:" + String(ma);
//           break;
//         case 3: {
//           lineText += "Mode:";
//           int m = abs(mod) % 5;
//           if (m == 0) lineText += "AUTO";
//           else if (m == 1) lineText += "OFF";
//           else if (m == 2) lineText += "ON";
//           else if (m == 3) lineText += "HALF";
//           else lineText += "TIMER";
//           break;
//         }
//         case 4:
//           lineText += "Delay:" + String(switchingDelay) + " s";
//           break;
//         case 5:
//           lineText += "HumLim:" + String(humLimit) + "%";
//           break;
//           case 6:
//           lineText += "Reconnect";
//           break;
//       }
//       printLine(i, lineText);
//     }
//   }
//   else if (settingsState == 1) { // Editing mode
//     switch(selectedField) {
//       case 1:
//         printLine(0, "Edit Min:");
//         printLine(1, String(mi));
//         printLine(2, "");
//         printLine(3, "Press to save");
//         break;
//       case 2:
//         printLine(0, "Edit Max:");
//         printLine(1, String(ma));
//         printLine(2, "");
//         printLine(3, "Press to save");
//         break;
//       case 3: {
//         String modeStr;
//         int m = abs(mod) % 5;
//         if(m==0) modeStr = "AUTO";
//         else if(m==1) modeStr = "OFF";
//         else if(m==2) modeStr = "ON";
//         else if(m==3) modeStr = "HALF";
//         else modeStr = "TIMER";
//         printLine(0, "Edit Mode:");
//         printLine(1, modeStr);
//         printLine(2, "");
//         printLine(3, "Press to save");
//         break;
//       }
//       case 4:
//         printLine(0, "Edit Delay:");
//         printLine(1, String(switchingDelay) + " s");
//         printLine(2, "");
//         printLine(3, "Press to save");
//         break;
//       case 5:
//         printLine(0, "Edit HumLim:");
//         printLine(1, String(humLimit) + "%");
//         printLine(2, "");
//         printLine(3, "Press to save");
//         break;
//       default:
//         break;
//     }
//   }
// }

// void loadSettingsEEPROM() {
//   EEPROM.get(EEPROM_ADDR_MIN, mi);
//   EEPROM.get(EEPROM_ADDR_MAX, ma);
//   EEPROM.get(EEPROM_ADDR_DELAY, switchingDelay);
//   EEPROM.get(EEPROM_ADDR_HUM_LIMIT, humLimit);
//   EEPROM.get(EEPROM_ADDR_TIMER_COUNT, timerCount);
//   EEPROM.get(EEPROM_ADDR_TIMER_START_DAY, timerStartDay);
//   EEPROM.get(EEPROM_ADDR_GAS_LIMIT, gasLimit);
//   if (ma == -1) ma = 32;
//   if (mi == -1) mi = 28;
//   if (switchingDelay == 0 || switchingDelay > 60) switchingDelay = 5;
//   if (humLimit == 0 || humLimit > 100) humLimit = 60;
//   if (timerCount > MAX_TIMERS) timerCount = MAX_TIMERS;
//   for (uint8_t i = 0; i < timerCount; i++) {
//     int addr = EEPROM_ADDR_TIMERS + i * TIMER_ENTRY_SIZE;
//     EEPROM.get(addr, timerEntries[i].dayOffset);
//     EEPROM.get(addr + 2, timerEntries[i].minTemp);
//     EEPROM.get(addr + 4, timerEntries[i].maxTemp);
//   }
// }

// //==================================
// // Encoder button logic for screen navigation and restart
// //==================================
// void checkEncoderButton() {
//   bool currentButtonState = digitalRead(ENCODER_BUTTON);
//   if (lastButtonState == HIGH && currentButtonState == LOW) {
//     buttonHoldStart = millis();
//     beepEncoder();
//   }
//   if (currentButtonState == LOW) {
//     if (!longPressTriggered && (millis() - buttonHoldStart >= 5000)) {
//       longPressTriggered = true;
//       screen.clear();
//       clearLineCache();
//       screen.setCursor(0, 0);
//       screen.print("Restarting...");
//       delay(1000);
//       ESP.restart();
//     }
//   }
//   if (lastButtonState == LOW && currentButtonState == HIGH) {
//     if (!longPressTriggered) {
//       if (screenMode == 1) {  
//         screenMode = 2;
//         settingsState = 0;
//         selectedField = 0;
//         settingsMenuOffset = 0;
//         clearLineCache();
//       }
//       else if (screenMode == 2) {
//         if (settingsState == 0) { 
//           if (selectedField == 0) {
//             screenMode = 1;
//             saveSettingsEEPROM();
//             clearLineCache();
//           } else {
//             settingsState = 1;
//             clearLineCache();
//           }
//           if (selectedField == 6) { // Reconnect selected
//             connectionStatus = CONNECTING;
//             connectWiFiBlynk();
//             screenMode = 1;
//             clearLineCache();
//           }
//         }
//         else if (settingsState == 1) {
//           settingsState = 0;
//           saveSettingsEEPROM();
//           clearLineCache();
//         }
//       }
//       lastInteraction = millis();
//     }
//     buttonHoldStart = 0;
//     longPressTriggered = false;
//   }
//   lastButtonState = currentButtonState;
  
// }

// //==================================
// // Blynk callbacks for sensor and control updates
// //==================================

// // For DS18B20 and DHT sensor updates
// void sendTemperature() {
//   if(t != lastSentT) {
//     Blynk.virtualWrite(V0, t);
//     lastSentT = t;
//   }
//   if(h != lastSentH) {
//     Blynk.virtualWrite(V1, h);
//     lastSentH = h;
//   }
//   if(mi != lastSentMi) {
//     Blynk.virtualWrite(V2, mi);
//     lastSentMi = mi;
//   }
//   if(ma != lastSentMa) {
//     Blynk.virtualWrite(V3, ma);
//     lastSentMa = ma;
//   }
// }

// // Update gas sensor reading on V? (we still use V6 for raw gas value)
// void sendGasSensor() {
//   int gasVal = analogRead(MQ135_BUS);
//   if(abs(gasVal - lastSentGas) > 50) { // Only send if change > 50
//     Blynk.virtualWrite(V6, gasVal);
//     lastSentGas = gasVal;
//   }
// }

// // Blynk callback to update min threshold
// BLYNK_WRITE(V2) {
//   int newMi = param.asInt();
//   if(newMi != mi) {
//     mi = newMi;
//     if(mi > ma) mi = ma;
//     lastSentMi = mi; // Prevent immediate resend
//     saveSettingsEEPROM();
//     Blynk.virtualWrite(V2, mi);
//   }
// }

// // Blynk callback to update max threshold
// BLYNK_WRITE(V3) {
//   int newMa = param.asInt();
//   if(newMa != ma) {
//     ma = newMa;
//     if(mi > ma) ma = mi;
//     lastSentMa = ma; // Prevent immediate resend
//     saveSettingsEEPROM();
//     Blynk.virtualWrite(V3, ma);
//   }
// }

// //------------------------------------
// // Blynk callbacks for Timer Mode Settings
// //------------------------------------
// // V9: Choose day (index) to modify
// BLYNK_WRITE(V9) {
//   int idx = param.asInt();
//   if (idx >= 0 && idx < timerCount) {
//     currentTimerIndex = idx;
//     Blynk.virtualWrite(V15, timerEntries[currentTimerIndex].minTemp);
//     Blynk.virtualWrite(V16, timerEntries[currentTimerIndex].maxTemp);
//   }
// }

// // V15: Timer min value adjustment
// BLYNK_WRITE(V15) {
//   int newMin = param.asInt();
//   if (currentTimerIndex < timerCount) {
//     timerEntries[currentTimerIndex].minTemp = newMin;
//   }
// }

// // V16: Timer max value adjustment
// BLYNK_WRITE(V16) {
//   int newMax = param.asInt();
//   if (currentTimerIndex < timerCount) {
//     timerEntries[currentTimerIndex].maxTemp = newMax;
//   }
// }

// // V12: Save timer data (button)
// BLYNK_WRITE(V12) {
//   saveSettingsEEPROM();
//   Blynk.virtualWrite(V12, "Timer saved");
// }

// // V11: Add new timer entry (button)
// BLYNK_WRITE(V11) {
//   if (timerCount < MAX_TIMERS) {
//     uint16_t newOffset = (timerCount > 0) ? timerEntries[timerCount - 1].dayOffset + 1 : 0;
//     timerEntries[timerCount].dayOffset = newOffset;
//     timerEntries[timerCount].minTemp = 28;
//     timerEntries[timerCount].maxTemp = 32;
//     timerCount++;
//     saveSettingsEEPROM();
//     Blynk.virtualWrite(V11, "Timer added");
//   } else {
//     Blynk.virtualWrite(V11, "Max timers reached");
//   }
// }

// // V14: Delete the latest timer entry (button)
// BLYNK_WRITE(V14) {
//   if (timerCount > 0) {
//     timerCount--;
//     saveSettingsEEPROM();
//     Blynk.virtualWrite(V14, "Timer deleted");
//   } else {
//     Blynk.virtualWrite(V14, "No timer to delete");
//   }
// }

// // V13: Reset chosen timer entry to default (28/32)
// BLYNK_WRITE(V13) {
//   if (currentTimerIndex < timerCount) {
//     timerEntries[currentTimerIndex].minTemp = 28;
//     timerEntries[currentTimerIndex].maxTemp = 32;
//     saveSettingsEEPROM();
//     Blynk.virtualWrite(V13, "Timer reset");
//   }
// }

// //------------------------------------
// // New Blynk callbacks for additional settings
// //------------------------------------
// // V17: Gas limit slider
// BLYNK_WRITE(V17) {
//   int newGasLimit = param.asInt();
//   if(newGasLimit != gasLimit) {
//     gasLimit = newGasLimit;
//     lastSentGasLimit = gasLimit;
//     saveSettingsEEPROM();
//   }
// }

// // V18: Humidity limit slider
// BLYNK_WRITE(V18) {
//   int newHumLimit = param.asInt();
//   if(newHumLimit != humLimit) {
//     humLimit = newHumLimit;
//     lastSentHumLimit = humLimit;
//     saveSettingsEEPROM();
//   }
// }

// // V8: Mode segmented switch (0=AUTO,1=OFF,2=ON,3=HALF,4=TIMER)
// BLYNK_WRITE(V8) {
//   mod = param.asInt();
//   Blynk.virtualWrite(V8, mod);
// }

// // V7: Start auto adjusting (timer mode) – set start day from RTC
// BLYNK_WRITE(V7) {
//   DateTime now = rtc.now();
//   timerStartDay = now.unixtime() / 86400;
//   saveSettingsEEPROM();
//   char buf[12];
//   sprintf(buf, "%02d/%02d/%02d", now.day(), now.month(), now.year() % 100);
//   Blynk.virtualWrite(V20, buf);  // V20 shows day started (dd/mm/yy)
// }

// //------------------------------------
// // DS3231 RTC functions and DS3231 temperature update (case temperature)
// //------------------------------------
// void updateRTCandDS3231Data() {
//   // Update RTC time from NTP less frequently
//   if (!offlineMode && WiFi.status() == WL_CONNECTED) {
//     unsigned long nowMillis = millis();
//     // Sync if it's the first time after connection OR if interval has passed
//     if (!initialNtpSyncDone || (nowMillis - lastNtpSync > NTP_SYNC_INTERVAL)) {
//       Serial.println("Attempting NTP Sync..."); // Add log to see when sync happens
//       configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // GMT+7 for Vietnam
//       struct tm timeinfo;
//       if (getLocalTime(&timeinfo, 5000)) { // Add a timeout (e.g., 5000ms)
//         Serial.println("NTP Sync Successful.");
//         DateTime ntpTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
//                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
//         rtc.adjust(ntpTime);  // Sync DS3231 with network time
//         lastNtpSync = nowMillis; // Update last sync time
//         initialNtpSyncDone = true; // Mark initial sync as done

//         // Initialize timerStartDay only if it hasn't been set
//         if (timerStartDay == 0) {
//           timerStartDay = ntpTime.unixtime() / 86400;
//           saveSettingsEEPROM();
//           char buf[12];
//           sprintf(buf, "%02d/%02d/%02d", ntpTime.day(), ntpTime.month(), ntpTime.year() % 100);
//           Blynk.virtualWrite(V20, buf);
//         }
//       } else {
//          Serial.println("NTP Sync Failed.");
//          // Optional: Handle failure, maybe retry sooner?
//       }
//     }
//   } else {
//       // Reset sync flag if we go offline
//       initialNtpSyncDone = false;
//   }

//   // Update DS3231 internal temperature (system box temperature) on V10
//   float rtcTemp = rtc.getTemperature();
//   Blynk.virtualWrite(V10, rtcTemp); // This is likely low-impact

//   // For auto adjusting details, update V21 (days since start) and V22 (timer count)
//   DateTime nowRTC = rtc.now();
//   uint32_t currentDay = nowRTC.unixtime() / 86400;
//   int daysElapsed = (timerStartDay > 0) ? (currentDay - timerStartDay) : 0;
//   if (daysElapsed < 0) daysElapsed = 0; // Sanity check
//   Blynk.virtualWrite(V21, daysElapsed);
//   Blynk.virtualWrite(V22, timerCount);
// }

// //==================================
// // Extended relay status sync: V23 - V26 for relay1-4
// //==================================
// void syncRelayStatusExtended() {
//   Blynk.virtualWrite(V23, digitalRead(relay1)==HIGH ? 0 : 255);
//   Blynk.virtualWrite(V24, digitalRead(relay2)==HIGH ? 0 : 255);
//   Blynk.virtualWrite(V25, digitalRead(relay3)==HIGH ? 0 : 255);
//   Blynk.virtualWrite(V26, digitalRead(relay4)==HIGH ? 0 : 255);
// }

// //==================================
// // Soil moisture reading (convert analog to %)
// //==================================
// float readSoilMoisture() {
//   int sensorVal = analogRead(SOIL_SENSOR);
//   // Map such that 4095 -> 0% and 0 -> 100%
//   float percentage = (4095 - sensorVal) * 100.0 / 4095;
//   return percentage;
// }

// //==================================
// // Send all sensor data to Blynk (including DS3231, gas sensor, and soil moisture)
// //==================================
// void sendSensorDataBlynk() {
//   if (!Blynk.connected()) return;
//   if(millis() - lastBlynkUpdate > BLYNK_UPDATE_INTERVAL) {
//     sendTemperature();
//     sendGasSensor();
    
//     // Soil moisture with 1% threshold
//     float soilMoist = readSoilMoisture();
//     if(fabs(soilMoist - lastSentSoil) > 1.0) {
//       Blynk.virtualWrite(V5, soilMoist);
//       lastSentSoil = soilMoist;
//       if(soilMoist > 35) notifyBlynk("Soil moisture above threshold!");
//     }
    
//     // Only send settings if changed
//     if(humLimit != lastSentHumLimit) {
//       Blynk.virtualWrite(V18, humLimit);
//       lastSentHumLimit = humLimit;
//     }
//     if(gasLimit != lastSentGasLimit) {
//       Blynk.virtualWrite(V17, gasLimit);
//       lastSentGasLimit = gasLimit;
//     }
    
//     syncRelayStatusExtended();
//     lastBlynkUpdate = millis();
//   }
// }

// //==================================
// // WiFi and Blynk connection logic
// //==================================
// void connectWiFiBlynk() {
//   connectionStatus = CONNECTING;
//   Serial.println("Scanning for preferred WiFi networks...");
//   int n = WiFi.scanNetworks();
//   bool foundPreferred = false;
//   String chosenSSID = "";
//   String chosenPass = "";
  
//   for (int i = 0; i < n; i++) {
//     String ssidFound = WiFi.SSID(i);
//     if (ssidFound.indexOf(ssid1) != -1) {
//       chosenSSID = ssid1;
//       chosenPass = pass1;
//       foundPreferred = true;
//       break;
//     } else if (ssidFound.indexOf(ssid2) != -1) {
//       chosenSSID = ssid2;
//       chosenPass = pass2;
//       foundPreferred = true;
//       break;
//     }
//   }
  
//   if (foundPreferred) {
//     Serial.print("Found preferred network: ");
//     Serial.println(chosenSSID);
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(chosenSSID.c_str(), chosenPass.c_str());
//     unsigned long startAttemptTime = millis();
//     while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
//       delay(100);
//     }
//   }
  
//   // if (WiFi.status() != WL_CONNECTED) {
//   //   Serial.println("Preferred network not found. Starting AP mode.");
//   //   startAPMode();
//   //   offlineMode = true;
//   // } else {
//   //   Serial.println("WiFi connected.");
//   //   offlineMode = false;
//   // }
//   if (WiFi.status() == WL_CONNECTED) {
//     connectionStatus = CONNECTED;
//     offlineMode = false;
//   } else {
//     connectionStatus = OFFLINE;
//     offlineMode = true;
//     //startAPMode();
//   }
//   Blynk.config(auth, blynkServer, blynkPort);
//   if (!offlineMode) {
//     Blynk.connect();
//   }
//   if (WiFi.status() == WL_CONNECTED) {
//     Blynk.config(auth, blynkServer, blynkPort);
//     if (Blynk.connect(5000)) { // 5s timeout
//       connectionStatus = CONNECTED;
//       offlineMode = false;
//       lastBlynkConnected = true;
//     } else {
//       connectionStatus = OFFLINE;
//       offlineMode = true;
//       lastBlynkConnected = false;
//     }
//   } else {
//     connectionStatus = OFFLINE;
//     offlineMode = true;
//     lastBlynkConnected = false;
//   }
// }

// //==================================
// // AP Mode and Web Server functions
// //==================================
// void startAPMode() {
//   IPAddress apIP(192, 168, 0, 1);
//   IPAddress gateway(192, 168, 0, 1);
//   IPAddress subnet(255, 255, 255, 0);
//   WiFi.softAPConfig(apIP, gateway, subnet);
  
//   const char* apSSID = "ESP32_Control";
//   const char* apPass = "esp32access";
//   WiFi.mode(WIFI_AP);
//   WiFi.softAP(apSSID, apPass);
//   Serial.print("AP Mode started. Static IP: ");
//   Serial.println(WiFi.softAPIP());
  
//   if (!SPIFFS.begin(true)) {
//     Serial.println("An error occurred while mounting SPIFFS");
//     return;
//   }
  
//   server.on("/", HTTP_GET, handleWebRoot);
//   server.on("/relay", HTTP_GET, [](){
//     String relayState = "R1:" + String(digitalRead(relay1)==HIGH ? "OFF " : "ON ") +
//                         "R2:" + String(digitalRead(relay2)==HIGH ? "OFF" : "ON");
//     server.send(200, "text/plain", relayState);
//   });
  
//   server.on("/toggle", HTTP_GET, [](){
//     String state = server.arg("state");
//     if (state == "on") {
//       digitalWrite(relay1, LOW);
//       digitalWrite(relay2, LOW);
//     } else if (state == "off") {
//       digitalWrite(relay1, HIGH);
//       digitalWrite(relay2, HIGH);
//     }
//     server.send(200, "text/plain", "OK");
//   });
  
//   server.begin();
// }

// void handleWebRoot() {
//   if (!SPIFFS.exists("/index.html")) {
//     server.send(404, "text/plain", "File not found");
//     Serial.println("File not found");
//     return;
//   }
//   File file = SPIFFS.open("/index.html", "r");
//   Serial.println("File found");
//   server.streamFile(file, "text/html");
//   file.close();
// }

// //==================================
// // Setup function
// //==================================
// void setup() {
//   Serial.begin(9600);
//   EEPROM.begin(EEPROM_SIZE);
//   loadSettingsEEPROM();
  
//   Wire.begin(21, 22);  // I2C for LCD and DS3231
//   ds18b20.begin();
//   dht.begin();
  
//   screen.begin(LCD_ADDRESS, LCD_LENGTH, LCD_HEIGHT);
//   screen.backlight();
//   clearLineCache();
  
//   pinMode(relay1, OUTPUT);
//   pinMode(relay2, OUTPUT);
//   pinMode(relay3, OUTPUT);
//   pinMode(relay4, OUTPUT);
  
//   pinMode(ENCODER_BUTTON, INPUT_PULLUP);
//   pinMode(ENCODER_PIN_A, INPUT_PULLUP);
//   pinMode(ENCODER_PIN_B, INPUT_PULLUP);
//   pinMode(BUZZER_PIN, OUTPUT);
  
//   lastA = digitalRead(ENCODER_PIN_A);
//   encoderPos = 0;
//   lastInteraction = millis();
  
//   // Initialize DS3231 RTC
//   if (!rtc.begin()) {
//     Serial.println("Couldn't find RTC");
//   }
//   if (rtc.lostPower()) {
//     Serial.println("RTC lost power, setting time from compile time");
//     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//   }
  
//   connectWiFiBlynk();
//   lastLcdClear = millis();
  
//   // Start encoder task on Core 1
//   xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, NULL, 2, NULL, 1);
  
//   ds18b20.setWaitForConversion(false);
// }

// //==================================
// // Main loop
// //==================================
// void loop() {
//   // Request DS18B20 temperature (nonblocking)
//   ds18b20.requestTemperatures();
//   t = ds18b20.getTempCByIndex(0);
//   if(t < -100) t = newTemp;
  
//   // Read humidity and temperature backup from DHT every 2 seconds
//   if (millis() - lastDHTRead >= DHT_READ_INTERVAL) {
//     float newHumidity = dht.readHumidity();
//     newTemp = dht.readTemperature();
//     if (!isnan(newHumidity)) {
//       h = newHumidity;
//     }
//     if (!isnan(newTemp) && t < -100) {
//       t = newTemp;
//     }
//     lastDHTRead = millis();
//   }
//   if (millis() - lastConnectionCheck > CONNECTION_CHECK_INTERVAL) {
//     bool currentBlynkConnected = Blynk.connected();
    
//     if (currentBlynkConnected != lastBlynkConnected) {
//       if (currentBlynkConnected) {
//         connectionStatus = CONNECTED;
//         offlineMode = false;
//         Serial.println("Reconnected to Blynk!");
//       } else {
//         connectionStatus = OFFLINE;
//         offlineMode = true;
//         Serial.println("Lost Blynk connection!");
//       }
//       lastBlynkConnected = currentBlynkConnected;
//       clearLineCache(); // Force LCD update
//     }
    
//     lastConnectionCheck = millis();
//   }
//   if (!offlineMode && WiFi.status() == WL_CONNECTED) {
//     Blynk.run();
//     sendSensorDataBlynk();
//     updateRTCandDS3231Data();
//   } else {
//     offlineMode = true;
//   }
//   if (!offlineMode && WiFi.status() == WL_CONNECTED) {
//     Blynk.run();
    
//     // Add heartbeat to maintain connection
//     Blynk.virtualWrite(V99, millis() / 1000); // Use unused virtual pin
    
//     sendSensorDataBlynk();
//     updateRTCandDS3231Data();
//   }
//   if (screenMode == 1) {
//     modeset();
//     updateHumidityRelays();
//   }
//   static long lastPos = 0;
//   if ((screenMode == 2) && encoderUpdated) {
//     long currentPos = encoderPos;
//     long delta = currentPos - lastPos;
//     lastPos = currentPos;
//     if (settingsState == 0) {
//       selectedField += delta;
//       if (selectedField < 0) selectedField = 0;
//       if (selectedField >= SETTINGS_TOTAL_ITEMS) selectedField = SETTINGS_TOTAL_ITEMS - 1;
//       lastInteraction = millis();
//     }
//     else if (settingsState == 1) {
//       if (delta != 0) {
//         if (selectedField == 1) { 
//           mi += delta;
//           if (mi > ma) mi = ma;
//         }
//         else if (selectedField == 2) { 
//           ma += delta;
//           if (ma < mi) ma = mi;
//         }
//         else if (selectedField == 3) { 
//           mod += delta;
//         }
//         else if (selectedField == 4) { 
//           switchingDelay += delta;
//           if (switchingDelay < 1) switchingDelay = 1;
//           if (switchingDelay > 60) switchingDelay = 60;
//         }
//         else if (selectedField == 5) { 
//           humLimit += delta;
//           if (humLimit < 1) humLimit = 1;
//           if (humLimit > 100) humLimit = 100;
//         }
//         lastInteraction = millis();
//       }
//     }
//     encoderUpdated = false;
//   }
  
//   checkEncoderButton();
  
//   // Timeout: return to Main screen after no interaction for 5 minutes
//   if ((screenMode == 2) && millis() - lastInteraction > SCREEN2_TIMEOUT) {
//     screenMode = 1;
//     clearLineCache();
//   }
  
//   if (screenMode == 1)
//     lcdScreen1();
//   else if (screenMode == 2)
//     lcdSettingsScreen();
  
//   // if (offlineMode && WiFi.getMode() == WIFI_AP) {
//   //   server.handleClient();
//   // }
// }