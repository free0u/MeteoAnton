#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"
#include <ESP8266HTTPClient.h>
#include "EspSaveCrash.h"

#include "BME280.h"
#include "DHTSensor.h"
#include "Led.h"
#include "MeteoLog.h"
#include "OLED.h"
#include "SensorDallasTemp.h"
#include "SensorsData.h"
#include "Timing.h"
#include "CO2SensorSenseAir.h"
#include "OLEDStates.h"
#include "Timeouts.h"
#include "SensorsCache.h"

#define BUTTON 0

OTAUpdate otaUpdate;

WiFiConfig *wifiConfig;
SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
Timing *timing;
CO2SensorSenseAir *co2;

SensorsCache *cache;
SensorsData *sensorsData;
long bootTime;

void setup() {

    Serial.begin(115200);
    // SaveCrash.clear();
    // SaveCrash.print();

    bootTime = now();

    // setup pins
    // pinMode(BUTTON, INPUT);
    pinMode(BUTTON, INPUT_PULLUP);

    // turn on LED until connected
    led.on();

    // setup log and OLED
    meteoLog = new MeteoLog();
    oled = new OLED(meteoLog);
    oled->showMessage("Booting...");

    // setup wifi connection
    oled->showMessage("Trying to connect WiFi");
    wifiConfig = new WiFiConfig();
    if (wifiConfig->connect()) {
        led.off();
    }
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // CO2
    oled->showMessage("CO2 init...");
    co2 = new CO2SensorSenseAir(D7, D8);
    oled->showMessage("CO2 init... Done");

    // DS18B20 temperature
    oled->showMessage("DS18B20 init...");
    temp = new SensorDallasTemp();
    oled->showMessage("DS18B20 init... Done");

    // DHT11
    oled->showMessage("DHT22 init...");
    dht = new DHTSensor(D1);
    oled->showMessage("DHT22 init... Done");

    // NTP and RTC
    oled->showMessage("NTP/RTC init...");
    timing = new Timing();
    oled->showMessage("NTP/RTC init... Done");

    oled->showMessage("SPIFFS init...");
    SPIFFS.begin();
    oled->showMessage("SPIFFS init... Done");

    oled->showMessage("Cache init...");
    cache = new SensorsCache();
    oled->showMessage("Cache init... Done");

    oled->showMessage("SensorsData init...");
    sensorsData = new SensorsData();
    oled->showMessage("SensorsData init... Done");
}

// timers
long timeButtonPress = 0;
long timeScanCo21 = -1e9;
long timeScanCo22 = -1e9;
long timeDataSend = -1e9;
long rtcTimeUpdate = -RTC_TIME_UPDATE_TIMEOUT;
long oledStateChangedTime = 0;
long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;

int buttonCount = 0;

void tryUpdateSensors() {
    float dsTemp;
    float dhtHum;
    long millisNow = millis();
    long timestampNow = -1;

    if (millisNow - sensorsDataUpdated > SENSORS_TIMEOUT) {
        sensorsDataUpdated = millisNow;

        meteoLog->add("Reading sensors...");
        dsTemp = temp->temperatureOne();

        meteoLog->add("dsTemp " + String(dsTemp));
        meteoLog->add("Reading sensors... Done");

        if (timestampNow == -1) {
            timestampNow = now();
        }

        if (!isnan(dsTemp)) {
            sensorsData->dsTemp.set(dsTemp, timestampNow);
        }

        sensorsData->uptime.set(millis() / 1000 / 60, timestampNow);
    }

    if (millis() - dhtSensorUpdated > DHT_TIMEOUT) {
        dhtSensorUpdated = millis();
        dhtHum = dht->humidity();
        meteoLog->add("dhtHum1 " + String(dhtHum));

        if (!isnan(dhtHum)) {
            if (timestampNow == -1) {
                timestampNow = now();
            }
            sensorsData->dhtHum.set(dhtHum, timestampNow);
        }
    }

    if (millis() - timeScanCo21 > CO2_TIMEOUT) {
        timeScanCo21 = millis();
        int co2uart = co2->read();

        if (co2uart > 0) {
            if (timestampNow == -1) {
                timestampNow = now();
            }
            sensorsData->co2.set(co2uart, timestampNow);
        }

        meteoLog->add("co2 ppm " + String(co2uart));
    }
}

String generateThingspeakPair(int ind, Sensor sensor) {
    float value = sensor.getIfUpdated();
    if (isnan(value)) {
        return "";
    }
    return "&field" + String(ind) + "=" + String(value);
}

// String generateThingspeakPair(int ind, int value) { return "&field" + String(ind) + "=" + String(value); }

// int sendDataTs() {
//     int up = millis() / 1000;
//     String url = "***REMOVED***";

//     url += generateThingspeakPair(1, sensorsData->co2uart);
//     url += generateThingspeakPair(2, up / 60);
//     url += generateThingspeakPair(3, sensorsData->co2);
//     url += generateThingspeakPair(4, sensorsData->dsTempOne);
//     url += generateThingspeakPair(5, sensorsData->bmeHum);

//     HTTPClient http;
//     http.begin(url);
//     int statusCode = http.GET();
//     http.end();
//     return statusCode;
// }

