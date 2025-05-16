#include "sensors.h"

// DS18B20 Instance
OneWire oneWire(OneWire_BUS);
DallasTemperature ds18b20(&oneWire);

// DHT Instance
DHT dht(DHT_BUS, DHT_TYPE);
unsigned long lastDHTReadTime = 0; // Use a distinct variable name
void setupSensors()
{
    ds18b20.begin();
    dht.begin();
    ds18b20.setWaitForConversion(false); // Use non-blocking mode
    ds18b20.requestTemperatures();       // Initial request
    Serial.println("Sensors initialized.");
}

void readAllSensors()
{

    // --- DS18B20 Temperature (Primary) ---
    // Request temperature conversion (non-blocking) - Do this frequently
    ds18b20.requestTemperatures();

    // Read the result when available (doesn't block significantly)
    float dsTemp = ds18b20.getTempCByIndex(0);
    if (dsTemp > -100 && dsTemp < 100)
    {               // Basic validity check
        t = dsTemp; // Update global 't'
    }
    else
    {
        // DS18B20 failed, rely on DHT backup temperature ('newTemp') if available
        if (!isnan(newTemp))
        {
            t = newTemp;
            Serial.println("Warning: DS18B20 read failed, using DHT temperature.");
        }
        else
        {
            Serial.println("Warning: DS18B20 read failed, DHT backup unavailable.");
            // 't' retains its last valid value in this case
        }
    }

    // --- DHT22 Temperature (Backup) & Humidity ---
    // Check if it's time to read the DHT sensor (respect interval)
    if (millis() - lastDHTReadTime >= DHT_READ_INTERVAL)
    {
        // Serial.println("Reading DHT22 Sensor..."); // Debug message

        // Read Temperature from DHT (for backup)
        float currentDHTTemp = dht.readTemperature();
        if (!isnan(currentDHTTemp))
        {
            newTemp = currentDHTTemp; // Update backup temp reading
        }
        else
        {
            Serial.println("Warning: Failed to read temperature from DHT sensor!");
            // Optionally clear newTemp or set to NaN?
            // newTemp = NAN;
        }

        // Read Humidity from DHT
        float currentHumidity = dht.readHumidity();
        // --- Optional Debugging ---
        // Serial.print("  Raw DHT H: "); Serial.print(currentHumidity);
        // Serial.print(" (is NaN: "); Serial.print(isnan(currentHumidity)); Serial.println(")");
        // --- End Debugging ---

        if (!isnan(currentHumidity))
        {
            h = currentHumidity; // Update global 'h'
        }
        else
        {
            Serial.println("Warning: Failed to read humidity from DHT sensor!");
            // Optionally set 'h' to a specific error value if needed, but leaving it
            // as the last known good value (or initial -1) is often okay.
            // h = -99; // Example error value
        }

        // Reset the DHT timer AFTER attempting both reads
        lastDHTReadTime = millis();

        // --- Debug Print Current Values ---
        // Serial.print("  Current Values: T(DS18B20/DHT)= "); Serial.print(t, 1);
        // Serial.print(" C, H(DHT)= "); Serial.print(h, 1); Serial.println(" %");
        // --- End Debug Print ---

    } // End of DHT read interval check

    // --- Other Sensors (Read on demand or in their own timed blocks if needed) ---
    // Gas and Soil are read by other functions when needed (e.g., updateHumidityRelays, sendSensorDataBlynk)

} // End readAllSensors()

int readGasSensor()
{
    return analogRead(MQ135_BUS);
}

float readSoilMoisture()
{
    int sensorVal = analogRead(SOIL_SENSOR);
    // Map the raw analog value (0-4095 for ESP32) to percentage (0-100%)
    // Adjust the mapping based on your sensor's characteristics (dry vs wet readings)
    // This is a simple linear inversion, assuming 4095=Dry (0%) and 0=Wet (100%)
    // You might need calibration: map(sensorVal, WET_VALUE, DRY_VALUE, 100, 0);
    float percentage = constrain(((4095.0 - sensorVal) * 100.0) / 4095.0, 0.0, 100.0);
    return percentage;
}

float readRTCTemperature()
{
    return rtc.getTemperature(); // DS3231 includes an internal temperature sensor
}