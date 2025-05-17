#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "globals.h"
#include <RTClib.h> // Ensure RTClib is included

void setupTime();         // Initializes the RTC
void updateRTCFromNTP();  // Attempts to sync RTC with NTP server
void checkAutoTimerAdd(); // Checks if a new day requires adding a default timer entry

#endif // TIME_MANAGER_H