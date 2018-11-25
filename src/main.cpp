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

#define BUTTON D4

OTAUpdate otaUpdate;

WiFiConfig *wifiConfig;
SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
Timing *timing;
CO2SensorSenseAir *co21;
CO2SensorSenseAir *co22;

SensorsCache *cache;
SensorsData *sensorsData;
long bootTime;

void setup() {
    Serial.begin(115200);

    // SaveCrash.clear();
    // SaveCrash.print();

    // pinMode(PWM, INPUT);

    // pinMode(PWM, OUTPUT);
    // digitalWrite(PWM, HIGH);

    bootTime = now();

    // Wire.begin();

    // setup pins
    // pinMode(BUTTON, INPUT);

    // turn on LED until connected
    // led.on();

    // setup log and OLED
    meteoLog = new MeteoLog();
    oled = new OLED(meteoLog);
    oled->showMessage("Booting...");

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay
    // disconnected
    oled->showMessage("Trying to connect WiFi");
    wifiConfig = new WiFiConfig();
    wifiConfig->connectWiFi(false);
    // led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // CO2
    oled->showMessage("CO2 init...");
    co21 = new CO2SensorSenseAir(D1, D2);
    co22 = new CO2SensorSenseAir(D7, D8);
    oled->showMessage("CO2 init... Done");

    // // DS18B20 temperature
    // oled->showMessage("DS18B20 init...");
    // temp = new SensorDallasTemp();
    // oled->showMessage("DS18B20 init... Done");

    // // BME280
    // oled->showMessage("BME280 init...");
    // bme = new BME280();
    // oled->showMessage("BME280 init... Done");

    // // DHT11
    // oled->showMessage("DHT11 init...");
    // dht = new DHTSensor();
    // oled->showMessage("DHT11 init... Done");

    // NTP and RTC
    oled->showMessage("NTP/RTC init...");
    timing = new Timing();
    oled->showMessage("NTP/RTC init... Done");

    Serial.print("SPIFFS: ");
    oled->showMessage("SPIFFS init...");
    SPIFFS.begin();
    oled->showMessage("SPIFFS init... Done");

    oled->showMessage("Cache init...");
    cache = new SensorsCache();
    oled->showMessage("Cache init... Done");

    oled->showMessage("SensorsData init...");
    sensorsData = new SensorsData();
    oled->showMessage("SensorsData init... Done");

    oledState = SENSORS;
}

// timers
long timeButtonPress = 0;
long timeScanCo21 = -1e9;
long timeScanCo22 = -1e9;
long timeDataSend = -10000;
long rtcTimeUpdate = -RTC_TIME_UPDATE_TIMEOUT;
long oledStateChangedTime = 0;
long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;

void tryUpdateSensors() {
    float dsTempOne;
    float dsTempTwo;
    float dhtHum;
    float bmePressure;
    float bmeHum;
    long millisNow = millis();
    long timestampNow = -1;

    if (millis() - timeScanCo21 > CO2_TIMEOUT) {

        Serial.println("now: " + String(now()));
        timing->dumpNtp();

        timeScanCo21 = millis();
        int co2uart = co21->read();

        if (co2uart > 0) {
            if (timestampNow == -1) {
                timestampNow = now();
            }
            sensorsData->co21.set(co2uart, timestampNow);
        }

        meteoLog->add("co2 1 ppm " + String(co2uart));
        meteoLog->add("co2 1 abc " + String(co21->getABC()));
    }

    if (millis() - timeScanCo22 > CO2_TIMEOUT) {
        timeScanCo22 = millis();
        int co2uart = co22->read();

        if (co2uart > 0) {
            if (timestampNow == -1) {
                timestampNow = now();
            }
            sensorsData->co22.set(co2uart, timestampNow);
        }

        meteoLog->add("co2 2 ppm " + String(co2uart));
        meteoLog->add("co2 2 abc " + String(co22->getABC()));
    }

    sensorsData->uptime.set(millis() / 1000 / 60, timestampNow);
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

bool wifiOn() { return true; }

long stt = millis();
bool is_calib = false;
// bool is_calib = true;

int cnt2 = 0;

void loop() {
    otaUpdate.handle();
    tryUpdateSensors();
    // sensorsData->serialize();
    // meteoLog->add(String(cnt2++));
    // float c = co2->read();
    // meteoLog->add(String(c));

    bool in_calib_range = false;

    int fr = 1543072117;
    if (now() > fr) {
        if (now() < fr + 15 * 60) {
            in_calib_range = true;
        }
    }
    in_calib_range = false;
    if (in_calib_range && millis() - stt > 30000) {
        // stt = millis();

        // Serial.println("\n\n\n\n\n\n\n");
        // Serial.println("Run calibration");
        // Serial.println("\n\n\n\n\n\n\n");
        // if (co21->calib()) {
        //     Serial.println("co21 calibrated");
        //     is_calib = true;
        // }
        // if (co22->calib()) {
        //     Serial.println("co22 calibrated");
        //     is_calib = true;
        // }
        // Serial.println("\n\n\n\n\n\n\n");
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

    if (millis() - timeDataSend > SERVER_SENDING_TIMEOUT) {
        // if (millis() - timeDataSend > 5000) {
        timeDataSend = millis();
        if (now() > 1542929780) {
            sendDataApi();
            Serial.println("sent");
        }
    }
    delay(100);

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
    //     oled->displayIp(NTP.getUptimeString(), timing->getRtcDateTime(), NTP.getTimeStr(), cache->getCachedCount());
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
