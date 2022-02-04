#ifndef TIMING2_H
#define TIMING2_H

#include <NTPClient.h>
#include <WiFiUdp.h>

#include "MeteoLog.h"

#define DEBUG_NTPClient

class Timing2 {
   private:
    WiFiUDP *ntpUDP;
    NTPClient *timeClient;
    MeteoLog *meteoLog;
    bool isInit = false;

   public:
    Timing2() {}
    void init(MeteoLog *meteoLog) {
        if (isInit) {
            return;
        }
        isInit = true;
        this->meteoLog = meteoLog;

        meteoLog->add("NTPClient");

        ntpUDP = new WiFiUDP();
        timeClient = new NTPClient(*ntpUDP, "ru.pool.ntp.org", 0, 60000);
        // timeClient = new NTPClient(*ntpUDP, "pool.ntp.org", 0, 60000);
        timeClient->begin();
    }

    unsigned long getTime() {
        timeClient->update();
        return timeClient->getEpochTime();
    }
};

#endif