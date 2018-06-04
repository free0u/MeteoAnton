#ifndef DHTSensor_h
#define DHTSensor_h

#include "DHT.h"
#define DHTPIN D2
#define DHTTYPE DHT11

class DHTSensor {
  private:
    DHT *dht;

  public:
    DHTSensor() {
        dht = new DHT(DHTPIN, DHTTYPE);
        dht->begin();
        Serial.println("DHT11 begin");
    }
    float humidity() { return dht->readHumidity(); }
};

#endif
