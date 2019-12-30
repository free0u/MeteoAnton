#include <ESP8266HTTPClient.h>
#include "BuildVersion.h"
#include "EspSaveCrash.h"
#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

#include "ESP8266httpUpdate.h"

#include "CO2SensorSenseAir.h"
#include "DHTSensor.h"

#include "BME280.h"
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
BME280 bme;

SensorsCache cache;
SensorsData sensorsData;
long bootTime;

DeviceConfig config;
long* sensorsUpdateTime;

void initSensors();

void processInternetUpdate(String const& device, String const& version) {
    ESPhttpUpdate.setLedPin(D4, LOW);
    t_httpUpdate_return ret =
        ESPhttpUpdate.update("***REMOVED***" + device + "&version=" + version);
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            meteoLog.add("[update] Update failed.");
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(),
                          ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            meteoLog.add("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:
            meteoLog.add("[update] Update ok.");  // may not be called since we reboot the ESP
            break;
    }
}

void recover() {
    led.on();
    delay(2000);
    Serial.println("Recover start");

    bool pressed = true;
    for (int i = 0; i < 10; i++) {
        int buttonState = digitalRead(BUTTON);
        Serial.println("button: " + String(buttonState));
        if (buttonState == HIGH) {
            pressed = false;
            break;
        }
        delay(200);
    }
    led.off();

    if (pressed) {
        for (int i = 0; i < 10; i++) {
            led.change();
            delay(200);
        }
        Serial.println("Starting wifi portal...");
        // wifiConfig.startPortal();
    }
    led.off();

    // led.on();
    ESPhttpUpdate.update("***REMOVED***" + String(ESP.getChipId()));
    // led.off();
    Serial.println("Recover end");
}

void setup() {
    Serial.begin(115200);

    return;

    pinMode(BUTTON, INPUT_PULLUP);
    led.off();

    recover();

    int chipId = ESP.getChipId();
    Serial.printf("\n%d", chipId);

    config = getDeviceConfigById(chipId);

    // SaveCrash.clear();
    // SaveCrash.print();

    bootTime = now();

    // setup log
    meteoLog.init();
    meteoLog.add("Booting...");

    // setup wifi connection
    led.on();
    meteoLog.add("Connecting to WiFi...");
    if (wifiConfig.connect()) {
        led.off();
        processInternetUpdate(config.deviceName, String(FIRMWARE_VERSION));
    }
    meteoLog.add("WiFi connected");

    // configure and start OTA update server
    meteoLog.add("OTA server init...");
    otaUpdate.init(config.deviceName);
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
    cache.init(config.sensorsApiUrl);
    meteoLog.add("Cache init... Done");

    // change MAC
    meteoLog.add(" *********** OLD ESP8266 MAC: *********** ");
    meteoLog.add(String(WiFi.macAddress()));

    uint8_t mac[6]{0xA8, 0xD8, 0xB4, 0x1D, 0xA4, 0xCE};
    if (config.deviceName == "wave") {
        wifi_set_macaddr(0, const_cast<uint8*>(mac));
    }

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
                temp.init(sensorConfig.pin1, &meteoLog);
                meteoLog.add("DS18B20 init... Done");
                break;
            case DHT_SENSOR:
                meteoLog.add("DHT22 init...");
                dht.init(sensorConfig.pin1);
                meteoLog.add("DHT22 init... Done");
                break;
            case CO2_SENSEAIR_SENSOR:
                meteoLog.add("CO2 init...");
                co2.init(sensorConfig.pin1, sensorConfig.pin2);
                meteoLog.add("CO2 init... Done");
                break;
            case BME_SENSOR:
                meteoLog.add("BME280 init...");
                Wire.begin(sensorConfig.pin1, sensorConfig.pin2);
                bme.init(sensorConfig.address[0]);
                meteoLog.add("BME280 init... Done");
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

float readSensor(SensorConfig& sensorConfig) {
    switch (sensorConfig.type) {
        case UPTIME_SENSOR:
            return millis() / 1000 / 60;
        case BUILD_VERSION_SENSOR:
            return BUILD_VERSION;
        case FIRMWARE_VERSION_SENSOR:
            return FIRMWARE_VERSION;
        case FREE_HEAP_SENSOR:
            return ESP.getFreeHeap();
        case DALLAS_SENSOR:
            return temp.getTemp(sensorConfig.address);
        case BME_SENSOR:
            return bme.humidity();
        case DHT_SENSOR:
            return dht.humidity();
        case CO2_SENSEAIR_SENSOR:
            return co2.read();
        default:
            return NAN;
    }
}

void tryUpdateSensors() {
    for (int i = 0; i < config.sensorsCount; i++) {
        SensorConfig& sensorConfig = config.sensors[i];
        Sensor& sensorData = sensorsData.sensors[i];

        if (checkTime(sensorsUpdateTime[i], sensorConfig.timeout)) {
            float value = readSensor(sensorConfig);
            meteoLog.add(String(sensorConfig.type) + " " + String(sensorConfig.debug_name) + " read: " + value);
            sensorData.set(value, now());
        }
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
        http.begin(config.sensorsApiUrl);
        http.setTimeout(10000);
        http.addHeader("Sensors-Names", sensorsData.sensorsNames);
        int statusCode = http.POST(sensorsData.serialize());
        if (statusCode != 200) {
            meteoLog.add("Code: " + String(statusCode));
            String payload = http.getString();
            meteoLog.add("Payload: " + payload);
        }

        http.end();
        meteoLog.add("Sending data complete");
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

int num = 0;
void loop() {
    /*
    emonlib test
    */

    num++;
    Serial.println("hello: " + String(num));

    delay(1000);
    return;

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

        meteoLog.add("Trying to update firmware...");
        processInternetUpdate(config.deviceName, String(FIRMWARE_VERSION));

        meteoLog.sendLog(config.deviceName, "***REMOVED***");
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
