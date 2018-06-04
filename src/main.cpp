#include "FS.h"
#include "SSD1306.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

OTAUpdate otaUpdate;

#include "images.h"
SSD1306 display(0x3C, D3, D5);
#define DEMO_DURATION 3000
typedef void (*Demo)(void);
int demoMode = 0;
int counter = 1;

#define LED D8
#define BUTTON D7
int cnt = 1;

void displayIp(SSD1306 display, int cnt) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2,
                       "IP:\n" + WiFi.localIP().toString() + "\n# " + String(cnt));
    display.display();
}

#include "SensorDallasTemp.h"
#include "BME280.h"

SensorDallasTemp *temp;
BME280 *bme;

// DHT11
#include "DHT.h"
#define DHTPIN D2     // what digital pin we're connected to
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    Serial.println("Booting and setup");

    // setup OLED
    display.init();
    display.flipScreenVertically();
    display.setContrast(255);
    display.setFont(ArialMT_Plain_10);

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    // displayIp(display, cnt);

    // configure and start OTA update server
    otaUpdate.setup();

    // DS18B20 temperature
    temp = new SensorDallasTemp();

    // BME280
    bme = new BME280();

    // DHT11
    dht.begin();
    Serial.println("DHT11 begin");

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    pinMode(D0, OUTPUT);
    pinMode(D4, OUTPUT);

    pinMode(BUTTON, INPUT);

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);
}

// OLED DEMO BEGIN
void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
}

void drawTextFlowDemo() {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawStringMaxWidth(
        0, 0, 128,
        "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}

void drawTextAlignmentDemo() {
    // Text alignment demo
    display.setFont(ArialMT_Plain_10);

    // The coordinates define the left starting point of the text
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "Left aligned (0,10)");

    // The coordinates define the center of the text
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 22, "Center aligned (64,22)");

    // The coordinates define the right end of the text
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 33, "Right aligned (128,33)");
}

void drawRectDemo() {
    // Draw a pixel at given position
    for (int i = 0; i < 10; i++) {
        display.setPixel(i, i);
        display.setPixel(10 - i, i);
    }
    display.drawRect(12, 12, 20, 20);

    // Fill the rectangle
    display.fillRect(14, 14, 17, 17);

    // Draw a line horizontally
    display.drawHorizontalLine(0, 40, 20);

    // Draw a line horizontally
    display.drawVerticalLine(40, 0, 20);
}

void drawCircleDemo() {
    for (int i = 1; i < 8; i++) {
        display.setColor(WHITE);
        display.drawCircle(32, 32, i * 3);
        if (i % 2 == 0) {
            display.setColor(BLACK);
        }
        display.fillCircle(96, 32, 32 - i * 3);
    }
}

void setBrightnessLed(int);

void drawProgressBarDemo() {
    int progress = (counter) % 100;
    // draw the progress bar
    display.drawProgressBar(0, 32, 120, 10, progress);

    setBrightnessLed(progress * 10 / 3);

    // draw the percentage as String
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(progress) + "%");
}

void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

Demo demos[] = {drawFontFaceDemo, drawTextFlowDemo,    drawTextAlignmentDemo, drawRectDemo,
                drawCircleDemo,   drawProgressBarDemo, drawImageDemo};
int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;
// OLED DEMO END

bool ledOn = false;

void switchLed() {
    if (ledOn) {
        analogWrite(LED, 0);
    } else {
        analogWrite(LED, counter % 100 * 10 / 3);
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

void loop() {
    if (cc++ % 2 == 0) {
        digitalWrite(D0, HIGH);
        digitalWrite(D4, HIGH);
    } else {
        // digitalWrite(D0, LOW);
        // digitalWrite(D4, LOW);
    }

    otaUpdate.handle();

    display.clear();
    // demos[demoMode]();
    demos[5]();
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 0, String(millis() / 1000 / 5));

    if (millis() - tempTime > 2000) {
        tempTime = millis();
        tempC = temp->printTemperature();
    }

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, String(tempC));

    display.display();

    if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
        // demoMode = (demoMode + 1) % demoLength;
        timeSinceLastModeSwitch = millis();
    }
    counter++;

    // BME 280 begin
    Serial.print("Temperature = ");
    Serial.print(bme->temperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme->pressure() / 100.0F);
    Serial.println(" mmHg");

    Serial.print("BME Humidity = ");
    Serial.print(bme->humidity());
    Serial.println(" %");

    // BME 280 end

    // DHT11 BEGIN
    float humidity = dht.readHumidity();
    Serial.print("DHT11 hum: ");
    Serial.println(humidity);
    // DHT11 END

    delay(1000);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        Serial.println("Button on");
        switchLed();

        if (millis() - timeButtonPress > 500) {
            timeButtonPress = millis();
            // demoMode = (demoMode + 1) % demoLength;
        }
    } else {
        // Serial.println("Button off");
    }

    // IF button LONG PRESS and UPTIME smaller 1 minute
    //     start WiFi Manager

    // IF OLED works longer 1 minute
    //     OLED state -> OFF
    // IF button PUSHED
    //     OLED state -> NEXT
}
