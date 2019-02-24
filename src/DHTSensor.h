#ifndef DHTSensor_h
#define DHTSensor_h

#include "DHT.h"

#define DHTPIN1 D3
#define DHTPIN2 D5
#define DHTTYPE DHT22

class DHTSensor {
  private:
    DHT *dht;

  public:
    DHTSensor(uint8_t pin) {
        dht = new DHT(pin, DHTTYPE);
        dht->begin();
    }
    float humidity() { return dht->readHumidity(); }
    float temp() { return dht->readTemperature(); }
};

#endif
