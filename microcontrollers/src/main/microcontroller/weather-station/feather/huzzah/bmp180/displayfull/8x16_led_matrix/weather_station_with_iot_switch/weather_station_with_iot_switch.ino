
/**
// POSSIBLE WEB SERVER CODE
// compiles:
        has good API
            https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/PathArgServer/PathArgServer.ino

        core ESP8266
            https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/server-examples.html

            https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/HelloServer/HelloServer.ino            


// havent't tried:
        adafruit temp and humitiy server

            https://learn.adafruit.com/esp8266-temperature-slash-humidity-webserver?view=all

*/


// LED matrix display: 
//
//      https://www.adafruit.com/product/3150
//
//      https://learn.adafruit.com/adafruit-8x16-led-matrix-featherwing/overview
//
// temperature and humidity sensor
//
//      https://www.adafruit.com/product/1603

#include "configuration.h"

#include "AdafruitIO_WiFi.h"

#include "Adafruit_BMP085.h"

#include "Adafruit_LEDBackpack.h"

#include <Adafruit_GFX.h>
#include <Wire.h>

AdafruitIO_WiFi io(ADAFRUIT_USERNAME, AIO_KEY, wifi_ssid, wifi_password);
 
Adafruit_BMP085 bmp;

float degreesCelsius;

float pressureReading;

// set up the Adafruit IO 'temperature' and 'humidity' feeds
AdafruitIO_Feed *temperature = io.feed("rainmaker-backyard-top-temperature");
AdafruitIO_Feed *humidity = io.feed("rainmaker-backyard-top-pressure");

// AIO post interval settings
unsigned long aioPostPreviousMillis = 0;
const long aioPostInterval = 1000 * 60 * 6;  // once every six minutes

// serial output interval settings
unsigned long serialPreviousMillis = 0;
const long serialInterval = 5000; // five seconds

// LED display interval settings
unsigned long ledDisplayPreviousMillis = 0;
const long ledDisplayInterval = 100;  // one tenth of a second

Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

// these next three hold values for the text scrolling
int8_t x=7;
int8_t xStart = 7;
int8_t xMin = -46;

String scrollText = "--temp";

char scrollTextBuffer[20] = "";

/**
 * Post the temperature and pressure values to 
 * the Adafruit IO service.
 */
void aioPost(unsigned long currentMillis)
{
    if(currentMillis - aioPostPreviousMillis >= aioPostInterval) 
    {
        aioPostPreviousMillis = currentMillis;

        // save fahrenheit (or celsius) to Adafruit IO
        temperature->save(degreesCelsius);

        Serial.println("posting to Adafruit IO");
        Serial.println();

        // save humidity to Adafruit IO
        humidity->save(pressureReading);
    }    
}

void ledDisplay(unsigned long currentMillis)
{
    if(currentMillis - ledDisplayPreviousMillis >= ledDisplayInterval) 
    {
        ledDisplayPreviousMillis = currentMillis;

        matrix.clear();
        matrix.setCursor(x,0);
        matrix.print(scrollText);
//        matrix.print("World");
        matrix.writeDisplay();
        
        x--;
        
        if(x == xMin)
        {
            x = xStart;
        }

        sensorReading();
    }
}

void sensorReading()
{
    // update the scrolling text
    int width = 8;
    int percision = 1;

    double degrees = degreesCelsius;

    bool FAHRENHEIT = true;

    if(FAHRENHEIT)
    {
        double fahrenheit = (9.0/5.0)*degreesCelsius + 32;

        degrees = fahrenheit;
    }

    scrollText = dtostrf(degrees, width, percision, scrollTextBuffer);

    if(FAHRENHEIT)
    {
        scrollText += " F";
    }
    else
    {
        scrollText += " C";
    }    
}

void loop() 
{
    io.run();
  
    unsigned long currentMillis = millis();

    serialOutput(currentMillis);
    
    aioPost(currentMillis);
    
    ledDisplay(currentMillis);
}


void serialOutput(unsigned long currentMillis)
{
    if(currentMillis - serialPreviousMillis >= serialInterval) 
    {
        serialPreviousMillis = currentMillis;
        
        degreesCelsius = bmp.readTemperature();
        Serial.print("Temperature = ");
        Serial.print(degreesCelsius);
        Serial.println(" *C");

        pressureReading = bmp.readPressure();
        Serial.print("Pressure = ");
        Serial.print(pressureReading);
        Serial.println(" Pa");
        Serial.println();    
    }
}

void setup() 
{
    // start the serial connection
    Serial.begin(9600);

    // wait for serial monitor to open
    while(! Serial);
    
    // initialize the temperature and pressure sensor
    bmp.begin();

    setupAdafruitIo();
    
    setupLedMatrix();
}

void setupAdafruitIo()
{
    // connect to io.adafruit.com
    Serial.println("\n\n");
    Serial.println("\n\n");  
    Serial.print("Connecting to Adafruit IO");
    io.connect();

    // wait for a connection
    while(io.status() < AIO_CONNECTED) 
    {
      Serial.print(".");
      delay(500);
    }

    // we are connected
    Serial.println();
    Serial.println(io.statusText());    
}

void setupLedMatrix()
{
    matrix.begin(0x70);  // pass in the address    
    
    matrix.setTextSize(1);
    matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix.setTextColor(LED_ON);
    matrix.setRotation(1);    
}
