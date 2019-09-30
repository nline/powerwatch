#pragma once

#include "lib/max7315.h"

class led {
public:

    led() {
        io.pinMode(0, OUTPUT);
        io.pinMode(1, OUTPUT);
        io.pinMode(2, OUTPUT);
        io.digitalWrite(0, HIGH);
        io.digitalWrite(1, HIGH);
        io.digitalWrite(2, HIGH);
    };

    void setRed(uint8_t state);
    void setGreen(uint8_t state);
    void setBlue(uint8_t state);
    void setBrightness(uint8_t brightness);
private:
    max7315 io;
};
