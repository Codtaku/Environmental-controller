#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include "globals.h"

void setupRelays();
void updateTemperatureRelay(); // Controls relays 1 & 2 in AUTO mode based on temp
void updateHumidityRelays();   // Controls relays 3 & 4 based on humidity and gas
void timerRelayControl();      // Controls relays 1 & 2 in TIMER mode
void modeset();                // Main function to call the correct control logic based on 'mod'

#endif // RELAY_CONTROL_H