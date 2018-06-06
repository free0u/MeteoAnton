#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"
#include <NtpClientLib.h>

OTAUpdate otaUpdate;

#define DEMO_DURATION 3000
typedef void (*Demo)(void);
int demoMode = 0;
int counter = 1;

#define LED D8
#define BUTTON D7
int cnt = 1;

#include "SensorDallasTemp.h"
#include "BME280.h"
#include "DHTSensor.h"
#include "OLED.h"
#include "MeteoLog.h"

SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;

#include "SensorsData.h"

void switchLed(bool);
long startTime;
void setup() {
    Serial.begin(115200);

    // setup pins
    pinMode(D0, OUTPUT);
    pinMode(D4, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(LED, OUTPUT);

    // setup log and OLED
    meteoLog = new MeteoLog();
    oled = new OLED(meteoLog);
    oled->showMessage("Booting...");

    // turn on LED until connected

    switchLed(true);

    startTime = now();

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    oled->showMessage("Trying to connect WiFi");
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    // displayIp(display, cnt);

    switchLed(false);
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

void setBrightnessLed(int);

bool ledOn = false;

void switchLed(bool turnOn) {
    if (!turnOn) {
        analogWrite(LED, 0);
    } else {
        analogWrite(LED, counter % 100 * 10 / 2);
    }
    ledOn = !ledOn;
}

void setBrightnessLed(int brightness) {
    // analogWrite(LED, brightness);
    analogWrite(LED, 10);
    //
}

void dimLed() {
    int newDim = cnt * 100 % 1024;
    analogWrite(LED, newDim);
    cnt++;
}

// ENUM
// LED state: OFF, SENSORS, NETWORK

long timeButtonPress = 0;

float tempC = 0;
long tempTime = 0;

int cc = 0;

SensorsData sensorsData;

bool font = true;

long sensorsDataUpdated = 0;

float dsTemp;
float dhtHum;
float bmeTemp;
float bmePressure;
float bmeHum;

int oledState = 2;
int oledStateNum = 3;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int EMPTY = 4;

void loop() {
    if (cc++ % 2 == 0) {
        digitalWrite(D0, HIGH);
        digitalWrite(D4, HIGH);
    } else {
        // digitalWrite(D0, LOW);
        // digitalWrite(D4, LOW);
    }

    otaUpdate.handle();

    // display.clear();
    // // demos[demoMode]();
    // demos[5]();
    // display.setTextAlignment(TEXT_ALIGN_RIGHT);
    // display.drawString(128, 0, String(millis() / 1000 / 5));

    if (millis() - sensorsDataUpdated > 2000) {
        sensorsDataUpdated = millis();

        dsTemp = temp->printTemperature();
        bmeTemp = bme->temperature();
        bmePressure = bme->pressure();
        bmeHum = bme->humidity();
        dhtHum = dht->humidity();

        meteoLog->add(String(bmeHum) + " %");
        sensorsData.dsTemp = dsTemp;
        sensorsData.bmeHum = bmeHum;
        sensorsData.bmePressure = bmePressure;
        sensorsData.dhtHum = dhtHum;
    }

    // display.setTextAlignment(TEXT_ALIGN_LEFT);
    // display.drawString(0, 0, String(tempC));
    // display.display();

    meteoLog->add(String(cnt++));

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(cnt++, NTP.getTimeStr());
    } else if (oledState == LOG) {
        oled->log();
    }

    // BME 280 begin
    Serial.print("Temperature = ");
    Serial.print(bmeTemp);
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print(bmePressure);
    Serial.println(" mmHg");
    Serial.print("BME Humidity = ");
    Serial.print(bmeHum);
    Serial.println(" %");
    // BME 280 end

    // DHT11 BEGIN
    Serial.print("DHT11 hum: ");
    Serial.println(dhtHum);
    // DHT11 END
    Serial.println("Start time: " + String(startTime) + "now(): " + String(now()));
    // Serial.println("");
    // Serial.println("");
    // Serial.print(" ");
    // Serial.print(NTP.getTimeDateString());
    // Serial.print(" ");
    // Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    // Serial.print("WiFi is ");
    // Serial.print(WiFi.isConnected() ? "connected" : "not connected");
    // Serial.print(". ");
    // Serial.print("Uptime: ");
    // Serial.print(NTP.getUptimeString());
    // Serial.print(" since ");
    // Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());
    // Serial.println("");
    // Serial.println("");

    delay(1000);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        Serial.println("Button on");

        if (millis() - timeButtonPress > 500) {
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
