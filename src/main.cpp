#include <ESP8266HTTPClient.h>
#include "BuildVersion.h"
#include "EspSaveCrash.h"
#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

#include "CO2SensorSenseAir.h"
#include "DHTSensor.h"

#include "Led.h"
#include "MeteoLog.h"
#include "SensorDallasTemp.h"
#include "SensorsCache.h"
#include "SensorsData.h"
#include "Timeouts.h"
#include "Timing.h"

#include "DevicesConfig.h"

// Call to ESpressif SDK
extern "C" {
#include <user_interface.h>
}

#define BUTTON 0

OTAUpdate otaUpdate;

WiFiConfig wifiConfig;
SensorDallasTemp temp;
DHTSensor dht;
MeteoLog meteoLog;
Led led;
Timing timing;
CO2SensorSenseAir co2;

SensorsCache cache;
SensorsData sensorsData;
long bootTime;

DeviceConfig config;
long* sensorsUpdateTime;

void initSensors();

void setup() {
    int chipId = ESP.getChipId();
    Serial.println(chipId);

    config = getDeviceConfigById(chipId);

    Serial.begin(115200);
    // SaveCrash.clear();
    // SaveCrash.print();

    bootTime = now();

    // setup pins
    // pinMode(BUTTON, INPUT);
    pinMode(BUTTON, INPUT_PULLUP);

    // setup log
    meteoLog.init();
    meteoLog.add("Booting...");

    // setup wifi connection
    led.on();
    meteoLog.add("Connecting to WiFi...");
    if (wifiConfig.connect()) {
        led.off();
    }
    meteoLog.add("WiFi connected");

    // configure and start OTA update server
    meteoLog.add("OTA server init...");
    otaUpdate.setup();
    meteoLog.add("OTA server init... Done");

    // initSensors
    initSensors();

    // NTP and RTC
    meteoLog.add("NTP/RTC init...");
    timing.init();
    meteoLog.add("NTP/RTC init... Done");

    meteoLog.add("SPIFFS init...");
    SPIFFS.begin();
    meteoLog.add("SPIFFS init... Done");

    meteoLog.add("Cache init...");
    cache.init();
    meteoLog.add("Cache init... Done");

    // change MAC
    meteoLog.add(" *********** OLD ESP8266 MAC: *********** ");
    meteoLog.add(String(WiFi.macAddress()));

    uint8_t mac[6]{0xA8, 0xD8, 0xB4, 0x1D, 0xAA, 0xCE};
    // wifi_set_macaddr(0, const_cast<uint8 *>(mac));

    meteoLog.add(" *********** NEW ESP8266 MAC:  *********** ");
    meteoLog.add(String(WiFi.macAddress()));
}

void initSensors() {
    meteoLog.add("SensorsData init...");
    sensorsUpdateTime = new long[config.sensorsCount];
    for (int i = 0; i < config.sensorsCount; i++) {
        sensorsUpdateTime[i] = -1e9;
    }
    sensorsData.init(config.sensors, config.sensorsCount);

    for (int i = 0; i < config.sensorsCount; i++) {
        SensorConfig& sensorConfig = config.sensors[i];
        switch (sensorConfig.type) {
            case DALLAS_SENSOR:  // DS18B20 temperature
                meteoLog.add("DS18B20 init...");
                temp.init(sensorConfig.pin1);
                meteoLog.add("DS18B20 init... Done");
                break;
            case DHT_SENSOR:
                // meteoLog.add("DHT22 init...");
                // dht.init(D3);  // 1 opus
                // dht = new DHTSensor(D1); // 2 wave
                // meteoLog.add("DHT22 init... Done");
                break;
            case CO2_SENSEAIR_SENSOR:
                // meteoLog.add("CO2 init...");
                // co2.init(D7, D8);
                // meteoLog.add("CO2 init... Done");
                break;
            default:;
        }
    }
    meteoLog.add("SensorsData init... Done");
}

// timers
long timeButtonPress = 0;
long timeScanCo21 = -1e9;
long timeScanCo22 = -1e9;
long timeDataSend = -1e9;
long rtcTimeUpdate = -RTC_TIME_UPDATE_TIMEOUT;
long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;

int buttonCount = 0;

bool checkTime(long& ts, long delay) {
    if (millis() - ts > delay) {
        ts = millis();
        return true;
    }
    return false;
}

