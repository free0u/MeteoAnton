#ifndef BME280_h
#define BME280_h

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#define SEALEVELPRESSURE_HPA (1013.25)

class BME280 {
   private:
    Adafruit_BME280 bme;

   public:
    BME280() {}
    void init(uint8_t addr) {
        if (!bme.begin(addr)) {
            Serial.println("Could not find a valid BME280 sensor, check wiring!");
        }
    }
    float temperature() { return bme.readTemperature(); }
    float humidity() { return bme.readHumidity(); }
    float pressure() { return bme.readPressure() * 0.75F / 100; }
};

#endif
