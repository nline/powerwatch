#include "mbed.h"
#include <stdio.h>

#include "board.h"

Serial pc(PB_6, PB_7);

int main(void) {
    while(1) {
        pc.printf("Testing\n");
        wait(5);
    }
}
