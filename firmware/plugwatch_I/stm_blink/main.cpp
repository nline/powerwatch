#include "mbed.h"
#include <stdio.h>

#include "board.h"

DigitalOut LED(PB_1);

int main(void) {

    while(1) {
        LED = 0;
        wait_ms(5000);
        LED = 1;
        wait_ms(5000);
    }
}
