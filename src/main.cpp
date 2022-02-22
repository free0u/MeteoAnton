#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>

#include "EspSaveCrash.h"
#include "helpers/CheckTime.h"
#include "modules/led/Led.h"
#include "modules/radio433/RxTx433.h"
#include "modules/web_api/WebApi.h"
#include "sensors/SensorDallasTemp/SensorDallasTemp.h"
#include "sensors/co2/CO2SensorSenseAir.h"
#include "sensors/dht/DHTSensor.h"
#include "sensors/electro/ElectroSensorStorage.h"
#include "sensors/electro/EmonLibSensor.h"
#include "sensors/store/SensorsCache.h"
#include "sensors/store/SensorsData.h"
#include "sensors/water/WaterSensorStorage.h"
#include "system/BuildVersion.h"
#include "system/DevicesConfig.h"
#include "system/MeteoLog.h"
#include "system/OTAUpdate.h"
#include "system/Timeouts.h"
#include "system/Timing.h"
#include "system/Timing2.h"
#include "system/WiFiConfig.h"

#define WEBSERVER_H
#include "ESPAsyncWebServer.h"

#define FLASH_BUTTON 0
#define LED_PIN D4

// Call to ESpressif SDK
// extern "C" {
// #include <user_interface.h>
// }

// utils
EspSaveCrash SaveCrash(0x0010, 0x0400);
Led led;
CheckTime checkTimeClass;
OTAUpdate otaUpdate;
MeteoLog meteoLog;
WebApi webApi;
// Timing timing;
Timing2 timing2;
// configs
WiFiConfig wifiConfig;
DeviceConfig config;
// hardware
ElectroSensorStorage electroSensorStorage;
WaterSensorStorage waterSensorStorage;
CO2SensorSenseAir co2;
SensorDallasTemp temp;
DHTSensor dht;
EmonLibSensor emonSensor;
RxTx433 rxtx;
// sensors storage
SensorsCache cache;
SensorsData sensorsData;

bool hasReceiver433 = false;
bool hasTransmitter433 = false;
bool hasIrmsSensor = false;
bool hasWaterSensor = false;

char* _debugOutputBuffer;
char* _debugOutputBufferForCrash;

void recover() {
    led.on();
    delay(2000);
    Serial.println("\n\nRecover start");

    bool pressed = true;
    for (int i = 0; i < 10; i++) {
        int buttonState = digitalRead(FLASH_BUTTON);
        Serial.println("button: " + String(buttonState));
        if (buttonState == HIGH) {
            pressed = false;
            break;
        }
        delay(200);
    }

    if (pressed) {
        for (int i = 0; i < 10; i++) {
            led.change();
            delay(200);
        }
        Serial.println("Starting wifi portal...");
        wifiConfig.startPortal();

        WiFiClient client;
        ESPhttpUpdate.update(client, RECOVER_FIRMWARE_URL + String(ESP.getChipId()));
    }

    Serial.println("Recover end");
}

String getCrash();
String makeCrash();
String clearCrash();

void initSensors();
void processInternetUpdate(String const& device, String const& version, bool callFromSetup);
void setupHandles();

