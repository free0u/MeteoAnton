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
#include "CO2Sensor.h"
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
CO2Sensor *co2;

SensorsCache *cache;
SensorsData *sensorsData;
long bootTime;

void setup() {
    Serial.begin(115200);

    SaveCrash.clear();
    SaveCrash.print();

    pinMode(PWM, INPUT);

    // pinMode(PWM, OUTPUT);
    // digitalWrite(PWM, HIGH);

    bootTime = now();

    Wire.begin();

    // setup pins
    pinMode(BUTTON, INPUT);

    // turn on LED until connected
    led.on();

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
    // wifiConfig->connectWiFi(false);
    led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // CO2
    oled->showMessage("CO2 init...");
    co2 = new CO2Sensor();
    oled->showMessage("CO2 init... Done");

    // DS18B20 temperature
    oled->showMessage("DS18B20 init...");
    temp = new SensorDallasTemp();
    oled->showMessage("DS18B20 init... Done");

    // BME280
    oled->showMessage("BME280 init...");
    bme = new BME280();
    oled->showMessage("BME280 init... Done");

    // DHT11
    oled->showMessage("DHT11 init...");
    dht = new DHTSensor();
    oled->showMessage("DHT11 init... Done");

    // NTP and RTC
    oled->showMessage("NTP/RTC init...");
    timing = new Timing();
    oled->showMessage("NTP/RTC init... Done");

    Serial.print("SPIFFS: ");
    oled->showMessage("SPIFFS init...");
    SPIFFS.begin();
    oled->showMessage("SPIFFS init... Done");

    SPIFFS.remove("/data.json");

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.println(fs_info.totalBytes);
    Serial.println(fs_info.usedBytes);

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
long timeScanCo2 = -1e9;
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

    if (millisNow - sensorsDataUpdated > SENSORS_TIMEOUT) {
        sensorsDataUpdated = millisNow;

        meteoLog->add("Reading sensors...");
        dsTempOne = temp->temperatureOne();
        dsTempTwo = temp->temperatureTwo();
        bmePressure = bme->pressure();
        bmeHum = bme->humidity();

        meteoLog->add("dsTempOne " + String(dsTempOne));
        meteoLog->add("dsTempTwo " + String(dsTempTwo));
        meteoLog->add("bmePressure " + String(bmePressure));
        meteoLog->add("bmeHum " + String(bmeHum));
        meteoLog->add("Reading sensors... Done");

        if (timestampNow == -1) {
            timestampNow = timing->getRtcTimestamp();
        }

        if (!isnan(dsTempOne)) {
            sensorsData->dsTempOne.set(dsTempOne, timestampNow);
        }

        if (!isnan(dsTempTwo)) {
            sensorsData->dsTempTwo.set(dsTempTwo, timestampNow);
        }

        if (!isnan(bmeHum)) {
            sensorsData->bmeHum.set(bmeHum, timestampNow);
        }

        if (!isnan(bmePressure)) {
            sensorsData->bmePressure.set(bmePressure, timestampNow);
        }

        if (millisNow - co2->getPpmUpateTime() < 10000) {
            sensorsData->co2.set(co2->getPpmPwm(), timestampNow);
        }

        sensorsData->uptimeMinutes.set(millis() / 1000 / 60, timestampNow);
    }

    if (millis() - dhtSensorUpdated > DHT_TIMEOUT) {
        dhtSensorUpdated = millis();
        dhtHum = dht->humidity();
        meteoLog->add("dhtHum " + String(dhtHum));

        if (!isnan(dhtHum)) {
            if (timestampNow == -1) {
                timestampNow = timing->getRtcTimestamp();
            }
            sensorsData->dhtHum.set(dhtHum, timestampNow);
        }
    }

    if (millis() - timeScanCo2 > CO2_TIMEOUT) {
        timeScanCo2 = millis();
        int co2uart = co2->getPpmUart();
        co2uart = 100;

        if (co2uart != -1) {
            if (timestampNow == -1) {
                timestampNow = timing->getRtcTimestamp();
            }
            sensorsData->co2uart.set(co2uart, timestampNow);
        }

        meteoLog->add("co2 ppm " + String(co2uart) + "(" + String(co2->getPpmPwm()) + ")");
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

bool wifiOn() { return true; }

void loop() {
    otaUpdate.handle();
    tryUpdateSensors();

    if (millis() - rtcTimeUpdate > RTC_TIME_UPDATE_TIMEOUT) {
        rtcTimeUpdate = millis();
        timing->updateRtc();
        meteoLog->add("RTC updated: " + timing->getRtcDateTime());
    }

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

    if (!oled->alwaysOn && millis() - oledStateChangedTime > 7000) {
        oledState = EMPTY;
    }

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(NTP.getUptimeString(), timing->getRtcDateTime(), NTP.getTimeStr(), cache->getCachedCount());
    } else if (oledState == LOG) {
        oled->displayLog();
    } else if (oledState == OLED_AUTO_OFF_SWITCH) {
        oled->displayAutoOffSwitch();
        if (millis() - oledStateChangedTime > 5000) {
            oled->alwaysOn = !oled->alwaysOn;
            oledState = SENSORS;
        }
    } else if (oledState == WIFI_CHANGE) {
        oled->displayWifiChange();
        if (millis() - oledStateChangedTime > 5000) {
            meteoLog->add("Changing wifi network");
            co2->detachCo2Interrupt();
            wifiConfig->startPortal();
            co2->attachCo2Interrupt();
            oledState = SENSORS;
        }
    } else if (oledState == EMPTY) {
        oled->displayEmpty();
    }

    delay(100);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        if (millis() - timeButtonPress > 500) {
            timeButtonPress = millis();
            oledStateChangedTime = millis();

            oledState = (oledState + 1) % oledStateNum;
            if (oledState == WIFI_CHANGE && millis() > 60 * 1000) { // 2 min
                oledState = (oledState + 1) % oledStateNum;
            }
            if (oledState == EMPTY) {
                oledState = (oledState + 1) % oledStateNum;
            }
        }
    }
}
