// This program does a test of all the hardware so you can get an example of how to read
// sensors, touchscreen, and display stuff!

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_QSPI_Flash.h"
#include "Adafruit_ADT7410.h"
#include <Adafruit_ImageReader.h> // Image-reading functions

#include "TouchScreen.h"
#include <SD.h>
#include <WiFiNINA.h>

//#include "mismatch.wav.h"
//#include "Coin.wav.h"
#include "coin.h"
    
#define RED_LED       13
#define TFT_RESET     24
#define TFT_BACKLIGHT 25
#define LIGHT_SENSOR  A2
#define SD_CS         32       
#define SPKR_SHUTDOWN 50

#define TFT_D0        34 // Data bit 0 pin (MUST be on PORT byte boundary)
#define TFT_WR        26 // Write-strobe pin (CCL-inverted timer output)
#define TFT_DC        10 // Data/command pin
#define TFT_CS        11 // Chip-select pin
#define TFT_RST       24 // Reset pin
#define TFT_RD         9 // Read-strobe pin
#define TFT_BACKLIGHT 25
// ILI9341 with 8-bit parallel interface:
Adafruit_ILI9341 tft = Adafruit_ILI9341(tft8bitbus, TFT_D0, TFT_WR, TFT_DC, TFT_CS, TFT_RST, TFT_RD);

Adafruit_QSPI_Flash flash;

Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();

#define YP A4  // must be an analog pin, use "An" notation!
#define XM A7  // must be an analog pin, use "An" notation!
#define YM A6   // can be a digital pin
#define XP A5   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
#define X_MIN  750
#define X_MAX  325
#define Y_MIN  840
#define Y_MAX  240

Adafruit_GFX_Button button1 = Adafruit_GFX_Button();

Adafruit_GFX_Button coin = Adafruit_GFX_Button();

Adafruit_Image img;
Adafruit_ImageReader reader;

void setup() 
{
    Serial.begin(9600);
//    Serial.begin(115200);
//    while (!Serial);

    Serial.println("All Test!");

    pinMode(RED_LED, OUTPUT);
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);

    pinMode(TFT_RESET, OUTPUT);
    digitalWrite(TFT_RESET, HIGH);
    delay(10);
    digitalWrite(TFT_RESET, LOW);
    delay(10);
    digitalWrite(TFT_RESET, HIGH);
    delay(10);

    tft.begin();

    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextWrap(true);
    tft.setCursor(0, 0);

    tft.print("QSPI Flash...");
    if (!flash.begin()) {
        Serial.println("Could not find flash on QSPI bus!");
        tft.setTextColor(ILI9341_RED);
        tft.println("FAILED");
        while (1);
    }
    uint8_t manid, devid;
    Serial.println("Reading Manuf iD");
    flash.GetManufacturerInfo(&manid, &devid);
    Serial.print("Manuf: ");
    Serial.println(manid, HEX);
    Serial.print("Device: ");
    Serial.println(devid, HEX);
    if (devid != 0x17) {
        tft.setTextColor(ILI9341_RED);
        tft.println("FAILED");
        Serial.println("QSPI Flash not found!");
        while (1);
    }
    tft.print("OK\n\tManuf: 0x");
    tft.println(manid, HEX);
    tft.print("\tDev: 0x");
    tft.println(devid, HEX);

    /*************** SD CARD */
    tft.setCursor(0, 48);
    tft.print("SD Card...");
    if (!SD.begin(SD_CS)) {
        Serial.println("Card init. failed!");
        tft.setTextColor(ILI9341_RED);
        tft.println("FAILED");
        tft.setTextColor(ILI9341_GREEN);
    } else {
        tft.println("OK!");
    }

    /*************** WiFi Module */

    tft.setCursor(0, 64);
    tft.print("WiFi Module...");
    WiFi.status();
    delay(100);
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("ESP32 SPI not found");
        tft.setTextColor(ILI9341_RED);
        tft.println("FAILED");
        tft.setTextColor(ILI9341_GREEN);
    } else {
        Serial.println("ESP32 SPI mode found");
        tft.println("OK!");
    }

    /*************** Temperature sensor */
    tft.setCursor(0, 80);
    tft.print("ADT7410...");
    if (!tempsensor.begin()) {
        Serial.println("Couldn't find ADT7410!");
        tft.setTextColor(ILI9341_RED);
        tft.println("FAILED");
        tft.setTextColor(ILI9341_GREEN);
    } else {
        Serial.println("ADT7410 found");
        tft.println("OK!");
    }

    button1.initButton(&tft, 100, 200, 100, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_BLACK, "Ploop", 2);
    button1.drawButton();

    coin.initButton(&tft, 120, 280, 100, 50, ILI9341_WHITE, ILI9341_YELLOW, ILI9341_BLACK, "Sound", 2);
    coin.drawButton();

    analogWriteResolution(12);
    analogWrite(A0, 128);
    pinMode(SPKR_SHUTDOWN, OUTPUT);
    digitalWrite(SPKR_SHUTDOWN, LOW);
    
    
    // display an image
    ImageReturnCode stat;    
    stat = reader.loadBMP("1.bmp", img);
