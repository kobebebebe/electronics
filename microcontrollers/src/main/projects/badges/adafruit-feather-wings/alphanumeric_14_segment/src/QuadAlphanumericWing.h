
#ifndef onebeartoe_QuadAlphanumericWing_h
#define onebeartoe_QuadAlphanumericWing_h

#include "/home/roberto/Versioning/owner/github/electronics/microcontrollers/src/main/projects/internet-clock/src/ArduinoLoopTask.h"

// Demo the quad alphanumeric display LED backpack kit
// scrolls through every character, then scrolls Serial
// input onto the display

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include "TextWingModes.h"

class QuadAlphanumericWing : public ArduinoLoopTask
{
    public:
        
        char displaybuffer[4] = {' ', ' ', ' ', ' '};

        String scrollingText = " hellow world -  ";
        
        int scrollingTextLength;
        
        TextWingModes wingMode;

        QuadAlphanumericWing();

        virtual void demo();

        virtual void oneLoop();

        virtual void setScrollingText(String text);
        
        virtual void setText(String text);
        
        virtual void setWingMode(TextWingModes mode);
    
    private:
    
        void scrollText();
        
        void serialInput();
};

#endif
