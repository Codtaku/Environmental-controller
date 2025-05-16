#include "relay_control.h"
#include "blynk_handler.h" // For notifications
#include "sensors.h"

void setupRelays()
{
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Initialize relays to a known safe state (usually OFF)
  // Assuming LOW = ON for typical relay modules (check your hardware!)
  digitalWrite(relay1, LOW); // OFF
  digitalWrite(relay2, LOW); // OFF
  digitalWrite(relay3, LOW); // OFF (Active Low)
  digitalWrite(relay4, LOW); // OFF (Active Low)

  Serial.println("Relays initialized to OFF state.");
}

// Controls Temperature Relays (relay1, relay2) in AUTO mode
void updateTemperatureRelay()
{
  static int currentAutoZone = -1; // 0=Cold, 1=Target, 2=Hot
  static int lastTargetZone = -1;
  static unsigned long zoneChangeStart = 0;

  int targetZone;

  // Determine target zone based on temperature 't' and thresholds 'mi', 'ma'
  if (t <= mi)
  {
    targetZone = 0; // Temperature is below or at minimum threshold (Cold)
  }
  else if (t > mi && t < ma)
  {
    targetZone = 1; // Temperature is within the target range (Target)
  }
  else
  {                 // t >= ma
    targetZone = 2; // Temperature is above or at maximum threshold (Hot)
  }

  // Check for extreme deviation (e.g., > 7 degrees outside range) - Optional override
  // if (t < mi - 7 || t > ma + 7) {
  //   targetZone = 1; // Force to intermediate state during extreme temps? Or maybe full heat/cool?
  //   Serial.println("Extreme temperature detected!");
  //   // notifyBlynk("Extreme temperature condition detected!"); // Rate limited
  // }

  // Apply switching delay only if the target zone has changed
  if (targetZone != lastTargetZone)
  {
    lastTargetZone = targetZone;
    zoneChangeStart = millis(); // Start delay timer
    // Serial.print("Target Temp Zone Changed: "); Serial.println(targetZone);
  }

  // If the target zone is stable for the duration of the delay, apply the change
  if (millis() - zoneChangeStart >= (switchingDelay * 1000UL))
  {
    // if (currentAutoZone != targetZone) {
    currentAutoZone = targetZone; // Update the current operating zone
    // Serial.print("Applying Temp Zone Change: "); Serial.println(currentAutoZone);

    // Set relay states based on the target zone (assuming LOW = ON)
    switch (targetZone)
    {
    case 0:                      // Too Cold -> Both Heaters ON
      digitalWrite(relay1, LOW); // Heater 1 ON
      digitalWrite(relay2, LOW); // Heater 2 ON
      // Serial.println("Temp Control: 2 Heaters ON");
      break;
    case 1:                       // Target Range -> One Heater ON
      digitalWrite(relay1, HIGH); // Heater 1 ON
      digitalWrite(relay2, LOW);  // Heater 2 OFF
      // Serial.println("Temp Control: 1 Heater ON");
      break;
    case 2:                       // Too Hot -> Both Heaters OFF
      digitalWrite(relay1, HIGH); // Heater 1 OFF
      digitalWrite(relay2, HIGH); // Heater 2 OFF
      // Serial.println("Temp Control: 0 Heaters ON (Cooling/Idle)");
      break;
    }
    // Relay status is sent to Blynk via syncRelayStatusExtended() in main loop
    //}
  }
}