void tryUpdateSensors() {
    float value;
    float dsTempIn1;
    float dsTempIn2;
    float dsTempOut;
    float dhtHum;
    long timestampNow = -1;

    for (int i = 0; i < config.sensorsCount; i++) {
        SensorConfig& sensorConfig = config.sensors[i];
        Sensor& sensorData = sensorsData.sensors[i];

        if (checkTime(sensorsUpdateTime[i], sensorConfig.timeout)) {
            switch (sensorConfig.type) {
                case UPTIME_SENSOR:
                    sensorData.set(millis() / 1000 / 60, now());
                    break;
                case BUILD_VERSION_SENSOR:
                    sensorData.set(BUILD_VERSION, now());
                    break;
                case FIRMWARE_VERSION_SENSOR:
                    sensorData.set(FIRMWARE_VERSION, now());
                    break;
                case FREE_HEAP_SENSOR:
                    sensorData.set(ESP.getFreeHeap(), now());
                    break;
                case DALLAS_SENSOR:
                    value = temp.getTemp(sensorConfig.address);
                    sensorData.set(value, now());
                    break;
                default:;
            }
        }
    }

    if (false && checkTime(sensorsDataUpdated, SENSORS_TIMEOUT)) {
        meteoLog.add("Reading sensors...");
        dsTempIn1 = temp.temperatureOne();
        dsTempIn2 = temp.temperatureTwo();
        dsTempOut = temp.temperatureThree();

        meteoLog.add("dsTempIn1 " + String(dsTempIn1));
        meteoLog.add("dsTempIn2 " + String(dsTempIn2));
        meteoLog.add("dsTempOut " + String(dsTempOut));
        meteoLog.add("Reading sensors... Done");

        if (timestampNow == -1) {
            timestampNow = now();
        }

        if (!isnan(dsTempIn1)) {
            sensorsData.sensors[0].set(dsTempIn1, timestampNow);
        }
        // if (!isnan(dsTempIn2)) {
        //     sensorsData.dsTempIn2.set(dsTempIn2, timestampNow);
        // }
        // if (!isnan(dsTempOut)) {
        //     sensorsData.dsTempOut.set(dsTempOut, timestampNow);
        // }

        // sensorsData.sensors[1].set(millis() / 1000 / 60, timestampNow);

        // sensorsData.sensors[2].set(BUILD_VERSION, timestampNow);
    }

    if (false && checkTime(dhtSensorUpdated, DHT_TIMEOUT)) {
        dhtHum = dht.humidity();
        meteoLog.add("dhtHum1 " + String(dhtHum));

        // if (!isnan(dhtHum)) {
        //     if (timestampNow == -1) {
        //         timestampNow = now();
        //     }
        //     sensorsData.dhtHum.set(dhtHum, timestampNow);
        // }
    }

    // if (checkTime(timeScanCo21, config.sensorsReadTimeout)) {
    //     int co2uart = co2.read();

    //     if (co2uart > 0) {
    //         if (timestampNow == -1) {
    //             timestampNow = now();
    //         }
    //         sensorsData.co2.set(co2uart, timestampNow);
    //     }

    //     meteoLog.add("co2 ppm " + String(co2uart));
    // }
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

//     url += generateThingspeakPair(1, sensorsData.co2uart);
//     url += generateThingspeakPair(2, up / 60);
//     url += generateThingspeakPair(3, sensorsData.co2);
//     url += generateThingspeakPair(4, sensorsData.dsTempOne);
//     url += generateThingspeakPair(5, sensorsData.bmeHum);

//     HTTPClient http;
//     http.begin(url);
//     int statusCode = http.GET();
//     http.end();
//     return statusCode;
// }

int sendDataApi(bool reallySend) {
    HTTPClient http;

    meteoLog.add("Sending data...");

    if (reallySend) {
        http.begin("***REMOVED***dino");
        http.setTimeout(10000);
        http.addHeader("Sensors-Names", sensorsData.sensorsNames);
        int statusCode = http.POST(sensorsData.serialize());
        String payload = http.getString();
        meteoLog.add("Code: " + String(statusCode));
        meteoLog.add("Payload: " + payload);

        http.end();
        meteoLog.add("Sending data complete...");
        return statusCode;
    } else {
        meteoLog.add("Sensors-Names: ", sensorsData.sensorsNames);
        meteoLog.add("Sensors-Data:");
        sensorsData.serialize();
        return 0;
    }
}

long dumpJsonTime = -1e9;

bool wifiOn() {
    return true;
    // return false;

    long secs = millis() / 1000;
    secs = now();
    long minutes = secs / 60 % 60;
    meteoLog.add("now(): " + String(now()));
    meteoLog.add("Minutes: " + String(minutes));
    meteoLog.add("Minutes %% 10: " + String(minutes));
    return minutes % 10 >= 5;
}

long stt = millis();
bool is_calib = false;
// bool is_calib = true;

int cnt2 = 0;

void loop() {
    // int buttonState2 = digitalRead(BUTTON);

    // meteoLog.add(String(cnt2++) + " test");
    // meteoLog.add("button: " + String(buttonState2));
    // delay(1000);
    // return;

    delay(100);
    otaUpdate.handle();
    tryUpdateSensors();

    // if (checkTime(timeDataSend, SERVER_SENDING_TIMEOUT)) {
    if (checkTime(timeDataSend, 10000)) {
        if (cache.empty()) {
            int statusCode = -1;
            if (wifiOn()) {
                statusCode = sendDataApi(true);
            }
            if (statusCode != 200) {
                cache.add(sensorsData);
            }
        } else {
            cache.clear();
            // cache.add(sensorsData);
            // if (wifiOn()) {
            //     cache.sendCache(sensorsData.sensorsNames);
            // }
        }
    }

    int buttonState = digitalRead(BUTTON);
    if (buttonState == LOW) {
        if (checkTime(timeButtonPress, 800)) {
            buttonCount++;
        }
    } else {
        buttonCount = 0;
    }

    if (buttonCount > 5) {
        buttonCount = 0;

        led.on();
        wifiConfig.startPortal();
        led.off();
    }
}
