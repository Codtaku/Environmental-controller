#include "time_manager.h"
#include <WiFi.h>           // Required for checking WiFi status and NTP config
#include "blynk_handler.h"  // For sending timer info (V20) and notifications
#include "eeprom_manager.h" // For saving timer data

// RTC Instance
RTC_DS3231 rtc;

// NTP configuration
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 7 * 3600; // GMT+7 offset for Vietnam in seconds
const int daylightOffset_sec = 0;    // No daylight saving offset
unsigned long lastNtpSync = 0;       // <-- DEFINE HERE
bool initialNtpSyncDone = false;     // <-- DEFINE HERE
void setupTime()
{
    // Initialize I2C if not already done (Wire.begin might be called in main setup)
    // Wire.begin(I2C_SDA, I2C_SCL); // Uncomment if I2C is not started elsewhere

    if (!rtc.begin())
    {
        Serial.println("FATAL: Couldn't find RTC! Check wiring/address.");
        // Optional: Halt execution or try fallback?
        // while(1) delay(1000);
    }
    else
    {
        Serial.println("RTC Initialized.");
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, setting time from compile time.");
        // Set the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // Note: This time will be incorrect until NTP sync occurs.
    }
    else
    {
        Serial.println("RTC has valid time.");
        DateTime now = rtc.now();
        Serial.print("Current RTC Time: ");
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print(" ");
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.println(now.second(), DEC);
    }

    // Initialize last NTP sync time
    lastNtpSync = 0; // Force NTP sync attempt soon after connection
    initialNtpSyncDone = false;
}

void updateRTCFromNTP()
{
    // Attempt NTP sync only if connected to WiFi and interval has passed or first time
    if (!offlineMode && WiFi.status() == WL_CONNECTED)
    {
        unsigned long nowMillis = millis();
        if (!initialNtpSyncDone || (nowMillis - lastNtpSync > NTP_SYNC_INTERVAL))
        {
            Serial.println("Attempting NTP Time Synchronization...");
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

            struct tm timeinfo;
            // Try to get local time with a timeout (e.g., 10 seconds)
            if (!getLocalTime(&timeinfo, 10000))
            {
                Serial.println("ERROR: Failed to obtain time from NTP server.");
                // Optional: Retry sooner?
                lastNtpSync = nowMillis - NTP_SYNC_INTERVAL + 15000; // Retry after 15 sec if failed
            }
            else
            {
                Serial.println("NTP Sync Successful.");
                DateTime ntpTime = DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                rtc.adjust(ntpTime);       // Adjust DS3231 RTC with the obtained network time
                lastNtpSync = nowMillis;   // Update last successful sync time
                initialNtpSyncDone = true; // Mark sync as done

                Serial.print("RTC Time Synced via NTP: ");
                Serial.print(ntpTime.year(), DEC);
                Serial.print('/');
                Serial.print(ntpTime.month(), DEC);
                Serial.print('/');
                Serial.print(ntpTime.day(), DEC);
                Serial.print(" ");
                Serial.print(ntpTime.hour(), DEC);
                Serial.print(':');
                Serial.print(ntpTime.minute(), DEC);
                Serial.print(':');
                Serial.println(ntpTime.second(), DEC);

                // If Timer mode start day hasn't been set yet, set it now after first successful sync
                if (timerStartDay == 0)
                {
                    timerStartDay = ntpTime.unixtime() / 86400; // Days since epoch
                    saveSettingsEEPROM();                       // Save the initialized start day
                    char buf[12];
                    sprintf(buf, "%02d/%02d/%02d", ntpTime.day(), ntpTime.month(), ntpTime.year() % 100);
                    // if (isBlynkConnected()) Blynk.virtualWrite(V20, buf); // Update Blynk display
                    Serial.print("Timer Mode Start Day Initialized: ");
                    Serial.println(timerStartDay);
                }
            }
        }
    }
    else
    {
        // Reset sync flag if we lose WiFi connection
        initialNtpSyncDone = false;
    }
}

// Checks if the day has rolled over and adds a default timer entry if needed and space allows
void checkAutoTimerAdd()
{
    // Only proceed if timer mode is potentially active or relevant
    // if (mod != 4) return; // Only add in timer mode? Or always add? Assume always for now.

    static uint32_t lastDayChecked = 0; // Track the last day we performed this check for
    DateTime now = rtc.now();
    uint32_t currentDay = now.unixtime() / 86400; // Get current day number

    // Don't run if RTC isn't valid (currentDay would be low/incorrect) or if we already checked today
    if (currentDay < 10000 || currentDay == lastDayChecked)
    { // Basic sanity check for valid date
        return;
    }

    // Check if a new timer entry should be added
    // Condition: Timer is enabled (count < max), and the last entry's offset is for a previous day
    if (timerCount < APP_MAX_TIMERS)
    {
        bool addNew = false;
        if (timerCount == 0)
        {
            // If no timers exist, and timerStartDay is set, maybe add the first one for day 0?
            // Let's assume adding is only triggered by user or day rollover after at least one exists.
        }
        else
        {
            // Check if the current day's offset is greater than the last entry's offset
            uint16_t currentDayOffset = 0;
            if (timerStartDay > 0 && currentDay >= timerStartDay)
            {
                currentDayOffset = currentDay - timerStartDay;
            }

            if (currentDayOffset > timerEntries[timerCount - 1].dayOffset)
            {
                addNew = true; // Current day offset is past the last scheduled day offset
            }
        }

        if (addNew)
        {
            Serial.print("New day detected (Day Offset ");
            Serial.print(currentDay - timerStartDay);
            Serial.print(" > Last Timer Offset ");
            Serial.print(timerEntries[timerCount - 1].dayOffset);
            Serial.println("). Adding default timer entry.");

            // Add a new entry for the day AFTER the last entry
            timerEntries[timerCount].dayOffset = timerEntries[timerCount - 1].dayOffset + 1;
            timerEntries[timerCount].minTemp = 28; // Default min
            timerEntries[timerCount].maxTemp = 32; // Default max
            timerCount++;
            saveSettingsEEPROM(); // Save the new entry and count
            notifyBlynk("Auto-added new daily timer schedule (Default 28/32).");
        }
    }

    lastDayChecked = currentDay; // Mark that we've checked for this day
}