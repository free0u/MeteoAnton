#include <ESP8266HTTPClient.h>
#include "EspSaveCrash.h"
#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

#include "BME280.h"
#include "CO2SensorSenseAir.h"
#include "DHTSensor.h"
#include "Led.h"
#include "MeteoLog.h"
#include "OLED.h"
#include "OLEDStates.h"
#include "SensorDallasTemp.h"
#include "SensorsCache.h"
#include "SensorsData.h"
#include "Timeouts.h"
#include "Timing.h"

// Call to ESpressif SDK
extern "C" {
#include <user_interface.h>
}

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
    dht = new DHTSensor(D3);  // 1 opus
    // dht = new DHTSensor(D1); // 2 wave
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

    ("SensorsData init...");
    sensorsData = new SensorsData();
    oled->showMessage("SensorsData init... Done");

    // change MAC
    oled->showMessage(" *********** OLD ESP8266 MAC: *********** ");
    oled->showMessage(String(WiFi.macAddress()));

    uint8_t mac[6]{0xA8, 0xD8, 0xB4, 0x1D, 0xAA, 0xCE};
    // wifi_set_macaddr(0, const_cast<uint8 *>(mac));

    oled->showMessage(" *********** NEW ESP8266 MAC:  *********** ");
    oled->showMessage(String(WiFi.macAddress()));
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
    float dsTempIn1;
    float dsTempIn2;
    float dsTempOut;
    float dhtHum;
    long millisNow = millis();
    long timestampNow = -1;

    if (millisNow - sensorsDataUpdated > SENSORS_TIMEOUT) {
        sensorsDataUpdated = millisNow;

        meteoLog->add("Reading sensors...");
        dsTempIn1 = temp->temperatureOne();
        dsTempIn2 = temp->temperatureTwo();
        dsTempOut = temp->temperatureThree();

        meteoLog->add("dsTempIn1 " + String(dsTempIn1));
        meteoLog->add("dsTempIn2 " + String(dsTempIn2));
        meteoLog->add("dsTempOut " + String(dsTempOut));
        meteoLog->add("Reading sensors... Done");

        if (timestampNow == -1) {
            timestampNow = now();
        }

        if (!isnan(dsTempIn1)) {
            sensorsData->dsTempIn1.set(dsTempIn1, timestampNow);
        }
        if (!isnan(dsTempIn2)) {
            sensorsData->dsTempIn2.set(dsTempIn2, timestampNow);
        }
        if (!isnan(dsTempOut)) {
            sensorsData->dsTempOut.set(dsTempOut, timestampNow);
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

    oled->showMessage("Sending data...");

    http.begin("***REMOVED***dino");
    http.setTimeout(10000);
    http.addHeader("Sensors-Names", sensorsData->sensorsNames);
    int statusCode = http.POST(sensorsData->serialize());
    String payload = http.getString();
    oled->showMessage("Code: " + String(statusCode));
    oled->showMessage("Payload: " + payload);

    http.end();
    oled->showMessage("Sending data complete...");
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
    // int buttonState2 = digitalRead(BUTTON);

    // Serial.println(String(cnt2++) + " test");
    // Serial.println("button: " + String(buttonState2));
    // delay(1000);
    // return;

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
}

bool checkTime(long &ts, long delay) {
    if (millis() - ts > delay) {
        ts = millis();
        return true;
    }
    return false;
}

void example() {
    long ts = 0;
    if (checkTime(ts, 800)) {
        // do something
    }
}