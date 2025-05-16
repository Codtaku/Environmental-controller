#include "eeprom_manager.h"
#include <EEPROM.h>

void setupEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);
    Serial.println("EEPROM Initialized.");
}

void saveSettingsEEPROM()
{
    Serial.println("Saving settings to EEPROM...");
    EEPROM.put(EEPROM_ADDR_MIN, mi);
    EEPROM.put(EEPROM_ADDR_MAX, ma);
    EEPROM.put(EEPROM_ADDR_DELAY, switchingDelay);
    EEPROM.put(EEPROM_ADDR_HUM_LIMIT, humLimit);
    EEPROM.put(EEPROM_ADDR_GAS_LIMIT, gasLimit); // Save Gas Limit

    // Save Timer Mode Data
    EEPROM.put(EEPROM_ADDR_TIMER_COUNT, timerCount);
    EEPROM.put(EEPROM_ADDR_TIMER_START_DAY, timerStartDay);

    // Save individual timer entries
    for (uint8_t i = 0; i < timerCount; i++)
    {
        // Calculate address for the current timer entry
        int addr = EEPROM_ADDR_TIMERS + i * TIMER_ENTRY_SIZE;
        if (addr + TIMER_ENTRY_SIZE <= EEPROM_SIZE)
        { // Check bounds
            EEPROM.put(addr, timerEntries[i].dayOffset);
            EEPROM.put(addr + 2, timerEntries[i].minTemp); // Offset by 2 bytes (size of uint16_t)
            EEPROM.put(addr + 4, timerEntries[i].maxTemp); // Offset by 4 bytes
        }
        else
        {
            Serial.println("Error: Not enough EEPROM space to save all timer entries!");
            break; // Stop saving if out of space
        }
    }
    // Clear any leftover timer entries in EEPROM if count decreased? Optional.
    // for (uint8_t i = timerCount; i < MAX_TIMERS; i++) {
    //      int addr = EEPROM_ADDR_TIMERS + i * TIMER_ENTRY_SIZE;
    //      // Write default/empty data or zeros
    // }

    if (EEPROM.commit())
    {
        Serial.println("EEPROM settings saved successfully.");
    }
    else
    {
        Serial.println("Error: EEPROM commit failed!");
    }
}