// Controls Humidity Relays (relay3, relay4) based on humidity 'h' and gas readings
void updateHumidityRelays()
{
  // Assuming relay3 = Dehumidifier/Vent (ON when humidity HIGH or gas HIGH) - Active LOW
  // Assuming relay4 = Humidifier/Mister (ON when humidity LOW) - Active LOW

  int gasVal = readGasSensor();   // Get current gas reading
  bool dehumidifyCommand = false; // Should relay 3 be ON?
  bool mistCommand = false;       // Should relay 4 be ON?

  // --- Humidity Control Logic ---
  // Use hysteresis to prevent rapid switching around humLimit
  float upperHumThreshold = humLimit + 3.0; // Turn ON dehumidifier if humidity > limit + hysteresis
  float lowerHumThreshold = humLimit - 3.0; // Turn ON humidifier if humidity < limit - hysteresis

  if (h > upperHumThreshold)
  {
    dehumidifyCommand = true; // Humidity is high, need to dehumidify
  }
  else if (h < lowerHumThreshold)
  {
    mistCommand = true; // Humidity is low, need to humidify/mist
  }
  // else: humidity is within the deadband (lowerHumThreshold to upperHumThreshold), do nothing based on humidity

  // --- Gas Override Logic ---
  // If gas level exceeds the limit, force dehumidifier/ventilation ON, regardless of humidity
  if (gasVal > gasLimit)
  {
    dehumidifyCommand = true; // Override: Turn ON dehumidifier/vent (Relay 3)
    // mistCommand = false;      // Ensure misting is OFF if venting due to gas
    // Serial.print("Gas Limit Exceeded ("); Serial.print(gasVal); Serial.print(" > "); Serial.print(gasLimit); Serial.println("), forcing Ventilation ON.");
    //  Optional: Notify if gas level is significantly high
    //  static unsigned long lastGasNotify = 0;
    //  if (gasVal > gasLimit * 1.2 && millis() - lastGasNotify > 600000) { // Notify max every 10 mins
    //      notifyBlynk("High gas reading detected! Venting activated.");
    //      lastGasNotify = millis();
    //  }
  }

  // --- Apply Commands to Relays (Active LOW assumed) ---
  digitalWrite(relay3, dehumidifyCommand ? LOW : HIGH); // Relay 3 ON if dehumidifyCommand is true
  digitalWrite(relay4, mistCommand ? LOW : HIGH);       // Relay 4 ON if mistCommand is true

  // Relay status is sent to Blynk via syncRelayStatusExtended() in main loop
}

// Controls Temperature Relays (relay1, relay2) in TIMER mode
void timerRelayControl()
{
  // Obtain current day count from RTC (days since epoch)
  DateTime now = rtc.now();
  uint32_t currentDay = now.unixtime() / 86400;

  // Calculate days passed since timer mode was started/reset
  int daysPassed = 0;
  if (timerStartDay > 0)
  { // Ensure timerStartDay has been initialized
    daysPassed = currentDay - timerStartDay;
    if (daysPassed < 0)
      daysPassed = 0; // Should not happen if RTC is correct
  }
  else
  {
    Serial.println("Warning: Timer mode active but Start Day not set!");
    // Optionally default to AUTO mode behavior or fixed thresholds?
    // updateTemperatureRelay(); // Fallback to AUTO mode?
    // return;
  }

  // Determine effective min/max temperatures based on the current day and timer entries
  int effectiveMin = 28; // Default min if no matching timer entry
  int effectiveMax = 32; // Default max if no matching timer entry
  bool entryFound = false;
  for (uint8_t i = 0; i < timerCount; i++)
  {
    // Find the latest entry whose dayOffset is less than or equal to daysPassed
    if (timerEntries[i].dayOffset <= (uint16_t)daysPassed)
    {
      effectiveMin = timerEntries[i].minTemp;
      effectiveMax = timerEntries[i].maxTemp;
      entryFound = true;
      // We keep checking to use the LATEST applicable entry
    }
    else
    {
      // Stop checking once we pass an entry for a future day offset
      break;
    }
  }
  if (!entryFound && timerCount > 0)
  {
    // If daysPassed is before the first entry's offset, maybe use the first entry? Or defaults?
    // Sticking to defaults for now if no entry's offset matches or is less than daysPassed.
    Serial.println("Timer Mode: No applicable schedule entry found for current day offset. Using defaults.");
  }
  else if (entryFound)
  {
    // Serial.print("Timer Mode: Using schedule entry. EffMin:"); Serial.print(effectiveMin); Serial.print(" EffMax:"); Serial.println(effectiveMax);
  }
  else
  {
    // Serial.println("Timer Mode: No schedule entries defined. Using defaults.");
  }

  // --- Temperature Control Logic (Similar to AUTO mode but uses effectiveMin/Max) ---
  static int currentTimerZone = -1; // 0=Cold, 1=Target, 2=Hot (based on effective thresholds)
  static int lastTimerZone = -1;
  static unsigned long timerZoneChangeStart = 0;

  int targetZone;

  if (t <= effectiveMin)
  {
    targetZone = 0; // Below or at effective minimum
  }
  else if (t > effectiveMin && t < effectiveMax)
  {
    targetZone = 1; // Within effective target range
  }
  else
  {                 // t >= effectiveMax
    targetZone = 2; // Above or at effective maximum
  }

  // Apply switching delay
  if (targetZone != lastTimerZone)
  {
    lastTimerZone = targetZone;
    timerZoneChangeStart = millis();
    // Serial.print("Target Timer Zone Changed: "); Serial.println(targetZone);
  }

  if (millis() - timerZoneChangeStart >= (switchingDelay * 1000UL))
  {
    if (currentTimerZone != targetZone)
    {
      currentTimerZone = targetZone;
      // Serial.print("Applying Timer Zone Change: "); Serial.println(currentTimerZone);

      // Set relay states based on the target zone (assuming LOW = ON, R1=Heat, R2=Cool)
      switch (targetZone)
      {
      case 0:                      // Too Cold -> Both Heaters ON
        digitalWrite(relay1, LOW); // Heater 1 ON
        digitalWrite(relay2, LOW); // Heater 2 ON
        // Serial.println("Temp Control: 2 Heaters ON");
        break;
      case 1:                       // Target Range -> One Heater ON
        digitalWrite(relay1, HIGH); // Heater 1 ON
        digitalWrite(relay2, LOW);  // Heater 2 OFF
        // Serial.println("Temp Control: 1 Heater ON");
        break;
      case 2:                       // Too Hot -> Both Heaters OFF
        digitalWrite(relay1, HIGH); // Heater 1 OFF
        digitalWrite(relay2, HIGH); // Heater 2 OFF
        // Serial.println("Temp Control: 0 Heaters ON (Cooling/Idle)");
        break;
      }
    }
  }
  // Humidity relays (3 & 4) are controlled independently by updateHumidityRelays() in all modes.
}

