#include "eeprom_manager.h"
#include <EEPROM.h>

// Define reasonable defaults and validation ranges for timer entries
#define TIMER_DEFAULT_MIN_TEMP 28
#define TIMER_DEFAULT_MAX_TEMP 32
#define TIMER_VALID_MIN_TEMP 0
#define TIMER_VALID_MAX_TEMP 50 // Adjust if your system supports higher/lower
#define TIMER_VALID_MAX_DAY_OFFSET 3650 // Max 10 years of offset

void setupEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);
    Serial.println("EEPROM Initialized.");
}

void saveSettingsEEPROM()
{
    Serial.println("Saving settings to EEPROM...");
    // Disable interrupts during critical EEPROM operations if necessary,
    // though EEPROM library itself might handle atomicity for single put/get.
    // For multiple puts forming a logical unit, consider if an intermediate power loss is a concern.
    // noInterrupts(); 

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
            EEPROM.put(addr + sizeof(timerEntries[i].dayOffset), timerEntries[i].minTemp); // Use sizeof for robustness
            EEPROM.put(addr + sizeof(timerEntries[i].dayOffset) + sizeof(timerEntries[i].minTemp), timerEntries[i].maxTemp);
        }
        else
        {
            Serial.println("Error: Not enough EEPROM space to save all timer entries!");
            break; // Stop saving if out of space
        }
    }

    if (EEPROM.commit())
    {
        Serial.println("EEPROM settings saved successfully.");
    }
    else
    {
        Serial.println("Error: EEPROM commit failed!");
    }
    // interrupts();
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
    EEPROM.get(EEPROM_ADDR_GAS_LIMIT, temp_gasLimit); 

    // Load Timer count and start day
    EEPROM.get(EEPROM_ADDR_TIMER_COUNT, temp_timerCount);
    EEPROM.get(EEPROM_ADDR_TIMER_START_DAY, temp_timerStartDay);

    // --- Validate Loaded Values and Apply Defaults ---
    // Temperature Thresholds
    if (temp_mi == -1 || temp_mi < -20 || temp_mi > 60)
    {            
        mi = 28; 
        Serial.println("EEPROM Min Temp invalid, using default.");
    }
    else
    {
        mi = temp_mi;
    }
    if (temp_ma == -1 || temp_ma < -20 || temp_ma > 60 || temp_ma < mi)
    {            
        ma = 32; 
        if (ma < mi) ma = mi; 
        Serial.println("EEPROM Max Temp invalid, using default.");
    }
    else
    {
        ma = temp_ma;
    }

    // Switching Delay
    if (temp_switchingDelay == 0xFFFFFFFF || temp_switchingDelay < 1 || temp_switchingDelay > 300)
    {                       
        switchingDelay = 5; 
        Serial.println("EEPROM Switching Delay invalid, using default.");
    }
    else
    {
        switchingDelay = temp_switchingDelay;
    }

    // Humidity Limit
    if (temp_humLimit == -1 || temp_humLimit < 0 || temp_humLimit > 100)
    {                  
        humLimit = 60; 
        Serial.println("EEPROM Humidity Limit invalid, using default.");
    }
    else
    {
        humLimit = temp_humLimit;
    }

    // Gas Limit
    if (temp_gasLimit == -1 || temp_gasLimit < 0 || temp_gasLimit > 4095)
    {                    
        gasLimit = 2000; 
        Serial.println("EEPROM Gas Limit invalid, using default.");
    }
    else
    {
        gasLimit = temp_gasLimit;
    }

    // Timer Count
    if (temp_timerCount == 0xFF || temp_timerCount > APP_MAX_TIMERS)
    {                   
        timerCount = 0; 
        Serial.println("EEPROM Timer Count invalid, using default (0).");
    }
    else
    {
        timerCount = temp_timerCount;
    }

    // Timer Start Day
    if (temp_timerStartDay == 0xFFFFFFFF || temp_timerStartDay == 0) // Also treat 0 as potentially uninitialized if NTP hasn't run
    {                      
        timerStartDay = 0; 
        Serial.println("EEPROM Timer Start Day invalid or not set, using default (0).");
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
                uint16_t loadedDayOffset;
                int loadedMinTemp, loadedMaxTemp;

                EEPROM.get(addr, loadedDayOffset);
                EEPROM.get(addr + sizeof(loadedDayOffset), loadedMinTemp);
                EEPROM.get(addr + sizeof(loadedDayOffset) + sizeof(loadedMinTemp), loadedMaxTemp);

                // Validate loadedDayOffset
                if (loadedDayOffset == 0xFFFF || loadedDayOffset > TIMER_VALID_MAX_DAY_OFFSET) {
                    timerEntries[i].dayOffset = i; // Default to index if invalid
                    Serial.printf("Timer[%d] DayOffset invalid (0x%X), using default %d.\n", i, loadedDayOffset, timerEntries[i].dayOffset);
                } else {
                    timerEntries[i].dayOffset = loadedDayOffset;
                }

                // Validate loadedMinTemp
                if (loadedMinTemp == -1 || loadedMinTemp < TIMER_VALID_MIN_TEMP || loadedMinTemp > TIMER_VALID_MAX_TEMP) {
                    timerEntries[i].minTemp = TIMER_DEFAULT_MIN_TEMP;
                    Serial.printf("Timer[%d] MinTemp invalid (%d), using default %d.\n", i, loadedMinTemp, timerEntries[i].minTemp);
                } else {
                    timerEntries[i].minTemp = loadedMinTemp;
                }

                // Validate loadedMaxTemp
                if (loadedMaxTemp == -1 || loadedMaxTemp < TIMER_VALID_MIN_TEMP || loadedMaxTemp > TIMER_VALID_MAX_TEMP || loadedMaxTemp < timerEntries[i].minTemp) {
                    timerEntries[i].maxTemp = TIMER_DEFAULT_MAX_TEMP;
                    if (timerEntries[i].maxTemp < timerEntries[i].minTemp) { // Ensure max is not less than (potentially defaulted) min
                        timerEntries[i].maxTemp = timerEntries[i].minTemp;
                    }
                    Serial.printf("Timer[%d] MaxTemp invalid (%d), using default %d.\n", i, loadedMaxTemp, timerEntries[i].maxTemp);
                } else {
                    timerEntries[i].maxTemp = loadedMaxTemp;
                }
                
            }
            else
            {
                Serial.println("Error: EEPROM bounds exceeded while loading timer entries. Stopping load.");
                timerCount = i; 
                break;
            }
        }
    }
    else
    {
        timerCount = 0; 
    }

    Serial.println("Settings loaded from EEPROM.");
    Serial.print(" Mi:"); Serial.print(mi);
    Serial.print(" Ma:"); Serial.print(ma);
    Serial.print(" Delay:"); Serial.print(switchingDelay);
    Serial.print(" HumLim:"); Serial.print(humLimit);
    Serial.print(" GasLim:"); Serial.print(gasLimit);
    Serial.print(" TimerCount:"); Serial.print(timerCount);
    Serial.print(" TimerStartDay:"); Serial.println(timerStartDay);
    
    for (int i = 0; i < timerCount; ++i)
    {
        Serial.print(" Timer["); Serial.print(i); Serial.print("]: ");
        Serial.print("DayOffset="); Serial.print(timerEntries[i].dayOffset);
        Serial.print(", Min="); Serial.print(timerEntries[i].minTemp);
        Serial.print(", Max="); Serial.println(timerEntries[i].maxTemp);
    }
}