void setup() {
    Serial.begin(115200);
    pinMode(FLASH_BUTTON, INPUT_PULLUP);  // flash button
    led.init(LED_PIN);                    // inner led
    recover();

    int chipId = ESP.getChipId();
    Serial.printf("\n\nChipId: %d\n\n", chipId);
    config = getDeviceConfigById(chipId);

    if (wifiConfig.connect(config.deviceName)) {
        led.off();
        processInternetUpdate(config.deviceName, String(FIRMWARE_VERSION), true);
    }

    _debugOutputBuffer = (char*)calloc(2048, sizeof(char));
    SaveCrash.print();
    SaveCrash.clear();

    meteoLog.init();
    meteoLog.add("Booting...");
    meteoLog.add("IP address: " + WiFi.localIP().toString());

    // configure and start OTA update server
    meteoLog.add("OTA server init...");
    otaUpdate.init(config.deviceName);
    meteoLog.add("OTA server init... Done");

    // init 433
    // meteoLog.add("433 init...");
    // if (!driver.init()) {
    //     meteoLog.add("433 init... Fail");
    // } else {
    //     meteoLog.add("433 init... Done OK");
    // }

    // NTP and RTC
    // meteoLog.add("NTP/RTC init...");
    // timing.init(&meteoLog);
    // meteoLog.add("NTP/RTC init... Done");

    meteoLog.add("Timing 2...");
    timing2.init(&meteoLog);
    meteoLog.add("Timing 2 init... Done");

    meteoLog.add("LittleFS init...");
    LittleFS.begin();
    meteoLog.add("LittleFS init... Done");

    // initSensors
    initSensors();

    meteoLog.add("Cache init...");
    cache.init(config.sensorsApiUrl);
    meteoLog.add("Cache init... Done");

    // change MAC
    meteoLog.add(" *********** OLD ESP8266 MAC: *********** ");
    meteoLog.add(String(WiFi.macAddress()));

    // use if device is banned in network
    // uint8_t mac[6]{0xA8, 0xD8, 0xB4, 0x1D, 0xA4, 0xCE};
    // if (config.deviceName == "wave") {
    //     wifi_set_macaddr(0, const_cast<uint8*>(mac));
    // }

    meteoLog.add(" *********** NEW ESP8266 MAC:  *********** ");
    meteoLog.add(String(WiFi.macAddress()));

    webApi.init();
    setupHandles();
}

void setupHandles() {
    webApi.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        // request->send(200, "text/plain", "123");
        request->send(200, "text/plain",
                      "millis: " + String(millis()) + "\n" + "Sensors-Names: " + sensorsData.sensorsNames + "\n" +
                          "Sensors: " + sensorsData.serialize());
    });

    webApi.on("/crash", HTTP_GET, [](AsyncWebServerRequest* request) {
        // request->send(200, "text/plain", "123");
        request->send(200, "text/plain", getCrash());
    });
    webApi.on("/clearCrash", HTTP_GET,
              [](AsyncWebServerRequest* request) { request->send(200, "text/plain", clearCrash()); });
    webApi.on("/makeCrash", HTTP_GET,
              [](AsyncWebServerRequest* request) { request->send(200, "text/plain", makeCrash()); });

    webApi.on("/led", HTTP_GET, [](AsyncWebServerRequest* request) {
        String inputMessage;
        if (request->hasParam("value")) {
            inputMessage = request->getParam("value")->value();
            analogWrite(LED_PIN, inputMessage.toInt());
        } else {
            inputMessage = "No message sent";
        }
        Serial.println(inputMessage);
        request->send(200, "text/plain", "OK");
    });
    webApi.begin();
}

void processInternetUpdate(String const& device, String const& version, bool callFromSetup) {
    ESPhttpUpdate.setLedPin(LED_PIN, LOW);
    WiFiClient client;
    String url = REGULAR_UPDATE_FIRMWARE_URL + device + "&version=" + version;
    if (callFromSetup) {
        url += "&afterStart=true";
    }
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            if (!callFromSetup) {
                meteoLog.add("[update] Update failed.");
            }
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(),
                          ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            if (!callFromSetup) {
                meteoLog.add("[update] Update no Update.");
            }
            break;
        case HTTP_UPDATE_OK:
            if (!callFromSetup) {
                meteoLog.add("[update] Update ok.");  // may not be called since we reboot the ESP
            }
            break;
    }
    led.off();
}

String getCrash() {
    strcpy(_debugOutputBuffer, "");
    SaveCrash.crashToBuffer(_debugOutputBuffer);
    return String(_debugOutputBuffer);
}

String makeCrash() {
    // cppcheck-suppress1 unusedVariable
    int ssss = 1 / 0;
    strcpy(_debugOutputBufferForCrash, "crash it");
    return "crashed";
}

String clearCrash() {
    SaveCrash.clear();
    return "cleared";
}

float powerSpent = -1e9;
float waterSpent = -1e9;