int sendDataApi() {
    HTTPClient http;
    http.begin("***REMOVED***");
    http.setTimeout(5000);
    http.addHeader("Sensors-Names", sensorsData->sensorsNames);
    int statusCode = http.POST(sensorsData->serialize());
    http.end();
    return statusCode;
}

long dumpJsonTime = -1e9;

bool wifiOn() {
    return true;
    // return false;

    long secs = millis() / 1000;
    secs = now();
    long minutes = secs / 60 % 60;
    Serial.println("now(): " + String(now()));
    Serial.println("Minutes: " + String(minutes));
    Serial.println("Minutes %% 10: " + String(minutes));
    return minutes % 10 >= 5;
}

long stt = millis();
bool is_calib = false;
// bool is_calib = true;

int cnt2 = 0;

void loop() {
    delay(100);
    otaUpdate.handle();
    tryUpdateSensors();

    if (millis() - timeDataSend > SERVER_SENDING_TIMEOUT) {
        timeDataSend = millis();

        if (cache->empty()) {
            int statusCode = -1;
            if (wifiOn()) {
                statusCode = sendDataApi();
            }
            if (statusCode != 200) {
                cache->add(sensorsData);
            }
        } else {
            cache->add(sensorsData);
            if (wifiOn()) {
                cache->sendCache(sensorsData->sensorsNames);
            }
        }
    }

    int buttonState = digitalRead(BUTTON);
    if (buttonState == LOW) {
        if (millis() - timeButtonPress > 800) {
            timeButtonPress = millis();
            buttonCount++;
        }
    } else {
        buttonCount = 0;
    }

    if (buttonCount > 5) {
        buttonCount = 0;

        led.on();
        wifiConfig->startPortal();
        led.off();
    }

    // if (!is_calib && millis() - stt > 30000) {
    //     stt = millis();
    //     Serial.println("\n\n\n\n\n\n\n");
    //     Serial.println("Run calibration");
    //     Serial.println("\n\n\n\n\n\n\n");
    //     if (co21->calib()) {
    //         Serial.println("co21 calibrated");
    //         is_calib = true;
    //     }
    //     if (co22->calib()) {
    //         Serial.println("co22 calibrated");
    //         is_calib = true;
    //     }
    // }

    // if (millis() - timeDataSend > SERVER_SENDING_TIMEOUT) {
    //     // if (millis() - timeDataSend > 5000) {
    //     timeDataSend = millis();
    //     if (now() > 1542929780) {
    //         sendDataApi();
    //         Serial.println("sent");
    //     }
    // }

    // if (millis() - stt > 60 * 1000 && !is_calib) {
    //     // co2->setZero();
    //     // is_calib = true;
    // }

    // if (millis() - rtcTimeUpdate > RTC_TIME_UPDATE_TIMEOUT) {
    //     rtcTimeUpdate = millis();
    //     timing->updateRtc();
    //     meteoLog->add("RTC updated: " + timing->getRtcDateTime());
    // }

    // if (millis() - timeDataSend > SERVER_SENDING_TIMEOUT) {
    //     timeDataSend = millis();

    //     if (cache->empty()) {
    //         int statusCode = -1;
    //         if (wifiOn()) {
    //             statusCode = sendDataApi();
    //         }
    //         if (statusCode != 200) {
    //             cache->add(sensorsData);
    //         }
    //     } else {
    //         cache->add(sensorsData);
    //         if (wifiOn()) {
    //             cache->sendCache(sensorsData->sensorsNames);
    //         }
    //     }
    // }

    // if (!oled->alwaysOn && millis() - oledStateChangedTime > 7000) {
    //     oledState = EMPTY;
    // }

    // if (oledState == SENSORS) {
    //     oled->displaySensorsData(sensorsData);
    // } else if (oledState == NETWORK) {
    //     oled->displayIp(NTP.getUptimeString(), timing->getRtcDateTime(), NTP.getTimeStr(),
    // cache->getCachedCount());
    // } else if (oledState == LOG) {
    //     oled->displayLog();
    // } else if (oledState == OLED_AUTO_OFF_SWITCH) {
    //     oled->displayAutoOffSwitch();
    //     if (millis() - oledStateChangedTime > 5000) {
    //         oled->alwaysOn = !oled->alwaysOn;
    //         oledState = SENSORS;
    //     }
    // } else if (oledState == WIFI_CHANGE) {
    //     oled->displayWifiChange();
    //     if (millis() - oledStateChangedTime > 5000) {
    //         meteoLog->add("Changing wifi network");
    //         co2->detachCo2Interrupt();
    //         wifiConfig->startPortal();
    //         co2->attachCo2Interrupt();
    //         oledState = SENSORS;
    //     }
    // } else if (oledState == EMPTY) {
    //     oled->displayEmpty();
    // }

    // delay(100);
    // int buttonState = digitalRead(BUTTON);
    // if (buttonState == HIGH) {
    //     if (millis() - timeButtonPress > 500) {
    //         timeButtonPress = millis();
    //         oledStateChangedTime = millis();

    //         oledState = (oledState + 1) % oledStateNum;
    //         if (oledState == WIFI_CHANGE && millis() > 60 * 1000) { // 2 min
    //             oledState = (oledState + 1) % oledStateNum;
    //         }
    //         if (oledState == EMPTY) {
    //             oledState = (oledState + 1) % oledStateNum;
    //         }
    //     }
    // }
}
