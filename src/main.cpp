#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

#include <ESP8266HTTPClient.h>
#include "EspSaveCrash.h"

OTAUpdate otaUpdate;

#define BUTTON D4
int cnt = 0;

#define PWM D8

#include "BME280.h"
#include "DHTSensor.h"
#include "Led.h"
#include "MeteoLog.h"
#include "OLED.h"
#include "SensorDallasTemp.h"
#include "SensorsData.h"
#include "Timing.h"

SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
Timing *timing;
long bootTime;

void scan() {
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

    // delay(5000); // wait 5 seconds for next scan
}

int oledState = 2;
int oledStateNum = 3;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int EMPTY = 4;

#include <MHZ19_uart.h>
const int rx_pin = D1; // Serial rx pin no
const int tx_pin = D2; // Serial tx pin no
MHZ19_uart mhz19;

int prevVal = LOW;
long th, tl, h, l, ppm = 0;
long ppmUpdateTime = -1;
void PWM_ISR() {
    long tt = millis();
    int val = digitalRead(PWM);

    if (val == HIGH) {
        if (val != prevVal) {
            h = tt;
            tl = h - l;
            prevVal = val;
        }
    } else {
        if (val != prevVal) {
            l = tt;
            th = l - h;
            prevVal = val;
            ppm = 5000 * (th - 2) / (th + tl - 4);
            ppmUpdateTime = tt;
        }
    }
}

void setup() {
    Serial.begin(115200);

    // SaveCrash.clear();
    SaveCrash.print();

    pinMode(PWM, INPUT);
    attachInterrupt(digitalPinToInterrupt(PWM), PWM_ISR, CHANGE);
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
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // CO2
    oled->showMessage("CO2 init...");
    mhz19.begin(rx_pin, tx_pin);
    mhz19.setAutoCalibration(true);
    // mhz19.setAutoCalibration(false);
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

        if (millis() - ppmUpdateTime < 10000) {
            sensorsData.co2 = ppm;
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

void loop() {
    otaUpdate.handle();
    tryUpdateSensors();

    if (millis() - rtcTimeUpdate > RTC_TIME_UPDATE_TIMEOUT) {
        rtcTimeUpdate = millis();
        timing->updateRtc();
        meteoLog->add("RTC updated: " + timing->getRtcDateTime());
    }

    if (millis() - timeScan > 3000) {
        timeScan = millis();
        co2ppm = mhz19.getPPM();
        meteoLog->add("co2 ppm " + String(co2ppm) + "(" + String(ppm) + ")");
    }

    if (millis() - timeDataSend > 16000) {
        timeDataSend = millis();
        int up = millis() / 1000;
        String url = "***REMOVED***&field1=" + String(co2ppm) +
                     "&field2=" + String(up / 60) + "&field3=" + String(ppm) +
                     "&field4=" + String(sensorsData.dsTempOne);

        HTTPClient http;
        http.begin(url);
        int statusCode = http.GET();
        meteoLog->add("http code " + String(statusCode));
    }

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        RtcDateTime compiled;
        compiled.InitWithEpoch32Time(millis() / 1000 - 6 * 60 * 60);

        oled->displayIp(cnt++, NTP.getUptimeString(), timing->getRtcDateTime(), NTP.getTimeStr());
        // oled->displayIp(cnt++, rtcToString(compiled), rtcToString(Rtc.GetDateTime()), NTP.getTimeStr());
    } else if (oledState == LOG) {
        oled->displayLog();
    }

    delay(100);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        if (millis() - timeButtonPress > 500) {
            // Serial.println("Button on");

            timeButtonPress = millis();
            oledState = (oledState + 1) % oledStateNum;
            // Serial.println("OLED state: " + String(oledState));
        }
    }

    // IF button LONG PRESS and UPTIME smaller 1 minute
    //     start WiFi Manager

    // IF OLED works longer 1 minute
    //     OLED state -> OFF
    // IF button PUSHED
    //     OLED state -> NEXT
}