long lastLoopTimeStamp = 0;
long maxLoopTime = 0;
long maxUpdateSensorTime = 0;

void initSensors() {
    meteoLog.add("SensorsData init...");
    checkTimeClass.initSensorUpdateTime(config.sensorsCount);
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
                dht.init(sensorConfig.pin1, sensorConfig.pin2);
                meteoLog.add("DHT22 init... Done");
                break;
            case CO2_SENSEAIR_SENSOR:
                meteoLog.add("CO2 init...");
                co2.init(sensorConfig.pin1, sensorConfig.pin2);
                meteoLog.add("CO2 init... Done");
                break;
            case TRANSMITTER_433:
                meteoLog.add("433 transmitter init...");
                if (!rxtx.init(sensorConfig.pin1, sensorConfig.pin2, sensorConfig.pin3)) {
                    meteoLog.add("433 transmitter init... Fail");
                } else {
                    meteoLog.add("433 transmitter init... Done");
                    hasTransmitter433 = true;
                }
                break;
            case RECEIVER_433:
                meteoLog.add("433 receiver init...");
                if (!rxtx.init(sensorConfig.pin1, sensorConfig.pin2, sensorConfig.pin3)) {
                    meteoLog.add("433 receiver init... Fail");
                } else {
                    meteoLog.add("433 receiver init... Done");
                    hasReceiver433 = true;
                }
                break;
            // case BME_SENSOR:
            //     meteoLog.add("BME280 init...");
            //     Wire.begin(sensorConfig.pin1, sensorConfig.pin2);
            //     bme.init(sensorConfig.address[0]);
            //     meteoLog.add("BME280 init... Done");
            //     break;
            case IRMS_SENSOR:
                meteoLog.add("EmonLib Irms init...");
                emonSensor.init(sensorConfig.param1, sensorConfig.param2);
                meteoLog.add("EmonLib Irms init... Done");
                hasIrmsSensor = true;
                break;
            case POWER_SENSOR:
                meteoLog.add("EmonLib power init...");
                emonSensor.init(sensorConfig.param1, sensorConfig.param2);
                meteoLog.add("EmonLib power init... Done");
                hasIrmsSensor = true;
                break;
            case POWER_SPENT:
                // powerSpent = 0;
                powerSpent = electroSensorStorage.get();
                break;
            case WATER_SPENT:
                pinMode(sensorConfig.pin1, INPUT);
                hasWaterSensor = true;
                // waterSpent = 0;
                // waterSensorStorage.save(0);
                waterSpent = waterSensorStorage.get();
            default:;
        }
    }
    meteoLog.add("SensorsData init... Done");

    meteoLog.add("electroSensorStorage init read: " + String(powerSpent));
    meteoLog.add("waterSensorStorage init read: " + String(waterSpent));
}

// timers
// long timeButtonPress = 0;
long timeScanCo21 = -1e9;
long timeScanCo22 = -1e9;
// long time433Send = -1e9;
// long timeIrmsSumSpend = -1e9;
long rtcTimeUpdate = -RTC_TIME_UPDATE_TIMEOUT;
long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;

// irms
long irmsSumSpentIntervalSumTimestamp = -1e9;
float irmsSumSpentIntervalSumValue = 0;
long irmsSumSpentSmallTimestamp = -1e9;
float irmsSumSpentSmallValue = 0;

int buttonCount = 0;

// bool checkTime2(long& ts, long unsigned delay) {
//     if (millis() - ts > delay) {
//         ts = millis();
//         return true;
//     }
//     return false;
// }

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
        case RSSI_SENSOR:
            return WiFi.RSSI();
        case DALLAS_SENSOR:
            return temp.getTemp(sensorConfig.address);
        // case BME_SENSOR:
        //     return bme.humidity();
        case DHT_SENSOR:
            return dht.humidity();
        case DHT_SENSOR_TEMP:
            return dht.temp();
        case CO2_SENSEAIR_SENSOR:
            return co2.read();
        case IRMS_SENSOR:
            return emonSensor.Irms();
        case POWER_SENSOR:
            return emonSensor.power();
        // case POWER_SUM:
        //     return NAN;
        case POWER_SPENT:
            return powerSpent;
        case WATER_SPENT:
            return waterSpent;
        case MAX_LOOP_TIME:
            return maxLoopTime > 0 ? maxLoopTime : NAN;
        case UPDATE_SENSORS_TIME:
            return maxUpdateSensorTime > 0 ? maxUpdateSensorTime : NAN;
        default:
            return NAN;
    }
}

