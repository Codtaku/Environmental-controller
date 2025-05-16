#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include "globals.h"

void setupEEPROM();
void saveSettingsEEPROM(); // Saves main settings and timer data
void loadSettingsEEPROM(); // Loads main settings and timer data

#endif // EEPROM_MANAGER_H