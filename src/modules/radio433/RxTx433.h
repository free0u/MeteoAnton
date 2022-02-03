#ifndef RxTx433_h
#define RxTx433_h

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h>  // Not actually used but needed to compile
#endif

class RxTx433 {
   private:
    RH_ASK *driver;

   public:
    RxTx433() {}

    bool init(int rx, int tx, int ptt) {
        driver = new RH_ASK(2000, rx, tx, ptt);

        return driver->init();
    }
    // float humidity() { return dht->readHumidity(); }
    // float temp() { return dht->readTemperature(); }

    void send(int var, String &devName) {
        String s = String(devName + " cnt: " + String(var));
        const char *msg = s.c_str();
        driver->send((uint8_t *)msg, strlen(msg));
        driver->waitPacketSent();
    }

    bool receive(uint8_t *buf, uint8_t *len) {
        // return driver->recv(buf, len);
        Serial.println("ERROR 433");
        return false;
    }
};

#endif
