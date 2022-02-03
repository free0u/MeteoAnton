#ifndef DHTSensor_h
#define DHTSensor_h

#include "DHT.h"

// #define DHTPIN1 D3
// #define DHTPIN2 D5
// #define DHTTYPE DHT22
// #define DHTTYPE DHT11

class DHTSensor {
   private:
    DHT *dht;
    bool inited = false;

   public:
    DHTSensor() {}
    void init(uint8_t pin, int dhttype) {
        if (!inited) {
            dht = new DHT(pin, dhttype);
            dht->begin();
            inited = true;
        }
    }
    float humidity() { return dht->readHumidity(); }
    float temp() { return dht->readTemperature(); }
};

#endif
