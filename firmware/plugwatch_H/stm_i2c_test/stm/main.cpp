#include "mbed.h"
#include <stdio.h>

#include "board.h"

DigitalOut LED(PB_1);
I2CSlave i2c(PA_10, PA_9);

int main(void) {
    //Set the i2c address
    i2c.address(0x1A);
    
    uint8_t point = 0x00;
    uint8_t val;
    while(1) {
        int i = i2c.receive();
        switch (i) {
        case I2CSlave::ReadAddressed:
            val = point + 16;
            i2c.write((char *)(&val), 1);
        break;
        case I2CSlave::WriteAddressed:
            i2c.read((char *)(&point), 1);
        break;
        }
    }
}
