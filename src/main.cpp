#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

#include <ESP8266HTTPClient.h>
#include "EspSaveCrash.h"

OTAUpdate otaUpdate;

#define BUTTON D4

#include "BME280.h"
#include "DHTSensor.h"
#include "Led.h"
#include "MeteoLog.h"
#include "OLED.h"
#include "SensorDallasTemp.h"
#include "SensorsData.h"
#include "Timing.h"
#include "CO2Sensor.h"

WiFiConfig *wifiConfig;
SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
Timing *timing;
CO2Sensor *co2;

long bootTime;

int oledState = 2;
int oledStateNum = 6;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int OLED_AUTO_OFF_SWITCH = 3;
const int WIFI_CHANGE = 4;
const int EMPTY = 5;

void setup() {
    Serial.begin(115200);

    // SaveCrash.clear();
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

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);

    oledState = SENSORS;
}

long timeButtonPress = 0;

SensorsData sensorsData;

#define SENSORS_TIMEOUT 10000
#define DHT_TIMEOUT 10000

long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;
void tryUpdateSensors() {
    float dsTempOne;
    float dsTempTwo;
    float dhtHum;
    float bmePressure;
    float bmeHum;

    if (millis() - sensorsDataUpdated > SENSORS_TIMEOUT) {
        sensorsDataUpdated = millis();

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

        if (!isnan(dsTempOne)) {
            sensorsData.dsTempOne = dsTempOne;
        }

        if (!isnan(dsTempTwo)) {
            sensorsData.dsTempTwo = dsTempTwo;
        }

        if (!isnan(bmeHum)) {
            sensorsData.bmeHum = bmeHum;
        }

        if (!isnan(bmePressure)) {
            sensorsData.bmePressure = bmePressure;
        }

        if (millis() - co2->getPpmUpateTime() < 10000) {
            sensorsData.co2 = co2->getPpmPwm();
            ;
        } else {
            sensorsData.co2 = NAN;
        }
    }

    if (millis() - dhtSensorUpdated > DHT_TIMEOUT) {
        dhtSensorUpdated = millis();
        dhtHum = dht->humidity();
        meteoLog->add("dhtHum " + String(dhtHum));

        if (!isnan(dhtHum)) {
            sensorsData.dhtHum = dhtHum;
        }
    }
}

long timeScan = -1e9;
long timeDelta = 1000000;

int cc = 0;

long timeDataSend = -10000;

int co2ppm;

#define RTC_TIME_UPDATE_TIMEOUT 3600000 // 1 hour
// #define RTC_TIME_UPDATE_TIMEOUT 5000
long rtcTimeUpdate = -RTC_TIME_UPDATE_TIMEOUT;

long oledStateChangedTime = 0;

void loop() {
    otaUpdate.handle();
    tryUpdateSensors();

    if (millis() - rtcTimeUpdate > RTC_TIME_UPDATE_TIMEOUT) {
        rtcTimeUpdate = millis();
        timing->updateRtc();
        meteoLog->add("RTC updated: " + timing->getRtcDateTime());
    }

    if (millis() - timeScan > 3000) {
        Serial.println("Always on: " + String(oled->alwaysOn));
        timeScan = millis();
        co2ppm = co2->getPpmUart();
        meteoLog->add("co2 ppm " + String(co2ppm) + "(" + String(co2->getPpmPwm()) + ")");
    }

    if (millis() - timeDataSend > 16000) {
        timeDataSend = millis();
        int up = millis() / 1000;
        String url = "***REMOVED***&field1=" + String(co2ppm) +
                     "&field2=" + String(up / 60) + "&field3=" + String(co2->getPpmPwm()) +
                     "&field4=" + String(sensorsData.dsTempOne) + "&field5=" + String(sensorsData.bmeHum);

        HTTPClient http;
        http.begin(url);
        int statusCode = http.GET();
        meteoLog->add("http code " + String(statusCode));
    }

    if (!oled->alwaysOn && millis() - oledStateChangedTime > 7000) {
        oledState = EMPTY;
    }

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(NTP.getUptimeString(), timing->getRtcDateTime(), NTP.getTimeStr());
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
