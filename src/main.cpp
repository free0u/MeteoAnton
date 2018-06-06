#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"
#include <NtpClientLib.h>

OTAUpdate otaUpdate;

#define BUTTON D7
int cnt = 0;

#include "SensorDallasTemp.h"
#include "BME280.h"
#include "DHTSensor.h"
#include "OLED.h"
#include "MeteoLog.h"
#include "SensorsData.h"
#include "Led.h"

SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
long bootTime;

void setup() {
    Serial.begin(115200);
    bootTime = now();

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
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    oled->showMessage("Trying to connect WiFi");
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

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

    // NTP
    oled->showMessage("NTP init...");
    NTP.begin("pool.ntp.org", 5);
    NTP.setInterval(63);
    oled->showMessage("NTP init... Done");

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);
}

int oledState = 2;
int oledStateNum = 3;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int EMPTY = 4;

long timeButtonPress = 0;

SensorsData sensorsData;

long sensorsDataUpdated = 0;
long dhtSensorUpdated;
void tryUpdateSensors() {
    float dsTemp;
    float dhtHum;
    float bmePressure;
    float bmeHum;

    if (millis() - sensorsDataUpdated > 30000) {
        sensorsDataUpdated = millis();

        meteoLog->add("Reading sensors...");
        dsTemp = temp->printTemperature();
        bmePressure = bme->pressure();
        bmeHum = bme->humidity();

        meteoLog->add("dsTemp " + String(dsTemp));
        meteoLog->add("bmePressure " + String(bmePressure));
        meteoLog->add("bmeHum " + String(bmeHum));
        meteoLog->add("Reading sensors... Done");

        if (!isnan(dsTemp)) {
            sensorsData.dsTemp = dsTemp;
        }

        if (!isnan(bmeHum)) {
            sensorsData.bmeHum = bmeHum;
        }

        if (!isnan(bmePressure)) {
            sensorsData.bmePressure = bmePressure;
        }
    }

    if (millis() - dhtSensorUpdated > 10000) {
        dhtSensorUpdated = millis();
        dhtHum = dht->humidity();
        meteoLog->add("dhtHum " + String(dhtHum));

        if (!isnan(dhtHum)) {
            sensorsData.dhtHum = dhtHum;
        }
    }
}

void loop() {
    otaUpdate.handle();

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(cnt++, NTP.getTimeStr());
    } else if (oledState == LOG) {
        oled->log();
    }

    tryUpdateSensors();

    delay(100);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        if (millis() - timeButtonPress > 500) {
            Serial.println("Button on");

            timeButtonPress = millis();
            oledState = (oledState + 1) % oledStateNum;
            Serial.println("OLED state: " + String(oledState));
        }
    }

    // IF button LONG PRESS and UPTIME smaller 1 minute
    //     start WiFi Manager

    // IF OLED works longer 1 minute
    //     OLED state -> OFF
    // IF button PUSHED
    //     OLED state -> NEXT
}