//    stat = reader.loadBMP("/1.bmp", img);
    Serial.print("image load status: ");
    reader.printStatus(stat);
}

bool debug = false;

void loop() 
{
    digitalWrite(RED_LED, HIGH);
    tft.setTextColor(ILI9341_WHITE);
    // read light sensor
    tft.fillRect(160, 100, 240, 16, ILI9341_BLACK);
    tft.setCursor(0, 100);
    uint16_t light = analogRead(LIGHT_SENSOR);

    if(debug)
    {    
        Serial.print("light sensor: ");
        Serial.println(light);
    }
    
    tft.print("Light sensor: ");
    tft.println(light);

    // read temp sensor
    tft.fillRect(150, 116, 240, 16, ILI9341_BLACK);
    tft.setCursor(0, 116);
    float temp = tempsensor.readTempC();
    
    if(debug)
    {
        Serial.print("temp sensor: ");
        Serial.println(temp, 2);
    }
    
    tft.print("Temp sensor: ");
    tft.println(temp, 2);

    // externals
    tft.fillRect(0, 132, 240, 32, ILI9341_BLACK);
    tft.setCursor(0, 132);
    float d3 = (float) analogRead(A1) * 3.3 / 1024;
    float d4 = (float) analogRead(A3) * 3.3 / 1024;

    if(debug)
    {
        Serial.print("STEMMA: ");
        Serial.print(d3, 1);
        Serial.print(", ");
        Serial.print(d4, 1);
        Serial.println();
    }
    
    tft.print("D3: ");
    tft.println(d3, 1);
    tft.print("D4: ");
    tft.println(d4, 1);

    tft.fillRect(80, 164, 240, 16, ILI9341_BLACK);
    tft.setCursor(0, 164);
    tft.print("Touch: ");

    TSPoint p = ts.getPoint();
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!
    if (p.z > ts.pressureThreshhold) 
    {
        Serial.print("X = ");
        Serial.print(p.x);
        Serial.print("\tY = ");
        Serial.print(p.y);
        Serial.print("\tPressure = ");
        Serial.println(p.z);

        int16_t x = map(p.x, X_MIN, X_MAX, 0, 240);
        int16_t y = map(p.y, Y_MIN, Y_MAX, 0, 320);
        tft.print("(");
        tft.print(x);
        tft.print(", ");
        tft.print(y);
        tft.println(")");

        if (coin.contains(x, y)) 
        {
            Serial.println("Ding!");
            coin.press(true);
        } 
        else 
        {
            coin.press(false);
        }
        
        if (button1.contains(x, y)) 
        {
            Serial.println("Ding!");
            button1.press(true);
        } 
        else 
        {
            button1.press(false);
        }
    } 
    else 
    {
        coin.press(false);
        button1.press(false);
    }
    
    if (coin.justPressed() || button1.justPressed() ) 
    {
        coin.drawButton(true);

        playCoin();
    }

    if (coin.justReleased()) {
        coin.drawButton(false);
    }
    
    img.draw(tft, 75, 180);

    digitalWrite(RED_LED, LOW);
    delay(20);
}

void playCoin() 
{
    digitalWrite(SPKR_SHUTDOWN, HIGH);

    uint32_t i, prior, usec = 1000000L / SAMPLE_RATE;
    prior = micros();
    for (uint32_t i = 0; i<sizeof (coinaudio); i++) {
        uint32_t t;

        while ((t = micros()) - prior < usec)
            ;

        analogWrite(A0, (uint16_t) coinaudio[i]);
        prior = t;
    }

    digitalWrite(SPKR_SHUTDOWN, LOW);
}

