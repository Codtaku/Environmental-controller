#ifndef SENSORS_H
#define SENSORS_H

#include "globals.h"

void setupSensors();
void readAllSensors();
// void readTemperatures(); // Reads both DS18B20 and DHT temp (for backup)
// void readHumidity();     // Reads DHT humidity
int readGasSensor();
float readSoilMoisture();
float readRTCTemperature(); // Reads temperature from DS3231

#endif // SENSORS_H