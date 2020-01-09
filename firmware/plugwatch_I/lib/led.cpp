#include <Particle.h>

#include "lib/max7315.h"
#include "lib/led.h"

void led::setRed(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(0, LOW);
    } else if (state == LOW) {
        io.digitalWrite(0, HIGH);
    }
}

void led::setGreen(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(2, LOW);
    } else if (state == LOW) {
        io.digitalWrite(2, HIGH);
    }
}

void led::setBlue(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(1, LOW);
    } else if (state == LOW) {
        io.digitalWrite(1, HIGH);
    }
}

//only let the user control the bottom four bits of brightness
void led::setBrightness(uint8_t brightness) {
    io.setGlobalIntensity(brightness | 0xF0);
}