// Main mode dispatcher - called from loop() when screenMode is 1 (Main)
void modeset()
{
  // Sanitize mode value
  int currentMode = abs(mod) % 5; // 0=AUTO, 1=OFF, 2=ON, 3=HALF, 4=TIMER

  switch (currentMode)
  {
  case 0:                     // AUTO Mode
    updateTemperatureRelay(); // Control R1/R2 based on t, mi, ma
    break;
  case 1:                       // Manual OFF Mode
    digitalWrite(relay1, HIGH); // R1 OFF
    digitalWrite(relay2, HIGH); // R2 OFF
    break;
  case 2:                      // Manual ON Mode (Full Heat/Cool?) - Adjust logic as needed
    digitalWrite(relay1, LOW); // R1 ON (Heat?)
    digitalWrite(relay2, LOW); // R2 ON (Cool?) - Maybe only one should be ON?
    break;
  case 3:                       // Manual HALF Mode (e.g., Heat only, or Cool only?) - Adjust logic
    digitalWrite(relay1, LOW);  // R1 ON (Heat?)
    digitalWrite(relay2, HIGH); // R2 OFF (Cool?)
    break;
  case 4:                // TIMER Mode
    timerRelayControl(); // Control R1/R2 based on timer schedule effective thresholds
    break;
  }

  // Humidity/Gas control runs independently in all operational modes (AUTO, TIMER, maybe manual?)
  // If you want R3/R4 OFF in Manual OFF mode, add conditions here or within updateHumidityRelays.
  // For simplicity, let's assume R3/R4 operate based on humidity/gas regardless of main mode,
  // except maybe in full Manual OFF (mode 1).
  //   if (currentMode == 1) { // Manual OFF
  //        digitalWrite(relay3, HIGH); // Force R3 OFF
  //        digitalWrite(relay4, HIGH); // Force R4 OFF
  //   } else {
  updateHumidityRelays(); // Control R3/R4 based on humidity/gas
  //   }
}