long timeTestDiff;
bool timeTestDiffCheck(String t) {
    long diff = millis() - timeTestDiff;
    timeTestDiff += diff;
    if (diff > 10 && t != "delay") {
        meteoLog.add("send_time " + t + ": " + String(diff));
    }
    return true;
}

void tryUpdateSensors() {
    for (int i = 0; i < config.sensorsCount; i++) {
        SensorConfig& sensorConfig = config.sensors[i];
        Sensor& sensorData = sensorsData.sensors[i];

        // if (sensorConfig.type == POWER_SUM) {
        //     continue;
        //     // сенсор обновляется в другом месте, когда приходит следующий интервал суммирования watt
        // }

        timeTestDiffCheck("before " + sensorConfig.debug_name);
        // if (checkTime2(sensorsUpdateTime[i], sensorConfig.timeout)) {
        if (checkTimeClass.checkSensorByInd(i, sensorConfig.timeout)) {
            float value = readSensor(sensorConfig);
            meteoLog.add(String(sensorConfig.type) + " " + String(sensorConfig.debug_name) + " read: " + value);
            sensorData.set(value, now());
            timeTestDiffCheck("after " + sensorConfig.debug_name);
        }
    }
}

int sendDataApi(bool reallySend) {
    meteoLog.add("Sending data...");

    if (reallySend) {
        // http.begin("http://ya.ru");
        // http.setTimeout(10000);
        // http.GET();
        // http.end();

        WiFiClient client;
        HTTPClient http;
        http.begin(client, config.sensorsApiUrl);
        http.setTimeout(10000);
        http.addHeader("Sensors-Names", sensorsData.sensorsNames);
        int statusCode = http.POST(sensorsData.serialize());
        if (statusCode != 200) {
            meteoLog.add("Code: " + String(statusCode));
            String payload = http.getString();
            meteoLog.add("Payload: " + payload);
        }

        http.end();
        meteoLog.add(sensorsData.serialize());
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

// TODO make handle to switch
bool wifiOn() {
    return true;
    // return false;

    long secs = millis() / 1000;
    secs = now();
    long minutes = secs / 60 % 60;
    meteoLog.add("now(): " + String(now()));
    meteoLog.add("Minutes: " + String(minutes));
    meteoLog.add("Minutes %% 10: " + String(minutes));
    // return minutes % 10 >= 5;
    return minutes % 2 == 0;
}

long stt = millis();
bool is_calib = false;
// bool is_calib = true;

int cnt2 = 0;

int num = 0;
int cacheCount = 0;

int cnt433 = 0;

// void clearIrmsSensorSum() {
//     for (int i = 0; i < config.sensorsCount; i++) {
//         SensorConfig& sensorConfig = config.sensors[i];
//         Sensor& sensorData = sensorsData.sensors[i];

//         if (sensorConfig.type != POWER_SUM) {
//             continue;
//         }
//         sensorData.set(NAN, now());
//     }
// }

// long testButtonWater = -1e9;

int waterButtonState = -1;

void loop() {
    timeTestDiff = millis();

    if (lastLoopTimeStamp == 0) {
        lastLoopTimeStamp = millis();
    } else {
        long dd = millis() - lastLoopTimeStamp;
        lastLoopTimeStamp += dd;
        if (maxLoopTime < dd) {
            maxLoopTime = dd;
        }
    }

    /*
    emonlib test
    */

    // Serial.println("I: " + String(emonSensor.Irms()));
    // Serial.println("Power: " + String(emonSensor.power()));

    // delay(100);
    // otaUpdate.handle();
    // return;

    // int buttonState2 = digitalRead(FLASH_BUTTON);

    // meteoLog.add(String(cnt2++) + " test");
    // meteoLog.add("button: " + String(buttonState2));
    // delay(1000);
    // return;

    /*
    emonlib test
    */

    delay(100);

    timeTestDiffCheck("delay");

    otaUpdate.handle();
    timeTestDiffCheck("ota handle");

    long updateSensorStart = millis();
    tryUpdateSensors();
    timeTestDiffCheck("tryUpdateSensors");

    long updateSensorTimeDiff = millis() - updateSensorStart;
    if (maxUpdateSensorTime < updateSensorTimeDiff) {
        maxUpdateSensorTime = updateSensorTimeDiff;
    }

    // if (checkTime(testButtonWater OK, 1000) && hasWaterSensor) {
    if (checkTimeClass.checkButtonWater(1000) && hasWaterSensor) {
        int newButtonState = digitalRead(D2);
        if (waterButtonState == -1) {
            waterButtonState = newButtonState;
        } else {
            if (waterButtonState != newButtonState) {
                delay(50);
                newButtonState = digitalRead(D2);
                if (waterButtonState != newButtonState) {
                    waterButtonState = newButtonState;

                    waterSpent += 5;
                    // led.change();
                    meteoLog.add("===== Water Sensor: add 5 liters. total: " + String(waterSpent));

                    waterSensorStorage.save(waterSpent);
                }
            }
        }

        meteoLog.add("===== Water Sensor button: " + String(newButtonState));
        meteoLog.add("===== Water Sensor total: " + String(waterSpent));

        // float value = electroSensorStorage.get();
        // meteoLog.add("electroSensorStorage: " + String(value));
        // electroSensorStorage.save(value + 1);
    }

    int SMALL_INTERVAL = 3000;
    // if (checkTime(timeIrmsSumSpend OK, SMALL_INTERVAL) && hasIrmsSensor) {
    if (checkTimeClass.checkIrmsSumSpend(SMALL_INTERVAL) && hasIrmsSensor) {
        float res = electroSensorStorage.save(powerSpent);
        meteoLog.add("electroSensorStorage init write: " + String(powerSpent) + " res: " + String(res));
        long ts = millis();

        meteoLog.add("IRMSsum start. Since last: " + String(ts - irmsSumSpentSmallTimestamp));
        // rxtx.send(cnt433, config.deviceName);
        // meteoLog.add("433 SENT !!!");

        long st = millis();
        float power = emonSensor.power();
        long timeWork = millis() - st;

        if (ts - irmsSumSpentSmallTimestamp < 6000) {
            // long deltaKw = power - irmsSumSpentSmallValue;
            float avgKw = (power + irmsSumSpentSmallValue) / 2;
            meteoLog.add("IRMSsum avgWatt: " + String(avgKw));
            float kwSpent = avgKw / 60 / 60 * ((ts - irmsSumSpentSmallTimestamp) / 1000.0);
            irmsSumSpentIntervalSumValue += kwSpent;

            powerSpent += kwSpent;

            // meteoLog.add("IRMSsum deltaKw: " + String(deltaKw));
            meteoLog.add("IRMSsum wattSpent: " + String(kwSpent));
        }

        // if (ts - irmsSumSpentIntervalSumTimestamp > 30000 && ts - irmsSumSpentIntervalSumTimestamp < 300000) {
        //     meteoLog.add("IRMSsum sum: " + String(irmsSumSpentIntervalSumValue));
        //     meteoLog.add("IRMSsum sum time: " + String(ts - irmsSumSpentIntervalSumTimestamp));

        //     for (int i = 0; i < config.sensorsCount; i++) {
        //         SensorConfig& sensorConfig = config.sensors[i];
        //         Sensor& sensorData = sensorsData.sensors[i];

        //         if (sensorConfig.type != POWER_SUM) {
        //             continue;
        //         }

        //         float value = irmsSumSpentIntervalSumValue;
        //         meteoLog.add(String(sensorConfig.type) + " " + String(sensorConfig.debug_name) + " read: " + value);
        //         sensorData.set(value, now());
        //     }

        //     irmsSumSpentIntervalSumValue = 0;
        //     irmsSumSpentIntervalSumTimestamp = ts;
        // } else if (ts - irmsSumSpentIntervalSumTimestamp > 300000) {
        //     irmsSumSpentIntervalSumValue = 0;
        //     irmsSumSpentIntervalSumTimestamp = ts;
        // }

        irmsSumSpentSmallTimestamp = ts;
        irmsSumSpentSmallValue = power;

        meteoLog.add("IRMSsum, time spent: " + String(timeWork));
    }

    if (hasReceiver433) {
        uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
        uint8_t buflen = sizeof(buf);

        if (rxtx.receive(buf, &buflen)) {
            meteoLog.add("433 receive length: " + String(buflen));
            String s;
            for (int i = 0; i < buflen; i++) {
                s += (char)buf[i];
            }
            meteoLog.add("433 receiev: " + s);
        } else {
            // meteoLog.add("433 receive nothing");
        }
    }

    // if (checkTime(time433Send OK, 1000) && hasTransmitter433) {
    if (checkTimeClass.check433Send(1000) && hasTransmitter433) {
        cnt433++;
        rxtx.send(cnt433, config.deviceName);
        meteoLog.add("433 SENT !!!");
    }

    timeTestDiffCheck("another sensors");

    // if (checkTime(timeDataSend OK, SERVER_SENDING_TIMEOUT)) {
    if (checkTimeClass.checkSendSensorToServer(10000)) {
        // if (hasReceiver433) {
        //     uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
        //     uint8_t buflen = sizeof(buf);

        //     if (rxtx.receive(buf, &buflen)) {
        //         meteoLog.add("433 receive length: " + String(buflen));
        //         String s;
        //         for (int i = 0; i < buflen; i++) {
        //             s += (char)buf[i];
        //         }
        //         meteoLog.add("433 receiev: " + s);
        //     } else {
        //         // meteoLog.add("433 receive nothing");
        //     }
        // }

        meteoLog.add("check time");
        setTime((time_t)timing2.getTime());
        meteoLog.add(String(timing2.getTime()));

        long rs = millis();
        int rssi = WiFi.RSSI();
        rs = millis() - rs;
        meteoLog.add("rssi " + String(rssi) + " " + String(rs));

        timeTestDiffCheck("before beginning send");
        if (cache.empty()) {
            int statusCode = -1;
            if (wifiOn()) {
                statusCode = sendDataApi(true);
                timeTestDiffCheck("after sendDataApi");
            }
            if (statusCode != 200) {
                cache.add(sensorsData);
            }
            // clearIrmsSensorSum();
        } else {
            // cache.clear();
            cacheCount++;
            if (cacheCount % 7 == 0 || true) {
                cache.add(sensorsData);
                // clearIrmsSensorSum();
            }
            if (wifiOn()) {
                cache.sendCache(sensorsData.sensorsNames);
            }
        }
        timeTestDiffCheck("after send");

        maxLoopTime = 0;
        maxUpdateSensorTime = 0;

        meteoLog.add("Trying to update firmware...");
        processInternetUpdate(config.deviceName, String(FIRMWARE_VERSION), false);
        timeTestDiffCheck("after processInternetUpdate");

        meteoLog.sendLog(config.deviceName, SEND_LOG_URL);
        timeTestDiffCheck("after sendLog");
    }

    int buttonState = digitalRead(FLASH_BUTTON);
    if (buttonState == LOW) {
        // if (checkTime(timeButtonPress OK, 800)) {
        if (checkTimeClass.checkButtonPress(800)) {
            buttonCount++;
            meteoLog.add(String(buttonState));
        }
    } else {
        buttonCount = 0;
    }

    if (buttonCount > 5) {
        meteoLog.add(String(buttonState));

        buttonCount = 0;

        led.on();
        wifiConfig.startPortal();
        led.off();
    }

    timeTestDiffCheck("end loop");
}