void loadSettingsEEPROM()
{
    Serial.println("Loading settings from EEPROM...");

    // Temporary variables to read into
    int temp_mi, temp_ma, temp_humLimit, temp_gasLimit;
    unsigned long temp_switchingDelay;
    uint8_t temp_timerCount;
    uint32_t temp_timerStartDay;

    // Load main settings
    EEPROM.get(EEPROM_ADDR_MIN, temp_mi);
    EEPROM.get(EEPROM_ADDR_MAX, temp_ma);
    EEPROM.get(EEPROM_ADDR_DELAY, temp_switchingDelay);
    EEPROM.get(EEPROM_ADDR_HUM_LIMIT, temp_humLimit);
    EEPROM.get(EEPROM_ADDR_GAS_LIMIT, temp_gasLimit); // Load Gas Limit

    // Load Timer count and start day
    EEPROM.get(EEPROM_ADDR_TIMER_COUNT, temp_timerCount);
    EEPROM.get(EEPROM_ADDR_TIMER_START_DAY, temp_timerStartDay);

    // --- Validate Loaded Values and Apply Defaults ---
    // Check if EEPROM was initialized (often reads as -1 or 255)
    // Temperature Thresholds
    if (temp_mi == -1 || temp_mi < -20 || temp_mi > 60)
    {            // Check for uninitialized or unreasonable value
        mi = 28; // Default Min Temp
        Serial.println("EEPROM Min Temp invalid, using default.");
    }
    else
    {
        mi = temp_mi;
    }
    if (temp_ma == -1 || temp_ma < -20 || temp_ma > 60 || temp_ma < mi)
    {            // Check unreasonable or max < min
        ma = 32; // Default Max Temp
        if (ma < mi)
            ma = mi; // Ensure max >= min after defaulting
        Serial.println("EEPROM Max Temp invalid, using default.");
    }
    else
    {
        ma = temp_ma;
    }

    // Switching Delay
    if (temp_switchingDelay == 0xFFFFFFFF || temp_switchingDelay < 1 || temp_switchingDelay > 300)
    {                       // Check uninitialized or unreasonable range (e.g., 1-300 seconds)
        switchingDelay = 5; // Default Delay
        Serial.println("EEPROM Switching Delay invalid, using default.");
    }
    else
    {
        switchingDelay = temp_switchingDelay;
    }

    // Humidity Limit
    if (temp_humLimit == -1 || temp_humLimit < 0 || temp_humLimit > 100)
    {                  // Check uninitialized or 0-100 range
        humLimit = 60; // Default Humidity Limit
        Serial.println("EEPROM Humidity Limit invalid, using default.");
    }
    else
    {
        humLimit = temp_humLimit;
    }

    // Gas Limit
    if (temp_gasLimit == -1 || temp_gasLimit < 0 || temp_gasLimit > 4095)
    {                    // Check uninitialized or 0-4095 range
        gasLimit = 2000; // Default Gas Limit
        Serial.println("EEPROM Gas Limit invalid, using default.");
    }
    else
    {
        gasLimit = temp_gasLimit;
    }

    // Timer Count
    if (temp_timerCount == 0xFF || temp_timerCount > APP_MAX_TIMERS)
    {                   // Check uninitialized or > max allowed
        timerCount = 0; // Default: No timers
        Serial.println("EEPROM Timer Count invalid, using default (0).");
    }
    else
    {
        timerCount = temp_timerCount;
    }

    // Timer Start Day
    if (temp_timerStartDay == 0xFFFFFFFF)
    {                      // Check uninitialized
        timerStartDay = 0; // Default: Not started
        Serial.println("EEPROM Timer Start Day invalid, using default (0).");
    }
    else
    {
        timerStartDay = temp_timerStartDay;
    }

    // Load individual timer entries IF timerCount is valid and > 0
    if (timerCount > 0 && timerCount <= APP_MAX_TIMERS)
    {
        for (uint8_t i = 0; i < timerCount; i++)
        {
            int addr = EEPROM_ADDR_TIMERS + i * TIMER_ENTRY_SIZE;
            if (addr + TIMER_ENTRY_SIZE <= EEPROM_SIZE)
            {
                EEPROM.get(addr, timerEntries[i].dayOffset);
                EEPROM.get(addr + 2, timerEntries[i].minTemp);
                EEPROM.get(addr + 4, timerEntries[i].maxTemp);
                // Add validation for loaded timer entry values if needed
                // e.g., if (timerEntries[i].minTemp < 0 || timerEntries[i].maxTemp > 50) ... reset to defaults
            }
            else
            {
                Serial.println("Error: EEPROM bounds exceeded while loading timer entries. Stopping load.");
                timerCount = i; // Adjust count to only include successfully loaded entries
                break;
            }
        }
    }
    else
    {
        timerCount = 0; // Ensure count is 0 if the loaded value was invalid
    }

    Serial.println("Settings loaded from EEPROM.");
    Serial.print(" Mi:");
    Serial.print(mi);
    Serial.print(" Ma:");
    Serial.print(ma);
    Serial.print(" Delay:");
    Serial.print(switchingDelay);
    Serial.print(" HumLim:");
    Serial.print(humLimit);
    Serial.print(" GasLim:");
    Serial.print(gasLimit);
    Serial.print(" TimerCount:");
    Serial.print(timerCount);
    Serial.print(" TimerStartDay:");
    Serial.println(timerStartDay);
    // Print loaded timer entries for verification
    for (int i = 0; i < timerCount; ++i)
    {
        Serial.print(" Timer[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.print("DayOffset=");
        Serial.print(timerEntries[i].dayOffset);
        Serial.print(", Min=");
        Serial.print(timerEntries[i].minTemp);
        Serial.print(", Max=");
        Serial.println(timerEntries[i].maxTemp);
    }
}